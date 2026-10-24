"""Run the actual staged console function with synthetic sensitive arguments."""
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]

class PublicConsoleTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which('cc'),'native compiler unavailable')
    def test_console_never_outputs_text_or_arbitrary_fields(self):
        source=(ROOT/'firmware-overlay/esp32/components/muse/muse_chat_text.c').read_text()
        a=source.index('void muse_hatch_console(');b=source.index('\n}\n',a)+3
        function=source[a:b]
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)
            (p/'test.c').write_text('#include <stdio.h>\n'+function+'''
int main(void){muse_hatch_console("private-event-name","synthetic private conversation",
"\\\"authorization\\\":\\\"%s\\\"","synthetic-private-value");return 0;}
''')
            r=subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(p/'test.c'),'-o',str(p/'test')],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stderr)
            r=subprocess.run([str(p/'test')],capture_output=True,text=True,check=True)
            self.assertEqual(r.stdout,'@chat {"type":"event"}\n')
            self.assertEqual(r.stderr,'')
