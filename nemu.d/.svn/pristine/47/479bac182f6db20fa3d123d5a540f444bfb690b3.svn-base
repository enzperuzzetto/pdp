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

'''This module contains functions to improve IP or MAC adress manipulations'''

def ips2ipt(ips):
    '''"x.x.x.x" -> [x,x,x,x]'''
    return map(int, ips.split('.'))

def ipt2ips(ipt):
    '''"[x,x,x,x] -> "x.x.x.x"'''
    return (''.join(map((lambda x: x+'.'), map(str, ipt)))).rstrip('.')

def hws2hwt(hws):
    '''"x:x:x:x:x:x" -> [x,x,x,x,x,x]'''
    return map((lambda x: int(x, 16)), hws.split(':'))

def hwt2hws(hwt):
    '''"[x,x,x,x,x,x] -> "x:x:x:x:x:x"'''
    def padding_wrapper(x):
        '''Padding for hardware address'''
        if len(x) == 0:
            return '00'
        elif len(x) == 1:
            return '0' + x
        return x
    this = map(padding_wrapper, map((lambda x: x.lstrip('0x')), map(hex, hwt)))
    return (''.join(map((lambda x: x+':'), this))).rstrip(':')
