#! /bin/bash
if [[ $1 == 6 ]]; then
    socat -v PIPE udp6-recvfrom:6070,fork
else
    socat -v PIPE udp-recvfrom:6070,fork
fi
