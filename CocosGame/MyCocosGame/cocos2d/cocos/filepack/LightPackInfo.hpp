//
//  LightPackInfo.hpp
//  xiaoche!!
//
//  Created by liusiyang on 18/03/11.
//  文件打包信息
//
//  LightPacker是一个轻量级的文件打包系统
//  目前开源或者现有的文件打包系统，都是基于早期端游的方案。
//  在这些方案里，不仅有着各种复杂的组织，另外还牵扯到更新删除等问题
//  简单即真理！鉴于复杂的代码不仅维护者阅读困难，另外有了BUG也不好找
//  于是周六晚上思考一番，打算自做一个简易的文件系统。
//  1. 不考虑大文件压缩。传统的文件打包是对文件分块压缩，这样避免打包超大文件时
//     因为内存不足而引发错误。我们的方案是整文件压缩，因为我们不考虑打包大文件
//  2. 不考虑文件的添加和删除。因为手机apk中的文件无法更改。
//  3. 支持灵活的压缩方式，主要是lzma和snappy。一个是压缩比大，一个是效率更高。
//  4. 不支持超过4GB文件！
//

#ifndef LightPackInfo_hpp
#define LightPackInfo_hpp

#include <inttypes.h>

const int PACKAGE_FILE_SIGN = 'LIPK';
const int CURRENT_VERSION = 0x0001;


#define COMFLAG_NO_COMPRESSION  0
#define COMFLAG_ZLIB            1
#define COMFLAG_LZMA            2
#define COMFLAG_SNAPPY          3

//////////////////////////////////
#pragma pack(push)
#pragma pack(1)
struct LightPackageHeader
{
    int32_t	sign;
    int32_t	version;
    int32_t	headerSize;
    int32_t	fileCount;
    int32_t	fileEntryOffset;
    int32_t fileNameOffset;
    int32_t fileNameLength;
    int32_t reserved[18];
};

// 索引入口
struct LightFileEntry
{
    uint32_t hash;
    uint32_t offset;
};

// 文件块
struct LightFileHeader
{
    uint32_t hash;
    int32_t flag;
    uint32_t size;
    uint32_t originSize;
    uint32_t contentCrc;
    uint32_t reserved[4];
};
#pragma pop()


// 文件名hash
uint32_t hash_djb2(const char *str);
// 获取crc值
uint32_t crc32c(const unsigned char *buf, int len, uint32_t crc);


#endif /* LightPackInfo_hpp */
