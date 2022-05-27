import sys
from traintastic import read_locale_file, write_locale_file


def locale_add_missing(source, destination):
    src_strings = read_locale_file(source)[0]
    (strings, header, garbage) = read_locale_file(destination)
    for src in src_strings:
        if not any((s['id'] == src['id'] and s['ns'] == src['ns']) for s in strings):
            src['hash'] = True
            strings.append(src)
    write_locale_file(destination, header, strings, garbage)


if __name__ == "__main__":
    locale_add_missing(sys.argv[1], sys.argv[2])
