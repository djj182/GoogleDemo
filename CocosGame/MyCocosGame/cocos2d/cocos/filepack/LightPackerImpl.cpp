
#include "LightPackerImpl.hpp"
#include <stdio.h>
#include "zlib.h"
//#include "Lzma86.h"
//#include "snappy-c.h"

// POSIX dependencies
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>     /* PATH_MAX */
#include <sys/stat.h>   /* mkdir(2) */
#include <errno.h>
#include <libgen.h>

extern bool rmtree(const char path[]);
extern int mkdir_p(const char *path);

CLightReadFile::CLightReadFile(LightPacker* packer)
: m_packer(packer)
, m_virtualPos(0)
{
    
}

bool CLightReadFile::init(std::string const& short_fn)
{
    unsigned int offset = m_packer->getOffset(short_fn);
    if (offset == 0) {
        return false;
    }
    
    m_offset = offset;
    m_packer->readContent(m_offset, &m_fileHeader, sizeof(m_fileHeader));
    
    return true;
}

int CLightReadFile::size() const
{
    return m_fileHeader.originSize;
}

int CLightReadFile::seek(int pos)
{
    m_virtualPos = pos;
    return 0;
}

int CLightReadFile::tell() const
{
    return m_virtualPos;
}

int CLightReadFile::read(void* buffer, int size)
{
    if (m_fileHeader.flag == 0) {
        m_packer->readContent(m_offset + sizeof(LightFileHeader), buffer, size);
        return size;
    }
    
    if (size < m_fileHeader.originSize) {
        return -1;
    }
    
    uLong com_size = m_fileHeader.size;
    char* com_buffer = new char[com_size];
    m_packer->readContent(m_offset + sizeof(LightFileHeader), com_buffer, com_size);
    
    uLong outSize = size;
    int ret;
    
    switch (m_fileHeader.flag) {
        case COMFLAG_ZLIB:
            ret = uncompress((Bytef *)buffer, (uLongf *)&outSize, (const Bytef *)com_buffer, com_size);
            if (ret != Z_OK) {
                ret = -1;
            }
            break;
            
//        case COMFLAG_LZMA:
//            ret = Lzma86_Decode((Byte *)buffer, (SizeT *)&outSize, (const Byte *)com_buffer, &com_size);
//            if (ret != SZ_OK) {
//                ret = -1;
//            }
//            break;
//
//        case COMFLAG_SNAPPY:
//            ret = snappy_uncompress(com_buffer, com_size, (char*)buffer, &outSize);
//            if (ret != SNAPPY_OK) {
//                ret = -1;
//            }
//            break;
            
        default:
            ret = -1;
            break;
    }
    
    delete[] com_buffer;
    
    if (ret == -1) {
        return -1;
    }
    
    return outSize;
}

void CLightReadFile::close()
{
    delete this;
}



LightPacker::LightPacker()
: m_fp(NULL)
{
    
}

LightPacker::~LightPacker()
{
    close();
}

// 打开打包文件
bool LightPacker::openPackFile(std::string const& filename)
{
    if (m_fp) {
        fclose(m_fp);
    }
    
    m_fp = fopen(filename.c_str(), "rb");
    if (m_fp == NULL) {
        return false;
    }
    
    fread(&m_header, sizeof(m_header), 1, m_fp);
    
    readEntrys();
    m_packageFilename = filename;
    return true;
}

std::string LightPacker::packageFilename() const
{
    return m_packageFilename;
}

void LightPacker::close()
{
    if (m_fp) {
        fclose(m_fp);
    }
    
    m_fp = NULL;
}

// 文件数量
int LightPacker::getFileCount() const
{
    int c = m_header.fileCount;
    return c;
}

// 是否有文件
bool LightPacker::hasFile(std::string const& filename) const
{
    if (getOffset(filename) == 0) {
        return false;
    }
    
    return true;
}

// 创建一个文件读取器
ILightReadFile* LightPacker::createReadFile(std::string const& filename)
{
    CLightReadFile* p = new CLightReadFile(this);
    if (p->init(filename)) {
        return p;
    }
    
    delete p;
    return NULL;
}

// 读取所有入口
bool LightPacker::readEntrys()
{
    int c = m_header.fileCount;
        
    LightFileEntry* entrys = new LightFileEntry[c];
    
    readContent(sizeof(m_header), entrys, sizeof(LightFileEntry)*c);
    
    for (int i=0;i<c;++i)
    {
        unsigned int hash = entrys[i].hash;
        unsigned int offset = entrys[i].offset;
        
        m_entrys[hash] = offset;
    }
    
    delete[] entrys;
    
    return true;
}

// 获取entry的入口
unsigned int LightPacker::getOffset(std::string const& short_fn) const
{
    unsigned int hash = hash_djb2(short_fn.c_str());
    auto ite = m_entrys.find(hash);
    if (ite == m_entrys.end()) {
        return 0;
    }
    
    return ite->second;
}

// 读取所有的文件名列表
void LightPacker::readAllFilenames(std::vector<std::string>& files)
{
    int len = m_header.fileNameLength;
    char* buffer = new char[len];
    if (buffer == NULL) {
        return;
    }
    
    readContent(m_header.fileNameOffset, buffer, len);
    
    char* oneName = buffer;
    for (int i=0;i<m_header.fileCount*2;++i)
    {
        std::string s = oneName;
        files.push_back(s);
        
        oneName += (s.length() + 1);
        if (oneName[0] == 0) {
            break;
        }
    }
    
    delete[] buffer;
    
    return;
}

// 解包
int LightPacker::extract(std::string const& destPath)
{
    std::string dest = destPath;
    if (dest.empty()) {
        return -1;
    }
    
    rmtree(destPath.c_str());
    mkdir(destPath.c_str(), 0777);

    if (dest[dest.length()-1] != '/') {
        dest += "/";
    }
    
    std::vector<std::string> files;
    readAllFilenames(files);

    
    for (auto ite = files.begin(); ite != files.end(); ++ite)
    {
        std::string& s = (*ite);
        
        printf("extract : %s\n", s.c_str());
        extractOne(dest, s);
    }
    
    return 0;
}

// 解包一个文件
int LightPacker::extractOne(std::string const& destPath, std::string const& short_fn)
{
    unsigned int hash = hash_djb2(short_fn.c_str());
    auto ite2 = m_entrys.find(hash);
    if (ite2 == m_entrys.end()) {
        return -1;
    }
    
    ILightReadFile* f = createReadFile(short_fn);
    
    int readSize = f->size();
    char* buffer = new char[readSize];
    
    int retSize = f->read(buffer, readSize);
    
    std::string fullName = destPath + short_fn;
    
    // 检测目录是否存在
    char* p = strdup(fullName.c_str());
    char *aux = dirname(p);
    mkdir_p(aux);
    free(p);
    
    FILE* destFile = fopen(fullName.c_str(), "w+");
    if (destFile) {
        fwrite(buffer, 1, f->size(), destFile);
        fclose(destFile);
    }
    else {
        printf("error for %s\n", short_fn.c_str());
    }
    
    delete[] buffer;
    
    f->close();

    return 0;
}


// 获取一个LightFileHeader
bool LightPacker::getLightFileHeader(unsigned int offset, LightFileHeader& fileHeader)
{
    fseek(m_fp, offset, SEEK_SET);
    fread(&fileHeader, sizeof(fileHeader), 1, m_fp);
    
    return true;
}

// 读取一组数据
int LightPacker::readContent(unsigned int offset, void* buffer, int size)
{
    fseek(m_fp, offset, SEEK_SET);
    fread(buffer, 1, size, m_fp);
    
    return true;
}
