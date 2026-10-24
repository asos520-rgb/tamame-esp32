#!/usr/bin/env python3
"""Offline avatar packer. No resizing, identity inference, uploads or metadata copy."""
import argparse
from collections import Counter
import json
from pathlib import Path
import re
import shutil
import tempfile

from PIL import Image

STATES = ('boot', 'idle', 'listening', 'thinking', 'speaking', 'happy', 'error', 'off')
ANCHORS = {
    'left_eye': [19, 31, 28, 38], 'right_eye': [34, 31, 43, 38],
    'mouth': [28, 38, 35, 43], 'clothing': [20, 46, 43, 54],
}
COLORS = {'skin': '#B78260', 'cheek': '#9B5F4D', 'mouth': '#59382E'}

class PackError(ValueError):
    pass

def read_profile(path):
    try:
        p = json.loads(path.read_text(encoding='utf-8'))
    except (ValueError, OSError):
        raise PackError('profile must be readable JSON') from None
    if not isinstance(p, dict) or set(p) != {'version', 'template', 'anchors', 'colors'}:
        raise PackError('profile requires only version, template, anchors and colors')
    if type(p['version']) is not int or p['version'] != 1 or p['template'] != 'humanoid64-v1':
        raise PackError('unsupported profile version or template')
    # v1 intentionally rejects arbitrary anatomy: accessories still use fixed geometry.
    if (p['anchors'] != ANCHORS or not isinstance(p['anchors'], dict)
            or any(not isinstance(rect, list) or any(type(v) is not int for v in rect)
                   for rect in p['anchors'].values())):
        raise PackError('v1 requires the documented fixed humanoid64 anchors')
    if not isinstance(p['colors'], dict) or set(p['colors']) != set(COLORS):
        raise PackError('colors must contain skin, cheek and mouth')
    if any(not isinstance(v, str) or not re.fullmatch(r'#[0-9A-Fa-f]{6}', v) for v in p['colors'].values()):
        raise PackError('colors must be #RRGGBB')
    return p

def load_frames(source):
    frames = []
    for state in STATES:
        path = source / (state + '.png')
        if path.is_symlink() or not path.is_file():
            raise PackError(state + ': missing PNG or symlink input')
        try:
            with Image.open(path) as image:
                if image.format != 'PNG' or image.size != (64, 64) or getattr(image, 'n_frames', 1) != 1:
                    raise PackError(state + ': expected a single 64x64 PNG')
                if image.mode not in {'RGBA', 'LA', 'P'} or (image.mode == 'P' and 'transparency' not in image.info):
                    raise PackError(state + ': explicit alpha is required')
                frame = image.convert('RGBA')
                alpha = frame.getchannel('A')
                if not set(alpha.get_flattened_data()).issubset({0, 255}):
                    raise PackError(state + ': partial alpha is unsupported; use 0 or 255')
                bbox = alpha.getbbox()
                if bbox is None:
                    raise PackError(state + ': empty silhouette')
                if bbox[0] < 3 or bbox[1] < 3 or bbox[2] > 61 or bbox[3] > 61:
                    raise PackError(state + ': leave at least three transparent pixels at every edge')
                # Drop all source info; new objects below cannot inherit EXIF/text chunks.
                frames.append(frame.copy())
        except PackError:
            raise
        except (OSError, ValueError, Image.DecompressionBombError):
            raise PackError(state + ': unreadable PNG') from None
    return frames

def pack_pixels(frames):
    opaque = [rgb[:3] for frame in frames for rgb in frame.get_flattened_data() if rgb[3] == 255]
    counts = Counter(opaque)
    # Reserve exact black, even if quantizing a colorful pack. Transparency is independent.
    ordered = sorted((c for c in counts if c != (0, 0, 0)), key=lambda c: (-counts[c], c))
    if len(ordered) <= 255:
        palette = [(0, 0, 0)] + ordered
    else:
        samples = Image.new('RGB', (len(opaque), 1)); samples.putdata(opaque)
        quantized = samples.quantize(colors=255, method=Image.Quantize.MEDIANCUT)
        raw = quantized.getpalette()
        palette = [(0, 0, 0)] + [tuple(raw[i:i+3]) for i in range(0, 255 * 3, 3)]
    palette += [(0, 0, 0)] * (256-len(palette))
    lookup = {}
    for rgb in counts:
        lookup[rgb] = 0 if rgb == (0, 0, 0) else min(range(256), key=lambda i: sum((rgb[k]-palette[i][k])**2 for k in range(3)))
    indexed, masks = [], []
    for frame in frames:
        indices, mask = [], bytearray(512)
        for at, pixel in enumerate(frame.get_flattened_data()):
            is_opaque = pixel[3] == 255
            indices.append(lookup[pixel[:3]] if is_opaque else 0)
            if is_opaque:
                mask[at//8] |= 1 << (at % 8)
        indexed.append(indices); masks.append(list(mask))
    return palette, indexed, masks

def array(name, c_type, rows, width):
    lines = ['static const ' + c_type + ' ' + name + '[' + str(len(rows)) + '][' + str(len(rows[0])) + '] = {']
    for row in rows:
        lines.append('{')
        for start in range(0, len(row), width):
            lines.append(','.join(str(x) for x in row[start:start+width]) + ',')
        lines.append('},')
    return '\n'.join(lines + ['};', ''])

def write_pack(dest, frames, profile):
    palette, indexed, masks = pack_pixels(frames)
    rgb565 = [((r>>3)<<11)|((g>>2)<<5)|(b>>3) for r,g,b in palette]
    preamble = '/* Generated locally. Contains avatar appearance; keep private unless shareable. */\n#pragma once\n#include <stdint.h>\n'
    header = preamble + 'static const uint16_t portrait_palette[256] = {\n' + ','.join(map(str, rgb565)) + '\n};\n'
    (dest/'portrait_assets.h').write_text(header + array('portrait_images','uint8_t',indexed,64))
    (dest/'style_silhouette.h').write_text(preamble + array('style_silhouette','uint8_t',masks,32))
    lines = [preamble, '#define PET_AVATAR_TEMPLATE_VERSION 1']
    for part, coords in profile['anchors'].items():
        for suffix,value in zip(('X0','Y0','X1','Y1'),coords):
            lines.append('#define PET_AVATAR_'+part.upper()+'_'+suffix+' '+str(value))
    for name,color in profile['colors'].items():
        lines.append('#define PET_AVATAR_'+name.upper()+' 0x'+color[1:].upper()+'u')
    (dest/'avatar_profile.h').write_text('\n'.join(lines)+'\n')
    # Reconstruct from the actual quantized RGB565 payload, without original metadata.
    sheet = Image.new('RGB', (256, 128), '#303038')
    for n, (indices, mask) in enumerate(zip(indexed, masks)):
        preview = Image.new('RGB',(64,64),'#303038')
        pixels = []
        for at, index in enumerate(indices):
            if (mask[at//8]>>(at%8))&1:
                c=rgb565[index]; r=(c>>11)&31; g=(c>>5)&63; b=c&31
                pixels.append(((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2)))
            else:
                pixels.append((48,48,56))
        preview.putdata(pixels);sheet.paste(preview,((n%4)*64,(n//4)*64))
    sheet.save(dest/'preview.png')
    (dest/'pack.json').write_text(json.dumps({
        'format':1,'template':'humanoid64-v1','states':list(STATES),
        'size':[64,64],'payload_bytes':256*2+8*4096+8*512,
        'private_by_default':True,'renderer_integration':'requires staged renderer and later firmware integration',
    },ensure_ascii=False,indent=2)+'\n')

def convert(source, profile_path, output):
    source = source.resolve()
    # Do not allow user input or existing output to be silently replaced.
    if output.exists() or output.is_symlink():
        raise PackError('output already exists; choose a new directory')
    if not output.parent.is_dir():
        raise PackError('output parent must already exist')
    profile = read_profile(profile_path)
    frames = load_frames(source)
    temp = Path(tempfile.mkdtemp(prefix='.avatar-pack-', dir=output.parent))
    try:
        write_pack(temp, frames, profile)
        temp.chmod(0o700)
        for p in temp.iterdir():
            p.chmod(0o600)
        temp.rename(output)
    finally:
        if temp.exists():
            shutil.rmtree(temp)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input',type=Path,required=True)
    parser.add_argument('--profile',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args = parser.parse_args()
    try:
        convert(args.input,args.profile,args.output)
    except PackError as e:
        parser.exit(2,'Avatar pack rejected: '+str(e)+'\n')
    print('Avatar pack created locally; do not publish personal packs.')
