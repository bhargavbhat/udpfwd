# udpfwd
Simple UDP Relay/Passthru/Forwarder that transparently forwards UDP traffic to another destination

### Assumptions & Limitations
- Client initiates communication, this is needed as the relay needs to know client socket parameters for sending the server's reply back to the client
- Code is UDP and IPv4 only
- Max. message size and local/remote end socket parameters are fixed (can be changed in source code)

### Building
`cd src && make`

### Testing
The 3 scripts in `test` folder can be used to perform testing:

- `test_server.sh` uses `socat` to create a simple UDP server which echoes back whatever is sent to it
- `test_client.sh` uses `socat` to create a simple UDP client into which user can type in data to be sent to the server
- `test_fwd.sh` is used by the `test` target of the `Makefile` to start `udpfwd` such that it forwards data from the Test Client to the Test Server

`netcat` can also be used like so: `nc -ul 6070` to start an interactive server (user can type in responses) rather than a simple echo server.

### Dependencies
Requires `netcat` and `socat` for testing
