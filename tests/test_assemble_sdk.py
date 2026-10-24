import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('assemble_sdk',ROOT/'tools/assemble_sdk.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

class AssembleTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.root=Path(self.temp.name)
        self.repo=self.root/'upstream';self.repo.mkdir()
        self.package=self.root/'package';self.package.mkdir()
        self.output=self.root/'assembled'
        env={**os.environ,'GIT_AUTHOR_NAME':'Test Fixture','GIT_AUTHOR_EMAIL':'test@invalid',
             'GIT_COMMITTER_NAME':'Test Fixture','GIT_COMMITTER_EMAIL':'test@invalid'}
        def git(*args):return subprocess.check_output(['git','-C',str(self.repo),*args],env=env,stderr=subprocess.DEVNULL)
        git('init');(self.repo/'esp32').mkdir();(self.repo/'esp32/base.c').write_text('int base_version=1;\n')
        (self.repo/'LICENSE').write_text('Synthetic test licence\n');git('add','.');git('commit','-m','fixture')
        sha=git('rev-parse','HEAD').decode().strip()
        (self.repo/'esp32/base.c').write_text('uncommitted private content\n')
        (self.repo/'esp32/sdkconfig').write_text('private local config\n')
        overlay=self.package/'firmware-overlay/esp32/add.c';overlay.parent.mkdir(parents=True);overlay.write_text('int extension=2;\n')
        self.plan={'upstream':{'commit':sha},'overlay_root':'firmware-overlay','overlay_files':[
            {'path':'esp32/add.c','sha256':hashlib.sha256(overlay.read_bytes()).hexdigest()}],'required_generated_assets':[]}
        self.save()
    def save(self):(self.package/'source-plan.json').write_text(json.dumps(self.plan))
    def tearDown(self):self.temp.cleanup()
    def test_exports_committed_base_only_and_no_git_or_local_config(self):
        module.assemble(self.package,self.repo,self.output)
        self.assertEqual((self.output/'esp32/base.c').read_text(),'int base_version=1;\n')
        self.assertTrue((self.output/'esp32/add.c').is_file())
        self.assertFalse((self.output/'.git').exists());self.assertFalse((self.output/'esp32/sdkconfig').exists())
        with self.assertRaises(ValueError):module.assemble(self.package,self.repo,self.output)
    def test_tampering_and_missing_assets_fail_before_output(self):
        self.plan['overlay_files'][0]['sha256']='0'*64;self.save()
        with self.assertRaises(ValueError):module.assemble(self.package,self.repo,self.output)
        self.assertFalse(self.output.exists())
        self.plan['overlay_files']=[];self.plan['required_generated_assets']=[{'path':'esp32/missing.h'}];self.save()
        with self.assertRaises(ValueError):module.assemble(self.package,self.repo,self.output)
        self.assertFalse(self.output.exists())
    def test_path_traversal_rejected(self):
        for name in ['../private','/absolute','esp32/../../private','esp32\\private']:
            with self.assertRaises(ValueError):module.safe_path(name)
