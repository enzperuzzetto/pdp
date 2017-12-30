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
    fd = VRc(name='sshd_root', id='01', vrouter=vrouter)
    port = 22
    if 'port' in kargs:
	port = int(kargs['port'])
    fd.write('touch /usr/local/etc/ssh/sshd_config;')
    com = 'Port ' + str(port) + '\n'
    com += 'Protocol 2\n'
    com += 'UsePrivilegeSeparation yes\n'
    com += 'SyslogFacility AUTH\n'
    com += 'LogLevel INFO\n'
    com += 'LoginGraceTime 120\n'
    com += 'PermitRootLogin no\n'
    com += 'StrictModes yes\n'
    com += 'PermitEmptyPasswords yes\n'
    com += 'ChallengeResponseAuthentication no\n'
    com += 'AllowTcpForwarding yes\n'
    com += 'PrintMotd no\n'
    com += 'PrintLastLog yes\n'
    com += 'TCPKeepAlive yes\n'
    com += 'Banner /etc/issue\n'
    com += 'AcceptEnv LANG LC_*\n'
    com += 'PidFile /var/run/sshd.pid\n'
    com += 'Subsystem sftp /usr/local/libexec/sftp-server\n'
    com = 'echo -e "'+ com +'" > /usr/local/etc/ssh/sshd_config;'
    fd.write(com)
    fd.write('echo "NEmu VRouter [' + str(vrouter.name) + ']" > /etc/issue;')
    fd.close()

    fd = VRc(name='sshd_start', id='02', vrouter=vrouter)
    fd.write('/usr/local/etc/init.d/openssh start;')
    fd.close()

def help():
    ret = dict()
    ret['syn'] = 'Service("sshd")'
    ret['desc'] = 'Starts a SSHD server daemon at startup'
    ret['args'] = list()
    ret['args'].append('port [int] : server listening port (Default: 22)')
    return ret

