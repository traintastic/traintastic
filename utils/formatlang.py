import sys
import os
import re
import codecs

def format_lang_file(filename):
    with codecs.open(filename, 'r', 'utf-8') as f:
        lines = f.readlines()

    header = ''
    for line in lines:
        if line.startswith('##'):
            header += line
        else:
            break

    r = re.compile(r'^(#|)(([a-z0-9_\.:]*):([a-z0-9_\.-]+))=(.*)$')
    strings = []
    garbage = ''
    for line in lines:
        m = r.match(line)
        if m is not None:
            strings.append({
                'id': m.group(2),
                'ns': m.group(3),
                'value': m.group(5),
                'hash': m.group(1) == '#',
              })
        elif line != '\n' and not line.startswith('##'):
            garbage += line

    txt = header
    ns = None
    for s in sorted(strings, key=lambda k: k['id']):
        if ns != s['ns']:
            ns = s['ns']
            txt += '\n'
        txt += ('#' if s['hash'] else '') + s['id'] + '=' + s['value'] + '\n'

    if garbage != '':
        txt += '\n\n#! Garbage\n' + garbage

    i = 0
    while True:
        backup = filename + '~' + str(i)
        if not os.path.exists(backup):
            break
        i += 1

    os.rename(filename, backup)

    with codecs.open(filename, 'w', 'utf-8') as f:
        f.write(txt)


if __name__ == "__main__":
    for f in sys.argv[1:]:
        format_lang_file(f)
