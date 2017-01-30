#! /bin/bash
socat -v PIPE udp-recvfrom:6070,fork
