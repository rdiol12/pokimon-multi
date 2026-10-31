#!/usr/bin/env python3
"""Copies chosen screenshots (upscaled 3x), an overview sheet and the full
map into a folder for sharing.

Usage: publish.py OUT_DIR MAP_PNG shot.png[=name] ...
"""
import os, shutil, subprocess, sys

out, full = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
picked = []
for i, arg in enumerate(sys.argv[3:], 1):
    src, _, name = arg.partition('=')
    name = name or os.path.basename(src)[6:-4]
    dst = os.path.join(out, f'{i:02d}-{name}.png')
    subprocess.check_call(['convert', src, '-filter', 'point', '-resize', '300%', dst])
    picked.append((src, name))
subprocess.check_call(['convert', full, os.path.join(out, 'full-map.png')])
args = []
for src, name in picked:
    args += ['-label', name, src]
subprocess.check_call(['montage', *args, '-tile', '4x', '-geometry', '480x320+4+4', '-pointsize', '18',
                       '-background', '#202028', '-fill', 'white', os.path.join(out, 'overview.png')])
print('published', len(picked), 'shots to', out)
