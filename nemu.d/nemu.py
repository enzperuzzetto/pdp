#!/usr/bin/env python

# ------------------------------------------------------
# -- NEmu : The Network Emulator for Mobile Universes --
# ------------------------------------------------------

# Copyright (C) 2011-2016  Vincent Autefage

#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.

#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Lesser General Public License for more details.

#    You should have received a copy of the GNU Lesser General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

# http://nemu.valab.net

'''NEmu start script'''

import os
import sys
_nemu_rootd = '.'

sys.dont_write_bytecode = False

try:
    _nemu_rootd = os.environ["NEMUROOT"]
except KeyError:
    pass

if not _nemu_rootd in sys.path:
    sys.path.append(_nemu_rootd)

from nemu import nemu_main
from nemu import *
nemu_main()
