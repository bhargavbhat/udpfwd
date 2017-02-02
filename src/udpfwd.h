#ifndef UDPFWD_H
#define UDPFWD_H

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<string.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<net/if.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>
#include<arpa/inet.h>

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif
#ifndef   NI_MAXSERV
#define   NI_MAXSERV 32
#endif

// VERBOSE logs UDP payload to console in hex
#define VERBOSE

// KEEPALIVE_TIMEOUT timeout for the KEEPALIVE feature in seconds
// basically "recvfrom()" will listen for messages xx seconds
// before returning and sending the KEEPALIVE packet
// Set to 0 DISABLE KEEPALIVE
// Set to any other value to ENABLE KEEPALIVE
const unsigned long KEEPALIVE_TIMEOUT = 30; // in sec

// KEEPALIVE_PACKET contents of the KEEPALIVE Packet
// Currently, NULL since we're sending a 0 length packet
const char* KEEPALIVE_PACKET = NULL;

// KEEPALIVE_PACKET_LEN length of KEEPALIVE_PACKET
// Currently, 0 since we're sending a 0 length packet
const unsigned long KEEPALIVE_PACKET_LEN = 0;

// max UDP payload size
const unsigned long BUF_SIZ = 128;

// print message contents only if VERBOSE is defined
#ifdef VERBOSE
    #define PRINTBUF printbuf
#else
    #define PRINTBUF(...) 
#endif

// helper to print error and quit
void die(const char* msg)
{
    printf("%s", msg);
    exit(EXIT_FAILURE);
}

// helper to print IP and port given sockaddr
void printaddr(const char* msg, const struct sockaddr* sock)
{
    if(!sock)
        return;
    if(msg)
        printf("%s ", msg);

    char addr[NI_MAXHOST];
    memset(addr, 0, sizeof(addr));
    int port = 0;

    if(sock->sa_family == AF_INET)
    {
        struct sockaddr_in *v4 = (void*) sock;
        inet_ntop(AF_INET, &(v4->sin_addr), addr, INET_ADDRSTRLEN);
        port = (int) ntohs(v4->sin_port);
    }
    else if(sock->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *v6 = (void*) sock;
        inet_ntop(AF_INET6, &(v6->sin6_addr), addr, INET6_ADDRSTRLEN);
        port = (int) ntohs(v6->sin6_port);
    }
    else
        die("Unknown socket family");

    printf("ADDR: %s\t\t\tPORT:%d\n", addr, port);
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

// helper to compare socket params
int sockcmp(const struct sockaddr* lhs, const struct sockaddr* rhs)
{
    if(lhs->sa_family != rhs->sa_family)
        return 1;

    if(lhs->sa_family == AF_INET)
    {
        struct sockaddr_in *v4lhs = (void*)lhs, *v4rhs = (void*)rhs;
        if(ntohl(v4lhs->sin_addr.s_addr) != ntohl(v4rhs->sin_addr.s_addr))
            return 1;
        else if(ntohs(v4lhs->sin_port) != ntohs(v4rhs->sin_port))
            return 1;
        else
            return 0;
    }
    else if(lhs->sa_family == AF_INET6)
    {
        struct sockaddr_in6 *v6lhs = (void*)lhs, *v6rhs = (void*)rhs;
        if(memcmp(v6lhs->sin6_addr.s6_addr, v6rhs->sin6_addr.s6_addr, sizeof(v6lhs->sin6_addr.s6_addr)) != 0)
            return 1;
        else if(ntohs(v6lhs->sin6_port) !=  ntohs(v6rhs->sin6_port))
            return 1;
        else if(v6lhs->sin6_flowinfo != v6rhs->sin6_flowinfo)
            return 1;
        else if(v6lhs->sin6_scope_id != v6rhs->sin6_scope_id)
            return 1;
        else
            return 0;
    }
    else
    {
        die("Unknown socket family");
    }
    return 0;
}
#endif // UDPFWD_H
