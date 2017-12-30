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

'''This module contains message and display functions'''

import time
import os
import sys
from nemu.var import NemuVar

class NemuMsg():
    '''Nemu message and log class'''
    txtcolor = dict({
            "base" : "\033[0m",
            "black" : "\033[30m",
            "red" : "\033[31m",
            "green" : "\033[32m",
            "yellow" : "\033[33m",
            "blue" : "\033[34m",
            "magenta" : "\033[35m",
            "cyan" : "\033[36m",
            "white" : "\033[37m"
            })    

def tcolor(c, out):
    '''Changes the text in color <c> on the file descriptor <out>'''
    if NemuVar.color and not c == None:
        out.write(NemuMsg.txtcolor[c])

def tbase(c, out):
    '''Changes the text in color <c> on the file descriptor <out>'''
    if NemuVar.color and not c == None:
        out.write(NemuMsg.txtcolor['base'])

def printc_common(com, color, out):
    '''Prints the string <com> with the color <color> on file descriptor <out>'''
    tcolor(color, out)
    out.write(com)
    tbase(color, out)
    out.write("\n")
    out.flush()

def printc(com, color='base'):
    '''Prints the string <com> on stdout and in the log file'''
    if not NemuVar.quiet:
        printc_common(com, color, sys.stdout)
    printlog(com)

def printerr(com, color='red'):
    '''Prints the string <com> on stderr and in the log file'''
    printc_common(com, color, sys.stderr)
    printlog(com)
    
def printok():
    '''Prints an "ok" string on stdout and in the log file'''
    printc('Done.', 'green')

def printlog(com, logf=None, dtime=True):
    '''Prints the string <com> in the log file'''
    if logf == None:
	logf = NemuVar.logf
    if not os.path.isfile(logf):
        return
    logfd = open(logf, 'a')
    for i in NemuMsg().txtcolor.values():
        if i in com:
            com = com.replace(i, '')
    if dtime:
    	printc_common("[" + time.asctime() + "]\t" + com, None, logfd)
    else:
    	printc_common("\t\t\t\t" + com, None, logfd)
    logfd.close()

def printfd(com, fd):
    '''Prints the string <com> in fd'''
    if fd == None:
        print com
    else:
        printc_common(com, None, fd)

def printstr(x):
    '''Formats x'''
    if x == None:
        return 'None'
    if isinstance(x, str):
    	return '\''+ str(x) +'\''
    return str(x)

def baseps1(name='nemu'):
    '''Returns a colored PS1'''
    ps1 = ''
    if NemuVar.color:
        ps1 += NemuMsg.txtcolor['blue']
    ps1 += coreps1(name)
    if NemuVar.color: 
        ps1 += NemuMsg.txtcolor['base']
    return ps1

def coreps1(name):
    '''Returns a basic PS1'''
    return '[' + str(name) + ']~> '

def basebann():
    '''Returns the NEmu start banner'''
    bann = ''
    if NemuVar.color:
        bann += NemuMsg.txtcolor['green']
    bann += 'Welcome to the Network Emulator for Mobile Universes [NEmu]'
    if NemuVar.color:
        bann += NemuMsg.txtcolor['base']
    return bann
