import os
import re
import codecs
import cmarkgfm  # pip3 install cmarkgfm
from .builder import Builder


class HTMLBuilder(Builder):
    """Traintastic HTML manual builder base class"""

    def _file_to_html(self, page):
        with codecs.open(os.path.join(self._language_dir, page['markdown']), 'r', 'utf-8') as md:
            html = cmarkgfm.github_flavored_markdown_to_html(md.read(), options=cmarkgfm.Options.CMARK_OPT_UNSAFE)

        # parse id
        html = re.sub(r'<h([1-6])([^>]*)>(.*) {#([a-z0-9-]+)}</h\1>', r'<h\1\2 id="\4">\3</h\1>', html)

        # replace -> by
        html = re.sub(r'-&gt;', '\u2794', html)

        # set target="_blank" for external links:
        html = re.sub(r'<a([^>]+href="http(s|)://)', r'<a target="_blank"\1', html)

        # lua
        html = re.sub(r'(<pre lang="lua"><code>)(.+)(</code></pre>)', self._highlight_lua, html, flags=re.DOTALL)

        # change img title attribute to figcaption
        html = re.sub(r'(<img[^>]+)title="([^">]*)"([^>]*>)',
            lambda m:
                '<figure>' +
                m.group(1) + m.group(3) +
                '<figcaption>' + m.group(2) + '</figcaption></figure>',
                html)

        # handle badges
        html = html.replace('$badge:lua$', '<span class="badge badge-lua">Lua</span>')
        html = re.sub(r'\$badge:since:v([0-9]+\.[0-9]+(|\.[0-9]+))\$', r'<span class="badge badge-since">&ge; \1\2</span>', html)

        # handle notes
        html = html.replace('<p>Note: ', '<p class="note"><span class="label">Note</span> ')

        return html

    def _highlight_replace(self, code, css_class):
        return '<span class="' + css_class + '">' + re.sub(r'<span[^>]*>(.+?)</span>', r'\1', code) + '</span>'

    def _highlight_lua(self, m):
        code = m.group(2)
        code = re.sub(r'\b(math|table|string|class|enum|set|log|world)\b', r'<span class="global">\1</span>', code)  # globals
        code = re.sub(r'\b([A-Z_][A-Z0-9_]*)\b', r'<span class="const">\1</span>', code)  # CONSTANTS
        code = re.sub(r'\b(and|break|do|else|elseif|end|false|for|function|goto|if|in|local|nil|not|or|repeat|return|then|true|until|while)\b', r'<span class="keyword">\1</span>', code)  # keywords
        code = re.sub(r'(\a.*?[^\\]\a|\a\a)', lambda m: self._highlight_replace(m.group(1), 'text'), code)
        code = re.sub(r"('.*?[^\\]'|'')", lambda m: self._highlight_replace(m.group(1), 'text'), code)
        code = re.sub(r'(--.*)$', lambda m: self._highlight_replace(m.group(1), 'comment'), code, flags=re.MULTILINE)  # single line comments
        return m.group(1) + code + m.group(3)
