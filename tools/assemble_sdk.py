#!/usr/bin/env python3
"""Assemble pinned committed SDK + reviewed public overlay, without Git history.

Does not clone, use working-tree files, install dependencies, build or flash.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import tarfile
import tempfile

def safe_path(name):
    p=PurePosixPath(name)
    if p.is_absolute() or '..' in p.parts or '\\' in name or not p.parts:
        raise ValueError('unsafe source path')
    return p

def assemble(package,upstream,output):
    plan=json.loads((package/'source-plan.json').read_text())
    if output.exists() or output.is_symlink():raise ValueError('output already exists')
    if not output.parent.is_dir():raise ValueError('output parent must exist')
    for entry in plan['overlay_files']:
        rel=safe_path(entry['path']);source=package/plan['overlay_root']/str(rel)
        if source.is_symlink() or not source.is_file() or hashlib.sha256(source.read_bytes()).hexdigest()!=entry['sha256']:
            raise ValueError('overlay does not match reviewed manifest')
    for entry in plan['required_generated_assets']:
        if not (package/plan['overlay_root']/str(safe_path(entry['path']))).is_file():
            raise ValueError('required public artwork is missing')
    commit=plan['upstream']['commit']
    if len(commit)!=40 or any(x not in '0123456789abcdef' for x in commit):raise ValueError('invalid pinned commit')
    # Export objects, never copy .git/config, untracked files, or a user's sdkconfig.
    archive=subprocess.run(['git','-C',str(upstream),'archive',commit,'esp32','LICENSE'],check=True,capture_output=True).stdout
    temp=Path(tempfile.mkdtemp(prefix='.public-sdk-',dir=output.parent))
    try:
        with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
            for member in tar:
                rel=safe_path(member.name);dest=temp/str(rel)
                if not dest.resolve().is_relative_to(temp.resolve()):raise ValueError('archive path escape')
                if member.isdir():dest.mkdir(parents=True,exist_ok=True)
                elif member.isfile():
                    dest.parent.mkdir(parents=True,exist_ok=True)
                    dest.write_bytes(tar.extractfile(member).read());dest.chmod(member.mode&0o777)
                else:raise ValueError('archive links and special files are unsupported')
        for entry in plan['overlay_files']:
            dest=temp/entry['path'];dest.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(package/plan['overlay_root']/entry['path'],dest)
        temp.rename(output)
    finally:
        if temp.exists():shutil.rmtree(temp)

if __name__=='__main__':
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--upstream',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    ap.add_argument('--package',type=Path,default=Path(__file__).resolve().parents[1])
    a=ap.parse_args()
    try:assemble(a.package,a.upstream,a.output)
    except (ValueError,OSError,subprocess.CalledProcessError):ap.exit(2,'Assembly rejected; check pinned source, manifest and destination.\n')
    print('Public source assembled from pinned objects; no history, private config, build or flash copied.')
