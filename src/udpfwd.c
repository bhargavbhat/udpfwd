#include "udpfwd.h"

int main(int argc, char** argv)
{
    // check command line params
    if(argc != 5)
        die("Usage      : udpfwd localip localport remoteip remoteport\n"
            "Example #1 : udpfwd 127.0.0.1 3040 11.22.33.44 5566\n"
            "Example #2 : udpfwd ::1 6060 ::1 8899\n");

    // socket parameters
    const char* localip     = argv[1];
    const char* localport   = argv[2];
    const char* remoteip    = argv[3];
    const char* remoteport  = argv[4];
    
    // socket descriptor
    int sockfd = -1;
    
    struct addrinfo hints;
    struct addrinfo *loc = NULL, *addr_itr = NULL, *rem = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family         = AF_UNSPEC;
    hints.ai_socktype       = SOCK_DGRAM;
    hints.ai_flags          = AI_PASSIVE;

    // get available addresses to bind to for local socket
    int err = getaddrinfo(localip, localport, &hints, &loc);
    if (err != 0)
        die("getaddrinfo() - local socket");

    // try to bind
    for (addr_itr = loc; addr_itr != NULL; addr_itr = addr_itr->ai_next)
    {
        // if can't create socket on this address, try next one
        if((sockfd = socket(addr_itr->ai_family, addr_itr->ai_socktype,
                            addr_itr->ai_protocol)) == -1)
            continue;

        // socket created successfully, setup options

        // set reuse address option (allow port reuse without delay)
        int enable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
                    (char *)&enable, sizeof(enable)) < 0)
            die("setsockopt(SO_REUSEADDR)\n");

        // set socket recv timeout (for use with KEEPALIVE feature)
        // SO_KEEPALIVE exists, yes, but it is to be used with TCP only
        // Here KEEPALIVE is implemented "by hand" using recvfrom with timeout
        // and sending a KEEPALIVE packet before going back to listening
        struct timeval tv;
        tv.tv_sec = KEEPALIVE_TIMEOUT;
        tv.tv_usec = 0;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                    &tv, sizeof(tv)) < 0)
            die("setsockopt(SO_RCVTIMEO)\n");

        // attempt to bind, if successful, break out of the loop
        if (bind(sockfd, addr_itr->ai_addr, addr_itr->ai_addrlen) == 0)
            break;

        // if unable to bind, close socket
        close(sockfd);
    }

    // if unable to bind to any address, terminate program
    if (addr_itr == NULL) 
        die("bind()");

    // if able to bind, print address where program is listening
    printaddr("LISN", loc->ai_addr);
    
    // free structs
    freeaddrinfo(loc);
    addr_itr = loc = NULL;

    // reset 
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family         = AF_UNSPEC;
    hints.ai_socktype       = SOCK_DGRAM;

    // get available addresses for remote socket
    err = getaddrinfo(remoteip, remoteport, &hints, &rem);
    if (err != 0) 
        die("getaddrinfo - remote socket");

    // for storing client socket params
    struct sockaddr_storage stor = {0};
    char is_addr_stored = 0;

    while(1)
    {
        struct sockaddr_storage rcvsck = {0};
        unsigned int siz = sizeof(rcvsck);
        
        char buf[BUF_SIZ];
        memset(buf, 0, BUF_SIZ);

        int ret = 0;

        // attempt to receive some data
        if((ret=recvfrom(sockfd, buf, BUF_SIZ, 0, 
                        (struct sockaddr *)&rcvsck, &siz)) < 0)
        {
            // control reaches here when "recvfrom()" timeout or error 
            if( KEEPALIVE_TIMEOUT != 0)
            {
                // timeout case : send KEEPALIVE packet to server
                printaddr("K->R", rem->ai_addr);
                sendto(sockfd, KEEPALIVE_PACKET, KEEPALIVE_PACKET_LEN, 
                        0, rem->ai_addr, rem->ai_addrlen);

                // ... and go back to listening for real messages
                continue;
            }
            else
            {
                // error case : terminate
                die("recvfrom()\n");
            }
        }

        // check if sender params matches remote socket
        if(sockcmp((struct sockaddr *) &rcvsck, rem->ai_addr) == 0)
        {
            // control reaches here when sender is the remote end
            // forward the packet to the client, this is possible
            // only if the client endpoint parameters are stored
            // if not stored, just wait for the next message
            if(!is_addr_stored)
            {
                printaddr("NOT STORED", (struct sockaddr*) &rcvsck);
                PRINTBUF("MESG", buf, ret);
                continue;
            }
            printaddr("R->S", (struct sockaddr*) &stor);
            PRINTBUF("MESG", buf, ret);
    
            if((err = sendto(sockfd, buf, ret, 0, (struct sockaddr*) &stor, sizeof(stor))) != ret)
                die("unable to send full message");
        }
        else 
        {
            if(!is_addr_stored)
            {
                // control reaches here when the client sends the
                // first message out to the remote end, need to 
                // store the client endpoint paramters at this stage
                printaddr("STORING", (struct sockaddr*) &rcvsck);
                memcpy(&stor, &rcvsck, sizeof(struct sockaddr));
                is_addr_stored = 1;
            }
            printaddr("C->R", rem->ai_addr);
            PRINTBUF("MESG", buf, ret);
            
            if((err = sendto(sockfd, buf, ret, 0, rem->ai_addr, rem->ai_addrlen)) != ret)
                die("unable to send full message");
        }
    }

    //cleanup
    freeaddrinfo(rem);
    close(sockfd);
}
