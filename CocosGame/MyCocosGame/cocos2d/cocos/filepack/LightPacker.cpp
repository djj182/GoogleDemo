#include "LightPacker.hpp"
#include "LightPackerImpl.hpp"
#include "ZipPackerImpl.hpp"

// 创建及销毁打包器
ILightPackage* createLightPackage(int type)
{
    ILightPackage* ret = nullptr;
    if (type == LIGHT_PACKER_ZIP) {
        ret = new cocos2d::ZipFilePacker;
    }
    else if (type == LIGHT_PACKER_CUSTOM) {
        ret = new LightPacker;
    }
    else {
        
    }
    
    return ret;
}

void destroyLightPackage(ILightPackage* package)
{
    package->close();
    delete package;
}

