import sys
import string
import os
import posixpath
import codecs
import re
import json
import operator
import shutil
import datetime
import textwrap


class LuaDoc:
    """
    Documentation generator for Traintastic's builtin Lua scripting language
    """

    DEFAULT_LANGUAGE = 'en-us'
    FILENAME_INDEX = 'index.md'
    FILENAME_GLOBALS = 'globals.md'
    FILENAME_PV = 'pv.md'
    FILENAME_ENUM = 'enum.md'
    FILENAME_SET = 'set.md'
    FILENAME_OBJECT = 'object/index.md'
    FILENAME_EXAMPLES = 'examples.md'
    FILENAME_INDEX_AZ = 'index-az.md'

    missing_terms = []
    version = None

    def __init__(self, project_root: str) -> None:
        self._project_root = project_root
        self._globals = LuaDoc._find_globals(project_root)
        self._enums = LuaDoc._find_enums(project_root)
        self._sets = LuaDoc._find_sets(project_root)
        self._libs = LuaDoc._find_libs(project_root)
        self._objects = LuaDoc._find_objects(project_root)
        self._object_categories, self._object_category_other = LuaDoc._build_category_tree(self._objects)
        self._examples = LuaDoc._find_examples(project_root)
        self._patch()
        self._add_cross_references()
        self.set_language(LuaDoc.DEFAULT_LANGUAGE)

    def set_language(self, language: str) -> None:
        self._language = language
        self._terms = LuaDoc._load_terms(language)
        self.missing_terms = []

    def _get_term(self, term: str, optional: bool = False) -> str:
        if term not in self._terms:
            if optional:
                return None
            if term not in self.missing_terms:
                self.missing_terms.append(term)
            return '<span style="color:red">$' + term + '$</span>'

        definition = self._terms[term]
        definition = re.sub(r'{ref:([a-z0-9_\.]+?)(|#[a-z0-9_]+)(|\|.+?)}', self._ref_link, definition)
        return definition

    def _load_terms(language: str) -> dict:
        terms = {}
        for item in json.loads(LuaDoc._read_file(posixpath.join(os.path.dirname(__file__), 'luadoc', 'terms', language + '.json'))):
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
                            enum['see_also'].append('[`' + object['lua_name'] + '.' + item['lua_name'] + '`](../' + object['filename'] + '#' + item['lua_name'] + ')')
                    for set in self._sets:
                        if set['cpp_name'] == cpp_type:
                            set['see_also'].append('[' + object['lua_name'] + '.' + item['lua_name'] + '](../' + object['filename'] + '#' + item['lua_name'] + ')')

    def _ref_link(self, m: re.Match) -> str:
        id = m.group(1)
        fragment = m.group(2)
        title = m.group(3).lstrip('|')
        if id.startswith('enum.'):
            id = id.removeprefix('enum.')
            for enum in self._enums:
                if enum['lua_name'] == id:
                    return '[' + (self._get_term(enum['name']) if title == '' else title) + '](' + enum['filename'] + fragment + ')'
        elif id.startswith('set.'):
            id = id.removeprefix('set.')
            for set in self._sets:
                if set['lua_name'] == id:
                    return '[' + (self._get_term(set['name']) if title == '' else title) + '](' + set['filename'] + fragment + ')'
        elif id.startswith('object.'):
            for object in self._objects:
                if object['lua_name'] == id:
                    return '[' + (self._get_term(object['name']) if title == '' else title) + '](' + object['filename'] + fragment + ')'
        elif id == 'globals':
            return '[' + (self._get_term('globals:title') if title == '' else title) + '](' + self.FILENAME_GLOBALS + fragment + ')'
        elif id == 'enum':
            return '[' + (self._get_term('enum:title') if title == '' else title) + '](' + self.FILENAME_ENUM + fragment + ')'
        elif id == 'set':
            return '[' + (self._get_term('set:title') if title == '' else title) + '](' + self.FILENAME_SET + fragment + ')'
        elif id == 'object':
            return '[' + (self._get_term('object:title') if title == '' else title) + '](' + self.FILENAME_OBJECT + fragment + ')'
        elif id == 'pv':
            return '[' + (self._get_term('pv:title') if title == '' else title) + '](' + self.FILENAME_PV + fragment + ')'

        return '<span style="color:red">' + m.group(0) + '</span>'

    def _fix_links(md: str, cur_dir: str):
        if cur_dir != '':
            md = re.sub(r'\]\(((enum|set|object)/[^\)]+)\)', lambda m : LuaDoc._fix_link_helper(m, cur_dir), md)
        return md

    def _fix_link_helper(m: re.Match, cur_dir: str):
        link = m[1]
        if link.startswith(cur_dir + '/'):
            link = link[len(cur_dir) + 1:]
        else:
            link = '../' + link
        return '](' + link + ')'

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
            f.write('[//]: # (This file is generated by luadoc.py, DO NOT EDIT)' + os.linesep + os.linesep)
            f.write(contents)

    def _copy_file(src: str, dst: str) -> None:
        os.makedirs(dst, mode=0o755, exist_ok=True)
        shutil.copyfile(src, posixpath.join(dst, os.path.basename(src)))

    def _find_globals(project_root: str) -> dict:
        globals = []

        sandbox_cpp = LuaDoc._read_file(posixpath.join(project_root, 'server', 'src', 'lua', 'sandbox.cpp'))

        # lua base lib:
        for args in re.findall(r'addBaseLib\(\s*L\s*,[^{]*{(.+?)}\);', sandbox_cpp, flags=re.DOTALL):
            for name in re.findall(r'"([a-z0-9_]+)"', args):
                globals.append(name)

        # lua libs:
        globals += re.findall(r'addLib\(\s*L\s*,\s*LUA_[A-Z]+\s*,\s*luaopen_([a-z]+)\s*,', sandbox_cpp)

        # setfield:
        for name in re.findall(r'lua_setfield\(\s*L\s*,\s*-2\s*,\s*"([A-Za-z][A-Za-z_]*)"\s*\);', sandbox_cpp):
            globals.append(name)

        globals = LuaDoc._load_data(globals, posixpath.join(os.path.dirname(__file__), 'luadoc', 'globals.json'))

        return globals

    def _find_enums(project_root: str) -> list:
        enums_hpp = LuaDoc._read_file(posixpath.join(project_root, 'server', 'src', 'lua', 'enums.hpp'))
        m = re.findall(r'#define LUA_ENUMS([ \nA-Za-z0-9_,\\]+)\n\n', enums_hpp)
        m = re.sub(r'[ \\\n]+', '', m[0])
        enums = []
        for cpp_name in m.split(','):
            filename = posixpath.join(project_root, 'shared', 'src', 'traintastic', 'enum', cpp_name.lower() + '.hpp')
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
                'filename': posixpath.join('enum', info.group(1).lower() + '.md'),
                'name': 'enum.' + info.group(1).lower() + ':title',
                'cpp_name': cpp_name,
                'lua_name': info.group(1),
                'items': items,
                'see_also': []
                })

        return enums

    def _find_sets(project_root: str) -> list:
        sets_hpp = LuaDoc._read_file(posixpath.join(project_root, 'server', 'src', 'lua', 'sets.hpp'))
        m = re.findall(r'#define LUA_SETS([ \nA-Za-z0-9_,\\]+)\n\n', sets_hpp)
        m = re.sub(r'[ \\\n]+', '', m[0])
        sets = []
        for cpp_name in m.split(','):
            filename = posixpath.join(project_root, 'shared', 'src', 'traintastic', 'set', cpp_name.lower() + '.hpp')
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
                'filename': posixpath.join('set', info.group(1).lower() + '.md'),
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
        sandbox_cpp = LuaDoc._read_file(posixpath.join(project_root, 'server', 'src', 'lua', 'sandbox.cpp'))
        for name, args in re.findall(r'addLib\(\s*L\s*,\s*LUA_[A-Z]+\s*,\s*luaopen_([a-z]+)\s*,\s*{(.+?)}\);', sandbox_cpp, flags=re.DOTALL):
            items = []
            for item_name in re.findall(r'"([a-z0-9_]+)"', args):
                items.append(item_name)
            libs[name] = {
                'filename': name + '.md',
                'name': name + ':title',
                'lua_name': name,
                'items': items}

        # log lib:
        name = 'log'
        items = []
        log_hpp = LuaDoc._read_file(posixpath.join(project_root, 'server', 'src', 'lua', 'log.hpp'))
        for item_name in re.findall(r'static\s+int\s+([a-z]+)\(\s*lua_State\s*\*\s*L\s*\)', log_hpp):
            items.append(item_name)
        libs[name] = {
            'filename': name + '.md',
            'name': name + ':title',
            'lua_name': name,
            'items': items}

        # class lib:
        name = 'class'
        items = []
        class_hpp = LuaDoc._read_file(posixpath.join(project_root, 'server', 'src', 'lua', 'class.hpp'))
        for item_name in re.findall(r'static\s+int\s+([a-zA-Z]+)\(\s*lua_State\s*\*\s*L\s*\)', class_hpp):
            if item_name == 'getClass':
                item_name = 'get'
            items.append(item_name)
        class_cpp = LuaDoc._read_file(posixpath.join(project_root, 'server', 'src', 'lua', 'class.cpp'))
        for item_name in re.findall(r'registerValue<[a-zA-Z0-9]+>\(\s*L\s*,\s*"([A-Z_]+)"\s*\);', class_cpp):
            if item_name == 'getClass':
                item_name = 'get'
            items.append(item_name)
        libs[name] = {
            'filename': name + '.md',
            'name': name + ':title',
            'lua_name': name,
            'items': items}

        # load meta data:
        for name, lib in libs.items():
            filename = posixpath.join(os.path.dirname(__file__), 'luadoc', name + '.json')
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
        for root, dirs, files in os.walk(posixpath.join(project_root, 'server', 'src')):
            for file in files:
                if not file.endswith('.hpp'):
                    continue
                filename_hpp = posixpath.join(root, file)
                hpp = LuaDoc._read_file(filename_hpp)
                m = re.search(r'class\s*([A-Za-z0-9]+)\s*(final|)\s*(:[^;]+?|){', hpp, flags=re.DOTALL)
                if m is None:
                    continue
                base_classes = []
                for base_class, _ in re.findall(r'public\s*([A-Za-z0-9]+)(<[A-Za-z0-9]+>|)', m.group(3)):
                    if not base_class.startswith('std'):
                        base_classes.append(base_class)
                lua_filename_hpp = posixpath.join(project_root, 'server', 'src', 'lua', 'object', os.path.basename(filename_hpp))
                lua_filename_cpp = os.path.splitext(lua_filename_hpp)[0] + '.cpp'
                cpp_classes[m.group(1)] = {
                    'filename_hpp': filename_hpp,
                    'lua_filename_hpp': lua_filename_hpp if os.path.exists(lua_filename_hpp) else None,
                    'lua_filename_cpp': lua_filename_cpp if os.path.exists(lua_filename_cpp) else None,
                    'base_classes': base_classes}

        # indentify those that can be used in Lua:
        class_cpp = LuaDoc._read_file(posixpath.join(project_root, 'server', 'src', 'lua', 'class.cpp'))
        for cpp_name in re.findall(r'registerValue<([a-zA-Z0-9]+)>\(\s*L\s*,\s*"[A-Z0-9_]+"\s*\);', class_cpp):
            if cpp_name not in cpp_classes:
                raise RuntimeError('class {:s} not found'.format(cpp_name))

            lua_name = 'object.' + cpp_name.lower()
            items = LuaDoc._find_object_items(cpp_classes, cpp_name)
            category = LuaDoc._get_object_category(cpp_classes, cpp_name)

            objects.append({
                'filename': posixpath.join('object', cpp_name.lower() + '.md'),
                'lua_name': lua_name,
                'name': lua_name + ':title',
                'cpp_name': cpp_name,
                'term_prefix': lua_name + '.',
                'category': category,
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
            m = re.search(cpp_item_name + r'({|\()\s*[\*]?this\s*,\s*"([a-z0-9_]+)".*?(PropertyFlags::ScriptReadOnly|PropertyFlags::ScriptReadWrite|MethodFlags::ScriptCallable|EventFlags::Scriptable)[^}]*}', cpp)
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

        item = LuaDoc._load_data(items, posixpath.join(posixpath.join(os.path.dirname(__file__), 'luadoc', 'object', cpp_name.lower() + '.json')))

        # get special items that aren't detected:
        items += LuaDoc._get_special_object_items(cpp_name, term_prefix)

        for cpp_base_class in cpp_class['base_classes']: # todo cache this
            items += LuaDoc._find_object_items(cpp_classes, cpp_base_class)

        return items

    def _get_object_category(cpp_classes, cpp_name):
        for cpp_class in [cpp_name] + cpp_classes[cpp_name]['base_classes']:
            if cpp_class.startswith('Board') or cpp_class.endswith('Tile'):
                category = ['boards_tiles']
                if cpp_class.startswith('Turnout'):
                    category += ['turnouts']
                elif cpp_class.startswith('Signal'):
                    category += ['signals']
                elif cpp_class.startswith(('Block', 'Sensor', 'NXButton')):
                    category += ['control']
                elif cpp_class.startswith(('Straight', 'Tunnel', 'Curve', 'Bridge', 'Cross', 'Buffer')):
                    category += ['standard']
                elif cpp_class in ['PushButtonTile', 'SwitchTile', 'LabelTile']:
                    category += ['miscellaneous']
                return category
            if cpp_class.startswith('Decoder'):
                return ['hardware', 'decoders']
            if cpp_class == 'Interface' or cpp_class == 'InterfaceStatus':
                return ['hardware', 'interfaces']
            if cpp_class.startswith('Input'):
                return ['hardware', 'inputs']
            if cpp_class.startswith('Output') or cpp_class.endswith('Output'):
                return ['hardware', 'outputs']
            if cpp_class.startswith('Identification'):
                return ['hardware', 'identification']
            if cpp_class in ['Clock', 'World']:
                return ['world']
            if cpp_class in ['Throttle', 'Train', 'TrainList', 'RailVehicleList'] or cpp_class.endswith('RailVehicle'):
                return ['trains_vehicles']
        return []

    def _get_or_create_category(categories, name):
        for category in categories:
            if category['id'] == name:
                return category
        category = {'id': name, 'name': 'object.category.' + name + ':title', 'items': [], 'categories': []}
        categories.append(category)
        return category

    def _build_category_tree(objects: list):
        categories = []
        other = []

        for object in objects:
            item = {'id': object['cpp_name'].lower(), 'href': object['filename'], 'title': object['name']}
            if not object['category']:
                other.append(item)
                continue

            current_level = categories
            for i, category_name in enumerate(object['category']):
                category = LuaDoc._get_or_create_category(current_level, category_name)
                if i == len(object['category']) - 1:
                    category["items"].append(item)
                current_level = category["categories"]

        return categories, other

    def _find_examples(project_root: str) -> dict:
        examples = {}
        for root, dirs, files in os.walk(posixpath.join(project_root, 'manual', 'luadoc', 'example')):
            for file in files:
                if not file.endswith('.lua'):
                    continue

                id = os.path.splitext(file)[0]
                examples[id] = {
                    'id': id,
                    'name': 'example.' + id + ':title',
                    'filename': posixpath.join('example', id + '.md'),
                    'code': LuaDoc._read_file(posixpath.join(root,file))}

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

    def _patch(self) -> None:
        """ Patch stuff that is hard to determine with a bit of regex magic """
        for object in self._objects:
            # remove get_output output_channels:
            if object['cpp_name'] in ['HSI88Interface']:
                object['items'] = [item for item in object['items'] if item['lua_name'] not in ['get_output', 'output_channels']]

            # make get_input() channel arg non optional:
            if object['cpp_name'] in ['ECoSInterface', 'HSI88Interface', 'Z21Interface']:
                for item in object['items']:
                    if item['lua_name'] == 'get_input':
                        item['parameters'][0]['optional'] = False

    def build(self, output_dir: str) -> None:
        # reset missing terms
        self.missing_terms = []

        self._build_globals(output_dir)
        self._build_pv(output_dir)
        self._build_enums(output_dir)
        self._build_sets(output_dir)
        for _, lib in self._libs.items():
            self._build_lib(output_dir, lib)
        self._build_objects(output_dir)
        self._build_examples(output_dir)
        self._build_index_az(output_dir)

    def _build_items_md(self, items: list, term_prefix: str, lua_prefix: str = '') -> str:
        md = ''
        items = sorted(items, key=operator.itemgetter('lua_name'))

        constants = [item for item in items if item['type'] == 'constant']
        if len(constants) > 0:
            md += '## ' + self._get_term('constants') + os.linesep + os.linesep
            for item in constants:
                item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix
                md += '### `' + lua_prefix + item['lua_name'] + '`' + os.linesep
                md += self._get_term(item_term_prefix + item['lua_name'].lower() + ':description') + os.linesep + os.linesep

        libraries = [item for item in items if item['type'] == 'library']
        if len(libraries) > 0:
            md += '## ' + self._get_term('libraries') + os.linesep + os.linesep
            for item in libraries:
                md += '### `' + lua_prefix + item['lua_name'] + '`' + os.linesep
                if item['lua_name'] == 'enum':
                    href = LuaDoc.FILENAME_ENUM
                elif item['lua_name'] == 'set':
                    href = LuaDoc.FILENAME_SET
                else:
                    href = self._libs[item['lua_name']]['filename']
                md += '[' + self._get_term(item['lua_name'] + ':title') + '](' + href + ')' + os.linesep + os.linesep

        objects = [item for item in items if item['type'] == 'object']
        if len(objects) > 0:
            md += '## ' + self._get_term('objects') + os.linesep + os.linesep
            for item in objects:
                item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix
                md += '### `' + lua_prefix + item['lua_name'] + '`' + os.linesep
                md += self._get_term(item_term_prefix + item['lua_name'].lower() + ':description') + os.linesep + os.linesep

        properties = [item for item in items if item['type'] == 'property']
        if len(properties) > 0:
            md += '## ' + self._get_term('properties') + os.linesep + os.linesep
            for item in properties:
                item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix
                md += '### `' + lua_prefix + item['lua_name'] + '`' + os.linesep
                md += '  ' + self._get_term(item_term_prefix + item['lua_name'].lower() + ':description') + os.linesep + os.linesep

        for function_or_method in ['function', 'method']:
            functions = [item for item in items if item['type'] == function_or_method]
            if len(functions) > 0:
                md += '## ' + self._get_term(function_or_method + 's') + os.linesep + os.linesep
                for item in functions:
                    item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix

                    md += '### `' + lua_prefix
                    if item['lua_name'] == '__get':
                        md += '['
                    else:
                        md += item['lua_name'] + '('

                    optional = 0
                    for p in item['parameters']:
                        is_optional = 'optional' in p and p['optional']
                        is_first = p == item['parameters'][0]
                        if is_optional:
                            md += '[' if is_first else ' ['
                            optional += 1
                        if not is_first:
                            md += ','
                        if not is_optional and optional > 0:
                            md += ']' * optional
                            optional = 0
                        if not is_first:
                            md += ' '
                        md += p['name']
                        if is_optional and 'default' in p:
                            md += ' = ' + str(p['default'])
                    md += ']' * optional
                    if item['lua_name'] == '__get':
                        md += ']'
                    else:
                        md += ')'
                    md += '` {#' + item['lua_name'] + '}' + os.linesep + os.linesep

                    md += self._get_term(item_term_prefix + item['lua_name'].lower() + ':description') + os.linesep + os.linesep

                    for qualifier in ['warning', 'note', 'tip']:
                        description = item_term_prefix + item['lua_name'].lower() + '.' + qualifier + ':description'
                        if description in self._terms:
                            md += '!!! ' + qualifier
                            title = item_term_prefix + item['lua_name'].lower() + '.' + qualifier + ':title'
                            if title in self._terms:
                                md += ' "' + self._get_term(title) + '"'
                            md += os.linesep + textwrap.indent(self._get_term(description), ' ' * 4) + os.linesep + os.linesep

                    if len(item['parameters']) > 0:
                        md += '**' + self._get_term('parameters') + ':**' + os.linesep + os.linesep
                        for param in item['parameters']:
                            md += '- `' + param['name'] + '`  ' + os.linesep
                            md += '  ' + self._get_term(item_term_prefix + item['lua_name']  + '.parameter.' + param['name'] + ':description') + os.linesep + os.linesep

                    if item['return_values'] > 0:
                        md += '**' + self._get_term('return_values') + '**  ' + os.linesep
                        md += self._get_term(item_term_prefix + item['lua_name']  + ':return_values') + os.linesep + os.linesep

                    if 'examples' in item and len(item['examples']) != 0:
                        md += '<h4>' + self._get_term('example' if len(item['examples']) == 1 else 'examples') + '</h4>' + os.linesep
                        for example in item['examples']:
                            md += '```lua' + os.linesep + example['code'] + '```' + os.linesep + os.linesep + os.linesep

        events = [item for item in items if item['type'] == 'event']
        if len(events) > 0:
            md += '## ' + self._get_term('events') + os.linesep + os.linesep

            if text := self._get_term(term_prefix.rstrip('.') + ':events_overview', optional=True):
                md += text + os.linesep + os.linesep

            for item in events:
                item_term_prefix = item['term_prefix'] if 'term_prefix' in item else term_prefix

                md += '### `' + item['lua_name'] + '`' + os.linesep + os.linesep
                md += self._get_term(term_prefix + item['lua_name'].lower() + ':description') + os.linesep + os.linesep

                md += '**Handler signature**' + os.linesep + os.linesep + '`function ('
                for p in item['parameters']:
                    md += p['name'] + ', '
                md += 'user_data)`' + os.linesep + os.linesep

                md += '**' + self._get_term('arguments') + '**' + os.linesep + os.linesep
                for param in item['parameters']:
                    md += '- `' + param['name'] + '` - ' + self._get_term(item_term_prefix + item['lua_name']  + '.parameter.' + param['name'] + ':description') + os.linesep + os.linesep
                md += '- `user_data` - ' + self._get_term('event.parameter.user_data:description') + os.linesep + os.linesep

        return md

    def _build_see_also_md(self, items: list):
        if len(items) == 0:
            return ''
        md = '## ' + self._get_term('see_also') + os.linesep + os.linesep
        for item in items:
            md += '- ' + item + os.linesep
        md += os.linesep
        return md

    def _build_index(self, output_dir: str) -> None:
        md = '# ' + self._get_term('index:title') + os.linesep + os.linesep
        if self.version is not None:
          md += '<p class="center large">v' + self._version + os.linesep
        md += '<p class="center dim small">' + datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S') + os.linesep
        md += self._get_term('index:description') + os.linesep + os.linesep
        md += '<ul>' + os.linesep
        md += '  <li><a href="' + LuaDoc.FILENAME_GLOBALS + '">' + self._get_term('globals:title') + '</a></li>' + os.linesep
        for k in sorted(list(self._libs.keys()) + ['enum', 'set']):
            if k == 'enum':
                md += '  <li><a href="' + LuaDoc.FILENAME_ENUM + '">' + self._get_term('enum:title') + '</a></li>' + os.linesep
            elif k == 'set':
                md += '  <li><a href="' + LuaDoc.FILENAME_SET + '">' + self._get_term('set:title') + '</a></li>' + os.linesep
            else:
                lib = self._libs[k]
                md += '  <li><a href="' + lib['filename'] + '">' + self._get_term(lib['name']) + '</a></li>' + os.linesep
        md += '  <li><a href="' + LuaDoc.FILENAME_OBJECT + '">' + self._get_term('object:title') + '</a></li>' + os.linesep
        md += '  <li><a href="' + LuaDoc.FILENAME_EXAMPLES + '">' + self._get_term('example:title') + '</a></li>' + os.linesep
        md += '  <li><a href="' + LuaDoc.FILENAME_INDEX_AZ + '">' + self._get_term('index-az:title') + '</a></li>' + os.linesep
        md += '</ul>' + os.linesep
        md += '</div>' + os.linesep
        LuaDoc._write_file(posixpath.join(output_dir, LuaDoc.FILENAME_INDEX), md)

    def _build_globals(self, output_dir: str) -> None:
        md = '# ' + self._get_term('globals:title') + os.linesep + os.linesep
        md += self._get_term('globals:description') + os.linesep + os.linesep
        md += self._build_items_md(self._globals, 'globals.')
        LuaDoc._write_file(posixpath.join(output_dir, LuaDoc.FILENAME_GLOBALS), md)

    def _build_pv(self, output_dir: str) -> None:
        md = '# ' + self._get_term('pv:title') + os.linesep + os.linesep
        md += LuaDoc._md_paragraph(self._get_term('pv:paragraph_1'))
        md += LuaDoc._md_paragraph(self._get_term('pv:paragraph_2'))

        md += '## ' + self._get_term('pv.storing:title') + os.linesep + os.linesep
        md += LuaDoc._md_paragraph(self._get_term('pv.storing:paragraph_1'))
        md += LuaDoc._md_lua_code(LuaDoc._read_file(posixpath.join(self._project_root, 'manual', 'luadoc', 'example', 'pv', 'storingpersistentdata.lua')))
        md += '!!! note' + os.linesep
        md += '    ' + LuaDoc._md_paragraph(self._get_term('pv:storing_note'))

        md += '## ' + self._get_term('pv.retrieving:title') + os.linesep + os.linesep
        md += LuaDoc._md_paragraph(self._get_term('pv.retrieving:paragraph_1'))
        md += LuaDoc._md_lua_code(LuaDoc._read_file(posixpath.join(self._project_root, 'manual', 'luadoc', 'example', 'pv', 'retrievingpersistentdata.lua')))

        md += '## ' + self._get_term('pv.deleting:title') + os.linesep + os.linesep
        md += LuaDoc._md_paragraph(self._get_term('pv.deleting:paragraph_1'))
        md += LuaDoc._md_lua_code(LuaDoc._read_file(posixpath.join(self._project_root, 'manual', 'luadoc', 'example', 'pv', 'deletingpersistentdata.lua')))

        md += '## ' + self._get_term('pv.checking:title') + os.linesep + os.linesep
        md += LuaDoc._md_paragraph(self._get_term('pv.checking:paragraph_1'))
        md += LuaDoc._md_lua_code(LuaDoc._read_file(posixpath.join(self._project_root, 'manual', 'luadoc', 'example', 'pv', 'checkingforpersistentdata.lua')))

        LuaDoc._write_file(posixpath.join(output_dir, LuaDoc.FILENAME_PV), md)

    def _build_enums(self, output_dir: str) -> None:
        md = '# ' + self._get_term('enum:title') + os.linesep + os.linesep
        md += self._get_term('enum:description') + os.linesep + os.linesep
        md += '## ' + self._get_term('constants') + os.linesep
        md += '<dl>' + os.linesep
        for enum in self._enums:
            md += '  <dt id="' + enum['lua_name'] + '"><a href="' + enum['filename'] + '"><code>enum.' + enum['lua_name'] + '</code></a></dt>' + os.linesep
            md += '  <dd>' + self._get_term(enum['name']) + '</dd>' + os.linesep
        md += '</dl>' + os.linesep
        LuaDoc._write_file(posixpath.join(output_dir, LuaDoc.FILENAME_ENUM), md)

        for enum in self._enums:
            self._build_lib(output_dir, enum, 'enum.')

    def _build_sets(self, output_dir: str) -> None:
        md = '# ' + self._get_term('set:title') + os.linesep + os.linesep
        md += self._get_term('set:description') + os.linesep + os.linesep
        md += '## ' + self._get_term('constants') + os.linesep
        md += '<dl>' + os.linesep
        for set in self._sets:
            md += '  <dt id="' + set['lua_name'] + '"><a href="' + set['filename'] + '"><code>' + set['lua_name'] + '</code></a></dt>' + os.linesep
            md += '  <dd>' + self._get_term(set['name']) + '</dd>' + os.linesep
        md += '</dl>' + os.linesep
        LuaDoc._write_file(posixpath.join(output_dir, LuaDoc.FILENAME_SET), md)

        for set in self._sets:
            self._build_lib(output_dir, set, 'set.')

    def _build_lib(self, output_dir: str, lib: dict, parent_lib: str = '') -> None:
        md = '# ' + self._get_term(lib['name']) + os.linesep + os.linesep
        md += self._get_term(parent_lib + lib['lua_name'] + ':description') + os.linesep + os.linesep
        md += self._build_items_md(lib['items'], parent_lib + lib['lua_name'] + '.', parent_lib + lib['lua_name'] + '.')
        if 'see_also' in lib:
            md += self._build_see_also_md(lib['see_also'])
        LuaDoc._write_file(posixpath.join(output_dir, lib['filename']), LuaDoc._fix_links(md, os.path.dirname(lib['filename'])))

    def _build_object_category(self, category: dict, level: int = 2) -> str:
        md = ('#' * level) + ' ' + category['name'] + os.linesep
        items = [{**v, 'title': self._get_term(v['title'])} for v in category['items']]
        for item in sorted(items, key=operator.itemgetter('title')):
            md += '- [' + item['title'] + '](' + item['href'] + ')' + os.linesep
        categories = [{**v, 'name': self._get_term(v['name'])} for v in category['categories']]
        for category in sorted(categories, key=operator.itemgetter('name')):
            md += self._build_object_category(category, level + 1)
        md += os.linesep
        return md


    def _build_objects(self, output_dir: str) -> None:
        md = '# ' + self._get_term('object:title') + os.linesep + os.linesep
        md += self._get_term('object:description') + os.linesep + os.linesep

        categories = [{**v, 'name': self._get_term(v['name'])} for v in self._object_categories]
        for category in sorted(categories, key=operator.itemgetter('name')):
            md += self._build_object_category(category)

        if len(self._object_category_other) > 0:
            md += '## ' + self._get_term('object.category.other:title') + os.linesep
            items = [{**v, 'title': self._get_term(v['title'])} for v in self._object_category_other]
            for item in sorted(items, key=operator.itemgetter('title')):
                md += '- [' + item['title'] + '](' + item['href'] + ')' + os.linesep
            md += os.linesep

        LuaDoc._write_file(posixpath.join(output_dir, LuaDoc.FILENAME_OBJECT), LuaDoc._fix_links(md, os.path.dirname(LuaDoc.FILENAME_OBJECT)))

        for object in self._objects:
            self._build_object(output_dir, object)

    def _build_object(self, output_dir: str, object: dict) -> None:
        md = '# ' + self._get_term(object['name']) + os.linesep + os.linesep
        md += self._get_term(object['term_prefix'].rstrip('.') + ':description') + os.linesep + os.linesep
        md += self._build_items_md(object['items'], object['term_prefix'])
        LuaDoc._write_file(posixpath.join(output_dir, object['filename']), LuaDoc._fix_links(md, os.path.dirname(object['filename'])))

    def _build_examples(self, output_dir: str) -> None:
        md = '# ' + self._get_term('example:title') + os.linesep + os.linesep
        md += self._get_term('example:description') + os.linesep + os.linesep
        items = []
        for example in self._examples.values():
            items.append({'href': example['filename'], 'title': self._get_term(example['name'])})
        for item in sorted(items, key=operator.itemgetter('title')):
            md += '- [' + item['title'] + '](' + item['href'] + ')' + os.linesep
        md += os.linesep
        LuaDoc._write_file(posixpath.join(output_dir, LuaDoc.FILENAME_EXAMPLES), md)

        for example in self._examples.values():
            self._build_example(output_dir, example)

    def _build_example(self, output_dir: str, example: dict) -> None:
        md = '# ' + self._get_term(example['name']) + os.linesep + os.linesep
        md += self._get_term('example.' + example['id'] + ':description') + os.linesep + os.linesep
        md += LuaDoc._md_lua_code(example['code']) + os.linesep + os.linesep
        LuaDoc._write_file(posixpath.join(output_dir, example['filename']), md)

    def _build_index_az(self, output_dir: str) -> None:
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

        md = '# ' + self._get_term('index-az:title') + os.linesep + os.linesep
        md += self._get_term('index-az:description') + os.linesep + os.linesep
        md += '<ul class="index-az-nav">' + os.linesep
        for letter in alphabet:
            if len(index[letter]) != 0:
                md += '  <li><a href="#' + letter + '">' + letter + '</a></li>' + os.linesep
            else:
                md += '  <li class="dim">' + letter + '</li>' + os.linesep

        md += '</ul>' + os.linesep
        md += '<div class="index-az">' + os.linesep
        for letter in alphabet:
            md += '<h4 id="' + letter + '"'
            if len(index[letter]) == 0:
                md += ' class="dim"'
            md += '>' + letter + '</h4>' + os.linesep
            if len(index[letter]) != 0:
                md += '<ul>' + os.linesep
                for item in sorted(index[letter], key=operator.itemgetter('title', 'sub_title')):
                    md += '  <li><a href="' + item['href'] + '">' + item['title'] + '</a> <small class="dim">' + item['sub_title'] + '</small></li>' + os.linesep
                md += '</ul>' + os.linesep
        md += '</div>' + os.linesep
        LuaDoc._write_file(posixpath.join(output_dir, LuaDoc.FILENAME_INDEX_AZ), md)

    def _md_paragraph(text: str) -> str:
        return text + os.linesep + os.linesep

    def _md_list(items: list) -> str:
        return ('- ' + (os.linesep + os.linesep + '- ').join(items) + os.linesetp + os.linesep) if len(items) > 0 else ''

    def _md_lua_code(code: str) -> str:
        return '```lua' + os.linesep + code.rstrip('\n\r') + os.linesep + '```' + os.linesep + os.linesep
