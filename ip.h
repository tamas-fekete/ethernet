typedef struct ip_header
{
    uint8_t ihl:4;      //internet header length
    uint8_t version:4;  // version IPv4 or IPv6
    uint8_t tos;        // type of service 
    uint16_t len;       // total length of IP datagram
    uint16_t id;        // identification
    uint16_t frag_offset;   // fragment offset
    uint8_t ttl;        // time to live
    uint8_t proto;      //protocol
    uint16_t csum;      //checksum
    uint8_t saddr[4];     // source ip address
    uint8_t daddr[4];     // destination ip address
    uint8_t data[];     
    
}__attribute__((packed)) ip_header;
typedef struct ip_header_dummy
{
    uint8_t ihl:4;      //internet header length
    uint8_t version:4;  // version IPv4 or IPv6
    uint8_t tos;        // type of service 
    uint16_t len;       // total length of IP datagram
    uint16_t id;        // identification
    uint16_t frag_offset;   // fragment offset
    uint8_t ttl;        // time to live
    uint8_t proto;      //protocol
    // uint16_t csum;      //checksum
    uint8_t saddr[4];     // source ip address
    uint8_t daddr[4];     // destination ip address
    uint8_t data[];     
    
}__attribute__((packed)) ip_header_dummy;


typedef struct icmp_v4
{
    uint8_t type;
    uint8_t code;
    uint16_t csum;
    uint8_t data[];
}__attribute__((packed)) icmp_v4;

typedef struct icmp_v4_echo
{
    uint16_t id;
    uint16_t seq;
    uint8_t data[];
}__attribute__ ((packed)) icmp_v4_echo;

typedef struct udp_header
{
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t length;
    uint16_t csum;  // optional in udp
    uint8_t data[];
}__attribute__((packed)) udp_header;

uint16_t checksum(void *addr, int count)
{
    /* Compute Internet Checksum for "count" bytes
     *         beginning at location "addr".
     * Taken from https://tools.ietf.org/html/rfc1071
     */

    uint32_t sum = 0;
    uint16_t * ptr = addr;

    while( count > 1 )  {
        /*  This is the inner loop */
        sum += * ptr++;
        count -= 2;
    }

    /*  Add left-over byte, if any */
    if( count > 0 )
        sum += * (uint8_t *) ptr;

    /*  Fold 32-bit sum to 16 bits */
    while (sum>>16)
        sum = (sum & 0xffff) + (sum >> 16);

    return ~sum;
}
