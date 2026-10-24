"""Fail on missing common Chinese, current UI or representative reply glyphs."""
from pathlib import Path
import importlib.util
import json
import re
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('font_union',ROOT/'tools/prepare_multilingual_font.py');font=importlib.util.module_from_spec(spec);spec.loader.exec_module(font)
class MultilingualFontTests(unittest.TestCase):
    def test_common_repertoire_is_complete_and_deduplicated(self):
        big5,gb=font.primary_sets();self.assertEqual((len(big5),len(gb),len(big5|gb)),(5401,3755,6722))
        text=(font.FONTS/'noto_tc_charset.txt').read_text();self.assertEqual(len(text),len(set(text)))
        self.assertFalse((big5|gb|font.ui_characters())-set(text))
        generated={chr(int(x,16)) for x in re.findall(r'/\* U\+([0-9A-Fa-f]+)',(font.FONTS/'muse_font_noto_tc_16.c').read_text())}
        self.assertFalse(set(text)-generated)
    def test_three_reply_samples_need_no_replacement_or_translation(self):
        samples=json.loads((ROOT/'tests/fixtures/multilingual-replies.json').read_text());self.assertEqual(set(samples),{'en','zh-CN','zh-TW'})
        charset=set((font.FONTS/'noto_tc_charset.txt').read_text())
        for language,text in samples.items():
            self.assertFalse(set(text)-charset,language);self.assertNotIn('\ufffd',text)
    def test_generation_metadata_matches_shipped_files(self):
        import hashlib
        m=json.loads((font.FONTS/'multilingual-coverage.json').read_text())
        self.assertEqual(m['source_cmap_missing'],0)
        self.assertEqual(m['font_sha256'],font.FONT_SHA256)
        self.assertEqual(m['charset_sha256'],hashlib.sha256((font.FONTS/'noto_tc_charset.txt').read_bytes()).hexdigest())
        self.assertEqual(m['font_c_sha256'],hashlib.sha256((font.FONTS/'muse_font_noto_tc_16.c').read_bytes()).hexdigest())
        self.assertEqual(m['generated_glyphs'],len((font.FONTS/'noto_tc_charset.txt').read_text()))
