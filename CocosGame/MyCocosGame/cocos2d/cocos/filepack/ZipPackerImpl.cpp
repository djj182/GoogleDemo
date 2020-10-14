
#include "ZipPackerImpl.hpp"
#include <assert.h>
using namespace cocos2d;

CZipFileReadFile::CZipFileReadFile(ZipFilePacker* packer)
: m_packer(packer)
, m_size(0)
{
    
}

CZipFileReadFile::~CZipFileReadFile()
{
//    unzCloseCurrentFile(m_packer->m_zipHandle);
    return;
}

bool CZipFileReadFile::init(std::string const& short_fn)
{
    m_short_fn = short_fn;
    m_size = m_packer->m_zipFile->getFileUncompressSize(short_fn);
    if (m_size >= 0) {
        return true;
    }
    
    return false;
}

int CZipFileReadFile::size() const
{
    return m_size;
}

int CZipFileReadFile::seek(int pos)
{
    return 0;
}

int CZipFileReadFile::tell() const
{
    return 0;
}

int CZipFileReadFile::read(void* buffer, int size)
{
    ssize_t readsize = size;
    int ret = m_packer->m_zipFile->readData(m_short_fn, (unsigned char*)buffer, &readsize);
    return ret;
}

void CZipFileReadFile::close()
{
    delete this;
}

//////////////////////////////////////////////////////////////
// 打开打包文件
ZipFilePacker::ZipFilePacker()
: m_zipFile(nullptr)
{
    
}

ZipFilePacker::~ZipFilePacker()
{
    close();
}

bool ZipFilePacker::openPackFile(std::string const& filename)
{
    close();
    
    m_zipFile = new ZipFile(filename);
    if (m_zipFile == nullptr) {
        return false;
    }

    m_packageFilename = filename;
    
    // for debug
    // dump();
    return true;
}

std::string ZipFilePacker::packageFilename() const
{
    return m_packageFilename;
}

void ZipFilePacker::close()
{
    delete m_zipFile;
    m_zipFile = nullptr;
}

// 文件数量
int ZipFilePacker::getFileCount() const
{
    return 0;
}
    
    // 是否有文件
bool ZipFilePacker::hasFile(std::string const& filename) const
{
    if (m_zipFile && m_zipFile->fileExists(filename)) {
        return true;
    }
    
    return false;
}

// 创建一个文件读取器
ILightReadFile* ZipFilePacker::createReadFile(std::string const& filename)
{
    CZipFileReadFile* p = new CZipFileReadFile(this);
    if (p->init(filename)) {
        return p;
    }
    
    delete p;
    return NULL;
}

// 解包
int ZipFilePacker::extract(std::string const& destPath)
{
    return -1;
}

// 输出
void ZipFilePacker::dump()
{
    printf("ZIP : begin\n");
    std::string strName = m_zipFile->getFirstFilename();
    while (!strName.empty())
    {
        printf("ZIP : %s\n", strName.c_str());
        strName = m_zipFile->getNextFilename();
    }
    
    return;
}

