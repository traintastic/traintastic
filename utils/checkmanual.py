import sys
import os
import codecs
import re

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))


def print_error(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def read_file(filename):
    with codecs.open(os.path.join(PROJECT_ROOT, filename), 'r', 'utf-8') as f:
        return f.read()


def write_file(filename, value):
    with codecs.open(os.path.join(PROJECT_ROOT, filename), 'w', 'utf-8') as f:
        f.write(value)


def check_messages():
    success = True

    # read info from hpp file:
    logmessage_hpp = os.path.join(PROJECT_ROOT, 'shared', 'src', 'traintastic', 'enum', 'logmessage.hpp')
    hpp = read_file(logmessage_hpp)

    messages = re.findall(r'\b(D|I|N|W|E|C|F)(\d{4})_[A-Z0-9_]+[ ]*=[ ]*LogMessageOffset::[a-z]+[ ]+\+[ ]+(\d{4})', hpp)
    for message in messages:
        if message[1] != message[2]:
          print_error('Error: ' + message[0] + message[1] + ' != ' + message[2] + ' in ' + logmessage_hpp)
          success = False

    success &= check_message_file('D', 'debug.md', messages)
    success &= check_message_file('I', 'info.md', messages)
    success &= check_message_file('N', 'notice.md', messages)
    success &= check_message_file('W', 'warning.md', messages)
    success &= check_message_file('E', 'error.md', messages)
    success &= check_message_file('C', 'critical.md', messages)
    success &= check_message_file('F', 'fatal.md', messages)

    return success


def check_message_file(prefix, filename, messages):
    success = True
    md_file = os.path.join(PROJECT_ROOT, 'manual', 'traintasticmanual', 'en-us', 'messages', filename)
    md = read_file(md_file)
    for message in messages:
        if prefix == message[0]:
            id = message[0] + message[1]
            if re.search('^## ' + id + ': .*{#' + id.lower() + '}$', md, re.MULTILINE) is None:
                print_error('Error: ' + id + ' missing in ' + md_file)
                success = False

    return success


def check_lua_enum():
    success = True

    # read info from hpp files:
    enums_hpp = read_file(os.path.join('server', 'src', 'lua', 'enums.hpp'))
    m = re.findall(r'#define LUA_ENUMS([ \nA-Za-z0-9_,\\]+)\n\n', enums_hpp)
    m = re.sub(r'[ \\\n]+', '', m[0])
    enums = []
    for cpp_name in m.split(','):
        hpp = read_file(os.path.join('shared', 'src', 'traintastic', 'enum', cpp_name.lower() + '.hpp'))
        enum = re.search(r'enum class ' + cpp_name + r'[ ]*(:[ ]*[A-Za-z0-9_]+|)[ \n]*{(.+?)};', hpp, re.DOTALL)
        values = [{'cpp_name': m[0], 'description': m[3]} for m in re.findall(r'^[ ]*([A-Za-z0-9_]+)[ ]*=[ ]*.+?[ ]*(,|)[ ]*(//!< (.+)|)\n', enum.group(2), re.MULTILINE)] if enum is not None else None
        info = re.search(r'TRAINTASTIC_ENUM\([ ]*' + cpp_name + r'[ \n]*,[ \n]*"([a-z0-9_]+)"[ \n]*,[ \n]*\d+[ \n]*,[ \n]*{(.+?)}[ \n]*\);', hpp, re.DOTALL)

        if info is not None:
            for value in values:
                m = re.search(cpp_name + '::' + value['cpp_name'] + r'[ ]*,[ ]*"([A-Za-z0-9_]+)"', info.group(2))
                if m is not None:
                    value['lua_name'] = m.group(1).upper()

        enums.append({
            'cpp_name': cpp_name,
            'lua_name': info.group(1) if info is not None else None,
            'values': values,
            })

    # check manual markdown files
    for enum in enums:
        md_file = os.path.join(PROJECT_ROOT, 'manual', 'traintasticmanual', 'en-us', 'lua', 'library', 'enum', enum['lua_name'].replace('_', '') + '.md')
        if not os.path.exists(md_file):
            print_error('Error: ' + md_file + ' doesn\'t exist')

            md = \
              '# `enum.' + enum['lua_name'] + '` {#lua-enum-' + enum['lua_name'].replace('_', '') + '}' + os.linesep + \
              os.linesep + \
              'TODO' + os.linesep + \
              os.linesep + \
              '## Constants' + os.linesep

            for value in enum['values']:
                md += \
                    os.linesep + \
                    '### `enum.' + enum['lua_name'] + '.' + value['lua_name'] + '`'+ os.linesep + \
                    (value['description'] if value['description'] != '' else 'TODO') + os.linesep

            write_file(md_file, md)
            print('Bootstrapped: ' + md_file)
            success = False
            continue

        md = read_file(md_file)

        # check if all values exist
        for value in enum['values']:
            label = '`enum.' + enum['lua_name'] + '.' + value['lua_name'] + '`'
            if re.search('^### ' + label, md, re.MULTILINE) is None:
                print_error('Error: ' + label + ' missing in ' + md_file)
                success = False

    return success


def check_lua_set():
    success = True

    # read info from hpp files:
    sets_hpp = read_file(os.path.join('server', 'src', 'lua', 'sets.hpp'))
    m = re.findall(r'#define LUA_SETS([ \nA-Za-z0-9_,\\]+)\n\n', sets_hpp)
    m = re.sub(r'[ \\\n]+', '', m[0])
    sets = []
    for cpp_name in m.split(','):
        hpp = read_file(os.path.join('shared', 'src', 'traintastic', 'set', cpp_name.lower() + '.hpp'))
        set = re.search(r'enum class ' + cpp_name + r'[ ]*(:[ ]*[A-Za-z0-9_]+|)[ \n]*{(.+?)};', hpp, re.DOTALL)
        values = [{'cpp_name': m[0], 'description': m[3]} for m in re.findall(r'^[ ]*([A-Za-z0-9_]+)[ ]*=[ ]*.+?[ ]*(,|)[ ]*(//!< (.+)|)\n', set.group(2), re.MULTILINE)] if set is not None else None
        info = re.search(r'TRAINTASTIC_SET\([ ]*' + cpp_name + r'[ \n]*,[ \n]*"([a-z0-9_]+)"[ \n]*,[ \n]*\d+[ \n]*,[ \n]*\(.+?\),[ \n]*{(.+?)}[ \n]*\);', hpp, re.DOTALL)

        if info is not None:
            for value in values:
                m = re.search(cpp_name + '::' + value['cpp_name'] + r'[ ]*,[ ]*"([A-Za-z0-9_]+)"', info.group(2))
                if m is not None:
                    value['lua_name'] = m.group(1).upper()

        sets.append({
            'cpp_name': cpp_name,
            'lua_name': info.group(1) if info is not None else None,
            'values': values,
            })

    # check manual markdown files
    for set in sets:
        md_file = os.path.join(PROJECT_ROOT, 'manual', 'traintasticmanual', 'en-us', 'lua', 'library', 'set', set['lua_name'].replace('_', '') + '.md')
        if not os.path.exists(md_file):
            print_error('Error: ' + md_file + ' doesn\'t exist')

            md = \
              '# `set.' + set['lua_name'] + '` {#lua-set-' + set['lua_name'].replace('_', '') + '}' + os.linesep + \
              os.linesep + \
              'TODO' + os.linesep + \
              os.linesep + \
              '## Constants' + os.linesep

            for value in set['values']:
                md += \
                    os.linesep + \
                    '### `set.' + set['lua_name'] + '.' + value['lua_name'] + '`' + os.linesep + \
                    (value['description'] if value['description'] != '' else 'TODO') + os.linesep

            write_file(md_file, md)
            print('Bootstrapped: ' + md_file)
            success = False
            continue

        md = read_file(md_file)

        # check if all values exist
        for value in set['values']:
            label = '`set.' + set['lua_name'] + '.' + value['lua_name'] + '`'
            if re.search('^### ' + label, md, re.MULTILINE) is None:
                print_error('Error: ' + label + ' missing in ' + md_file)
                success = False

    return success


success = True
success &= check_messages()
success &= check_lua_enum()
success &= check_lua_set()

if not success:
    sys.exit(1)
