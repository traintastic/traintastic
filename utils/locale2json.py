import sys
import os
import json
import codecs
from traintastic import read_locale_file


def to_json(filename_txt, filename_json):
    (strings, header, garbage) = read_locale_file(filename_txt)
    out = []
    for s in strings:
        if not s['hash']:
            out.append({'term': s['id'], 'definition': s['value']})
    with codecs.open(filename_json, 'w', 'utf-8') as f:
        json.dump(out, f, indent='  ')


if __name__ == "__main__":
    for f in sys.argv[1:]:
        to_json(f, os.path.splitext(f)[0] + '.json')
