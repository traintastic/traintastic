#!/usr/bin/env python3

import sys
import os
from argparse import ArgumentParser


def detect_version():
    if 'GITHUB_ACTIONS' in os.environ:
        import codecs
        import re

        with codecs.open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'shared', 'traintastic.cmake')), 'r', 'utf-8') as file:
            version = re.findall(r'^set\(TRAINTASTIC_VERSION ([0-9\.]+)\)$', file.read(), re.MULTILINE)[0]

        if os.environ['GITHUB_REF_TYPE'] == 'branch':
            version += '-' + os.environ['CI_REF_NAME_SLUG'] + '-' + os.environ['GITHUB_RUN_NUMBER'] + '-' + os.environ['CI_SHA_SHORT']

        return version
    else:
        return None


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
