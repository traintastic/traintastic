import os
import re

def detect_version():
    if 'GITHUB_ACTIONS' in os.environ:
        import codecs
        import re

        with codecs.open(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', 'shared', 'traintastic.cmake')), 'r', 'utf-8') as file:
            version = re.findall(r'^set\(TRAINTASTIC_VERSION ([0-9\.]+)\)$', file.read(), re.MULTILINE)[0]

        if os.environ['GITHUB_REF_TYPE'] == 'branch':
            version += '-' + os.environ['CI_REF_NAME_SLUG'] + '-' + os.environ['GITHUB_RUN_NUMBER'] + '-' + os.environ['CI_SHA_SHORT']

        return version
    else:
        return None


def highlight_replace(code: str, css_class: str, clickable_links: bool = False) -> str:
    code = re.sub(r'<span[^>]*>(.+?)</span>', r'\1', code)  # remove other highlighting
    if clickable_links:
        code = re.sub(r'(http(s|)://[^ \n\r]+)', r'<a href="\1">\1</a>', code)
    return '<span class="' + css_class + '">' + code + '</span>'


def highlight_lua(code: str) -> str:
    code = re.sub(r'\b(math|table|string|class|enum|set|log|world)\b', r'<span class="global">\1</span>', code)  # globals
    code = re.sub(r'\b([A-Z_][A-Z0-9_]*)\b', r'<span class="const">\1</span>', code)  # CONSTANTS
    code = re.sub(r'\b(and|break|do|else|elseif|end|false|for|function|goto|if|in|local|nil|not|or|repeat|return|then|true|until|while)\b', r'<span class="keyword">\1</span>', code)  # keywords
    code = re.sub(r'\b((|-|\+)[0-9]+(\\.[0-9]*|)((e|E)(|-|\+)[0-9]+|))\b', r'<span class="number">\1</span>', code)  # numbers: infloat, decimal
    code = re.sub(r'\b((|-|\+)0(x|X)([0-9a-fA-F]*\.[0-9a-fA-F]*|[0-9a-fA-F]+)((p|P)(|-|\+)[0-9a-fA-F]+|))\b', r'<span class="number">\1</span>', code)  # numbers: float, hex
    code = re.sub(r'\b([a-zA-Z_][a-zA-Z0-9_]*)(?=\()', r'<span class="function">\1</span>', code)  # functions
    code = re.sub(r'(\a.*?[^\\]\a|\a\a)', lambda m: highlight_replace(m.group(1), 'text'), code)
    code = re.sub(r"('.*?[^\\]'|'')", lambda m: highlight_replace(m.group(1), 'text'), code)
    code = re.sub(r'(--.*)$', lambda m: highlight_replace(m.group(1), 'comment', clickable_links=True), code, flags=re.MULTILINE)  # single line comments
    code = re.sub(r'(?<!-)(--\[\[.+?\]\])', lambda m: highlight_replace(m.group(1), 'comment', clickable_links=True), code, flags=re.DOTALL)  # multi line comments
    return code
