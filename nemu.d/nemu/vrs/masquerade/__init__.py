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
    fd = VRc(name='masquerade', id='00', vrouter=vrouter, mode='a+')
    com = 'iptables -t nat -A POSTROUTING'
    if 'ipsrc' in kargs:
        com += ' --source ' + kargs['ipsrc']
    if 'ipdst' in kargs:
        com += ' --destination ' + kargs['ipdst']
    if 'portsrc' in kargs:
        com += ' --source-port ' + str(kargs['portsrc'])
    if 'portdst' in kargs:
        com += ' --destination-port ' + str(kargs['portdst'])
    if 'ifsrc' in kargs:
        iface = vrouter.findiface(kargs['ifsrc'])
        com += ' --in-interface eth' + str(iface.id)
    if 'ifdst' in kargs:
        iface = vrouter.findiface(kargs['ifdst'])
        com += ' --out-interface eth' + str(iface.id)
    if 'proto' in kargs:
        com += ' --protocol ' + str(kargs['proto'])
    com += ' -j MASQUERADE;'
    fd.write(com)
    fd.close()

def help():
    ret = dict()
    ret['syn'] = 'Service("masquerade", ... , type)'
    ret['desc'] = 'Enables NAT masquerading on several network IP adresses'
    ret['args'] = list()
    ret['args'].append('ipsrc [string] : network IP source address (Default: None)')
    ret['args'].append('ipdst [string] : network IP destination address (Default: None)')
    ret['args'].append('proto [string] : network protocol, tcp, udp or icmp (Default: None)')
    ret['args'].append('ifsrc [int] : network source interface (Default: None)')
    ret['args'].append('ifdst [int] : network destination interface (Default: None)')
    ret['args'].append('portsrc [int] : network IP source port (Default: None)')
    ret['args'].append('portdst [int] : network IP destination port (Default: None)')
    return ret
