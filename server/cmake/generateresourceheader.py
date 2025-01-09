#
# This file is part of the traintastic source code.
# See <https://github.com/traintastic/traintastic>.
#
# Copyright (C) 2024-2025 Reinder Feenstra
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

import sys
import os
import re
import textwrap

if len(sys.argv) != 4:
    print(f"Usage: {sys.argv[0]} <input dir> <input file> <output header>")
    sys.exit(1)

input_file = os.path.join(sys.argv[1], sys.argv[2])
input_file_ext = os.path.splitext(input_file)[1].lstrip('.')

namespaces = ['Resource'] + os.path.dirname(sys.argv[2]).replace('../', '').split('/')
variable = re.sub(r'[\.]+','_', os.path.basename(sys.argv[2]).lower())
guard = '_'.join(namespaces).upper() + '_' + re.sub(r'[\.]+','_', os.path.basename(sys.argv[3]).upper())

is_binary = input_file_ext not in ['html', 'css', 'js']

with open(input_file, 'rb' if is_binary else 'r') as f:
    contents = f.read()

os.makedirs(os.path.dirname(sys.argv[3]), exist_ok=True)

if is_binary:
    size = len(contents)
    contents = ', '.join(['std::byte{' + str(by) + '}' for by in contents])

    contents = '\n  '.join(textwrap.wrap(contents, width=120))

    with open(sys.argv[3], 'w') as f:
        f.write(f'''// Auto-generated, do not edit, it will be overwritten

#ifndef {guard}
#define {guard}

#include <array>

namespace {'::'.join(namespaces)}
{{

constexpr std::array<std::byte, {size}> {variable}{{{{
  {contents}
}}}};

}}

#endif
''')

else:  # text
    with open(sys.argv[3], 'w') as f:
        f.write(f'''// Auto-generated, do not edit, it will be overwritten

#ifndef {guard}
#define {guard}

#include <string_view>

namespace {'::'.join(namespaces)}
{{

constexpr std::string_view {variable} = R"~#!({contents})~#!";

}}

#endif
''')
