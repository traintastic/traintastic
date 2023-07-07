import json
import codecs
import struct


def json_to_lang(filename_json, filename_lang):
    with codecs.open(filename_json, 'r', 'utf-8') as f:
        strings = json.load(f)

    with open(filename_lang, 'wb') as f:
        for s in strings:
            if s['definition'] is None:
                continue

            term = s['term'].encode('utf-8')
            f.write(struct.pack('<I', len(term)))  # term length in bytes
            f.write(term)
            if len(term) % 4 != 0:
                f.write(bytes([0] * (4 - len(term) % 4)))

            definition = s['definition'].encode('utf-8')
            f.write(struct.pack('<I', len(definition)))  # definition length in bytes
            f.write(definition)
            if len(definition) % 4 != 0:
                f.write(bytes([0] * (4 - len(definition) % 4)))


if __name__ == "__main__":
    import os
    import re
    path = os.path.realpath(os.path.dirname(__file__))
    for item in os.scandir(path):
        if re.match(r'^([a-z]{2}-[a-z]{2}|neutral)\.json$', item.name) is not None:
            filename_json = os.path.join(path, item.name)
            filename_lang = os.path.splitext(filename_json)[0] + '.lang'
            if not os.path.exists(filename_lang) or os.path.getmtime(filename_json) > os.path.getmtime(filename_lang):
                print('Building language file: {:s} => {:s}'.format(os.path.basename(filename_json), os.path.basename(filename_lang)))
                json_to_lang(filename_json, filename_lang)
