//
//  ZipPackerImpl.hpp
//  xiaoche!!
//
//  Created by liusiyang on 18/03/13.
//  创建一个ZIP包系统
//

#ifndef ZipPackerImpl_hpp
#define ZipPackerImpl_hpp

#include "LightPacker.hpp"
#include "base/ZipUtils.h"

namespace cocos2d
{
    
class ZipFilePacker;
class CZipFileReadFile : public ILightReadFile
{
public:
    CZipFileReadFile(ZipFilePacker* packer);
    ~CZipFileReadFile();
    
    bool init(std::string const& short_fn);
    
    virtual int size() const;
    
    virtual int seek(int pos);
    
    virtual int tell() const;
    
    virtual int read(void* buffer, int size);
    
    virtual void close();
    
private:
    ZipFilePacker* m_packer;
    std::string m_short_fn;
    int m_size;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class ZipFilePacker : public ILightPackage
{
    friend class CZipFileReadFile;
public:
    ZipFilePacker();
    ~ZipFilePacker();
    
    // 打开打包文件
    virtual bool openPackFile(std::string const& filename);
    virtual std::string packageFilename() const;
    virtual void close();
    
    // 文件数量
    virtual int getFileCount() const;
    
    // 是否有文件
    virtual bool hasFile(std::string const& filename) const;
    
    // 创建一个文件读取器
    virtual ILightReadFile* createReadFile(std::string const& filename);
    
    // 解包
    virtual int extract(std::string const& destPath);
    
    // 输出
    void dump();
    
    ZipFile* m_zipFile;
    std::string m_packageFilename;
};

};

#endif /* ZipPackerImpl_hpp */
