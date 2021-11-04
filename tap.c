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
}ethernet_header;


int main(void)
{
    int i = 1;
    int fd;
    int nbytes;
    char buf[1600];

    ethernet_header *message = malloc(sizeof(ethernet_header) + 3*sizeof(char));
    
    message->dmac[5] = 0xef;
    message->dmac[4] = 0x36;
    message->dmac[3] = 0x26;
    message->dmac[2] = 0xda;
    message->dmac[1] = 0x10;
    message->dmac[0] = 0xca;
    
    message->smac[5] = 0xee;
    message->smac[4] = 0x35;
    message->smac[3] = 0x25;
    message->smac[2] = 0xd9;
    message->smac[1] = 0x09;
    message->smac[0] = 0xc9;
    
    message->ethertype.lowerByte = 3; // payload is 3 bytes long
    message->ethertype.upperByte = 0;

    message->payload[0] = 'a';
    message->payload[1] = 'b';
    message->payload[2] = 'c';
    
    fd = tap_open("tap0"); /* devname = if.if_name = "tap0" */
    printf("Device tap0 opened\n");
    /*while(1){
        nbytes = read(fd, buf, sizeof(buf));
        printf("%d. Read %d bytes from tap0\n",i++, nbytes);
    }*/
    write(fd, message, 17);
    printf("Size of ethernet_header: %d\n", sizeof(struct ethernet_header));

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
    /* after the ioctl call the fd is "connected" to tun device specified by devname ("tun0", "tun1") */
    
    return fd;
}



















