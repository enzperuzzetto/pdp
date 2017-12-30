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

import os
from nemu.vrc import VRc
from nemu.path import mpath

def init(vrouter, *largs, **kargs):
    fd = VRc(name='nfsd_root', id='00', vrouter=vrouter)
    fd.write('mkdir -p /nfs;')
    fd.write('chmod 644 /etc/exports;')
    fd.write('echo "/nfs 127.0.0.1(ro,fsid=0,no_root_squash,no_subtree_check,sync)" > /etc/exports;')
    fd.close()
    
    fd = VRc(name='nfsd_conf', id='01', vrouter=vrouter, mode='a')
    root = str(kargs['root'])
    auth = '*'
    cfg = 'rw,insecure,all_squash,no_root_squash,no_subtree_check,anonuid=1000,anongid=1000,sync'
    if 'auth' in kargs:
        auth = kargs['auth']
    if len(largs) > 0:
        cfg = reduce(lambda x,y: str(x) + ',' + str(y), largs)
    tag = mpath('/nfsd', os.path.basename(root))
    fd.write('mkdir -p ' + tag)
    fd.write('mount --bind ' + root + ' ' + tag)
    fd.wite('echo "' + tag + ' ' + str(auth) + '(' + str(cfg) + ')" >> /etc/exports;')
    fd.close()
    
    fd = VRc(name='nfsd_start', id='03', vrouter=vrouter)
    fd.write('/usr/local/etc/init.d/nfs-server start;')
    fd.write('exportfs -ra;')
    fd.close()


def help():
    ret = dict()
    ret['syn'] = 'Service("nfsd", ... , root, auth)'
    ret['desc'] = 'Starts a NFSD server daemon at startup'
    ret['args'] = list()
    ret['args'].append('[list of strings] : server configuration')
    ret['args'].append('root [string] : server root directory')
    ret['args'].append('auth [string] : server authorization (Default: *)')
    return ret
