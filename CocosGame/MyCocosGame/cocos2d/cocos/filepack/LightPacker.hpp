//
//  LightPacker.hpp
//  xiaoche!!
//
//  Created by liusiyang on 18/03/12.
//  接口类（具体实现详见相关代码）
//  注意，由于这个类是在仓促中完成。并没有对线程安全做足够的测试
//  同时，cocos的unzip这个库也不是thread safe的，所以请先自行加锁
//  
//

#ifndef LightPacker_hpp
#define LightPacker_hpp

#include <string>

class ILightReadFile
{
public:
    virtual int size() const = 0;
    
    virtual int seek(int pos) = 0;
    
    virtual int tell() const = 0;
    
    virtual int read(void* buffer, int size) = 0;
    
    virtual void close() = 0;
};


// 打包文件入口
class ILightPackage
{
public:
    
    // 打开打包文件
    virtual bool openPackFile(std::string const& filename) = 0;
    virtual std::string packageFilename() const = 0;
    virtual void close() = 0;
    
    // 文件数量
    virtual int getFileCount() const = 0;
    
    // 是否有文件
    virtual bool hasFile(std::string const& filename) const = 0;
    
    // 创建一个文件读取器
    virtual ILightReadFile* createReadFile(std::string const& filename) = 0;
    
    // 解包
    virtual int extract(std::string const& destPath) = 0;
};

// zip打包器
#define LIGHT_PACKER_ZIP       0
// 自定义打包器
#define LIGHT_PACKER_CUSTOM    1

// 创建及销毁打包器
ILightPackage* createLightPackage(int type);
void destroyLightPackage(ILightPackage* package);



#endif /* LightPacker_hpp */
