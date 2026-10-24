#!/usr/bin/env python3
"""Pack the reviewed fictional artwork atlases into fixed firmware formats.

Input layout: avatar 4x2, scenes 3x2 (last tile unused). Deterministic technical
normalization only: largest alpha component, nearest-neighbor and fixed palette.
"""
import argparse
from collections import Counter, deque
import json
from pathlib import Path
from PIL import Image
import avatar_pack

SCENES=('gym','desk','living','bedroom','cafe')

def cell(image,col,row,cols,rows):
    return image.crop((col*image.width//cols,row*image.height//rows,
                       (col+1)*image.width//cols,(row+1)*image.height//rows))

def silhouette(image):
    alpha=image.getchannel('A');w,h=image.size
    opaque={i for i,a in enumerate(alpha.get_flattened_data()) if a>=128}
    best=set()
    while opaque:
        first=opaque.pop();component={first};queue=deque([first])
        while queue:
            i=queue.popleft();x=i%w;y=i//w
            for dx,dy in ((1,0),(-1,0),(0,1),(0,-1)):
                nx,ny=x+dx,y+dy;j=ny*w+nx
                if 0<=nx<w and 0<=ny<h and j in opaque:
                    opaque.remove(j);component.add(j);queue.append(j)
        if len(component)>len(best):best=component
    if not best:raise ValueError('atlas cell has no opaque character')
    mask=Image.new('L',image.size);mask.putdata([255 if i in best else 0 for i in range(w*h)])
    clean=image.copy();clean.putalpha(mask)
    return clean.crop(mask.getbbox())

def build(avatar_path,scene_path,dest):
    if dest.exists():raise ValueError('destination already exists')
    with Image.open(avatar_path) as im:avatars=im.convert('RGBA')
    with Image.open(scene_path) as im:scenes=im.convert('RGB')
    dest.mkdir(parents=True);(dest/'avatar').mkdir();(dest/'scenes').mkdir()
    frames=[]
    for n,state in enumerate(avatar_pack.STATES):
        sprite=silhouette(cell(avatars,n%4,n//4,4,2))
        # Fixed template: head/eyes/mouth and body geometry are reviewed at 64x64.
        sprite=sprite.resize((36,45),Image.Resampling.NEAREST)
        frame=Image.new('RGBA',(64,64));frame.paste(sprite,(14,13))
        # Match the existing wearable template without moving the eye/mouth rows:
        # retain head, extend the neck two rows, shirt to y54, feet to y58.
        fitted=Image.new('RGBA',(64,64))
        fitted.paste(frame.crop((0,0,64,42)),(0,0))
        fitted.paste(frame.crop((0,41,64,42)).resize((64,2),Image.Resampling.NEAREST),(0,42))
        fitted.paste(frame.crop((0,42,64,50)).resize((64,11),Image.Resampling.NEAREST),(0,44))
        fitted.paste(frame.crop((0,50,64,58)).resize((64,4),Image.Resampling.NEAREST),(0,55))
        frame=fitted
        frame.save(dest/'avatar'/(state+'.png'));frames.append(frame)
    skin=Counter(rgb[:3] for frame in frames for y in range(33,42) for x in range(21,43)
                 if (rgb:=frame.getpixel((x,y)))[3] and rgb[0]>160 and rgb[0]>rgb[1]>rgb[2]).most_common(1)[0][0]
    profile={'version':1,'template':'humanoid64-v1','anchors':avatar_pack.ANCHORS,
             'colors':{'skin':'#%02X%02X%02X'%skin,'cheek':'#%02X%02X%02X'%tuple(int(v*f) for v,f in zip(skin,(.87,.72,.71))),'mouth':'#843F2B'}}
    (dest/'profile.json').write_text(json.dumps(profile,indent=2)+'\n')
    avatar_pack.convert(dest/'avatar',dest/'profile.json',dest/'pack')
    palettes=[];pixels=[]
    for n,name in enumerate(SCENES):
        scene=cell(scenes,n%3,n//3,3,2).resize((116,116),Image.Resampling.NEAREST)
        indexed=scene.quantize(colors=256,method=Image.Quantize.MEDIANCUT)
        raw=indexed.getpalette();palette=[((raw[i]>>3)<<11)|((raw[i+1]>>2)<<5)|(raw[i+2]>>3) for i in range(0,768,3)]
        palettes.append(palette);pixels.append(list(indexed.get_flattened_data()))
        # New image objects discard source metadata.
        out=Image.new('RGB',(116,116));out.putdata(scene.get_flattened_data());out.save(dest/'scenes'/(name+'.png'))
    header='/* Fictional public scenes; generated using prepare_default_art.py. */\n#pragma once\n#include <stdint.h>\n'
    header+=avatar_pack.array('style_scene_palette','uint16_t',palettes,16)
    header+=avatar_pack.array('style_scene_pixels','uint8_t',pixels,64)
    (dest/'pack/style_scene_assets.h').write_text(header)
    (dest/'asset-origin.json').write_text(json.dumps({'origin':'built-in imagegen, fictional public designs approved as visual direction',
        'contains_real_person':False,'avatar_layout':'4x2 row-major','scene_layout':'3x2 row-major, first five tiles',
        'normalization':'alpha >=128 largest component; 36x45 at (14,13); head unchanged, neck extended, shirt mapped to y44..54, feet y55..58; nearest-neighbor; RGB565 palettes',
        'states':list(avatar_pack.STATES),'scenes':list(SCENES),'hardware_verified':False},indent=2)+'\n')

if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--avatar-atlas',type=Path,required=True)
    ap.add_argument('--scene-atlas',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args();build(a.avatar_atlas,a.scene_atlas,a.output)
