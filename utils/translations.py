import sys
import os
import json
from collections import OrderedDict
from operator import itemgetter

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
POEDITOR_PROJECT_ID = 622757
LANGUAGES = [
    'en-us',
    'de-de',
    'es-es',
    'fr-fr',
    'fy-nl',
    'it-it',
    'nl-nl',
    'pl-pl',
    'sv-se',
]


def poeditor_language_code(code: str) -> str:
    return code if code.startswith('en') else code[:2]


def innosetup_isl_path(language: str) -> str:
    return os.path.join(PROJECT_ROOT, 'package', 'innosetup', language) + '.isl'


def read_innosetup_isl(language: str) -> dict:
    import configparser
    isl = configparser.ConfigParser()
    with open(innosetup_isl_path(language), 'r', encoding='utf-8-sig') as f:
        isl.read_file(f)
    return dict(isl['CustomMessages'])


def write_innosetup_isl(language: str, terms: dict) -> None:
    import configparser

    # remove empty
    terms = {k: v for k, v in terms.items() if v}

    isl = configparser.ConfigParser(delimiters=['='])
    isl['CustomMessages'] = OrderedDict(sorted(terms.items(), key=lambda t: t[0]))

    isl_file = innosetup_isl_path(language)
    with open(isl_file, 'w', encoding='utf-8-sig') as f:
        isl.write(f, space_around_delimiters=False)
    print('Wrote: {:s}'.format(isl_file))


def traintastic_json_path(language: str) -> str:
    return os.path.join(PROJECT_ROOT, 'shared', 'translations', language) + '.json'


def read_traintastic_terms(language: str) -> list:
    with open(traintastic_json_path(language), 'r', encoding='utf8') as f:
        return json.load(f)


def write_traintastic_json(language: str, terms: list) -> None:
    file = traintastic_json_path(language)

    # remove empty stuff
    for term in terms:
        for k in ['context', 'term_plural', 'reference', 'comment']:
            if k in term and term[k] == '':
                del term[k]
        if 'fuzzy' in term and term['fuzzy'] == 0:
            del term['fuzzy']

    terms = sorted(terms, key=itemgetter('term'))

    with open(file, 'w', encoding='utf8') as f:
        json.dump(terms, f, indent=4)
    print('Wrote: {:s}'.format(file))


def pull(args: list):
    from poeditor import POEditorAPI

    api = POEditorAPI(api_token=os.environ['POEDITOR_TOKEN'])

    if len(args) > 0:
        languages = args
    else:
        languages = LANGUAGES

    for language in languages:
        url, filename = api.export(
            project_id=POEDITOR_PROJECT_ID,
            language_code=poeditor_language_code(language),
            file_type='json',
            local_file=None)

        with open(filename, 'r', encoding='utf-8-sig') as f:
            terms = json.load(f)

        innosetup_terms = {}
        traintastic_terms = []
        for item in terms:
            if item['term'].startswith('innosetup:'):
                innosetup_terms[item['term'].removeprefix('innosetup:')] = item['definition']
            else:
                traintastic_terms.append(item)

        write_traintastic_json(language, traintastic_terms)
        write_innosetup_isl(language, innosetup_terms)

        os.unlink(filename)

    return 0


def push(args: list):
    import tempfile
    import time
    from poeditor import POEditorAPI

    api = POEditorAPI(api_token=os.environ['POEDITOR_TOKEN'])

    if len(args) > 0:
        languages = args
    else:
        languages = LANGUAGES

    wait = False
    for language in languages:
        if wait:
            print('API rate limit, sleeping...')
            time.sleep(30)  # API has rate limit

        fd, tmp_file = tempfile.mkstemp(suffix='.json')
        try:
            terms = read_traintastic_terms(language)
            for k, v in read_innosetup_isl(language).items():
                terms.append({'term': 'innosetup:' + k, 'definition': v})

            with os.fdopen(fd, 'w', encoding='utf8') as f:
                json.dump(terms, f, indent=4)

            print('Updating: {:s}'.format(language))
            api.update_terms_translations(
                project_id=POEDITOR_PROJECT_ID,
                language_code=poeditor_language_code(language),
                file_path=tmp_file,
                overwrite=True,
                sync_terms=(language == 'en-us'))
        finally:
            os.unlink(tmp_file)

        wait = True

    return 0


def add_missing(args: list):
    if len(args) != 2:
        print('Usage: add-missing <src alng> <dst_lang>', file=sys.stderr)
        sys.exit(1)

    dst_lang = args[1]
    dst = read_traintastic_terms(dst_lang)
    dst = [v for v in dst if v['definition'] != '']  # remove empty (= unstranslated)
    dst_terms = [v['term'] for v in dst]
    n = 0
    for v in read_traintastic_terms(args[0]):
        if v['term'] not in dst_terms:
            dst.append(v)
            n += 1
    print('Added {:d} missing terms'.format(n))
    write_traintastic_json(dst_lang, dst)


def sort(args: list):
    if len(args) > 0:
        languages = args
    else:
        languages = LANGUAGES

    for language in languages:
        write_traintastic_json(language, read_traintastic_terms(language))


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: {:s} <command> [languages...]".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    cmd = sys.argv[1].replace('-', '_')
    for sub_command in [pull, push, add_missing, sort]:
        if cmd == sub_command.__name__:
            sys.exit(sub_command(sys.argv[2:]))

    print("Unknown sub command: {:s}".format(sys.argv[1]), file=sys.stderr)
    sys.exit(1)
