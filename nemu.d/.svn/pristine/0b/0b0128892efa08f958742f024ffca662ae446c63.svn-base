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

import random
from nemu.vrc import VRc
from nemu.vnode import VNode

def init(vrouter, *largs, **kargs):
    fd = VRc(name='dnsmasq_core', id='00', vrouter=vrouter)
    fd.write('cp /etc/hosts /etc/hosts.cache;')
    fd.write('echo "127.0.0.1 localhost" > /etc/hosts;')
    #fd.write('echo "127.0.0.1 ' + vrouter.name  + '" >> /etc/hosts;')
    fd.write('tail -n +3 /etc/hosts.cache >> /etc/hosts;')
    fd.write('rm -f /etc/hosts.cache;')
    fd.write('echo "all-servers" > /etc/dnsmasq.conf;')
    fd.write('echo "strict-order" >> /etc/dnsmasq.conf;')
    fd.write('echo "expand-hosts" >> /etc/dnsmasq.conf;')
    fd.write('echo "log-queries" >> /etc/dnsmasq.conf;')
    fd.write('echo "log-dhcp" >> /etc/dnsmasq.conf;')
    dhcp = True
    dns = True
    if 'type' in kargs:
        dhcp = False
        dns = False
        if 'dhcp' in kargs['type']:
            dhcp = True
        if 'dns' in kargs['type']:
            dns = True
    
    #if 'ifaces' in kargs:
    #    for i in kargs['ifaces']:
    #        iface = vrouter.findiface(i)
    #        fd.write('echo "interface=eth' + str(iface.id) + '" >> /etc/dnsmasq.conf;')
    #else:
    if not 'ifaces' in kargs:
        for i in vrouter.ifaces:
            fd.write('echo "interface=eth' + str(i.id) + '" >> /etc/dnsmasq.conf;')
    fd.close()

    fd = VRc(name='dnsmasq_conf', id='01', vrouter=vrouter, mode='a+')
    if dns:
        fd.write('echo "domain=' + kargs['domain'] + ',' + kargs['net'] + '" >> /etc/dnsmasq.conf;')
        #fd.write('echo "127.0.0.1 ' + str(vrouter.name) + '.' + kargs['domain'] + ' ' + str(vrouter.name) + '" >> /etc/hosts;')
    if dhcp:
        id = random.randint(0, 1000)

	if 'ifaces' in kargs:
    	    for i in kargs['ifaces']:
    	        iface = vrouter.findiface(i)
    	        fd.write('echo "interface=eth' + str(iface.id) + '" >> /etc/dnsmasq.conf;')

        if 'start' and 'end' in kargs:
            com = 'echo "dhcp-range=range:' + str(id)  + ','  + kargs['start'] + ',' + kargs['end']
        else:
            com = 'echo "dhcp-range=range:' + str(id)  + ','  + kargs['net'].rsplit('/')[0] + ',static'
        if 'time' in kargs:
            com += ',' + kargs['time']
        com += '" >> /etc/dnsmasq.conf;'
        fd.write(com)
        if 'router' in kargs:
            fd.write('echo "dhcp-option=range:' + str(id) + ',option:router,' + kargs['router']  + '" >> /etc/dnsmasq.conf;')
    for i in largs:
        ip = None
        hw = None
        name = None
        time = None
        node = None
        if 'node' in i:
            _node = i['node']
            node = _node
	    niface= 0
	    if ':' in _node:
                niface = str(_node.split(':')[1])
                node = str(_node.split(':')[0])
            vnode = VNode.set[node]
            name = vnode.name
            iface  = vnode.findiface(niface)
            hw = iface.hw
        if 'ip' in i:
            ip = i['ip']
        if 'hw' in i:
            hw = i['hw']
        if 'name' in i:
            name = i['name']
        if 'time' in i:
            time = i['time']
        if dhcp and ip:
            com = 'echo "dhcp-host='
            com += str(hw)
            if name and dns:
                com += ',' + str(name)
            if ip:
                com += ',' + str(ip)
            if time:
                com += ',' + str(time)
            com += '" >> /etc/dnsmasq.conf;'
            fd.write(com)
        if dns and name and ip:
            fd.write('echo "' + str(ip) + ' ' + str(name) + '.' + kargs['domain'] + ' ' + str(name) + '" >> /etc/hosts;')
    fd.close()

    if 'forwards' in kargs:
        fd = VRc(name='dnsmasq_forwards', id='02', vrouter=vrouter)
        com = 'echo "dhcp-option=option:dns-server'
        for i in kargs['forwards']:
            com += ',' + i
        com += '" >> /etc/dnsmasq.conf;'
        fd.write(com)
        fd.write('mv /etc/resolv.conf /etc/resolv.dnsmasq;')            
        fd.write('echo "nameserver ' + i + '" >> /etc/resolv.conf;')
        fd.write('cat /etc/resolv.dnsmasq >> /etc/resolv.conf;')
        fd.write('rm /etc/resolv.dnsmasq;')
        fd.close()

    fd = VRc(name='dnsmasq_start', id='03', vrouter=vrouter)
    fd.write('touch /var/dnsmasq.leases;')
    fd.write('touch /var/run/dnsmasq.pid;')
    fd.write('dnsmasq --pid-file=/var/run/dnsmasq.pid --dhcp-leasefile=/var/dnsmasq.leases;')
    fd.close()


def help():
    ret = dict()
    ret['syn'] = 'Service("dnsmasq", ... , type, ifaces, net, domain, start, end, router, time, forwards)'
    ret['desc'] = 'Starts a DNSMASQ (DNS or DHCP) server daemon at startup'
    ret['args'] = list()
    ret['args'].append('[list of dict] : host nodes:')
    ret['args'].append('\t. node [string] : VNode name and optional interface number (<name>[:<iface>])')
    ret['args'].append('\t. ip [string] : IP address')
    ret['args'].append('\t. hw [string] : HW address')
    ret['args'].append('\t. name [string] : hostname')
    ret['args'].append('\t. time [string] : addressed time')
    ret['args'].append('type [list of strings] : dns or dhcp')
    ret['args'].append('ifaces [list of int] : list of interfaces (Default: all)')
    ret['args'].append('net [string] : network IP address')
    ret['args'].append('domain [string] : network domain name')
    ret['args'].append('start [string] : network IP start address')
    ret['args'].append('end [string] : network IP end address')
    ret['args'].append('router [string] : optional gateway IP')
    ret['args'].append('time [string] : addressed time')
    ret['args'].append('forwards [list of strings] : DNS servers to contact')
    return ret
