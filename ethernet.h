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
