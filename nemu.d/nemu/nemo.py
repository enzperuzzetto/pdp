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

'''This module manages mobility'''

import re

from nemu.msg import printstr
from nemu.vair import VAirLink

class NemoParser():
    def __init__(self, nodes):
        self.nodes = list()
        self.nodes.extend(nodes)
        
    def parse(self, line):
        match = re.search('\[[\d\.]+\] (\d+) (\d+) (start|stop|update) ([\d\.]+) ([\d\.]+) ([\d\.]+) ([\d\.]+) [-\d\.]+', line)
        ret = list()
        if match:
            ncli = int(match.group(1))
            nsrv= int(match.group(2))
            act = str(match.group(3))
            bw = int(match.group(4))
            dl = float(match.group(5))
            jit = int(match.group(6))
            ber = float(match.group(7))
            cli = str(self.nodes[ncli-1])
            srv = str(self.nodes[nsrv-1])
            if act == 'start':
                if not VAirLink.set[srv].isiface(cli):
                    subret = 'Join(' + printstr(cli) + ',' + printstr(srv) + ')'
                    ret.append(subret)
                    subret = 'ConfIface(' + printstr(cli + ':' + srv) + ',direction="in",state="down"' + ')'
                    ret.append(subret)
                    subret = 'ConfIface(' + printstr(srv + ':' + cli) + ',direction="out",state="down"' + ')'
                    ret.append(subret)
                subret = 'ConfIface(' + printstr(cli + ':' + srv) + ',direction="out",state="up",bandwidth=' + printstr(bw) + ',delay=' + printstr(dl) + ',jitter=' + printstr(jit) + ',ber=' + printstr(ber) + ')'
                ret.append(subret)
                subret = 'ConfIface(' + printstr(srv + ':' + cli) + ',direction="in",state="up",bandwidth=' + printstr(bw) + ',delay=' + printstr(dl) + ',jitter=' + printstr(jit) + ',ber=' + printstr(ber) + ')'
                ret.append(subret)
                return ret
            if act == 'update':
                subret = 'ConfIface(' + printstr(cli + ':' + srv) + ',direction="out",state="up",bandwidth=' + printstr(bw) + ',delay=' + printstr(dl) + ',jitter=' + printstr(jit) + ',ber=' + printstr(ber) + ')'
                ret.append(subret)
                subret = 'ConfIface(' + printstr(srv + ':' + cli) + ',direction="in",state="up",bandwidth=' + printstr(bw) + ',delay=' + printstr(dl) + ',jitter=' + printstr(jit) + ',ber=' + printstr(ber) + ')'
                ret.append(subret)
                return ret
            if act == 'stop':
                if VAirLink.set[srv].findiface(cli).outstate == "down":
                    subret = 'Unjoin(' + printstr(cli + ':' + srv) + ')'
                    ret.append(subret)
                else:
                    subret = 'ConfIface(' + printstr(cli + ':' + srv) + ',direction="out",state="down"' + ')'
                    ret.append(subret)
                    subret = 'ConfIface(' + printstr(srv + ':' + cli) + ',direction="in",state="down"' + ')'
                    ret.append(subret)
                return ret
        else:
            match = re.search('\[[\d\.]+\] (EOS)', line)
            if match:
                subret = 'self.stop()'
                ret.append(subret)
        return ret
