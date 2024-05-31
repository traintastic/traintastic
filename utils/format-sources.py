#!/usr/bin/python3

import os
import subprocess

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

def format_sources(path):
    for subdir, dirs, files in os.walk(path):
        for filename in files:
            if filename.endswith('.hpp') or filename.endswith('.cpp') or filename.endswith('.tpp'):
                file = os.path.join(subdir, filename)
                print(file)
                subprocess.check_call(['clang-format', '-i', file])

if __name__ == "__main__":
    SOURCE_DIRS = [
        os.path.join('client', 'src'),
        os.path.join('server', 'src'),
        os.path.join('server', 'test'),
        os.path.join('shared', 'src')]

    for source_dir in SOURCE_DIRS:
        format_sources(os.path.abspath(os.path.join(PROJECT_ROOT, source_dir)))
