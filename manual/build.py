#!/usr/bin/env python3

import sys
import os
import codecs
import shutil
import mkdocs.config
import mkdocs.commands
import mkdocs.commands.build
import yaml
from luadoc import LuaDoc


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


class TraintasticHelp:

    version = ''
    language = 'en'
    _luadoc = None

    def __init__(self, base_dir: str):
        self.base_dir = os.path.abspath(base_dir)
        self.output_dir = os.path.join(self.base_dir, 'output')
        self._luadoc = LuaDoc(os.path.join(self.base_dir, '..'))

    def build(self) -> bool:
        shutil.rmtree(os.path.join(self.output_dir, self.language), ignore_errors=True)

        lua_dir = os.path.join(self.base_dir, 'docs', self.language, 'appendix', 'lua')
        #shutil.rmtree(lua_dir, ignore_errors=True)
        self._luadoc.build(lua_dir)

        self._write_mkdocs_yml()

        cfg = mkdocs.config.load_config(os.path.join(self.base_dir, 'config', self.language, 'mkdocs.yml'))
        cfg.plugins.on_startup(command='build', dirty=False)
        try:
            mkdocs.commands.build.build(cfg)
        finally:
            cfg.plugins.on_shutdown()

        return True

    def _write_mkdocs_yml(self):
        lua_libs = {}
        for _, lib in self._luadoc._libs.items():
            lua_libs[lib['lua_name']] = {self._luadoc._get_term(lib['name']): os.path.join('appendix', 'lua', lib['filename'])}

        lua_enums = [{'All': os.path.join('appendix', 'lua', LuaDoc.FILENAME_ENUM)}]
        for obj in self._luadoc._enums:
            lua_enums.append({self._luadoc._get_term(obj['name']): os.path.join('appendix', 'lua', obj['filename'])})

        lua_sets = [{'All': os.path.join('appendix', 'lua', LuaDoc.FILENAME_SET)}]
        for obj in self._luadoc._sets:
            lua_sets.append({self._luadoc._get_term(obj['name']): os.path.join('appendix', 'lua', obj['filename'])})

        lua_objects = [{'All': os.path.join('appendix', 'lua', LuaDoc.FILENAME_OBJECT)}]
        for obj in self._luadoc._objects:
            lua_objects.append({self._luadoc._get_term(obj['name']): os.path.join('appendix', 'lua', obj['filename'])})

        lua_ref = [
            {'Introduction': 'appendix/lua/index.md'},
            {'Lua basics': 'appendix/lua/basics.md'},
            {self._luadoc._get_term('globals:title'): os.path.join('appendix', 'lua', LuaDoc.FILENAME_GLOBALS)},
            {self._luadoc._get_term('pv:title'): os.path.join('appendix', 'lua', LuaDoc.FILENAME_PV)},
            lua_libs['class'],
            {self._luadoc._get_term('enum:title'): lua_enums},
            lua_libs['log'],
            lua_libs['math'],
            {self._luadoc._get_term('set:title'): lua_sets},
            lua_libs['string'],
            lua_libs['table'],
            {'Objects': lua_objects},
            {'Examples': os.path.join('appendix', 'lua', LuaDoc.FILENAME_EXAMPLES)},
            {'Index A-Z': os.path.join('appendix', 'lua', LuaDoc.FILENAME_INDEX_AZ)},
        ]

        config = {
            'site_name': 'Traintastic manual',
            'docs_dir': os.path.join('../../docs', self.language),
            'extra_css': ['assets/extra.css'],
            'site_dir': os.path.join(self.output_dir, self.language),
            'use_directory_urls': False,
            'theme': {
                'name': 'material',
                'custom_dir': '../../overrides/',
                'language': self.language,
                'palette': [
                    {
                        'scheme': 'default',
                        'toggle': {
                            'icon': 'material/brightness-7',
                            'name': 'Switch to dark mode'
                        }
                    },
                    {
                        'scheme': 'slate',
                        'toggle': {
                            'icon': 'material/brightness-4',
                            'name': 'Switch to light mode'
                        }
                    }
                ],
                'features': ['content.code.copy']
            },
            'markdown_extensions': [
                'attr_list',
                'admonition',
                {
                    'pymdownx.highlight': {
                        'anchor_linenums': True,
                        'line_spans': '__span',
                        'pygments_lang_class': True
                    }
                },
                'pymdownx.inlinehilite',
                'pymdownx.snippets',
                'pymdownx.superfences'
            ],
            'plugins': {
                'search': {
                    'lang': self.language
                }
            },
            'nav': [
                {'Welcome': 'index.md'},
                {'Getting started': [
                    {'Installation': [
                        { 'Windows': 'installation/windows.md' },
                        { 'Linux': 'installation/linux.md' }
                    ]},
                    {'Quick start': [
                        {'Introduction': 'quickstart/index.md'},
                        {'Create your first world': 'quickstart/world.md'},
                        {'Connect to your command station': 'quickstart/command-station.md'}
                        # TODO: Draw board
                        # TODO: setup turnouts
                    ]}
                ]},
                {'Advanced topics': [
                    {'Interface configuration': [
                        {'Introduction': 'advanced/interface/index.md'},
                        {'DCC-EX': 'advanced/interface/dcc-ex.md'},
                        {'ECoS': 'advanced/interface/ecos.md'},
                        {'HSI-88': 'advanced/interface/hsi-88.md'},
                        {'LocoNet': 'advanced/interface/loconet.md'},
                        {'Märklin CAN': 'advanced/interface/marklin-can.md'},
                        {'Traintastic DIY': 'advanced/interface/traintastic-diy.md'},
                        {'WiThrottle': 'advanced/interface/withrottle.md'},
                        {'WLANmaus': 'advanced/interface/wlanmaus.md'},
                        {'XpressNet': 'advanced/interface/xpressnet.md'},
                        {'Z21': 'advanced/interface/z21.md'}
                    ]},
                    {'Scripting basics': 'advanced/scripting-basics.md'}
                ]},
                {'Troubleshooting': [
                    {'Interface connection errors': 'troubleshooting/interface-connection-errors.md'}
                ]},
                {'Appendix': [
                    {'Supported hardware': 'appendix/supported-hardware.md'},
                    {'LocoNet reference': 'appendix/loconet.md'},
                    {'XpressNet reference': 'appendix/xpressnet.md'},
                    {'Lua scripting reference': lua_ref},
                    {'Traintastic DIY protocol': 'appendix/traintastic-diy-protocol.md'},
                    {'Command line options': 'appendix/command-line-options.md'}
                ]},
                {'Uncategorized/WIP': [
                    {'Decoder function': 'wip/decoder-function.md'},
                    {'Input monitor': 'wip/input-monitor.md'},
                    {'Log messages': 'wip/log-messages.md'},
                    {'Board tiles': 'wip/board-tiles.md'},
                    {'Trains': 'wip/trains.md'},
                    {'Zones': 'wip/zones.md'}
                ]}
            ]
        }
        TraintasticHelp._write_file(os.path.join(self.base_dir, 'config', self.language, 'mkdocs.yml'), yaml.dump(config))

    def _write_file(filename: str, contents: str) -> None:
        os.makedirs(os.path.dirname(filename), mode=0o755, exist_ok=True)
        with codecs.open(filename, 'w', 'utf-8') as f:
            f.write(contents)


if __name__ == '__main__':
    from argparse import ArgumentParser

    # Standard options:
    parser = ArgumentParser()
    parser.add_argument('--version', default=detect_version())

    args = parser.parse_args(sys.argv[1:])

    help = TraintasticHelp(os.path.abspath(os.path.dirname(__file__)))
    help.version = args.version

    sys.exit(0 if help.build() else 1)
