#!/usr/bin/env python3
"""Build real LVGL font/label tests and render synthetic replies offline.

Requires matching LVGL sources, SDK Muse headers, a compiler and CMake.
Output stays in a new user-selected directory. Does not build device firmware,
flash, connect to a service, or translate sample text.
"""
from pathlib import Path
import argparse,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def check(lvgl,muse,output,cmake):
    if output.exists() or output.is_symlink():raise ValueError('Choose a new output directory')
    output.mkdir(parents=True)
    source=ROOT/'tests/font-native';build=output/'build'
    subprocess.run([cmake,'-S',str(source),'-B',str(build),'-DLVGL_SOURCE_DIR='+str(lvgl.resolve()),'-DSDK_MUSE_INCLUDE_DIR='+str(muse.resolve()),'-DCMAKE_BUILD_TYPE=Debug'],check=True)
    subprocess.run([cmake,'--build',str(build),'--parallel','4'],check=True)
    samples=json.loads((ROOT/'tests/fixtures/multilingual-replies.json').read_text());results=[]
    for index,(language,text) in enumerate(samples.items()):
        target=output/language;target.mkdir();sample=target/'reply.txt';sample.write_text(text)
        run=subprocess.run([str((build/'tamame_font_test').resolve()),str((ROOT/'firmware-overlay/esp32/components/muse/fonts/noto_tc_charset.txt').resolve()),str(sample.resolve()),language],cwd=target,check=True,capture_output=True,text=True)
        assert 'missing=0 unchanged_text=1' in run.stdout
        assert (target/'caption.ppm').is_file()
        wardrobe=subprocess.run([str((build/'tamame_wardrobe_test').resolve()),str(index)],cwd=target,check=True,capture_output=True,text=True)
        assert 'three_tabs=1 labels_fit=1 independent_glasses=1 save_cancel=1' in wardrobe.stdout
        results.append({'language':language,'result':run.stdout.strip(),'image':language+'/caption.ppm','wardrobe':wardrobe.stdout.strip(),'wardrobe_preview_art':'stubbed; actual production LVGL labels and controls'})
    (output/'results.json').write_text(json.dumps({'engine':'actual compiled LVGL glyph lookup and label snapshot','samples_are_synthetic':True,'hardware_tested':False,'results':results},indent=2)+'\n')
    print('All three actual LVGL replies have zero missing glyphs and unchanged text; production wardrobe labels, combined accessory/glasses selection, Apply and Cancel passed.')
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--lvgl',type=Path,required=True);p.add_argument('--muse-includes',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--cmake',default='cmake');a=p.parse_args();check(a.lvgl,a.muse_includes,a.output,a.cmake)
