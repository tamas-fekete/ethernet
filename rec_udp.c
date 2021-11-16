#include <fcntl.h> /* O_RDWR */
#include <string.h> /* memset(), memcpy() */
#include <stdio.h> /* perror(), printf(), fprintf() */
#include <stdlib.h> /* exit(), malloc(), free() */
#include <sys/ioctl.h> /* ioctl() */
#include <unistd.h> /* read(), close() */
#include <stdint.h> /* uint8_t, uint16_t */
#include <netinet/in.h> /* htonl(), htons() */

/* include for struct ifreq */

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

/* local */

#include "ethernet.h"
#include "arp.h"
#include "ip.h"


int tap_open(char *devname);
uint16_t checksum(void *addr, int count);

#define UDP_PAYLOAD (255U)


int main(void)
{
    int fd;
    int i;
    // send_arp();
    
    // ethernet header:
    ethernet_header *message = (ethernet_header*)malloc(sizeof(ethernet_header) + sizeof(ip_header) + sizeof(udp_header) + UDP_PAYLOAD);
    ethernet_header *rec_message = (ethernet_header*)malloc(sizeof(ethernet_header) + sizeof(ip_header) + sizeof(udp_header) + UDP_PAYLOAD);

    ip_header_dummy ip_message_dummy;
    ip_header *ip_message = (ip_header*)malloc(sizeof(ip_header));
    udp_header *udp_message = (udp_header*)malloc(sizeof(udp_header) + UDP_PAYLOAD);
    
    ip_header *rec_ip_message = NULL;
    udp_header *rec_udp_message = NULL;
    
    // broadcast
    message->dmac[5] = 0xdc;
    message->dmac[4] = 0x4c;
    message->dmac[3] = 0x91;
    message->dmac[2] = 0x67;
    message->dmac[1] = 0x2b;
    message->dmac[0] = 0x00;
    
    message->smac[5] = 0xbd;
    message->smac[4] = 0xad;
    message->smac[3] = 0x3b;
    message->smac[2] = 0x09;
    message->smac[1] = 0x2f;
    message->smac[0] = 0x2c;
    
    message->ethertype.lowerByte = 0x00; // ether type for IPv4
    message->ethertype.upperByte = 0x08;
    
    ip_message_dummy.ihl = 5;     // 5 32bit words in ip header
    ip_message_dummy.version = 4; // IPv4
    ip_message_dummy.tos = 0;
    ip_message_dummy.len = htons(sizeof(ip_header) + sizeof(udp_header) + UDP_PAYLOAD);      // total length including ip_header + data
    ip_message_dummy.id = 0;
    ip_message_dummy.ttl = 64;  // time to live
    ip_message_dummy.proto = 17; // UDP
    ip_message_dummy.saddr[0] = 192;
    ip_message_dummy.saddr[1] = 168;
    ip_message_dummy.saddr[2] = 0;
    ip_message_dummy.saddr[3] = 99;
    
    ip_message_dummy.daddr[0] = 169;
    ip_message_dummy.daddr[1] = 254;
    ip_message_dummy.daddr[2] = 159;
    ip_message_dummy.daddr[3] = 117;
    
    memcpy(ip_message, &ip_message_dummy, 10);
    memcpy(&(ip_message->saddr[0]), &(ip_message_dummy.saddr[0]), 8);
    
    ip_message->csum = checksum(&ip_message_dummy, sizeof(ip_header_dummy));
    // printf("IP message checksum: %x\n total length: %d\n", ip_message.csum, sizeof(ip_header) + sizeof(udp_header)); 
    
    udp_message->source_port = 0;
    udp_message->dest_port = htons(5000);
    udp_message->length = htons(8 + UDP_PAYLOAD); // 8 bytes header + UDP_PAYLOAD byte data
    udp_message->csum = 0; // not necessary
    
    for (i = 0; i < UDP_PAYLOAD; i++)
    {
        udp_message->data[i] = i;
    }
    
    
    memcpy(message->payload, ip_message, sizeof(ip_header));
    memcpy(((ip_header*)(message->payload))->data, udp_message, sizeof(udp_header) + UDP_PAYLOAD);
    
    fd = tap_open("tap0"); /* devname = if.if_name = "tap0" */
    printf("Device tap0 opened\n");
    printf("arp header: %d\n", sizeof(arp_header));
    printf("ip header: %d\n", sizeof(ip_header));
    printf("udp header: %d\n", sizeof(udp_header));
    printf("ethernet header: %d\n", sizeof(ethernet_header));
    
   /* for (i = 0; i < 255; )
    {
        write(fd, message, sizeof(ethernet_header) + sizeof(ip_header) + sizeof(udp_header) + UDP_PAYLOAD);
        ((udp_header*)(((ip_header*)(message->payload))->data))->data[0] = ++i;
    } */
    i = 0;
    rec_ip_message = (ip_header*)(rec_message->payload);
    rec_udp_message = (udp_header*)(rec_ip_message->data);
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t csum;
    while(1)
    {
        read(fd, rec_message, sizeof(ethernet_header) + sizeof(ip_header) + sizeof(udp_header) + UDP_PAYLOAD);
        if (rec_ip_message->proto == 17) // only UDP packets
        {
            printf("%d: Eth frame: Sender MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", i, rec_message->smac[0], rec_message->smac[1], 
                                                                          rec_message->smac[2], rec_message->smac[3], 
                                                                          rec_message->smac[4], rec_message->smac[5]);
            printf("%d: Eth frame: Destination MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", i, rec_message->dmac[0], rec_message->dmac[1], 
                                                                          rec_message->dmac[2], rec_message->dmac[3], 
                                                                          rec_message->dmac[4], rec_message->dmac[5]);     
            printf("IHL: %d | Version: %d | Total length: %d | Id: %d\n", rec_ip_message->ihl, rec_ip_message->version, ntohs(rec_ip_message->len), ntohs(rec_ip_message->id));
            printf("TimeToLive: %d | Protocol: %d | Check Sum:0x%04x\n",rec_ip_message->ttl, rec_ip_message->proto, ntohs(rec_ip_message->csum));
            printf("Source IP addr: %d.%d.%d.%d\n", rec_ip_message->saddr[0],rec_ip_message->saddr[1], rec_ip_message->saddr[2],rec_ip_message->saddr[3]);                                                                 
            printf("Destination IP addr: %d.%d.%d.%d\n", rec_ip_message->daddr[0], rec_ip_message->daddr[1], rec_ip_message->daddr[2], rec_ip_message->daddr[3]);
            printf("Source port: %d\n", ntohs(rec_udp_message->source_port);
            printf("Destination port: %d\n", ntohs(rec_udp_message->dest_port));
            printf("UDP payload length: %d\n", ntohs(rec_udp_message->length));
            printf("%s\n\n", rec_udp_message->data);
            i++;
        }
    }
    exit(0);
}
 

int tap_open(char *devname)
{
    struct ifreq ifr;
    int fd;
    int err;
    
    if ((fd = open("/dev/net/tun", O_RDWR)) == -1){
        perror("open /dev/net/tun");
        exit(1);
    }
    
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    strncpy(ifr.ifr_name, devname,IFNAMSIZ); // devname = "tun0" or "tun1"
    
    /* ioctl will use ifr.if_name as the of TUN interface to open: "tun0" */
    if ((err = ioctl(fd, TUNSETIFF, (void*)&ifr)) == -1){
        perror("ioctl tUNSETIFF");
        close(fd);
        exit(1);
    }
    
    return fd;
}

