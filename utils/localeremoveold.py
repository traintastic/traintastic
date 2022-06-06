import sys
from traintastic import read_locale_file, write_locale_file


def locale_remove_old(source, destination):
    src_strings = read_locale_file(source)[0]
    (strings, header, garbage) = read_locale_file(destination)

    strings = list(filter(lambda s: any((s['id'] == src['id'] and s['ns'] == src['ns']) for src in src_strings), strings))

    write_locale_file(destination, header, strings, garbage)


if __name__ == "__main__":
    locale_remove_old(sys.argv[1], sys.argv[2])
