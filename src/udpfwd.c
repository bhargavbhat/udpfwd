#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>

// socket parameters
static const char* localport   = "5080";
static const char* remoteip    = "127.0.0.1";
static const char* remoteport  = "6070";

// max UDP message size
static const unsigned long BUF_SIZ = 128;

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
    exit(1);
}

int main()
{
    // remote (final destn.) socket
    struct sockaddr_in rem  = {0};
    rem.sin_family          = AF_INET;
    rem.sin_addr.s_addr     = inet_addr(remoteip);
    rem.sin_port            = htons(atoi(remoteport));

    // local socket
    struct sockaddr_in loc  = {0};
    loc.sin_family          = AF_INET;
    loc.sin_addr.s_addr     = htonl(INADDR_ANY);
    loc.sin_port            = htons(atoi(localport));

    // var for storing socket params of
    // the first message sent
    struct sockaddr_in stor = {0};
    char is_addr_stored     = 0;

    // create socket
    int sockfd = -1;
    if((sockfd =socket(PF_INET, SOCK_DGRAM, 0)) == -1)
        die("socket()\n");
    
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
        if((ret=recvfrom(sockfd, buf, BUF_SIZ, 0, (struct sockaddr *)&rcvsck, &siz)) < 0)
            die("recvfrom()\n");

        printaddr("RECV", &rcvsck);
        printbuf("MESG", buf, ret);

        // check who the sender is
        if(rcvsck.sin_addr.s_addr==rem.sin_addr.s_addr && rcvsck.sin_port==rem.sin_port)
        {
            // control reaches here when sender is the remote end
            // forward the packet to the client, this is possible
            // only if the client endpoint parameters are stored
            // if not stored, just wait for the next message
            if(!is_addr_stored)
            {
                printaddr("NOT STORED", &rcvsck);
                printbuf("MESG", buf, ret);
                continue;
            }

            printaddr("STOR", &stor);
            printbuf("MESG", buf, ret);
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
            printaddr("SEND", &rem);
            printbuf("MESG", buf, ret);
            sendto(sockfd, buf, ret, 0, (struct sockaddr *)&rem, sizeof(rem));
        }
    }
}
