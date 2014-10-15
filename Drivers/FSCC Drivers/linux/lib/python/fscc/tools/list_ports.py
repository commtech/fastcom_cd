"""
    Copyright (C) 2014 Commtech, Inc.

    This file is part of pyfscc.

    pyfscc is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pyfscc is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with pyfscc.  If not, see <http://www.gnu.org/licenses/>.

"""

import os

# chose an implementation, depending on os
if os.name == 'nt':
    from fscc.tools.list_ports_windows import *
elif os.name == 'posix':
    from fscc.tools.list_ports_linux import *

if __name__ == '__main__':
    print(sorted(fsccports()))
