//
//  LightPacker.hpp
//  xiaoche!!
//
//  Created by liusiyang on 18/03/11.
//

#ifndef LightPackerImpl_hpp
#define LightPackerImpl_hpp

#include "LightPackInfo.hpp"
#include "LightPacker.hpp"
#include <map>
#include <string>
#include <vector>


class LightPacker;
class CLightReadFile : public ILightReadFile
{
public:
    CLightReadFile(LightPacker* packer);
    
    bool init(std::string const& short_fn);
    
    virtual int size() const;
    
    virtual int seek(int pos);
    
    virtual int tell() const;
    
    virtual int read(void* buffer, int size);
    
    virtual void close();

private:
    LightPacker* m_packer;
    unsigned int m_offset;
    std::string m_short_fn;
    LightFileHeader m_fileHeader;
    int m_virtualPos;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class LightPacker : public ILightPackage
{
    friend class CLightReadFile;
public:
    LightPacker();
    ~LightPacker();
    
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
    
    
private:
    // 读取所有入口
    bool readEntrys();
    
    // 获取entry的入口
    unsigned int getOffset(std::string const& short_fn) const;
    
    // 读取所有的文件名列表
    void readAllFilenames(std::vector<std::string>& files);
    
    // 解包一个文件
    int extractOne(std::string const& destPath, std::string const& short_fn);
    
    // 获取一个LightFileHeader
    bool getLightFileHeader(unsigned int offset, LightFileHeader& fileHeader);
    
    // 读取一组数据
    int readContent(unsigned int offset, void* buffer, int size);

private:
    std::string m_packageFilename;
    LightPackageHeader m_header;
    std::map<unsigned int, unsigned int> m_entrys;
    FILE* m_fp;
};




#endif /* LightPacker_hpp */
