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
    import sys

    if len(sys.argv) != 3:
        print("Usage: {:s} <json file> <lang file>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    json_to_lang(sys.argv[1], sys.argv[2])
