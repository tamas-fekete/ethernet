
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

void send_arp(void);


int main(void)
{
    int fd;
    // send_arp();
    
    // ethernet header:
    ethernet_header *message = malloc(sizeof(ethernet_header) + sizeof(ip_header) + sizeof(udp_header));

    ip_header_dummy ip_message_dummy;
    ip_header ip_message;
    udp_header udp_message;
    
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
    ip_message_dummy.len = htons(sizeof(ip_header) + sizeof(udp_header));      // total length including ip_header + data
    ip_message_dummy.id = 0;
    ip_message_dummy.ttl = 64;
    ip_message_dummy.proto = 17; // UDP
    ip_message_dummy.saddr[0] = 192;
    ip_message_dummy.saddr[1] = 168;
    ip_message_dummy.saddr[2] = 0;
    ip_message_dummy.saddr[3] = 99;
    
    ip_message_dummy.daddr[0] = 169;
    ip_message_dummy.daddr[1] = 254;
    ip_message_dummy.daddr[2] = 159;
    ip_message_dummy.daddr[3] = 117;
    
    memcpy(&ip_message, &ip_message_dummy, 10);
    memcpy(&(ip_message.saddr[0]), &(ip_message_dummy.saddr[0]), 8);
    
    ip_message.csum = checksum(&ip_message_dummy, 5);
    printf("IP message checksum: %x\n total length: %d\n", ip_message.csum, sizeof(ip_header) + sizeof(udp_header)); 
    
    udp_message.source_port = 0;
    udp_message.dest_port = htons(5000);
    udp_message.length = htons(8);
    udp_message.csum = 0; // not necessary
    
    memcpy(message->payload, &ip_message, sizeof(ip_header));
    memcpy(((ip_header*)(message->payload))->data, &udp_message, sizeof(udp_header));
    
    fd = tap_open("tap0"); /* devname = if.if_name = "tap0" */
    printf("Device tap0 opened\n");
    write(fd, message, sizeof(ethernet_header) + sizeof(arp_header));
    
    exit(0);
}

void send_arp(void)
{
    int fd;
    int nbytes;
    
    ethernet_header *message = malloc(sizeof(ethernet_header) + sizeof(arp_header));
    ethernet_header *received = malloc(sizeof(ethernet_header) + sizeof(arp_header));
    arp_header arp_message;
    arp_header *arp_received;
    
    
    
    // broadcast
    message->dmac[5] = 0xff;
    message->dmac[4] = 0xff;
    message->dmac[3] = 0xff;
    message->dmac[2] = 0xff;
    message->dmac[1] = 0xff;
    message->dmac[0] = 0xff;
    
    message->smac[5] = 0xbd;
    message->smac[4] = 0xad;
    message->smac[3] = 0x3b;
    message->smac[2] = 0x09;
    message->smac[1] = 0x2f;
    message->smac[0] = 0x2c;
    
    message->ethertype.lowerByte = 0x06; // ether type for arp 
    message->ethertype.upperByte = 0x08;
    
    arp_message.hwtype[0] =0;
    arp_message.hwtype[1] =1;   // ethernet

    arp_message.protype[0] = 0x08; // IPv4
    arp_message.protype[1] = 0x00;
    
    arp_message.hwsize = 6; 
    arp_message.prosize = 4;
    arp_message.opcode[0] = 0;
    arp_message.opcode[1] = 1;    // request

    arp_message.sender_mac[5] = 0xbd;
    arp_message.sender_mac[4] = 0xad;
    arp_message.sender_mac[3] = 0x3b;
    arp_message.sender_mac[2] = 0x09;
    arp_message.sender_mac[1] = 0x2f;
    arp_message.sender_mac[0] = 0x2c;

    arp_message.sender_ip[0] = 192;
    arp_message.sender_ip[1] = 168;
    arp_message.sender_ip[2] = 0;
    arp_message.sender_ip[3] = 99;

    arp_message.dest_mac[0] = 0x00;
    arp_message.dest_mac[1] = 0x00;
    arp_message.dest_mac[2] = 0x00;
    arp_message.dest_mac[3] = 0x00;
    arp_message.dest_mac[4] = 0x00;
    arp_message.dest_mac[5] = 0x00;

    /* windows computer on the network */
    arp_message.dest_ip[0] = 169;
    arp_message.dest_ip[1] = 254;
    arp_message.dest_ip[2] = 159;
    arp_message.dest_ip[3] = 117;
    
    /*arp_message.dest_ip[0] = 192;
    arp_message.dest_ip[1] = 168;
    arp_message.dest_ip[2] = 0;
    arp_message.dest_ip[3] = 182;*/
   
    memcpy(message->payload, &arp_message, sizeof(arp_header));
    
    fd = tap_open("tap0"); /* devname = if.if_name = "tap0" */
    printf("Device tap0 opened\n");
    write(fd, message, sizeof(ethernet_header) + sizeof(arp_header));
    printf("Size of ethernet_header: %d\n", sizeof(struct ethernet_header));
    printf("Who has %d.%d.%d.%d?\n", arp_message.dest_ip[0], arp_message.dest_ip[1], arp_message.dest_ip[2], arp_message.dest_ip[3]);
    
    nbytes = read(fd, received, sizeof(ethernet_header) + sizeof(arp_header));
    arp_received = (arp_header*)received->payload;
    
    printf("%02x:%02x:%02x:%02x:%02x:%02x has it! Read %d bytes.\n\n", received->smac[0], received->smac[1], 
                                                                       received->smac[2], received->smac[3], 
                                                                       received->smac[4], received->smac[5], nbytes);
    
    printf("Address Resolution Protocol\n");
    printf("Hardware type: %d\n", arp_received->hwtype[1]);
    printf("Protocol type: 0x%02x%02x\n", arp_received->protype[0], arp_received->protype[1]);
    printf("Hardware size: %d\n", arp_received->hwsize);
    printf("Protocol size: %d\n", arp_received->prosize);
    printf("Opcode: %d\n", arp_received->opcode[1]);
    
    printf("Sender MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", arp_received->sender_mac[0], arp_received->sender_mac[1], 
                                                                  arp_received->sender_mac[2], arp_received->sender_mac[3], 
                                                                  arp_received->sender_mac[4], arp_received->sender_mac[5]);
                                                                  
    printf("Sender IP address: %d.%d.%d.%d\n", arp_received->sender_ip[0], arp_received->sender_ip[1], 
                                               arp_received->sender_ip[2], arp_received->sender_ip[3]);     
                                               
    printf("Target MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", arp_received->dest_mac[0], arp_received->dest_mac[1], 
                                                                  arp_received->dest_mac[2], arp_received->dest_mac[3], 
                                                                  arp_received->dest_mac[4], arp_received->dest_mac[5]);
                                                                  
    printf("Target IP address: %d.%d.%d.%d\n", arp_received->dest_ip[0], arp_received->dest_ip[1], 
                                               arp_received->dest_ip[2], arp_received->dest_ip[3]); 
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



















