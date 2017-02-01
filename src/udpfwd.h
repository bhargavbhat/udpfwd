#ifndef UDPFWD_H
#define UDPFWD_H

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/time.h>

// VERBOSE logs UDP payload to console in hex
//#define VERBOSE

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
#endif // UDPFWD_H
