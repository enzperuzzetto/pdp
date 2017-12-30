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

from nemu.vrc import VRc

def init(vrouter, *largs, **kargs):
    fd = VRc(name='ifup', id='00', vrouter=vrouter, mode='a+')
    for i in largs:
        id = 0
        ip = ''
        if ':' in str(i):
            id = str(i.split(':')[0])
            ip = str(i.split(':')[1])
        iface = vrouter.findiface(id)
        fd.write('ifconfig eth' + str(iface.id) + ' ' + str(ip)  + ' up;')
	if ip:
	    fd.write('echo "' + str(ip) + ' ' + str(vrouter.name) + '" >> /etc/hosts;')
    fd.close()

def help():
    ret = dict()
    ret['syn'] = 'Service("ifup", ...)'
    ret['desc'] = 'Enables and configures several network interface cards at startup'
    ret['args'] = list()
    ret['args'].append('[list of str] : <iface id>[:<IP address>]')
    return ret
