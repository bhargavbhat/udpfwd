#! /bin/bash
if [[ $1 == 6 ]]; then
    socat - udp6:[::1]:5080
else
    socat - udp:localhost:5080
fi
