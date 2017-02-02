#! /bin/bash
if [[ $1 == 6 ]]; then
    ./udpfwd ::1 5080 ::1 6070
else
    ./udpfwd 127.0.0.1 5080 127.0.0.1 6070
fi
