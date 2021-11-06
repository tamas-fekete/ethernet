
#include <fcntl.h> /* O_RDWR */
#include <string.h> /* memset(), memcpy() */
#include <stdio.h> /* perror(), printf(), fprintf() */
#include <stdlib.h> /* exit(), malloc(), free() */
#include <sys/ioctl.h> /* ioctl() */
#include <unistd.h> /* read(), close() */
#include <stdint.h> /* uint8_t, uint16_t */

/* include for struct ifreq */

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

int tap_open(char *devname);


// Create struct to hold an ethernet frame:

typedef union etherType
{
    uint16_t ethertype;
    struct
    {
        uint8_t upperByte;
        uint8_t lowerByte;
    };
   
}etherType;
typedef struct ethernet_header
{
    uint8_t dmac[6];
    uint8_t smac[6];
    etherType ethertype;
    uint8_t payload[];
}__attribute__((packed)) ethernet_header;

typedef struct arp_header
{
    uint8_t hwtype[2];
    uint8_t protype[2];
    uint8_t hwsize;
    uint8_t prosize;
    uint8_t opcode[2];
    
    uint8_t sender_mac[6];
    uint8_t sender_ip[4];
    uint8_t dest_mac[6];
    uint8_t dest_ip[4];
 
}__attribute__((packed)) arp_header;

int main(void)
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
    
    printf("%02x:%02x:%02x:%02x:%02x:%02x has it! Read %d bytes.\n\n", received->smac[0], received->smac[1], received->smac[2], received->smac[3], received->smac[4], received->smac[5], nbytes);
    
    printf("Address Resolution Protocol\n");
    printf("Hardware type: %d\n", arp_received->hwtype[1]);
    printf("Protocol type: 0x%02x%02x\n", arp_received->protype[0], arp_received->protype[1]);
    printf("Hardware size: %d\n", arp_received->hwsize);
    printf("Protocol size: %d\n", arp_received->prosize);
    printf("Opcode: %d\n", arp_received->opcode[1]);
    printf("Sender MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", arp_received->sender_mac[0], arp_received->sender_mac[1], arp_received->sender_mac[2], arp_received->sender_mac[3], arp_received->sender_mac[4], arp_received->sender_mac[5]);
    printf("Sender IP address: %d.%d.%d.%d\n", arp_received->sender_ip[0], arp_received->sender_ip[1], arp_received->sender_ip[2], arp_received->sender_ip[3]);     
    printf("Target MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", arp_received->dest_mac[0], arp_received->dest_mac[1], arp_received->dest_mac[2], arp_received->dest_mac[3], arp_received->dest_mac[4], arp_received->dest_mac[5]);
    printf("Target IP address: %d.%d.%d.%d\n", arp_received->dest_ip[0], arp_received->dest_ip[1], arp_received->dest_ip[2], arp_received->dest_ip[3]); 
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



















