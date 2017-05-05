# udpfwd
Simple UDP Relay/Passthru/Forwarder that transparently forwards UDP traffic to another destination. Support IPv4 as well as IPv6 (AF independent implementation).

### Clone & Build

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/103abd5951734562b8624def54697e4f)](https://www.codacy.com/app/bhargavbhat/udpfwd?utm_source=github.com&utm_medium=referral&utm_content=bhargavbhat/udpfwd&utm_campaign=badger)
[![Build Status](https://travis-ci.org/bhargavbhat/udpfwd.svg?branch=master)](https://travis-ci.org/bhargavbhat/udpfwd)

```
git clone https://github.com/bhargavbhat/udpfwd.git
cd udpfwd/src
make
```

### Run
`udpfwd` REQUIRES three command-line arguments (in following order) to run:

- `localip`   : Address (interface) of local machine to bind on, eg: `127.0.0.1` or `::1`
- `localport` : Port to listen on the local machine.
- `remoteip`  : IPv4 address of the remote server (can be `127.0.0.1` or `::1` as well)
- `remoteport`: Port number of the remote server

Hence complete invocation to run `udpfwd` is  `./udpfwd 127.0.0.1 5080 127.0.0.1 6070`. This will cause the program to listen for traffic on `127.0.0.1:5080` and forward all messages received to `127.0.0.1:6070`

### Console Logs Explained

- `LISN ADDR: 0.0.0.0 	PORT: 5080`
    * Printed just as the program starts-up
    * Indicates the IPv4 Addr and Port the program is listening on
    
- `STORING ADDR: 127.0.0.1 	PORT: 49427`
    * Printed when the program receives the first message from the client
    * Indicates the IPv4 Addr and Port which will be receive all replies from the remote server
    
- `C->R ADDR: 127.0.0.1 	PORT: 6070`
    * Printed when a message from Client (`C`) is sent to the Remote server (`R`)
    * IP and Port of remote server is printed as well (`ADDR: 127.0.0.1 	PORT: 6070`)
    
- `R->S ADDR: 127.0.0.1 	PORT: 49427`
    * Printed when message from Remote server (`R`) is sent to the Stored client (`S`)
    * IP and Port of stored client is printed as well (`ADDR: 127.0.0.1 	PORT: 49427`)
    
- `K->R ADDR: 127.0.0.1 	PORT: 6070`
    * Printed when a KeepAlive message (`K`) is sent to the Remote server (`R`)
    * IP and Port of remote server is printed as well (`ADDR: 127.0.0.1 	PORT: 6070`)
    * Works only when KeepAlive messages are enabled
    
- Misc Logs/Messages:
  * If `VERBOSE` is defined, contents of UDP payload are logged (in both direction), eg: `MESG 0x31 0x32 0x33 0x0a`
  * In case of runtime errors, the program will print a short message (eg: `bind` -> Unable to bind to given port) and quit
  * In case sufficient arguments are not provided, a brief help message is printed and the program terminates 
  
### Test Scripts
The 3 scripts in `test` folder can be used to perform testing:

- `test_server.sh` uses `socat` to create a simple UDP server which echoes back whatever is sent to it
- `test_client.sh` uses `socat` to create a simple UDP client into which user can type in data to be sent to the server
- `test_fwd.sh` is used by the `test` target of the `Makefile` to start `udpfwd` such that it forwards data from the Test Client to the Test Server

`netcat` can also be used to start an interactive server (user can type in responses) rather than a simple echo server. This `netcat` test server can be via the script `test_nc_server.sh` present in the `test` folder.

Test scripts accept a single command-line paramter, which when set to `6` causes the client/server to start with `UDP6` 

### Assumptions, Limitations & Notes
- Key Assumption is that client initiates communication and therefore the client socket params can be saved when the first message is received. The saved socket params will be used as the destination for server replies (i.e. messages sent by server to `udpfwd`).
- Max. message size is fixed to `128` (can be changed in source code)
- UDP payload can be logged to console by defining `VERBOSE` and recompiling
- UDP KeepAlive messages (empty UDP packet with len=0) sent to server are supported. The Timeout value between packets can be adjusted in source code. A Timeout of `0` disables this feature. Contents of the KeepAlive message can be customized in code as well

### External Dependencies
- Requires `netcat` and `socat` for test scripts
