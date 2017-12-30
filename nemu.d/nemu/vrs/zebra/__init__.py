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
    fd = VRc(name='zebra_core', id='00', vrouter=vrouter)
    fd.write('mkdir -p /etc/quagga;')
    fd.write('echo "hostname ' + str(vrouter.name) + '" > /etc/quagga/zebra.conf;')
    fd.write('echo "password ' + str(vrouter.name) + '" >> /etc/quagga/zebra.conf;')
    fd.write('echo "enable password ' + str(vrouter.name) + '" >> /etc/quagga/zebra.conf;')
    fd.write('echo "log file /var/log/quagga/zebra.log" >> /etc/quagga/zebra.conf;')
    fd.write('echo "multicast" >> /etc/quagga/zebra.conf;')
    fd.write('echo "!" >> /etc/quagga/zebra.conf;')
    fd.write('echo "line vty" >> /etc/quagga/zebra.conf;')
    fd.write('echo "!" >> /etc/quagga/zebra.conf;')
    fd.write('echo "interface lo" >> /etc/quagga/zebra.conf;')
    fd.close()

    fd = VRc(name='zebra_conf', id='04', vrouter=vrouter, mode='a+')
    for i in kargs['nics']:
        iface = vrouter.findiface(i)
        fd.write('echo "interface eth' + str(iface.id) + '" >> /etc/quagga/zebra.conf;')
    fd.close()

    fd = VRc(name='zebra_start', id='02', vrouter=vrouter)
    fd.write("adduser -D quagga;")
    fd.write('mkdir -p /var/log/quagga;')
    fd.write('mkdir -p /var/run/quagga;')
    fd.write('/usr/local/sbin/zebra -d -f /etc/quagga/zebra.conf -i /var/run/zebra.pid -u root;')
    fd.close()

    if kargs['type'] == 'ospf':
        fd = VRc(name='ospf_core', id='01', vrouter=vrouter)
        fd.write('echo "hostname ' + str(vrouter.name) + '" > /etc/quagga/ospf.conf;')
        fd.write('echo "password ' + str(vrouter.name) + '" >> /etc/quagga/ospf.conf;')
        fd.write('echo "enable password ' + str(vrouter.name) + '" >> /etc/quagga/ospf.conf;')
        fd.write('echo "log file /var/log/quagga/ospf.log" >> /etc/quagga/ospf.conf;')
        fd.write('echo "router ospf" >> /etc/quagga/ospf.conf;')
        fd.write('echo "!" >> /etc/quagga/ospf.conf;')
        fd.write('echo "line vty" >> /etc/quagga/ospf.conf;')
        fd.write('echo "!" >> /etc/quagga/ospf.conf;')
        fd.write('echo "interface lo" >> /etc/quagga/ospf.conf;')
        fd.close()
            
        fd = VRc(name='ospf_conf', id='02', vrouter=vrouter, mode='a+')
        for i in kargs['nics']:
            iface = vrouter.findiface(i)
            fd.write('echo "interface eth' + str(iface.id) + '" >> /etc/quagga/ospf.conf;')
            fd.write('echo "network ' + str(kargs['net'][i]) + ' area '+ str(kargs['area'][i])  + '" >> /etc/quagga/ospf.conf;')
        fd.close()

        fd = VRc(name='ospf_daemon', id='03', vrouter=vrouter)
        fd.write('echo "zebra=2" > /etc/quagga/daemons;')
        fd.write('echo "bgpd=no" >> /etc/quagga/daemons;')
        fd.write('echo "ospfd=2" >> /etc/quagga/daemons;')
        fd.write('echo "ospf6d=no" >> /etc/quagga/daemons;')
        fd.write('echo "ripd=no" >> /etc/quagga/daemons;')
        fd.write('echo "ripngd=no" >> /etc/quagga/daemons;')
        fd.close()
        
        fd = VRc(name='ospf_start', id='04', vrouter=vrouter)
        fd.write('/usr/local/sbin/ospfd -d -f /etc/quagga/ospf.conf -i /var/run/ospfd.pid -u root;')
        fd.close()
        
    elif kargs['type'] == 'rip':
        fd = VRc(name='rip_core', id='01', vrouter=vrouter)
        fd.write('echo "hostname ' + str(vrouter.name) + '" > /etc/quagga/rip.conf;')
        fd.write('echo "password ' + str(vrouter.name) + '" >> /etc/quagga/rip.conf;')
        fd.write('echo "enable password ' + str(vrouter.name) + '" >> /etc/quagga/rip.conf;')
        fd.write('echo "log file /var/log/quagga/rip.log" >> /etc/quagga/rip.conf;')
        fd.write('echo "interface lo" >> /etc/quagga/rip.conf;')
        fd.write('echo "router rip" >> /etc/quagga/rip.conf;')
        fd.write('echo "!" >> /etc/quagga/rip.conf;')
        fd.write('echo "line vty" >> /etc/quagga/rip.conf;')
        fd.write('echo "!" >> /etc/quagga/rip.conf;')
        fd.write('echo "interface lo" >> /etc/quagga/rip.conf;')
        
        fd = VRc(name='rip_conf', id='02', vrouter=vrouter, mode='a+')
        for i in kargs['nics']:
            iface = vrouter.findiface(i)
            fd.write('echo "interface eth' + str(iface.id) + '" >> /etc/quagga/rip.conf;')
            fd.write('echo "network ' + str(kargs['net'][i]) + '" >> /etc/quagga/rip.conf;')
        fd.close()

        fd = VRc(name='rip_daemon', id='03', vrouter=vrouter)
        fd.write('echo "zebra=2" > /etc/quagga/daemons;')
        fd.write('echo "bgpd=no" >> /etc/quagga/daemons;')
        fd.write('echo "ospfd=no" >> /etc/quagga/daemons;')
        fd.write('echo "ospf6d=no" >> /etc/quagga/daemons;')
        fd.write('echo "ripd=2" >> /etc/quagga/daemons;')
        fd.write('echo "ripngd=no" >> /etc/quagga/daemons;')
        fd.close()
        
        fd = VRc(name='rip_start', id='04', vrouter=vrouter)
        fd.write('/usr/local/sbin/ripd -d -f /etc/quagga/rip.conf -i /var/run/ripd.pid -u root;')
        fd.close()


def help():
    ret = dict()
    ret['syn'] = 'Service("zebra", type, nics, net, area)'
    ret['desc'] = 'Starts a ZEBRA (RIP or OSPF) server daemon at startup'
    ret['args'] = list()
    ret['args'].append('type [string] : routing algorithme (rip pr ospf)')
    ret['args'].append('nics [list of int] : rourer network interface cards number')
    ret['args'].append('net [list of strings] : network IP addresses')
    ret['args'].append('area [list of strings] : network area addresses')
    return ret
