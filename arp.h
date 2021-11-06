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
