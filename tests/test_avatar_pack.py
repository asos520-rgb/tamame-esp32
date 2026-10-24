"""Synthetic-only conversion and native-renderer checks. No user portraits."""
import copy
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from PIL import Image, ImageDraw, PngImagePlugin

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('avatar_pack', ROOT/'tools/avatar_pack.py')
pack = importlib.util.module_from_spec(spec); spec.loader.exec_module(pack)

def fixture(root):
    source=root/'source';source.mkdir()
    for n,state in enumerate(pack.STATES):
        image=Image.new('RGBA',(64,64),(250,10,200,0))
        draw=ImageDraw.Draw(image)
        draw.rectangle((18,18,45,56),fill=(60+n*8,100,150,255))
        draw.rectangle((20,46,43,54),fill=(0,0,0,255))
        draw.rectangle((19,31,28,38),fill=(0,0,0,255))
        draw.rectangle((34,31,43,38),fill=(0,0,0,255))
        metadata=PngImagePlugin.PngInfo();metadata.add_text('private-note','SYNTHETIC_METADATA_MUST_NOT_PROPAGATE')
        image.save(source/(state+'.png'),pnginfo=metadata)
    profile=root/'profile.json'
    profile.write_text((ROOT/'examples/avatar-profile.json').read_text())
    return source,profile,root/'pack'

class AvatarTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.root=Path(self.temp.name)
        self.source,self.profile,self.output=fixture(self.root)
    def tearDown(self):
        self.temp.cleanup()
    def test_black_pixels_are_opaque_and_hidden_rgb_is_ignored(self):
        frames=pack.load_frames(self.source)
        palette,indices,masks=pack.pack_pixels(frames)
        at=50*64+30
        self.assertEqual(palette[indices[0][at]],(0,0,0))
        self.assertTrue((masks[0][at//8]>>(at%8))&1)
        self.assertEqual(masks[0][0],0)
        self.assertNotIn((250,10,200),palette)
    def test_missing_state_rejected_without_partial_output(self):
        (self.source/'off.png').unlink()
        with self.assertRaises(pack.PackError):pack.convert(self.source,self.profile,self.output)
        self.assertFalse(self.output.exists())
    def test_wrong_size_rejected(self):
        Image.new('RGBA',(128,128)).save(self.source/'idle.png')
        with self.assertRaises(pack.PackError):pack.load_frames(self.source)
    def test_partial_alpha_rejected(self):
        with Image.open(self.source/'idle.png') as source:image=source.copy()
        image.putpixel((30,30),(1,2,3,128));image.save(self.source/'idle.png')
        with self.assertRaises(pack.PackError):pack.load_frames(self.source)
    def test_rgb_without_alpha_rejected(self):
        Image.new('RGB',(64,64)).save(self.source/'idle.png')
        with self.assertRaises(pack.PackError):pack.load_frames(self.source)
    def test_empty_and_edge_touching_rejected(self):
        for pixel in (None,(0,0)):
            image=Image.new('RGBA',(64,64))
            if pixel:image.putpixel(pixel,(0,0,0,255))
            image.save(self.source/'idle.png')
            with self.assertRaises(pack.PackError):pack.load_frames(self.source)
    def test_symlink_input_rejected(self):
        (self.source/'idle.png').unlink()
        (self.source/'idle.png').symlink_to(self.source/'boot.png')
        with self.assertRaises(pack.PackError):pack.load_frames(self.source)
    def test_profile_rejects_arbitrary_geometry_and_injected_code(self):
        original=json.loads(self.profile.read_text())
        for field in ('anchors','colors','unknown'):
            p=copy.deepcopy(original)
            if field=='anchors':p[field]['mouth']=[0,0,100,100]
            elif field=='colors':p[field]['skin']='0x00; unexpected code'
            else:p[field]='private-name'
            self.profile.write_text(json.dumps(p))
            with self.assertRaises(pack.PackError):pack.read_profile(self.profile)
    def test_profile_coordinates_must_be_integers(self):
        p=json.loads(self.profile.read_text())
        p['anchors']['mouth'][0]=28.0
        self.profile.write_text(json.dumps(p))
        with self.assertRaises(pack.PackError):pack.read_profile(self.profile)
    def test_output_is_deterministic_metadata_free_and_non_overwriting(self):
        pack.convert(self.source,self.profile,self.output)
        with Image.open(self.output/'preview.png') as preview:
            self.assertNotIn('private-note',preview.info)
        for p in self.output.iterdir():
            self.assertNotIn(b'SYNTHETIC_METADATA',p.read_bytes())
            self.assertNotIn(str(self.source).encode(),p.read_bytes())
        other=self.root/'other';pack.convert(self.source,self.profile,other)
        hashes=lambda d:{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in d.iterdir()}
        self.assertEqual(hashes(self.output),hashes(other))
        with self.assertRaises(pack.PackError):pack.convert(self.source,self.profile,self.output)
    def test_more_than_256_colors_preserves_black_and_masks(self):
        frames=pack.load_frames(self.source)
        for frame in frames:
            for y in range(20,30):
                for x in range(20,45):frame.putpixel((x,y),(x*5,y*7,(x*y)%256,255))
        palette,indices,masks=pack.pack_pixels(frames)
        self.assertEqual(len(palette),256)
        self.assertTrue(all(0<=i<=255 for row in indices for i in row))
        self.assertEqual(palette[indices[0][50*64+30]],(0,0,0))
        self.assertEqual(sum(x.bit_count() for x in masks[0]),28*39)
    @unittest.skipUnless(shutil.which('cc'),'native C compiler unavailable')
    def test_native_renderer_all_modes_and_wardrobe_with_sanitizers(self):
        with Image.open(self.source/'idle.png') as src:image=src.copy()
        image.putpixel((20,32),(200,100,50,255));image.save(self.source/'idle.png')
        with Image.open(self.source/'off.png') as src:image=src.copy()
        image.putpixel((20,32),(0,0,0,0));image.save(self.source/'off.png')
        pack.convert(self.source,self.profile,self.output)
        for p in (ROOT/'renderer').iterdir():
            if p.suffix in {'.c','.h'}:shutil.copyfile(p,self.output/p.name)
        (self.output/'style_scene_assets.h').write_text(
            '#include <stdint.h>\nstatic const uint16_t style_scene_palette[5][256] = {{0x39e7},{0x39e7},{0x39e7},{0x39e7},{0x39e7}};\n'
            'static const uint8_t style_scene_pixels[5][13456] = {{0}};\n')
        harness=r'''
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include "muse_pixel.h"
#include "avatar_profile.h"
void muse_pixel_set_interaction(int);
static uint16_t rgb565(unsigned c){return ((c>>19)&31)<<11|((c>>10)&63)<<5|((c>>3)&31);}
int main(void){
 uint16_t pixels[64*64],preview[64*64];
 muse_pixel_set_size(64);muse_pixel_set_scene(1,0,0);
 muse_pose_t p={.mode=MUSE_MODE_IDLE,.t=0,.mode_t=2};
 muse_pixel_render(&p);muse_pixel_scale(pixels,64,0,63,0,63);
 assert(pixels[50*64+30]==0); /* opaque black clothes never reveal scene */
 assert(pixels[60*64+32]==0x39e7); /* true transparency shows scene */
 p.t=1.05f;muse_pixel_render(&p);muse_pixel_scale(pixels,64,0,63,0,63);
 assert(pixels[33*64+20]==rgb565(0xC86432)); /* transparent eyelid patch does not stamp black */
 muse_pixel_set_interaction(1);p.t=0;p.level=1;
 muse_pixel_render(&p);muse_pixel_scale(pixels,64,0,63,0,63);
 assert(pixels[48*64+26]==rgb565(PET_AVATAR_SKIN));
 unsigned cheek=PET_AVATAR_CHEEK;
 unsigned dimmed=((unsigned)(((cheek>>16)&255)*.9f)<<16)|((unsigned)(((cheek>>8)&255)*.9f)<<8)|(unsigned)((cheek&255)*.9f);
 assert(pixels[40*64+20]==rgb565(dimmed));
 muse_pixel_set_interaction(0);muse_pixel_set_style(1,0,0);
 muse_pixel_render(&p);muse_pixel_scale(pixels,64,0,63,0,63);
 assert(pixels[51*64+36]!=rgb565(0xF6F4E9)); /* no old logo */
 for(unsigned bg=0;bg<=5;bg++)for(unsigned cloth=0;cloth<3;cloth++)
 for(unsigned acc=0;acc<3;acc++)for(unsigned glasses=0;glasses<2;glasses++){
  muse_pixel_set_scene(bg,0,0);muse_pixel_set_style(cloth,acc,glasses);
  for(int mode=0;mode<MUSE_MODE_COUNT;mode++)for(int inter=0;inter<3;inter++)
  for(int frame=0;frame<8;frame++){
   p=(muse_pose_t){.mode=mode,.t=frame*.27f,.mode_t=frame*.31f,.happy=frame%2,.level=frame*.13f};
   muse_pixel_set_interaction(inter);muse_pixel_render(&p);muse_pixel_scale(pixels,64,0,63,0,63);
   muse_pixel_style_preview(preview,bg,cloth,acc,glasses);
   uint16_t after[64*64];muse_pixel_scale(after,64,0,63,0,63);
   for(int i=0;i<4096;i++)assert(after[i]==pixels[i]);
  }
 }
 assert(muse_pixel_scene_pixel(6,10,10)==0);
 assert(muse_pixel_scene_pixel(5,464,464)==0x39e7);
 assert(muse_pixel_scene_pixel(5,465,465)==0);
 return 0;
}
'''
        (self.output/'harness.c').write_text(harness)
        executable=self.output/'check'
        subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer',
            '-I',str(self.output),str(self.output/'muse_pixel.c'),str(self.output/'harness.c'),'-lm','-o',str(executable)],check=True,capture_output=True)
        result=subprocess.run([str(executable)],capture_output=True,text=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'},timeout=90)
        self.assertEqual(result.returncode,0,result.stderr)

if __name__=='__main__':unittest.main()
