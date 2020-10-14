
#include "LightPackInfo.hpp"

// djb2 CRC算法，碰撞相对较少，效率也还凑活，就用他吧（名字牛逼，效率肯定也牛逼！！）
// 下文有一个讨论，因为时间原因，我没细看，但是djb2算法应该是很多地方都在使用的。
// http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
uint32_t hash_djb2(const char *str)
{
    uint32_t hash = 5381;
    int c;
    
    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    
    return hash;
}


/* CRC-32 (Ethernet, ZIP, etc.) polynomial in reversed bit order. */
#define POLY 0xedb88320
uint32_t crc32c(const unsigned char *buf, int len, uint32_t crc)
{
    int k;
    
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return ~crc;
}
