#!/usr/bin/env python3

import sys
import os
from argparse import ArgumentParser
from traintasticmanualbuilder.utils import detect_version

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: " + sys.argv[0] + " <format>", file=sys.stderr)
        sys.exit(1)

    args = sys.argv[2:]  # remove program name and format

    # Standard options:
    parser = ArgumentParser()
    parser.add_argument('--base-dir', default=os.path.join(os.path.dirname(__file__), 'traintasticmanual'))
    parser.add_argument('--output-dir')
    parser.add_argument('--language', default='en-us')
    parser.add_argument('--version', default=detect_version())

    if sys.argv[1] == 'html-single-page':
        from traintasticmanualbuilder.htmlsinglepage import HTMLSinglePageBuilder
        HTMLSinglePageBuilder(parser.parse_args(args)).build()
    else:
        print("Unknown format: " + sys.argv[1], file=sys.stderr)
        sys.exit(1)
