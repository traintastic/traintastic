import sys
from traintastic import read_locale_file, write_locale_file


def format_locale(filename):
    (strings, header, garbage) = read_locale_file(filename)
    write_locale_file(filename, header, strings, garbage)


if __name__ == "__main__":
    for f in sys.argv[1:]:
        format_locale(f)
