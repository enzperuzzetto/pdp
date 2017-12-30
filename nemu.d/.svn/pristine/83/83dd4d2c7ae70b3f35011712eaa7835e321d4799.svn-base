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
    fd = VRc(name='lighttpd_root', id='00', vrouter=vrouter)
    fd.write('mkdir -p /etc/lighttpd;')
    fd.write('mkdir -p /var/log/lighttpd;')
    fd.write('mkdir -p /var/run/lighttpd;')
    fd.write('mkdir -p /etc/lighttpd/ssl;')
    fd.write('openssl genrsa -out /etc/lighttpd/ssl/ca.key 2048')
    fd.write('openssl req -new -x509 -days 3650 -subj "/O='+vrouter.name+'/OU='+vrouter.name+'/CN='+vrouter.name+'" -key /etc/lighttpd/ssl/ca.key -out /etc/lighttpd/ssl/ca.crt;')
    fd.close()
    
    fd = VRc(name='lighttpd_conf', id='01', vrouter=vrouter, mode='a+')
    root = str(kargs['root'])
    tag = mpath(os.path.basename(root))
    port = 80
    ssl = False
    if 'port' in kargs:
	port = int(kargs['port'])
    if 'ssl' in kargs:
        ssl = kargs['ssl']
    
    fd.write('touch /etc/lighttpd/lighttpd.' + tag + '.conf;')
    fd.write('touch /var/log/lighttpd/lighttpd.' + tag + '.log;')
    fd.write('touch /var/log/lighttpd/lighttpd.' + tag + '.error;')
    fd.write('touch /var/run/lighttpd/lighttpd.' + tag + '.pid;')
    fd.write('echo "server.document-root = \\"'+root+'\\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "server.port = \"' + str(port) + '\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "server.modules = (\\"mod_accesslog\\")" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "server.tag = \\"NEmu VRouter Lighttpd\\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "accesslog.filename = \\"/var/log/lighttpd/lighttpd.'+tag+'.log\\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "server.errorlog = \\"/var/log/lighttpd/lighttpd.'+tag+'.error\\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "server.pid-file = \\"/var/run/lighttpd/lighttpd.'+tag+'.pid\\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    if ssl:
        fd.write('mkdir -p /etc/lighttpd/ssl/' + tag + ';')
        fd.write('echo "ssl.engine = \\"enable\\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
        fd.write('echo "ssl.pemfile = \\"/etc/lighttpd/ssl/' + tag + '/cert.pem\\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
        fd.write('echo "ssl.ca-file = \\"/etc/lighttpd/ssl/ca.crt\\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    	fd.write('mkdir -p /etc/lighttpd/ssl/' + tag + ';')
        fd.write('openssl genrsa -out /etc/lighttpd/ssl/' + tag + '/cert.key 1024')
        fd.write('echo -e "\\n\\n" |openssl req -new -subj "/O='+vrouter.name+'/OU='+tag+'/CN='+tag+'.'+vrouter.name+'" -key /etc/lighttpd/ssl/' + tag + '/cert.key -out /etc/lighttpd/ssl/' + tag + '/cert.csr')
        fd.write('openssl x509 -req -in /etc/lighttpd/ssl/' + tag + '/cert.csr -out /etc/lighttpd/ssl/' + tag + '/cert.crt -sha1 -CA /etc/lighttpd/ssl/ca.crt -CAkey /etc/lighttpd/ssl/ca.key -CAcreateserial -days 3650')
        fd.write('cat /etc/lighttpd/ssl/' + tag + '/cert.key /etc/lighttpd/ssl/' + tag + '/cert.crt > /etc/lighttpd/ssl/' + tag + '/cert.pem')

    fd.write('echo "" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "index-file.names = (\\"index.php\\", \\"index.html\\", \\"index.htm\\")" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "mimetype.assign = (" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".pdf\\" => \\"application/pdf\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".sig\\" => \\"application/pgp-signature\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".spl\\" => \\"application/futuresplash\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".class\\" => \\"application/octet-stream\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".ps\\" => \\"application/postscript\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".torrent\\" => \\"application/x-bittorrent\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".dvi\\" => \\"application/x-dvi\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".gz\\" => \\"application/x-gzip\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".pac\\" => \\"application/x-ns-proxy-autoconfig\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".swf\\" => \\"application/x-shockwave-flash\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".tar.gz\\" => \\"application/x-tgz\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".tgz\\" => \\"application/x-tgz\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".tar\\" => \\"application/x-tar\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".zip\\" => \\"application/zip\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".mp3\\" => \\"audio/mpeg\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".m3u\\" => \\"audio/x-mpegurl\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".wma\\" => \\"audio/x-ms-wma\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".wax\\" => \\"audio/x-ms-wax\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".ogg\\" => \\"application/ogg\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".wav\\" => \\"audio/x-wav\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".gif\\" => \\"image/gif\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".jpg\\" => \\"image/jpeg\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".jpeg\\" => \\"image/jpeg\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".png\\" => \\"image/png\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".xbm\\" => \\"image/x-xbitmap\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".xpm\\" => \\"image/x-xpixmap\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".xwd\\" => \\"image/x-xwindowdump\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".css\\" => \\"text/css\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".html\\" => \\"text/html\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".htm\\" => \\"text/html\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".js\\" => \\"text/javascript\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".asc\\" => \\"text/plain\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".c\\" => \\"text/plain\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".cpp\\" => \\"text/plain\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".log\\" => \\"text/plain\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".conf\\" => \\"text/plain\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".text\\" => \\"text/plain\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".txt\\" => \\"text/plain\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".dtd\\" => \\"text/xml\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".xml\\" => \\"text/xml\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".mpeg\\" => \\"video/mpeg\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".mpg\\" => \\"video/mpeg\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".mov\\" => \\"video/quicktime\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".qt\\" => \\"video/quicktime\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".avi\\" => \\"video/x-msvideo\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".asf\\" => \\"video/x-ms-asf\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".asx\\" => \\"video/x-ms-asf\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".wmv\\" => \\"video/x-ms-wmv\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".bz2\\" => \\"application/x-bzip\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".tbz\\" => \\"application/x-bzip-compressed-tar\\"," >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo "\\".tar.bz2\\" => \\"application/x-bzip-compressed-tar\\"" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.write('echo ")" >> /etc/lighttpd/lighttpd.'+tag+'.conf;')

    fd.close()
    
    fd = VRc(name='lighttpd_start', id='02', vrouter=vrouter, mode='a+')
    fd.write('/usr/local/sbin/lighttpd -f /etc/lighttpd/lighttpd.'+tag+'.conf;')
    fd.close()

def help():
    ret = dict()
    ret['syn'] = 'Service("lighttpd", root, port, ssl)'
    ret['desc'] = 'Starts a LIGHTTPD server daemon at startup'
    ret['args'] = list()
    ret['args'].append('root [string] : server root directory')
    ret['args'].append('port [int] : server listening port (Default: 80)')
    ret['args'].append('ssl  [bool] : enables SSL on server (Default: False)')
    return ret
