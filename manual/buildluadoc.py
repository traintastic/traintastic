#!/usr/bin/env python3

import sys
import string
import os
import codecs
import re
import json
import operator
import shutil
import datetime
from traintasticmanualbuilder.utils import highlight_lua


class LuaDoc:
    """
    Documentation genrator for Traintastic's builtin Lua scripting language
    """

    DEFAULT_LANGUAGE = 'en-us'
    FILENAME_INDEX = 'index.html'
    FILENAME_GLOBALS = 'globals.html'
    FILENAME_ENUM = 'enum.html'
    FILENAME_SET = 'set.html'
    FILENAME_OBJECT = 'object.html'
    FILENAME_EXAMPLE = 'example.html'
    FILENAME_INDEX_AZ = 'index-az.html'

    missing_terms = []
    version = None

    def __init__(self, project_root: str) -> None:
        self._globals = LuaDoc._find_globals(project_root)
        self._enums = LuaDoc._find_enums(project_root)
        self._sets = LuaDoc._find_sets(project_root)
        self._libs = LuaDoc._find_libs(project_root)
        self._objects = LuaDoc._find_objects(project_root)
        self._examples = LuaDoc._find_examples(project_root)
        self._add_cross_references()
        self.set_language(LuaDoc.DEFAULT_LANGUAGE)

    def set_language(self, language: str) -> None:
        self._language = language
        self._terms = LuaDoc._load_terms(language)
        self.missing_terms = []

    def _get_term(self, term: str) -> str:
        if term not in self._terms:
            if term not in self.missing_terms:
                self.missing_terms.append(term)
            return '<span style="color:red">$' + term + '$</span>'

        definition = self._terms[term]
        definition = re.sub(r'`(.+?)`', r'<code>\1</code>', definition)
        definition = re.sub(r'\[([^\]]+)]\(([^\)]+)\)', r'<a href="\2">\1</a>', definition)
        definition = re.sub(r'{ref:([a-z0-9_\.]+?)(|#[a-z0-9_]+)(|\|.+?)}', self._ref_link, definition)
        return definition

    def _load_terms(language: str) -> dict:
        terms = {}
        for item in json.loads(LuaDoc._read_file(os.path.join(os.path.dirname(__file__), 'luadoc', 'terms', language + '.json'))):
            if item['definition'] is not None and item['definition'] != '':
                terms[item['term']] = item['definition']
        return terms

    def _add_cross_references(self):
        for object in self._objects:
            for item in object['items']:
                cpp_types = []
                if item['type'] == 'property' and 'cpp_template_type' in item:
                    cpp_types = [item['cpp_template_type']]
                elif item['type'] == 'method' and 'cpp_template_type' in item:
                    [result, args] = item['cpp_template_type'].rstrip(')').split('(')
                    cpp_types = [result] + [s.strip() for s in args.split(',')]
                elif item['type'] == 'event':
                    cpp_types = [s.strip() for s in item['cpp_template_type'].split(',')]

                for cpp_type in cpp_types:
                    for enum in self._enums:
                        if enum['cpp_name'] == cpp_type:
                            enum['see_also'].append('<a href="' + object['filename'] + '#' + item['lua_name'] + '"><code>' + object['lua_name'] + '.' + item['lua_name'] + '</code></a>')
                    for set in self._sets:
                        if set['cpp_name'] == cpp_type:
                            set['see_also'].append('<a href="' + object['filename'] + '#' + item['lua_name'] + '"><code>' + object['lua_name'] + '.' + item['lua_name'] + '</code></a>')

    def _ref_link(self, m: re.Match) -> str:
        id = m.group(1)
        fragment = m.group(2)
        title = m.group(3).lstrip('|')
        if id.startswith('enum.'):
            id = id.removeprefix('enum.')
            for enum in self._enums:
                if enum['lua_name'] == id:
                    return '<a href="' + enum['filename'] + fragment + '">' + (self._get_term(enum['name']) if title == '' else title) + '</a>'
        elif id.startswith('set.'):
            id = id.removeprefix('set.')
            for set in self._sets:
                if set['lua_name'] == id:
                    return '<a href="' + set['filename'] + fragment + '">' + (self._get_term(set['name']) if title == '' else title) + '</a>'
        elif id.startswith('object.'):
            for object in self._objects:
                if object['lua_name'] == id:
                    return '<a href="' + object['filename'] + fragment + '">' + (self._get_term(object['name']) if title == '' else title) + '</a>'

        return '<span style="color:red">' + m.group(0) + '</span>'

    def _load_data(items: list, filename: str) -> list:
        data = json.loads(LuaDoc._read_file(filename))

        new_items = []
        for item in items:
            if isinstance(item, str):
                item = {'lua_name': item}

            if item['lua_name'] not in data:
                raise RuntimeError('"{:s}" missing in {:s}'.format(item['lua_name'], filename))

            if 'parameters' in item:
                params_in_data = len(data[item['lua_name']]['parameters']) if 'parameters' in data[item['lua_name']] else 0
                if len(item['parameters']) != params_in_data:
                    raise RuntimeError('"{:s}" invalid number of parameters, expected {:d}, got {:d} in {:s}'.format(
                        item['lua_name'], len(item['parameters']), params_in_data, filename))

            item.update(data[item['lua_name']])

            if 'parameters' in item and len(item['parameters']) != 0:
                for i in range(len(item['parameters'])):
                    if 'name' not in item['parameters'][i]:
                        raise RuntimeError('name missing for parameter {:d} of {:s} in {:s}'.format(1 + i, item['lua_name'], filename))

            new_items.append(item)

        return new_items

    def _read_file(filename: str) -> str:
        with codecs.open(filename, 'r', 'utf-8') as f:
            return f.read()

    def _write_file(filename: str, contents: str) -> None:
        os.makedirs(os.path.dirname(filename), mode=0o755, exist_ok=True)
        with codecs.open(filename, 'w', 'utf-8') as f:
            f.write(contents)

    def _copy_file(src: str, dst: str) -> None:
        os.makedirs(dst, mode=0o755, exist_ok=True)
        shutil.copyfile(src, os.path.join(dst, os.path.basename(src)))

    def _find_globals(project_root: str) -> dict:
        globals = []

        sandbox_cpp = LuaDoc._read_file(os.path.join(project_root, 'server', 'src', 'lua', 'sandbox.cpp'))

        # lua base lib:
        for args in re.findall(r'addBaseLib\(\s*L\s*,[^{]*{(.+?)}\);', sandbox_cpp, flags=re.DOTALL):
            for name in re.findall(r'"([a-z0-9_]+)"', args):
                globals.append(name)

        # lua libs:
        globals += re.findall(r'addLib\(\s*L\s*,\s*LUA_[A-Z]+\s*,\s*luaopen_([a-z]+)\s*,', sandbox_cpp)

        # setfield:
        for name in re.findall(r'lua_setfield\(\s*L\s*,\s*-2\s*,\s*"([A-Za-z][A-Za-z_]*)"\s*\);', sandbox_cpp):
            globals.append(name)

        globals = LuaDoc._load_data(globals, os.path.join(os.path.dirname(__file__), 'luadoc', 'globals.json'))

        return globals

    def _find_enums(project_root: str) -> list:
        enums_hpp = LuaDoc._read_file(os.path.join(project_root, 'server', 'src', 'lua', 'enums.hpp'))
        m = re.findall(r'#define LUA_ENUMS([ \nA-Za-z0-9_,\\]+)\n\n', enums_hpp)
        m = re.sub(r'[ \\\n]+', '', m[0])
        enums = []
        for cpp_name in m.split(','):
            filename = os.path.join(project_root, 'shared', 'src', 'traintastic', 'enum', cpp_name.lower() + '.hpp')
            hpp = LuaDoc._read_file(filename)
            enum = re.search(r'enum class ' + cpp_name + r'[ ]*(:[ ]*[A-Za-z0-9_]+|)[ \n]*{(.+?)};', hpp, re.DOTALL)
            items = [{'cpp_name': m[0], 'description': m[3], 'type': 'constant'} for m in re.findall(r'^[ ]*([A-Za-z0-9_]+)[ ]*=[ ]*.+?[ ]*(,|)[ ]*(//!< (.+)|)\n', enum.group(2), flags=re.MULTILINE)] if enum is not None else None
            info = re.search(r'TRAINTASTIC_ENUM\([ ]*' + cpp_name + r'[ \n]*,[ \n]*"([a-z0-9_]+)"[ \n]*,[ \n]*\d+[ \n]*,[ \n]*{(.+?)}[ \n]*\);', hpp, re.DOTALL)

            if info is None:
                raise RuntimeError('Reading enum info failed for {:s}'.format(filename))

            for item in items:
                m = re.search(cpp_name + '::' + item['cpp_name'] + r'[ ]*,[ ]*"([A-Za-z0-9_]+)"', info.group(2))
                if m is not None:
                    item['lua_name'] = m.group(1).upper()

            items = filter(lambda item: 'lua_name' in item, items)

            enums.append({
                'filename': 'enum.' + info.group(1).lower() + '.html',
                'name': 'enum.' + info.group(1).lower() + ':title',
                'cpp_name': cpp_name,
                'lua_name': info.group(1),
                'items': items,
                'see_also': []
                })

        return enums

    def _find_sets(project_root: str) -> list:
        sets_hpp = LuaDoc._read_file(os.path.join(project_root, 'server', 'src', 'lua', 'sets.hpp'))
        m = re.findall(r'#define LUA_SETS([ \nA-Za-z0-9_,\\]+)\n\n', sets_hpp)
        m = re.sub(r'[ \\\n]+', '', m[0])
        sets = []
        for cpp_name in m.split(','):
            filename = os.path.join(project_root, 'shared', 'src', 'traintastic', 'set', cpp_name.lower() + '.hpp')
            hpp = LuaDoc._read_file(filename)
            set = re.search(r'enum class ' + cpp_name + r'[ ]*(:[ ]*[A-Za-z0-9_]+|)[ \n]*{(.+?)};', hpp, re.DOTALL)
            items = [{'cpp_name': m[0], 'description': m[3], 'type': 'constant'} for m in re.findall(r'\b([A-Za-z0-9_]+)[ ]*=[ ]*.+?[ ]*(,|)[ ]*(//!< (.+)|)\n', set.group(2))] if set is not None else None
            info = re.search(r'TRAINTASTIC_SET\([ ]*' + cpp_name + r'[ \n]*,[ \n]*"([a-z0-9_]+)"[ \n]*,[ \n]*\d[ \n]*,[ \n]*(\([^,]+?\))[ \n]*,[ \n]*{(.+?)}[ \n]*\);', hpp, re.DOTALL)

            if info is None:
                raise RuntimeError('Reading set info failed for {:s}'.format(filename))

            for item in items:
                m = re.search(cpp_name + '::' + item['cpp_name'] + r'[ ]*,[ ]*"([A-Za-z0-9_]+)"', info.group(3))
                if m is not None:
                    item['lua_name'] = m.group(1).upper()

            sets.append({
                'filename': 'set.' + info.group(1).lower() + '.html',
                'name': 'set.' + info.group(1).lower() + ':title',
                'cpp_name': cpp_name,
                'lua_name': info.group(1),
                'items': items,
                'see_also': []
                })

        return sets

    def _find_libs(project_root: str) -> dict:
        libs = {}

        # lua libs:
        sandbox_cpp = LuaDoc._read_file(os.path.join(project_root, 'server', 'src', 'lua', 'sandbox.cpp'))
        for name, args in re.findall(r'addLib\(\s*L\s*,\s*LUA_[A-Z]+\s*,\s*luaopen_([a-z]+)\s*,\s*{(.+?)}\);', sandbox_cpp, flags=re.DOTALL):
            items = []
            for item_name in re.findall(r'"([a-z0-9_]+)"', args):
                items.append(item_name)
            libs[name] = {
                'filename': name + '.html',
                'name': name + ':title',
                'lua_name': name,
                'items': items}

        # log lib:
        name = 'log'
        items = []
        log_hpp = LuaDoc._read_file(os.path.join(project_root, 'server', 'src', 'lua', 'log.hpp'))
        for item_name in re.findall(r'static\s+int\s+([a-z]+)\(\s*lua_State\s*\*\s*L\s*\)', log_hpp):
            items.append(item_name)
        libs[name] = {
            'filename': name + '.html',
            'name': name + ':title',
            'lua_name': name,
            'items': items}

        # class lib:
        name = 'class'
        items = []
        class_hpp = LuaDoc._read_file(os.path.join(project_root, 'server', 'src', 'lua', 'class.hpp'))
        for item_name in re.findall(r'static\s+int\s+([a-zA-Z]+)\(\s*lua_State\s*\*\s*L\s*\)', class_hpp):
            if item_name == 'getClass':
                item_name = 'get'
            items.append(item_name)
        class_cpp = LuaDoc._read_file(os.path.join(project_root, 'server', 'src', 'lua', 'class.cpp'))
        for item_name in re.findall(r'registerValue<[a-zA-Z0-9]+>\(\s*L\s*,\s*"([A-Z_]+)"\s*\);', class_cpp):
            if item_name == 'getClass':
                item_name = 'get'
            items.append(item_name)
        libs[name] = {
            'filename': name + '.html',
            'name': name + ':title',
            'lua_name': name,
            'items': items}

        # load meta data:
        for name, lib in libs.items():
            filename = os.path.join(os.path.dirname(__file__), 'luadoc', name + '.json')
            lib_json = json.loads(LuaDoc._read_file(filename))

            items = []
            for item_name in lib['items']:
                if item_name not in lib_json:
                    raise RuntimeError('"{:s}" missing in {:s}'.format(item_name, filename))

                item = lib_json[item_name] if item_name in lib_json else {}
                item['lua_name'] = item_name
                items.append(item)
            lib['items'] = items

        return libs

    def _find_objects(project_root: str) -> dict:
        objects = []

        # index all cpp classes:
        cpp_classes = {}
        for root, dirs, files in os.walk(os.path.join(project_root, 'server', 'src')):
            for file in files:
                if not file.endswith('.hpp'):
                    continue
                filename_hpp = os.path.join(root, file)
                hpp = LuaDoc._read_file(filename_hpp)
                m = re.search(r'class\s*([A-Za-z0-9]+)\s*(final|)\s*(:[^;]+?|){', hpp, flags=re.DOTALL)
                if m is None:
                    continue
                base_classes = []
                for base_class, _ in re.findall(r'public\s*([A-Za-z0-9]+)(<[A-Za-z0-9]+>|)', m.group(3)):
                    if not base_class.startswith('std'):
                        base_classes.append(base_class)
                lua_filename_hpp = os.path.join(project_root, 'server', 'src', 'lua', 'object', os.path.basename(filename_hpp))
                lua_filename_cpp = os.path.splitext(lua_filename_hpp)[0] + '.cpp'
                cpp_classes[m.group(1)] = {
                    'filename_hpp': filename_hpp,
                    'lua_filename_hpp': lua_filename_hpp if os.path.exists(lua_filename_hpp) else None,
                    'lua_filename_cpp': lua_filename_cpp if os.path.exists(lua_filename_cpp) else None,
                    'base_classes': base_classes}

        # indentify those that can be used in Lua:
        class_cpp = LuaDoc._read_file(os.path.join(project_root, 'server', 'src', 'lua', 'class.cpp'))
        for cpp_name in re.findall(r'registerValue<([a-zA-Z0-9]+)>\(\s*L\s*,\s*"[A-Z0-9_]+"\s*\);', class_cpp):
            if cpp_name not in cpp_classes:
                raise RuntimeError('class {:s} not found'.format(cpp_name))

            lua_name = 'object.' + cpp_name.lower()
            items = LuaDoc._find_object_items(cpp_classes, cpp_name)

            objects.append({
                'filename': lua_name + '.html',
                'lua_name': lua_name,
                'name': lua_name + ':title',
                'cpp_name': cpp_name,
                'term_prefix': lua_name + '.',
                'items': items
            })

        return objects

    def _find_object_items(cpp_classes: dict, cpp_name: str) -> list:
        items = []
        cpp_class = cpp_classes[cpp_name]
        term_prefix = 'object.' + cpp_name.lower() + '.'
        filename_hpp = cpp_class['filename_hpp']
        filename_cpp = os.path.splitext(filename_hpp)[0] + '.cpp'
        hpp = LuaDoc._read_file(filename_hpp)
        cpp = LuaDoc._read_file(filename_cpp) if os.path.exists(filename_cpp) else hpp
        for cpp_type, cpp_template_type, cpp_item_name in re.findall(r'(Property|VectorProperty|ObjectProperty|ObjectVectorProperty|Method|Event)<(.*?)>\s+([A-Za-z0-9_]+);', hpp):
            m = re.search(cpp_item_name + r'({|\()\s*[\*]?this\s*,\s*"([a-z0-9_]+)"[^}]*(PropertyFlags::ScriptReadOnly|PropertyFlags::ScriptReadWrite|MethodFlags::ScriptCallable|EventFlags::Scriptable)[^}]*}', cpp)
            if m is None:
                continue

            item = {
                'cpp_name': cpp_item_name,
                'cpp_type': cpp_type,
                'cpp_template_type': cpp_template_type,
                'lua_name': m.group(2),
                'term_prefix': term_prefix
                }

            if cpp_type in ['Property', 'VectorProperty', 'ObjectProperty', 'ObjectVectorProperty']:
                item['type'] = 'property'
            elif cpp_type == 'Method':
                item['type'] = 'method'
                args = re.search(r'\((.*?)\)', cpp_template_type).group(1)
                item['parameters'] = [] if args == '' else [{}] * len(args.split(','))
                item['return_values'] = 0 if cpp_template_type.startswith('void') else 1
            elif cpp_type == 'Event':
                item['type'] = 'event'
                item['parameters'] = [{}] * (0 if cpp_template_type == '' else len(cpp_template_type.split(',')))

            items.append(item)

        # check for Lua wrapper:
        if cpp_class['lua_filename_hpp'] is not None:
            hpp = LuaDoc._read_file(cpp_class['lua_filename_hpp'])
            for method_name in re.findall(r'static\s+int\s+([a-z][a-z0-9_]*)\(\s*lua_State\s*\*\s*L\s*\)', hpp):
                item = {
                        'lua_name': method_name,
                        'term_prefix': term_prefix,
                        'type': 'method'
                        }
                items.append(item)

        if cpp_class['lua_filename_cpp'] is not None:
            cpp = LuaDoc._read_file(cpp_class['lua_filename_cpp'])
            for property_name in re.findall(r'LUA_OBJECT_PROPERTY\(([a-z][a-z0-9_]*)\)', cpp):
                item = {
                        'lua_name': property_name,
                        'term_prefix': term_prefix,
                        'type': 'property'
                        }
                items.append(item)

        item = LuaDoc._load_data(items, os.path.join(os.path.join(os.path.dirname(__file__), 'luadoc', 'object', cpp_name.lower() + '.json')))

        # get special items that aren't detected:
        items += LuaDoc._get_special_object_items(cpp_name, term_prefix)

        for cpp_base_class in cpp_class['base_classes']: # todo cache this
            items += LuaDoc._find_object_items(cpp_classes, cpp_base_class)

        return items

    def _find_examples(project_root: str) -> dict:
        examples = {}
        for root, dirs, files in os.walk(os.path.join(project_root, 'manual', 'luadoc', 'example')):
            for file in files:
                if not file.endswith('.lua'):
                    continue

                id = os.path.splitext(file)[0]
                examples[id] = {
                    'id': id,
                    'name': 'example.' + id + ':title',
                    'filename': 'example.' + id + '.html',
                    'code': LuaDoc._read_file(os.path.join(root,file))}

        return examples

    def _get_special_object_items(cpp_name: str, term_prefix: str) -> list:
        items = []
        if cpp_name == 'ObjectList':
            items.append({
                'lua_name': '__get',
                'type': 'method',
                'term_prefix': term_prefix,
                'parameters': [{'name': 'index'}],
                'return_values': 1,
                })
        return items

    def build(self, output_dir: str) -> None:
        # reset missing terms
        self.missing_terms = []

        # create output dir
        os.makedirs(args.output_dir, mode=0o755, exist_ok=True)

        # css
        LuaDoc._copy_file(os.path.join(os.path.dirname(__file__), 'traintasticmanual', 'css', 'pure-min.css'), os.path.join(output_dir, 'css'))
        LuaDoc._copy_file(os.path.join(os.path.dirname(__file__), 'traintasticmanual', 'css', 'traintasticmanual.css'), os.path.join(output_dir, 'css'))

        nav = [{'title': self._get_term('index:nav'), 'href': LuaDoc.FILENAME_INDEX}]

        self._build_index(output_dir)
        self._build_globals(output_dir, nav)
        self._build_enums(output_dir, nav)
        self._build_sets(output_dir, nav)
        for _, lib in self._libs.items():
            self._build_lib(output_dir, nav, lib)
        self._build_objects(output_dir, nav)
        self._build_examples(output_dir, nav)
        self._build_index_az(output_dir, nav)

    def _build_items_html(self, items: list, term_prefix: str, lua_prefix: str = '') -> str:
        html = ''
        items = sorted(items, key=operator.itemgetter('lua_name'))

        constants = [item for item in items if item['type'] == 'constant']
        if len(constants) > 0:
            html += '<h2 id="constants">' + self._get_term('constants') + '</h2>' + os.linesep
            html += '<dl>' + os.linesep
            for item in constants:
                item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix
                html += '  <dt id="' + item['lua_name'] + '"><code>' + lua_prefix + item['lua_name'] + '</code>'
                if 'since' in item:
                    html += ' <span class="badge badge-since">&ge; ' + item['since'] + '</span>'
                if 'is_lua_builtin' in item and item['is_lua_builtin']:
                    html += ' <span class="badge badge-lua">Lua</span>'
                html += '</dt>' + os.linesep
                html += '  <dd>' + self._get_term(item_term_prefix + item['lua_name'].lower() + ':description') + '</dd>' + os.linesep
            html += '</dl>' + os.linesep

        libraries = [item for item in items if item['type'] == 'library']
        if len(libraries) > 0:
            html += '<h2 id="libraries">' + self._get_term('libraries') + '</h2>' + os.linesep
            html += '<dl>' + os.linesep
            for item in libraries:
                html += '  <dt id="' + item['lua_name'] + '"><code>' + lua_prefix + item['lua_name'] + '</code>'
                if 'since' in item:
                    html += ' <span class="badge badge-since">&ge; ' + item['since'] + '</span>'
                if 'is_lua_builtin' in item and item['is_lua_builtin']:
                    html += ' <span class="badge badge-lua">Lua</span>'
                html += '</dt>' + os.linesep
                if item['lua_name'] == 'enum':
                    href = LuaDoc.FILENAME_ENUM
                elif item['lua_name'] == 'set':
                    href = LuaDoc.FILENAME_SET
                else:
                    href = self._libs[item['lua_name']]['filename']
                html += '  <dd><a href="' + href + '">' + self._get_term(item['lua_name'] + ':title') + '</a></dd>' + os.linesep
            html += '</dl>' + os.linesep

        objects = [item for item in items if item['type'] == 'object']
        if len(objects) > 0:
            html += '<h2 id="objects">' + self._get_term('objects') + '</h2>' + os.linesep
            html += '<dl>' + os.linesep
            for item in objects:
                item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix
                html += '  <dt id="' + item['lua_name'] + '"><code>' + lua_prefix + item['lua_name'] + '</code>'
                if 'since' in item:
                    html += ' <span class="badge badge-since">&ge; ' + item['since'] + '</span>'
                html += '</dt>' + os.linesep
                html += '  <dd>' + self._get_term(item_term_prefix + item['lua_name'].lower() + ':description') + '</dd>' + os.linesep
            html += '</dl>' + os.linesep

        properties = [item for item in items if item['type'] == 'property']
        if len(properties) > 0:
            html += '<h2 id="properties">' + self._get_term('properties') + '</h2>' + os.linesep
            html += '<dl>' + os.linesep
            for item in properties:
                item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix
                html += '  <dt id="' + item['lua_name'] + '"><code>' + lua_prefix + item['lua_name'] + '</code>'
                if 'since' in item:
                    html += ' <span class="badge badge-since">&ge; ' + item['since'] + '</span>'
                html += '</dt>' + os.linesep
                html += '  <dd>' + self._get_term(item_term_prefix + item['lua_name'].lower() + ':description') + '</dd>' + os.linesep
            html += '</dl>' + os.linesep

        for function_or_method in ['function', 'method']:
            functions = [item for item in items if item['type'] == function_or_method]
            if len(functions) > 0:
                html += '<h2 id="' + function_or_method + 's">' + self._get_term(function_or_method + 's') + '</h2>'
                for item in functions:
                    item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix

                    html += '<h3 id="' + item['lua_name'] + '"><code>'
                    if item['lua_name'] == '__get':
                        html += '[]'
                    else:
                        html += item['lua_name']
                    if 'since' in item:
                        html += ' <span class="badge badge-since">&ge; ' + item['since'] + '</span>'
                    if 'is_lua_builtin' in item and item['is_lua_builtin']:
                        html += ' <span class="badge badge-lua">Lua</span>'
                    html += '</code></h3>' + os.linesep

                    html += '<code>' + lua_prefix
                    if item['lua_name'] == '__get':
                        html += '['
                    else:
                        html += item['lua_name'] + '('

                    optional = 0
                    for p in item['parameters']:
                        is_optional = 'optional' in p and p['optional']
                        is_first = p == item['parameters'][0]
                        if is_optional:
                            html += '[' if is_first else ' ['
                            optional += 1
                        if not is_first:
                            html += ', '
                        html += p['name']
                        if is_optional and 'default' in p:
                            html += ' = ' + str(p['default'])
                    html += ']' * optional
                    if item['lua_name'] == '__get':
                        html += ']'
                    else:
                        html += ')'
                    html += '</code>' + os.linesep

                    html += '<p>' + self._get_term(item_term_prefix + item['lua_name'].lower() + ':description') + '</p>' + os.linesep

                    if len(item['parameters']) > 0:
                        html += '<h4>' + self._get_term('parameters') + '</h4>' + os.linesep
                        html += '<dl>' + os.linesep
                        for param in item['parameters']:
                            html += '  <dt><code>' + param['name'] + '</code></dt>' + os.linesep
                            html += '  <dd>' + self._get_term(item_term_prefix + item['lua_name']  + '.parameter.' + param['name'] + ':description') + '</dd>' + os.linesep
                        html += '</dl>' + os.linesep

                    if item['return_values'] > 0:
                        html += '<h4>' + self._get_term('return_values') + '</h4>' + os.linesep
                        html += '<p>' + self._get_term(item_term_prefix + item['lua_name']  + ':return_values') + '</p>' + os.linesep

                    if 'examples' in item and len(item['examples']) != 0:
                        html += '<h4>' + self._get_term('example' if len(item['examples']) == 1 else 'examples') + '</h4>' + os.linesep
                        for example in item['examples']:
                            html += '<pre lang="lua"><code>' + highlight_lua(example['code']) + '</code></pre>' + os.linesep

        events = [item for item in items if item['type'] == 'event']
        if len(events) > 0:
            html += '<h2 id="events">' + self._get_term('events') + '</h2>'
            for item in events:
                item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix

                html += '<h3 id="' + item['lua_name'] + '"><code>' + item['lua_name'] + '</code>'
                if 'since' in item:
                    html += ' <span class="badge badge-since">&ge; ' + item['since'] + '</span>'
                html += '</h3>' + os.linesep
                html += '<p>' + self._get_term(term_prefix + item['lua_name'].lower() + ':description') + '</p>' + os.linesep

                html += 'Handler: <code>function ('
                for p in item['parameters']:
                    html += p['name'] + ', '
                html += 'user_data)</code>'

                html += '<dl>' + os.linesep
                for param in item['parameters']:
                    html += '  <dt><code>' + param['name'] + '</code></dt>' + os.linesep
                    html += '  <dd>' + self._get_term(item_term_prefix + item['lua_name']  + '.parameter.' + param['name'] + ':description') + '</dd>' + os.linesep
                html += '  <dt><code>user_data</code></dt>' + os.linesep
                html += '  <dd>' + self._get_term('event.parameter.user_data:description') + '</dd>' + os.linesep
                html += '</dl>' + os.linesep

        return html

    def _build_see_also_html(self, items: list):
        if len(items) == 0:
            return ''
        html = '<h2>' + self._get_term('see_also') + '</h2><ul>'
        for item in items:
            html += '<li>' + item + '</li>'
        html += '</ul>'
        return html

    def _build_index(self, output_dir: str) -> None:
        html = self._get_header(self._get_term('index:title'), [])
        html = html.replace('<h1>', '<h1 class="title">')
        if self.version is not None:
          html += '<p class="center large">v' + self._version + '</p>'
        html += '<p class="center dim small">' + datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S') + '</p>'
        html += '<p>' + self._get_term('index:description') + '</p>' + os.linesep
        html += '<ul>' + os.linesep
        html += '  <li><a href="' + LuaDoc.FILENAME_GLOBALS + '">' + self._get_term('globals:title') + '</a></li>' + os.linesep
        for k in sorted(list(self._libs.keys()) + ['enum', 'set']):
            if k == 'enum':
                html += '  <li><a href="' + LuaDoc.FILENAME_ENUM + '">' + self._get_term('enum:title') + '</a></li>' + os.linesep
            elif k == 'set':
                html += '  <li><a href="' + LuaDoc.FILENAME_SET + '">' + self._get_term('set:title') + '</a></li>' + os.linesep
            else:
                lib = self._libs[k]
                html += '  <li><a href="' + lib['filename'] + '">' + self._get_term(lib['name']) + '</a></li>' + os.linesep
        html += '  <li><a href="' + LuaDoc.FILENAME_OBJECT + '">' + self._get_term('object:title') + '</a></li>' + os.linesep
        html += '  <li><a href="' + LuaDoc.FILENAME_EXAMPLE + '">' + self._get_term('example:title') + '</a></li>' + os.linesep
        html += '  <li><a href="' + LuaDoc.FILENAME_INDEX_AZ + '">' + self._get_term('index-az:title') + '</a></li>' + os.linesep
        html += '</ul>' + os.linesep
        html += '</div>' + os.linesep
        LuaDoc._write_file(os.path.join(output_dir, LuaDoc.FILENAME_INDEX), html)

    def _build_globals(self, output_dir: str, nav: list) -> None:
        title = self._get_term('globals:title')
        html = self._get_header(title, nav + [{'title': title, 'href': LuaDoc.FILENAME_GLOBALS}])
        html += '<p>' + self._get_term('globals:description') + '</p>' + os.linesep
        html += self._build_items_html(self._globals, 'globals.')
        LuaDoc._write_file(os.path.join(output_dir, LuaDoc.FILENAME_GLOBALS), self._add_toc(html))

    def _build_enums(self, output_dir: str, nav: list) -> None:
        title = self._get_term('enum:title')
        nav_enums = nav + [{'title': title, 'href': LuaDoc.FILENAME_ENUM}]
        html = self._get_header(title, nav_enums)
        html += '<p>' + self._get_term('enum:description') + '</p>' + os.linesep
        html += '<h2 id="constants">' + self._get_term('constants') + '</h2>' + os.linesep
        html += '<dl>' + os.linesep
        for enum in self._enums:
            html += '  <dt id="' + enum['lua_name'] + '"><a href="' + enum['filename'] + '"><code>enum.' + enum['lua_name'] + '</code></a></dt>' + os.linesep
            html += '  <dd>' + self._get_term(enum['name']) + '</dd>' + os.linesep
        html += '</dl>' + os.linesep
        html += self._get_footer()
        LuaDoc._write_file(os.path.join(output_dir, LuaDoc.FILENAME_ENUM), self._add_toc(html))

        for enum in self._enums:
            self._build_lib(output_dir, nav_enums, enum, 'enum.')

    def _build_sets(self, output_dir: str, nav: list) -> None:
        title = self._get_term('set:title')
        nav_sets = nav + [{'title': title, 'href': LuaDoc.FILENAME_SET}]
        html = self._get_header(title, nav_sets)
        html += '<p>' + self._get_term('set:description') + '</p>' + os.linesep
        html += '<h2 id="constants">' + self._get_term('constants') + '</h2>' + os.linesep
        html += '<dl>' + os.linesep
        for set in self._sets:
            html += '  <dt id="' + set['lua_name'] + '"><a href="' + set['filename'] + '"><code>' + set['lua_name'] + '</code></a></dt>' + os.linesep
            html += '  <dd>' + self._get_term(set['name']) + '</dd>' + os.linesep
        html += '</dl>' + os.linesep
        html += self._get_footer()
        LuaDoc._write_file(os.path.join(output_dir, LuaDoc.FILENAME_SET), self._add_toc(html))

        for set in self._sets:
            self._build_lib(output_dir, nav_sets, set, 'set.')

    def _build_lib(self, output_dir: str, nav: list, lib: dict, parent_lib: str = '') -> None:
        title = self._get_term(lib['name'])
        html = self._get_header(title, nav + [{'title': title, 'href': lib['filename']}])
        html += '<p>' + self._get_term(parent_lib + lib['lua_name'] + ':description') + '</p>' + os.linesep
        html += self._build_items_html(lib['items'], parent_lib + lib['lua_name'] + '.', parent_lib + lib['lua_name'] + '.')
        if 'see_also' in lib:
            html += self._build_see_also_html(lib['see_also'])
        LuaDoc._write_file(os.path.join(output_dir, lib['filename']), self._add_toc(html))

    def _build_objects(self, output_dir: str, nav: list) -> None:
        title = self._get_term('object:title')
        nav_objects = nav + [{'title': title, 'href': LuaDoc.FILENAME_OBJECT}]
        html = self._get_header(title, nav_objects)
        html += '<p>' + self._get_term('object:description') + '</p>' + os.linesep

        items = []
        for object in self._objects:
            items.append({'id': object['cpp_name'].lower(), 'href': object['filename'], 'title': self._get_term(object['name'])})

        categories = json.loads(LuaDoc._read_file(os.path.join(os.path.dirname(__file__), 'luadoc', 'object.categories.json')))

        for key, category in categories.items():
            category['id'] = key
            category['title'] = self._get_term('object.category.' + key + ':title')
            category['items'] = []
            for object in category['objects']:
                print(object)
                for item in items:
                    if item['id'] == object:
                        category['items'].append(item)
                        items = [v for v in items if v['id'] != object]
                        break

        for category in sorted(categories.values(), key=operator.itemgetter('title')):
            html += '<h2 id="' + key + '">' + category['title'] + '</h2>'
            html += '<ul>' + os.linesep
            for item in sorted(category['items'], key=operator.itemgetter('title')):
                html += '  <li><a href="' + item['href'] + '">' + item['title'] + '</a></li>' + os.linesep
            html += '</ul>' + os.linesep

        if len(items) > 0:
            html += '<h2>' + self._get_term('object.category.other:title') + '</h2>'
            html += '<ul>' + os.linesep
            for item in sorted(items, key=operator.itemgetter('title')):
                html += '  <li><a href="' + item['href'] + '">' + item['title'] + '</a></li>' + os.linesep
            html += '</ul>' + os.linesep

        html += self._get_footer()
        LuaDoc._write_file(os.path.join(output_dir, LuaDoc.FILENAME_OBJECT), html)

        for object in self._objects:
            self._build_object(output_dir, nav_objects, object)

    def _build_object(self, output_dir: str, nav: list, object: dict) -> None:
        title = self._get_term(object['name'])
        html = self._get_header(title, nav + [{'title': title, 'href': object['filename']}])
        html += '<p>' + self._get_term(object['term_prefix'].rstrip('.') + ':description') + '</p>' + os.linesep
        html += self._build_items_html(object['items'], object['term_prefix'])
        LuaDoc._write_file(os.path.join(output_dir, object['filename']), self._add_toc(html))

    def _build_examples(self, output_dir: str, nav: list) -> None:
        title = self._get_term('example:title')
        nav_examples = nav + [{'title': title, 'href': LuaDoc.FILENAME_EXAMPLE}]
        html = self._get_header(title, nav_examples)
        html += '<p>' + self._get_term('example:description') + '</p>' + os.linesep
        html += '<ul>' + os.linesep
        items = []
        for example in self._examples.values():
            items.append({'href': example['filename'], 'title': self._get_term(example['name'])})
        for item in sorted(items, key=operator.itemgetter('title')):
            html += '  <li><a href="' + item['href'] + '">' + item['title'] + '</a></li>' + os.linesep
        html += '</ul>' + os.linesep
        html += self._get_footer()
        LuaDoc._write_file(os.path.join(output_dir, LuaDoc.FILENAME_EXAMPLE), html)

        for example in self._examples.values():
            self._build_example(output_dir, nav_examples, example)

    def _build_example(self, output_dir: str, nav: list, example: dict) -> None:
        title = self._get_term(example['name'])
        html = self._get_header(title, nav + [{'title': title, 'href': example['filename']}])
        html += '<p>' + self._get_term('example.' + example['id'] + ':description') + '</p>' + os.linesep
        html += '<pre lang="lua"><code>' + highlight_lua(example['code']) + '</code></pre>'
        LuaDoc._write_file(os.path.join(output_dir, example['filename']), html)

    def _build_index_az(self, output_dir: str, nav: list) -> None:
        alphabet = list(string.ascii_uppercase)
        index = {}
        for letter in alphabet:
            index[letter] = []

        # globals:
        for item in self._globals:
            index[item['lua_name'][0].upper()].append({'title': '<code>' + item['lua_name'] + '</code>', 'sub_title': '<code>globals</code>', 'href': LuaDoc.FILENAME_GLOBALS + '#' + item['lua_name']})

        # libs:
        for _, lib in self._libs.items():
            index[lib['lua_name'][0].upper()].append({'title': '<code>' + lib['lua_name'] + '</code>', 'sub_title': self._get_term(lib['name']), 'href': lib['filename']})
            for item in lib['items']:
                index[item['lua_name'][0].upper()].append({'title': '<code>' + item['lua_name'] + '</code>', 'sub_title': '<code>' + lib['lua_name'] + '</code>', 'href': lib['filename'] + '#' + item['lua_name']})

        # enums:
        index['E'].append({'title': '<code>enum</code>', 'sub_title': self._get_term('enum:title'), 'href': LuaDoc.FILENAME_ENUM})
        for enum in self._enums:
            for item in enum['items']:
                letter = item['lua_name'][0].upper()
                index[letter].append({'title': '<code>' + item['lua_name'] + '</code>', 'sub_title': '<code>enum.' + enum['lua_name'] + '</code>', 'href': enum['filename'] + '#' + item['lua_name']})

        # sets:
        index['S'].append({'title': '<code>set</code>', 'sub_title': self._get_term('set:title'), 'href': LuaDoc.FILENAME_SET})
        for set in self._sets:
            for item in set['items']:
                letter = item['lua_name'][0].upper()
                index[letter].append({'title': '<code>' + item['lua_name'] + '</code>', 'sub_title': '<code>set.' + set['lua_name'] + '</code>', 'href': set['filename'] + '#' + item['lua_name']})

        # objects:
        for object in self._objects:
            name = self._get_term(object['name'])
            letter = name[0].upper()
            if letter in index:
                index[letter].append({'title': name, 'sub_title': '<code>object</code>', 'href': object['filename']})

        title = self._get_term('index-az:title')
        html = self._get_header(title, nav + [{'title': title, 'href': set['filename']}])
        html += '<p>' + self._get_term('index-az:description') + '</p>' + os.linesep
        html += '<ul class="index-az-nav">' + os.linesep
        for letter in alphabet:
            if len(index[letter]) != 0:
                html += '  <li><a href="#' + letter + '">' + letter + '</a></li>' + os.linesep
            else:
                html += '  <li class="dim">' + letter + '</li>' + os.linesep

        html += '</ul>' + os.linesep
        html += '<div class="index-az">' + os.linesep
        for letter in alphabet:
            html += '<h4 id="' + letter + '"'
            if len(index[letter]) == 0:
                html += ' class="dim"'
            html += '>' + letter + '</h4>' + os.linesep
            if len(index[letter]) != 0:
                html += '<ul>' + os.linesep
                for item in sorted(index[letter], key=operator.itemgetter('title', 'sub_title')):
                    html += '  <li><a href="' + item['href'] + '">' + item['title'] + '</a> <small class="dim">' + item['sub_title'] + '</small></li>' + os.linesep
                html += '</ul>' + os.linesep
        html += '</div>' + os.linesep
        LuaDoc._write_file(os.path.join(output_dir, LuaDoc.FILENAME_INDEX_AZ), html)

    def _add_toc(self, html: str) -> str:
        toc = '<div class="toc toc-right"><span class="title" href="#">' + self._get_term('contents') + '</span>'

        current_depth = 0
        for tag, id, title in re.findall(r'<(h2|h3|dt) id="(.+?)">(.+?)</\1>', html):
            title = re.sub(r'<a[^>]*>(.*?)</a>', r'\1', title)  # remove links
            title = re.sub(r'^(<code>)[a-z0-9_\.]+\.', r'\1', title)  # remove Lua lib stuff
            title = re.sub(r'<span class="badge.+?</span>', '', title)  # remove badges
            title = title.strip()  # remove leading/tailing spaces
            depth = 2 if tag == 'h2' else 3
            if depth > current_depth:
                toc += '<ul>'
            elif depth < current_depth:
                toc += '</ul></li>'
            else:
                toc += '</li>'
            toc += '<li><a href="#' + id + '">' + title + '</a>'
            current_depth = depth
        toc += '</li></ul>' * (current_depth - 1)
        toc += '</div>'

        return html.replace('<!--TOC-->', toc)

    def _get_header(self, title: str, nav: list) -> str:
        menu = '  <li><a href="' + LuaDoc.FILENAME_GLOBALS + '">' + self._get_term('globals:title') + '</a></li>' + os.linesep
        for k in sorted(list(self._libs.keys()) + ['enum', 'set']):
            if k == 'enum':
                menu += '  <li><a href="' + LuaDoc.FILENAME_ENUM + '">' + self._get_term('enum:title') + '</a></li>' + os.linesep
            elif k == 'set':
                menu += '  <li><a href="' + LuaDoc.FILENAME_SET + '">' + self._get_term('set:title') + '</a></li>' + os.linesep
            else:
                lib = self._libs[k]
                menu += '  <li><a href="' + lib['filename'] + '">' + self._get_term(lib['name']) + '</a></li>' + os.linesep
        menu += '  <li><a href="' + LuaDoc.FILENAME_OBJECT + '">' + self._get_term('object:title') + '</a></li>' + os.linesep
        menu += '  <li><a href="' + LuaDoc.FILENAME_EXAMPLE + '">' + self._get_term('example:title') + '</a></li>' + os.linesep
        menu += '  <li><a href="' + LuaDoc.FILENAME_INDEX_AZ + '">' + self._get_term('index-az:title') + '</a></li>' + os.linesep

        nav_html = ''
        if len(nav) > 0:
            nav_html += '<nav><ul>' + os.linesep
            for item in nav:
                if item == nav[-1]:  # last
                    nav_html += '  <li>' + item['title'] + '</li>' + os.linesep
                else:
                    nav_html += '  <li><a href="' + item['href'] + '">' + item['title'] + '</a></li>' + os.linesep
            nav_html += '</ul></nav>' + os.linesep

        return '''<!doctype html>
<html lang="{language:s}">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>{title:s}</title>
  <link rel="stylesheet" href="./css/pure-min.css">
  <link rel="stylesheet" href="./css/traintasticmanual.css">
</head>
<body>
  <div id="layout">
    <div class="toc toc-left">
        <span class="title">Menu</span>
        <ul>
            {menu:s}
        </ul>
    </div>
    <!--TOC-->
    <div id="main" class="luadoc">
      {nav:s}
      <div class="content">
        <h1>{title:s}</h1>
'''.format(language=self._language, title=title, menu=menu, nav=nav_html)

    def _get_footer(self):
        return '''      </div>
    </div>
  </div>
</body>
</html>
'''


if __name__ == '__main__':
    from argparse import ArgumentParser
    from traintasticmanualbuilder.utils import detect_version

    # Standard options:
    parser = ArgumentParser()
    parser.add_argument('--project-root', default=os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
    parser.add_argument('--output-dir', default=os.path.abspath(os.path.join(os.path.dirname(__file__), 'build.luadoc')))
    parser.add_argument('--language', default=[None], action='append')
    parser.add_argument('--version', default=detect_version())

    args = parser.parse_args(sys.argv[1:])

    lua_doc = LuaDoc(args.project_root)
    lua_doc.verion = args.version

    for language in args.language:
        if language is not None:
            lua_doc.set_language(language)
        lua_doc.build(args.output_dir)

        if len(lua_doc.missing_terms) > 0:
            for term in lua_doc.missing_terms:
                print('Warning: missing or empty definition for term {:s}'.format(term))
            print('Warning: missing or empty definition for {:d} terms.'.format(len(lua_doc.missing_terms)))

    if False:
        for term in lua_doc.missing_terms:
            print('''  {
    "term": "''' + term + '''",
    "definition": ""
  },''')
