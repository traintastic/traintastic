import os
import re
import codecs
import pycmarkgfm  # pip3 install pycmarkgfm
from .builder import Builder


class HTMLBuilder(Builder):
    """Traintastic HTML manual builder base class"""

    def _file_to_html(self, page):
        with codecs.open(os.path.join(self._language_dir, page['markdown']), 'r', 'utf-8') as md:
            html = pycmarkgfm.gfm_to_html(md.read())

        # parse id
        html = re.sub(r'<h([1-6])([^>]*)>([^>]*) {#([a-z0-9-]+)}</h\1>', r'<h\1\2 id="\4">\3</h\1>', html)

        # replace -> by
        html = re.sub(r'-&gt;', '\u2794', html)

        # set target="_blank" for external links:
        html = re.sub(r'<a([^>]+href="http(s|)://)', r'<a target="_blank"\1', html)

        # change img title attribute to figcaption
        html = re.sub(r'(<img[^>]+)title="([^">]*)"([^>]*>)',
            lambda m:
                '<figure>' +
                m.group(1) + m.group(3) +
                '<figcaption>' + m.group(2) + '</figcaption></figure>',
                html)

        return html
