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
    fd = VRc(name='ntpd_root', id='00', vrouter=vrouter)
    fd.write('echo "driftfile /var/ntp.drift" > /etc/ntp.conf;')
    fd.write('echo "server 127.127.1.0" >> /etc/ntp.conf;')
    fd.write('echo "fudge 127.127.1.0 stratum 10" >> /etc/ntp.conf;')
    fd.write('touch /var/ntpd.drift;')
    fd.write('touch /var/run/ntp.pid;')
    fd.write('touch /var/log/ntp.log;')
    fd.close()

    fd = VRc(name='ntpd_conf', id='01', vrouter=vrouter, mode='a+')
    for i in largs:
        fd.write('echo "server ' + i + '" >> /etc/ntp.conf;')
    fd.close()

    fd = VRc(name='ntpd_start', id='02', vrouter=vrouter, mode='a+')
    for i in largs:
        fd.write('/usr/local/bin/ntpdate ' + i + ';')
    fd.close()

    fd = VRc(name='ntpd_start', id='03', vrouter=vrouter)
    fd.write('/usr/local/bin/ntpd -c /etc/ntp.conf -l /var/log/ntp.log -p /var/run/ntp.pid;')
    fd.close()


def help():
    ret = dict()
    ret['syn'] = 'Service("ntpd", ...)'
    ret['desc'] = 'NTP client and daemons at startup'
    ret['args'] = list()
    ret['args'].append('[list of strings] : network IP addresses of NTP top servers')
    return ret

