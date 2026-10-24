"""Validate the shipped fictional pack and run its actual renderer under ASan."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]
ART=ROOT/'assets/default-v2'
COMP=ROOT/'firmware-overlay/esp32/components/muse'

class DefaultArtTests(unittest.TestCase):
    def test_eight_frames_and_five_scenes_have_exact_format(self):
        states=json.loads((ART/'asset-origin.json').read_text())['states']
        self.assertEqual(len(states),8)
        for state in states:
            with Image.open(ART/'avatar'/(state+'.png')) as im:
                self.assertEqual(im.size,(64,64));self.assertEqual(im.mode,'RGBA')
                self.assertLessEqual(set(im.getchannel('A').get_flattened_data()),{0,255})
                x0,y0,x1,y1=im.getchannel('A').getbbox()
                self.assertTrue(3<=x0<x1<=61 and 3<=y0<y1<=61)
                self.assertFalse(im.info)
        scenes=list((ART/'scenes').glob('*.png'));self.assertEqual(len(scenes),5)
        for path in scenes:
            with Image.open(path) as im:
                self.assertEqual(im.size,(116,116));self.assertEqual(im.mode,'RGB');self.assertFalse(im.info)
        for name in ['portrait_assets.h','style_silhouette.h','avatar_profile.h','style_scene_assets.h']:
            self.assertEqual((ART/'pack'/name).read_bytes(),(COMP/'avatar'/name).read_bytes())
    @unittest.skipUnless(shutil.which('cc'),'native compiler unavailable')
    def test_actual_pack_all_modes_outfits_and_accessories(self):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)
            source=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "muse_pixel.h"
void muse_pixel_set_interaction(int);
int main(void){
 uint16_t pixels[4096],after[4096],preview[4096];
 muse_pixel_set_size(64);
 for(unsigned bg=0;bg<=5;bg++)for(unsigned cloth=0;cloth<3;cloth++)
 for(unsigned acc=0;acc<3;acc++)for(unsigned glasses=0;glasses<2;glasses++)
 for(int mode=0;mode<MUSE_MODE_COUNT;mode++)for(int interaction=0;interaction<3;interaction++)
 for(int frame=0;frame<8;frame++){
  muse_pixel_set_scene(bg,0,0);muse_pixel_set_style(cloth,acc,glasses);muse_pixel_set_interaction(interaction);
  muse_pose_t pose={.mode=mode,.t=frame*.27f,.mode_t=frame*.31f,.happy=frame%2,.level=frame*.13f};
  muse_pixel_render(&pose);muse_pixel_scale(pixels,64,0,63,0,63);
  muse_pixel_style_preview(preview,bg,cloth,acc,glasses);
  muse_pixel_scale(after,64,0,63,0,63);assert(!memcmp(pixels,after,sizeof pixels));
 }
 assert(muse_pixel_scene_pixel(6,30,30)==0);
 return 0;
}
'''
            (p/'test.c').write_text(source)
            r=subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',
                '-I',str(COMP),'-I',str(ROOT/'renderer'),'-I',str(ART/'pack'),
                str(COMP/'avatar/muse_pixel.c'),str(p/'test.c'),'-lm','-o',str(p/'test')],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stderr)
            r=subprocess.run([str(p/'test')],capture_output=True,text=True,timeout=60,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})
            self.assertEqual(r.returncode,0,r.stderr)
