import os

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
