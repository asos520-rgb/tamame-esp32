import importlib.util
import json
from pathlib import Path
import stat
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('prepare_user_config',ROOT/'tools/prepare_user_config.py')
config=importlib.util.module_from_spec(spec);spec.loader.exec_module(config)

class UserConfigTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.root=Path(self.temp.name)
        self.source=self.root/'input.json';self.private=self.root/'private';self.output=self.private/'sdkconfig.local'
        self.data={'tts_enabled':False,'api_key':'','voice_id':'','model':'example-model'}
    def tearDown(self):self.temp.cleanup()
    def write(self):self.source.write_text(json.dumps(self.data))
    def test_default_never_enables_network_provider(self):
        self.write();config.prepare(self.source,self.output,self.private)
        self.assertIn('CONFIG_MUSE_FISH_TTS=n',self.output.read_text())
        self.assertNotIn('CONFIG_MUSE_FISH_API_KEY',self.output.read_text())
        self.assertEqual(stat.S_IMODE(self.output.stat().st_mode),0o600)
        self.assertTrue(self.source.exists())
    def test_explicit_opt_in_saved_privately_without_echo(self):
        fixture_value='synthetic-test-credential'
        self.data.update(tts_enabled=True,api_key=fixture_value,voice_id='synthetic_voice');self.write()
        r=subprocess.run([sys.executable,'-B',str(ROOT/'tools/prepare_user_config.py'),'--input',str(self.source),'--output',str(self.output),'--private-root',str(self.private)],capture_output=True,text=True)
        self.assertEqual(r.returncode,0,r.stderr)
        self.assertNotIn(fixture_value,r.stdout+r.stderr)
        self.assertIn(fixture_value,self.output.read_text())
        self.assertNotIn('GADGET_SDK_TOKEN',self.output.read_text())
    def test_rejects_disabled_secret_and_missing_credentials(self):
        for enabled,key,voice in [(False,'secret','voice'),(True,'','voice'),(True,'secret',''),(True,'bad\nkey','voice')]:
            self.data.update(tts_enabled=enabled,api_key=key,voice_id=voice);self.write()
            with self.assertRaises(config.ConfigError):config.prepare(self.source,self.output,self.private)
            self.assertFalse(self.output.exists())
    def test_rejects_unknown_fields_and_output_escape(self):
        self.data['unexpected']='private-data';self.write()
        with self.assertRaises(config.ConfigError):config.prepare(self.source,self.output,self.private)
        del self.data['unexpected'];self.write()
        with self.assertRaises(config.ConfigError):config.prepare(self.source,self.root/'outside',self.private)
    def test_refuses_overwrite_and_symlink(self):
        self.write();config.prepare(self.source,self.output,self.private)
        before=self.output.read_bytes()
        with self.assertRaises(config.ConfigError):config.prepare(self.source,self.output,self.private)
        self.assertEqual(before,self.output.read_bytes())
        target=self.root/'link';target.symlink_to(self.source)
        with self.assertRaises(config.ConfigError):config.prepare(target,self.private/'new',self.private)
