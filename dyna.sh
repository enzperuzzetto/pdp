#!/bin/bash

echo "lauching router 1"

dynamips -P 7200 --idle-pc 0x6077b2bc -i 1 -X -T 2001 \
 -p 1:PA-FE-TX -s 1:0:udp:10003:127.0.0.1:10002 -p 2:PA-FE-TX -s 2:0:udp:10007:127.0.0.1:10006 ./c7200-jk9s-mz.124-13b.image &

echo "lauching router 2"

dynamips -P 7200 --idle-pc 0x6077b2bc -i 2 -X -T 2002 \
 -p 1:PA-FE-TX -s 1:0:udp:10005:127.0.0.1:10004 -p 2:PA-FE-TX -s 2:0:udp:10009:127.0.0.1:10008 ./c7200-jk9s-mz.124-13b.image &


