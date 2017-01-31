#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>

//#define VERBOSE

// max UDP message size
static const unsigned long BUF_SIZ = 128;

// print message contents only if VERBOSE is defined
#ifdef VERBOSE
    #define PRINTBUF printbuf
#else
    #define PRINTBUF(...) 
#endif

// helper to print IP and port given sockaddr
void printaddr(const char* msg, const struct sockaddr_in* sock)
{
    if(!sock)
        return;
    if(msg)
        printf("%s ", msg);
    printf("ADDR: %s \tPORT: %d\n", inet_ntoa(sock->sin_addr), 
            (int) ntohs(sock->sin_port));
}

// helper to print buffer as hex bytes
void printbuf(const char* msg, const char* buf, const unsigned int len)
{
    if(!buf || len == 0)
        return;
    if(msg)
        printf("%s ", msg);
    for(unsigned int i =0; i < len; ++i)
        printf("0x%02x ", (int) buf[i]);
    printf("\n");
}

// helper to print error and quit
void die(const char* msg)
{
    printf("%s", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    // check command line params
    if(argc != 4)
        die("Usage  : udpfwd localport remoteip remoteport\n"
            "Example: udpfwd 3040 11.22.33.44 5566\n");

    // socket parameters
    const char* localip     = "127.0.0.1";
    const char* localport   = argv[1];
    const char* remoteip    = argv[2];
    const char* remoteport  = argv[3];

    // remote (final destn.) socket
    struct sockaddr_in rem  = {0};
    rem.sin_family          = AF_INET;
    rem.sin_addr.s_addr     = inet_addr(remoteip);
    rem.sin_port            = htons(atoi(remoteport));

    // local socket
    struct sockaddr_in loc  = {0};
    loc.sin_family          = AF_INET;
    loc.sin_addr.s_addr     = inet_addr(localip);
    loc.sin_port            = htons(atoi(localport));

    // for storing client socket params
    struct sockaddr_in stor = {0};
    char is_addr_stored     = 0;

    // create socket
    int sockfd = -1;
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        die("socket()\n");

    // set reuse address option (allow port reuse without delay)
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
                (char *)&enable, sizeof(enable)) < 0)
        die("setsockopt()\n");
    
    // bind to local port
    if(bind(sockfd, (struct sockaddr *)&loc, sizeof(loc)))
        die("bind()\n");

    printaddr("LISN", &loc);

    while(1)
    {
        struct sockaddr_in rcvsck = {0};
        unsigned int siz = sizeof(rcvsck);
        
        char buf[BUF_SIZ];
        memset(buf, 0, BUF_SIZ);

        int ret = 0;

        // attempt to receive some data
        if((ret=recvfrom(sockfd, buf, BUF_SIZ, 0, 
                        (struct sockaddr *)&rcvsck, &siz)) < 0)
            die("recvfrom()\n");

        // check who the sender is
        if(rcvsck.sin_addr.s_addr == rem.sin_addr.s_addr 
                && rcvsck.sin_port == rem.sin_port)
        {
            // control reaches here when sender is the remote end
            // forward the packet to the client, this is possible
            // only if the client endpoint parameters are stored
            // if not stored, just wait for the next message
            if(!is_addr_stored)
            {
                printaddr("NOT STORED", &rcvsck);
                PRINTBUF("MESG", buf, ret);
                continue;
            }
            printaddr("R->S", &stor);
            PRINTBUF("MESG", buf, ret);
            sendto(sockfd, buf, ret, 0, (struct sockaddr *)&stor, sizeof(stor));
        }
        else 
        {
            if(!is_addr_stored)
            {
                // control reaches here when the client sends the
                // first message out to the remote end, need to 
                // store the client endpoint paramters at this stage
                printaddr("STORING", &rcvsck);
                stor.sin_addr = rcvsck.sin_addr;
                stor.sin_port = rcvsck.sin_port;
                is_addr_stored = 1;
            }
            printaddr("C->R", &rem);
            PRINTBUF("MESG", buf, ret);
            sendto(sockfd, buf, ret, 0, (struct sockaddr *)&rem, sizeof(rem));
        }
    }
}
