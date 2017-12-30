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
    fd = VRc(name='gateway', id='00', vrouter=vrouter, mode='a+')
    for i in largs:
        iface = vrouter.findiface(i)
        fd.write('/sbin/udhcpc -i eth' + str(iface.id) + ' -p /var/run/udhcpc.eth' + str(iface.id)  + '.pid;')
    fd.close()

def help():
    ret = dict()
    ret['syn'] = 'Service("gateway", ...)'
    ret['desc'] = 'Calls a DHCP request for severeal network interface cards at startup'
    ret['args'] = list()
    ret['args'].append('[list of int] : ifaces number')
    return ret
