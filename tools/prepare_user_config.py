#!/usr/bin/env python3
"""Generate a private Kconfig overlay offline. Never prints configuration values."""
import argparse
import json
import os
from pathlib import Path
import re

class ConfigError(ValueError):
    pass

def prepare(source, output, private_root):
    if source.is_symlink() or not source.is_file() or source.stat().st_size > 4096:
        raise ConfigError('invalid input file')
    try:
        data=json.loads(source.read_text(encoding='utf-8'))
    except (ValueError, UnicodeError):
        raise ConfigError('invalid JSON') from None
    if not isinstance(data,dict) or set(data)!={'tts_enabled','api_key','voice_id','model'}:
        raise ConfigError('unsupported configuration fields')
    if type(data['tts_enabled']) is not bool or any(not isinstance(data[k],str) for k in ('api_key','voice_id','model')):
        raise ConfigError('invalid field types')
    enabled=data['tts_enabled'];key=data['api_key'];voice=data['voice_id'];model=data['model']
    if not re.fullmatch(r'[A-Za-z0-9._-]{1,64}',model):
        raise ConfigError('invalid model identifier')
    if not enabled and (key or voice):
        raise ConfigError('disabled TTS must have blank credentials')
    if enabled and (not 1<=len(key)<=256 or any(ord(c)<33 or ord(c)>126 for c in key)
                    or not re.fullmatch(r'[A-Za-z0-9_-]{1,128}',voice)):
        raise ConfigError('enabled TTS requires valid local credentials')
    private_root=private_root.resolve()
    # Destination stays inside the explicitly selected private root, never in overlay sources.
    if output.is_symlink() or not output.resolve().is_relative_to(private_root) or output.resolve()==private_root:
        raise ConfigError('output must be inside the private directory')
    if output.exists():
        raise ConfigError('output already exists')
    output.parent.mkdir(parents=True,exist_ok=True,mode=0o700)
    text='# Private user configuration. Never publish config or firmware built with credentials.\n'
    text+='CONFIG_MUSE_FISH_TTS='+('y' if enabled else 'n')+'\n'
    if enabled:
        for k,v in [('MUSE_FISH_API_KEY',key),('MUSE_FISH_VOICE_ID',voice),('MUSE_FISH_MODEL',model)]:
            text+='CONFIG_'+k+'='+json.dumps(v)+'\n'
    # sdkconfig.defaults only sets existing Kconfig symbols; blank/default-disabled uses no identity.
    else:
        text+='# External speech is disabled. Pair Muse using your own account on the device.\n'
    fd=os.open(output,os.O_WRONLY|os.O_CREAT|os.O_EXCL,0o600)
    with os.fdopen(fd,'w',encoding='utf-8') as f:
        f.write(text)

if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--input',type=Path,required=True)
    ap.add_argument('--output',type=Path,default=Path('user-assets/sdkconfig.local'))
    ap.add_argument('--private-root',type=Path,default=Path('user-assets'))
    args=ap.parse_args()
    try:
        prepare(args.input,args.output,args.private_root)
    except (ConfigError,OSError) as e:
        ap.exit(2,'Configuration rejected; values suppressed.\n')
    print('Private configuration created; no build, network request or flash performed.')
