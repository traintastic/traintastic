import os
import re
import datetime
from .html import HTMLBuilder


class HTMLSinglePageBuilder(HTMLBuilder):
    """Single page HTML manual builder for Traintastic"""

    def __init__(self, args):
        if args.output_dir is None:
            args.output_dir = os.path.join(args.base_dir, 'build', 'html-single-page')
        super().__init__(args)
        self._md2id = {}
        self._version = args.version

    def _file_to_html(self, page):
        html = HTMLBuilder._file_to_html(self, page)

        basedir = os.path.dirname(os.path.join(self._language_dir, page['markdown']))

        # find id of <h1>
        m = re.search(r'<h1[^>]+id="([^"]+)"[^>]*>', html)
        if m is not None:
            self._md2id[os.path.normpath(page['markdown'])] = '#' + m.group(1)

        # correct href to md files:
        html = re.sub(r'href="([^"]+\.md)"', lambda m: 'href="' + os.path.relpath(os.path.abspath(os.path.join(basedir, m.group(1))), self._language_dir) + '"', html)

        # correct image src and copy image
        html = re.sub(r'(<img[^>]+src=")([^"]+)(")',
            lambda m:
                m.group(1) +
                self._output_copy_image(os.path.join(basedir, m.group(2))) +
                m.group(3), html)

        return html

    def subpages(self, page, depth=1):
        html = ''
        if 'pages' in page:
            for subpage in page['pages']:
                subhtml = self._file_to_html(subpage)
                subhtml = re.sub(r'<h([1-5])([^>]*)>(.*?)</h\1>', lambda m: '<h' + str(min(6, int(m.group(1)) + depth)) + m.group(2) + '>' + m.group(3) + '</h' + str(min(6, int(m.group(1))) + depth) + '>', subhtml)
                html += subhtml + self.subpages(subpage, depth + 1)
        return html

    def build(self):
        self._output_copy_files([
            'css/pure-min.css',
            'css/traintasticmanual.css',
            'js/traintasticmanual.js'])

        title = 'Traintastic manual'
        toc = {'preface': [], 'chapter': [], 'appendix': []}
        manual_html = ''
        for page in self._json:
            page_html = self._file_to_html(page) + self.subpages(page)

            m = re.findall(r'<h([1-2])([^>]*)>(.*?)</h\1>', page_html)
            if m is not None:
                toc_item = {'children': []}
                for g in m:
                    id = re.search(r'id="([a-z0-9-]+)"', g[1])
                    if id is None:
                        raise Exception('header `' + g[2] + '` has no id')
                    id = id.group(1)

                    if int(g[0]) == 1:
                        toc_item['title'] = g[2]
                        toc_item['id'] = id
                    elif int(g[0]) == 2:
                        toc_item['children'].append({'title': g[2], 'id': id})
                toc[page['type']].append(toc_item)

            manual_html += page_html

        html = '''<!doctype html>
<html lang="''' + self._language + '''">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>''' + title + '''</title>
  <link rel="stylesheet" href="./css/pure-min.css">
  <link rel="stylesheet" href="./css/traintasticmanual.css">
</head>
<body>
  <div id="layout">
    <div id="toc" class="toc toc-left">
      <span class="title" href="#">Index</span>
      <ul>
'''

        for item in toc['preface']:
            html += '            <li><a href="#' + item['id'] + '">' + item['title'] + '</a></li>' + os.linesep

        html += '''
      </ul>
      <ul>
'''

        for item in toc['chapter']:
            html += '            <li><a href="#' + item['id'] + '">' + item['title'] + '</a>'
            if len(item['children']) > 0:
                html += '<ul>' + os.linesep
                for subitem in item['children']:
                    html += '              <li><a href="#' + subitem['id'] + '">' + subitem['title'] + '</a></li>' + os.linesep
                html += '</ul>'
            html += '</li>' + os.linesep

        html += '''
      </ul>
      <ul>
        <li>Appendix</li><ul>
'''

        for item in toc['appendix']:
            html += '            <li><a href="#' + item['id'] + '">' + item['title'] + '</a></li>' + os.linesep

        html += '''
        </ul>
      </ul>
    </div>
    <div id="main">
      <div class="content">
        <h1 class="title">''' + title + '''</h1>'''

        if self._version is not None:
          html += '''
        <p class="center large">
          v''' + self._version + '''
        </p>'''

        html += '''
        <p class="center dim small">
          Build ''' + datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S') + '''
        </p>
''' + manual_html + '''
      </div>
    </div>
  </div>
  <script src="./js/traintasticmanual.js"></script>
</body>
</html>'''

        # replace .md links by #id:
        html = re.sub(r'href="([^"]+\.md)"', lambda m: 'href="' + self._md2id[m.group(1)] + '"', html)

        # replace .md#id links by #id:
        html = re.sub(r'href="[^"]+\.md(#[^"]+)"', lambda m: 'href="' + m.group(1) + '"', html)

        self._output_text_file(self._language + '.html', html)
