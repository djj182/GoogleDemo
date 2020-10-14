/****************************************************************************
 Copyright (c) 2008-2010 Ricardo Quesada
 Copyright (c) 2010-2012 cocos2d-x.org
 Copyright (c) 2011      Zynga Inc.
 Copyright (c) 2013-2014 Chukong Technologies Inc.
 http://www.cocos2d-x.org
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

// ideas taken from:
//     . The ocean spray in your face [Jeff Lander]
//        http://www.double.co.nz/dust/col0798.pdf
//     . Building an Advanced Particle System [John van der Burg]
//        http://www.gamasutra.com/features/20000623/vanderburg_01.htm
//   . LOVE game engine
//        http://love2d.org/
//
//
// Radius mode support, from 71 squared
//        http://particledesigner.71squared.com/
//
// IMPORTANT: Particle Designer is supported by cocos2d, but
// 'Radius Mode' in Particle Designer uses a fixed emit rate of 30 hz. Since that can't be guaranteed in cocos2d,
//  cocos2d uses a another approach, but the results are almost identical.
//

#include "2d/CCParticleSystem.h"

#include <string>

#include "2d/CCParticleBatchNode.h"
#include "renderer/CCTextureAtlas.h"
#include "base/base64.h"
#include "base/ZipUtils.h"
#include "base/CCDirector.h"
#include "renderer/CCTextureCache.h"
#include "deprecated/CCString.h"
#include "platform/CCFileUtils.h"

#include "platform/CCGL.h"
#include "renderer/CCGLProgram.h"
#include "renderer/CCGLProgramState.h"

using namespace std;

NS_CC_BEGIN

// ideas taken from:
//     . The ocean spray in your face [Jeff Lander]
//        http://www.double.co.nz/dust/col0798.pdf
//     . Building an Advanced Particle System [John van der Burg]
//        http://www.gamasutra.com/features/20000623/vanderburg_01.htm
//   . LOVE game engine
//        http://love2d.org/
//
//
// Radius mode support, from 71 squared
//        http://particledesigner.71squared.com/
//
// IMPORTANT: Particle Designer is supported by cocos2d, but
// 'Radius Mode' in Particle Designer uses a fixed emit rate of 30 hz. Since that can't be guaranteed in cocos2d,
//  cocos2d uses a another approach, but the results are almost identical.
//


inline void nomalize_point(float x, float y, particle_point* out)
{
    float n = x * x + y * y;
    // Already normalized.
    if (n == 1.0f)
        return;
    
    n = sqrt(n);
    // Too close to zero.
    if (n < MATH_TOLERANCE)
        return;
    
    n = 1.0f / n;
    out->x = x * n;
    out->y = y * n;
}

/**
 A more effect random number getter function, get from ejoy2d.
 */
inline static float RANDOM_M11(unsigned int *seed) {
    *seed = *seed * 134775813 + 1;
    union {
        uint32_t d;
        float f;
    } u;
    u.d = (((uint32_t)(*seed) & 0x7fff) << 8) | 0x40000000;
    return u.f - 3.0f;
}

ParticleData::ParticleData()
{
    memset(this, 0, sizeof(ParticleData));
}

bool ParticleData::init(int count)
{
    maxCount = count;
    
    posx= (float*)malloc(count * sizeof(float));
    posy= (float*)malloc(count * sizeof(float));
    startPosX= (float*)malloc(count * sizeof(float));
    startPosY= (float*)malloc(count * sizeof(float));
    colorR= (float*)malloc(count * sizeof(float));
    colorG= (float*)malloc(count * sizeof(float));
    colorB= (float*)malloc(count * sizeof(float));
    colorA= (float*)malloc(count * sizeof(float));
    deltaColorR= (float*)malloc(count * sizeof(float));
    deltaColorG= (float*)malloc(count * sizeof(float));
    deltaColorB= (float*)malloc(count * sizeof(float));
    deltaColorA= (float*)malloc(count * sizeof(float));
    size= (float*)malloc(count * sizeof(float));
    deltaSize= (float*)malloc(count * sizeof(float));
    rotation= (float*)malloc(count * sizeof(float));
    deltaRotation= (float*)malloc(count * sizeof(float));
    timeToLive= (float*)malloc(count * sizeof(float));
    atlasIndex= (unsigned int*)malloc(count * sizeof(unsigned int));
    
    modeA.dirX= (float*)malloc(count * sizeof(float));
    modeA.dirY= (float*)malloc(count * sizeof(float));
    modeA.radialAccel= (float*)malloc(count * sizeof(float));
    modeA.tangentialAccel= (float*)malloc(count * sizeof(float));
    
    modeB.angle= (float*)malloc(count * sizeof(float));
    modeB.degreesPerSecond= (float*)malloc(count * sizeof(float));
    modeB.deltaRadius= (float*)malloc(count * sizeof(float));
    modeB.radius= (float*)malloc(count * sizeof(float));
    
    return posx && posy && startPosY && startPosX && colorR && colorG && colorB && colorA &&
    deltaColorR && deltaColorG && deltaColorB && deltaColorA && size && deltaSize &&
    rotation && deltaRotation && timeToLive && atlasIndex && modeA.dirX && modeA.dirY &&
    modeA.radialAccel && modeA.tangentialAccel && modeB.angle && modeB.degreesPerSecond &&
    modeB.deltaRadius && modeB.radius;
}

void ParticleData::release()
{
    CC_SAFE_FREE(posx);
    CC_SAFE_FREE(posy);
    CC_SAFE_FREE(startPosX);
    CC_SAFE_FREE(startPosY);
    CC_SAFE_FREE(colorR);
    CC_SAFE_FREE(colorG);
    CC_SAFE_FREE(colorB);
    CC_SAFE_FREE(colorA);
    CC_SAFE_FREE(deltaColorR);
    CC_SAFE_FREE(deltaColorG);
    CC_SAFE_FREE(deltaColorB);
    CC_SAFE_FREE(deltaColorA);
    CC_SAFE_FREE(size);
    CC_SAFE_FREE(deltaSize);
    CC_SAFE_FREE(rotation);
    CC_SAFE_FREE(deltaRotation);
    CC_SAFE_FREE(timeToLive);
    CC_SAFE_FREE(atlasIndex);
    
    CC_SAFE_FREE(modeA.dirX);
    CC_SAFE_FREE(modeA.dirY);
    CC_SAFE_FREE(modeA.radialAccel);
    CC_SAFE_FREE(modeA.tangentialAccel);
    
    CC_SAFE_FREE(modeB.angle);
    CC_SAFE_FREE(modeB.degreesPerSecond);
    CC_SAFE_FREE(modeB.deltaRadius);
    CC_SAFE_FREE(modeB.radius);
}

ParticleSystem::ParticleSystem()
: _isBlendAdditive(false)
, _isAutoRemoveOnFinish(false)
, _plistFile("")
, _elapsed(0)
, _configName("")
, _emitCounter(0)
, _batchNode(nullptr)
, _atlasIndex(0)
, _transformSystemDirty(false)
, _allocatedParticles(0)
, _isActive(true)
, _particleCount(0)
, _duration(0)
, _life(0)
, _lifeVar(0)
, _angle(0)
, _angleVar(0)
, _emitterMode(Mode::GRAVITY)
, _startSize(0)
, _startSizeVar(0)
, _endSize(0)
, _endSizeVar(0)
, _startSpin(0)
, _startSpinVar(0)
, _endSpin(0)
, _endSpinVar(0)
, _emissionRate(0)
, _totalParticles(0)
, _texture(nullptr)
, _blendFunc(BlendFunc::ALPHA_PREMULTIPLIED)
, _opacityModifyRGB(false)
, _yCoordFlipped(1)
, _positionType(PositionType::FREE)
, m_initTextureByPlist(true)
{
    modeA.gravity.setZero();
    modeA.speed = 0;
    modeA.speedVar = 0;
    modeA.tangentialAccel = 0;
    modeA.tangentialAccelVar = 0;
    modeA.radialAccel = 0;
    modeA.radialAccelVar = 0;
    modeA.rotationIsDir = false;
    modeB.startRadius = 0;
    modeB.startRadiusVar = 0;
    modeB.endRadius = 0;
    modeB.endRadiusVar = 0;
    modeB.rotatePerSecond = 0;
    modeB.rotatePerSecondVar = 0;
    
    //lixu 20160125 get particle status
    if(Director::getInstance()->isDisplayStats())
    {
        creationCounter++;
//        Director::getInstance()->setGameStatusString(StringUtils::format("Particle system count %lu ",creationCounter));
    }
}
// implementation ParticleSystem

unsigned long ParticleSystem::creationCounter =0;

ParticleSystem * ParticleSystem::create(const std::string& plistFile)
{
    ParticleSystem *ret = new (std::nothrow) ParticleSystem();
    if (ret && ret->initWithFile(plistFile))
    {

        
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return ret;
}

ParticleSystem* ParticleSystem::createWithTotalParticles(int numberOfParticles)
{
    ParticleSystem *ret = new (std::nothrow) ParticleSystem();
    if (ret && ret->initWithTotalParticles(numberOfParticles))
    {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return ret;
}

bool ParticleSystem::init()
{
    return initWithTotalParticles(150);
}

bool ParticleSystem::initWithFile(const std::string& plistFile)
{
    bool ret = false;
    ssize_t __size = 0;
    unsigned char* buffer = nullptr;
    
    __size = ZipUtils::inflateCCZFile(plistFile.c_str(), &buffer);
    if (__size > 0)
    {
        string textureName("");
        string listFilePath = plistFile;
        if (listFilePath.find('/') != string::npos)
        {
            listFilePath = listFilePath.substr(0, listFilePath.rfind('/') + 1);
            textureName = this->initWithData(buffer, listFilePath.c_str());
        }
        else
        {
            textureName = this->initWithData(buffer, "");
        }
        
        if (!textureName.empty())
        {
            ret = true;
        }
    }
    else
    {
        _plistFile = FileUtils::getInstance()->fullPathForFilename(plistFile);
        ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(_plistFile);
        
        CCASSERT( !dict.empty(), "Particles: file not found");
        
        // FIXME: compute path from a path, should define a function somewhere to do it
        string listFilePath = plistFile;
        if (listFilePath.find('/') != string::npos)
        {
            listFilePath = listFilePath.substr(0, listFilePath.rfind('/') + 1);
            ret = this->initWithDictionary(dict, listFilePath.c_str());
        }
        else
        {
            ret = this->initWithDictionary(dict, "");
        }
        
    }
    
    
    
    return ret;
}
std::string ParticleSystem::initWithData(unsigned char* buffer, const char *dirname , int maxParticles)
{
    
    unsigned char* _buffer = buffer;
    
    size_t index = 4;
    int _maxParticles = 0;
    
    string _configName("");
    string textureName("");
    
    float _num1 = *(float*)(_buffer+index);
    _maxParticles = _num1;
    if (maxParticles>0)
    {
        _maxParticles = maxParticles;
    }
    
    // self, not super
    if(this->initWithTotalParticles(_maxParticles))
    {
        // Emitter name in particle designer 2.0
        int num;
        float num1;
        index += 4;
        //        num = *(uint16_t*)(_buffer+index);
        //        num = CC_SWAP_INT16_LITTLE_TO_HOST(num);
        //        printf("maxParticles=%d==num=%d", maxParticles, num);
        num = *(float*)(_buffer+index);
        int configNamelen = num;
        index += 4;
        if (configNamelen >0)
        {
            char tmp[512];
            memset(tmp, 0, sizeof(tmp));
            memcpy(tmp, _buffer+index, configNamelen);
            _configName = string(tmp);
            index += configNamelen;
        }
        
        //        size_t l = sizeof(float);
        //        printf("l=%d", (int)l);
        
        
        //        num1 = *(float*)(_buffer+index);
        //        float _minRadiusVariance = num1;
        
        //        printf("num1=%f", num1);
        
        //        yCoordFlipped
        //        index += 4;
        num1 = *(float*)(_buffer+index);
        _yCoordFlipped = num1 <= 0.0 ? 1 : (int)num1;
        
        //        printf("num1=%f", num1);
        
        //      angle
        index += 4;
        num1 = *(float*)(_buffer+index);
        _angle = num1;
        //        printf("num1=%f", num1);
        
        //        _angleVar
        index += 4;
        num1 = *(float*)(_buffer+index);
        _angleVar = num1;
        //        printf("num1=%f", num1);
        
        //      duration
        index += 4;
        num1 = *(float*)(_buffer+index);
        _duration = num1;
        //        printf("num1=%f", num1);
        
        //      blendFuncSource
        index += 4;
        num1 = *(float*)(_buffer+index);
        //        printf("num1=%f", num1);
        
        if (!_configName.empty())
        {
            _blendFunc.src = static_cast<int>(num1);
        }
        else
        {
            _blendFunc.src = num1;
        }
        
        
        //      _blendFunc.dst
        index += 4;
        num1 = *(float*)(_buffer+index);
        _blendFunc.dst = static_cast<int>(num1);
        //        printf("num1=%f", num1);
        
        
        //      startColorRed
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startColor.r = num1;
        //        printf("num1=%f", num1);
        
        //      startColorGreen
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startColor.g = num1;
        //        printf("num1=%f", num1);
        
        //      startColorBlue
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startColor.b = num1;
        //        printf("num1=%f", num1);
        
        //      startColorAlpha
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startColor.a = num1;
        //        printf("num1=%f", num1);
        
        //      startColorVarianceRed
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startColorVar.r = num1;
        //        printf("num1=%f", num1);
        
        //      startColorVarianceGreen
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startColorVar.g = num1;
        //        printf("num1=%f", num1);
        
        //      startColorVarianceBlue
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startColorVar.b = num1;
        //        printf("num1=%f", num1);
        
        //      startColorVarianceAlpha
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startColorVar.a = num1;
        //        printf("num1=%f", num1);
        
        //      finishColorRed
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endColor.r = num1;
        //        printf("num1=%f", num1);
        
        //      finishColorGreen
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endColor.g = num1;
        //        printf("num1=%f", num1);
        
        //      finishColorBlue
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endColor.b = num1;
        //        printf("num1=%f", num1);
        
        //      finishColorAlpha
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endColor.a = num1;
        //        printf("num1=%f", num1);
        
        //      finishColorVarianceRed
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endColorVar.r = num1;
        //        printf("num1=%f", num1);
        
        //      finishColorVarianceGreen
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endColorVar.g = num1;
        //        printf("num1=%f", num1);
        
        //      finishColorVarianceBlue
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endColorVar.b = num1;
        //        printf("num1=%f", num1);
        
        //      finishColorVarianceAlpha
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endColorVar.a = num1;
        //        printf("num1=%f", num1);
        
        //        // particle size
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startSize = num1;
        //        printf("num1=%f", num1);
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startSizeVar = num1;
        //        printf("num1=%f", num1);
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endSize = num1;
        //        printf("num1=%f", num1);
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endSizeVar = num1;
        //        printf("num1=%f", num1);
        
        
        //        // position
        index += 4;
        num1 = *(float*)(_buffer+index);
        float x = num1;
        //        printf("num1=%f", num1);
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        float y = num1;
        //        printf("num1=%f", num1);
        this->setPosition(Point(x,y) );
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        _posVar.x = num1;
        //        printf("num1=%f", num1);
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        _posVar.y = num1;
        //        printf("num1=%f", num1);
        
        // Spinning
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startSpin = num1;
        //        printf("num1=%f", num1);
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        _startSpinVar = num1;
        //        printf("num1=%f", num1);
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endSpin = num1;
        //        printf("num1=%f", num1);
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        _endSpinVar = num1;
        //        printf("num1=%f", num1);
        
        index += 4;
        num1 = *(float*)(_buffer+index);
        _emitterMode = (Mode) static_cast<int>(num1);
        //        printf("num=%d", num);
        
        index += 4;
        
        if (_emitterMode == ParticleSystem::Mode::GRAVITY)
        {
            //            // gravity
            num1 = *(float*)(_buffer+index);
            modeA.gravity.x = num1;
            //            printf("num1=%f", num1);
            
            index += 4;
            num1 = *(float*)(_buffer+index);
            modeA.gravity.y = num1;
            //            printf("num1=%f", num1);
            
            // speed
            index += 4;
            num1 = *(float*)(_buffer+index);
            modeA.speed = num1;
            //            printf("num1=%f", num1);
            
            index += 4;
            num1 = *(float*)(_buffer+index);
            modeA.speedVar = num1;
            //            printf("num1=%f", num1);
            
            // radial acceleration
            index += 4;
            num1 = *(float*)(_buffer+index);
            modeA.radialAccel = num1;
            //            printf("num1=%f", num1);
            
            index += 4;
            num1 = *(float*)(_buffer+index);
            modeA.radialAccelVar = num1;
            //            printf("num1=%f", num1);
            
            // tangential acceleration
            index += 4;
            num1 = *(float*)(_buffer+index);
            modeA.tangentialAccel = num1;
            //            printf("num1=%f", num1);
            
            index += 4;
            num1 = *(float*)(_buffer+index);
            modeA.tangentialAccelVar = num1;
            //            printf("num1=%f", num1);
            
            index += 4;
            //            num = *(uint16_t*)(_buffer+index);
            //            num = CC_SWAP_INT16_LITTLE_TO_HOST(num);
            num1 = *(float*)(_buffer+index);
            //            modeA.rotationIsDir = (num1>0.0);
            modeA.rotationIsDir = static_cast<bool>(num1);
            //            printf("dir=%d", num);
            
            
            index += 4;
            
        }
        else if (_emitterMode == ParticleSystem::Mode::RADIUS)
        {
            index += 36;
            
            
            num1 = *(float*)(_buffer+index);
            
            
            if (!_configName.empty())
            {
                modeB.startRadius = static_cast<int>(num1);
            }
            else
            {
                modeB.startRadius = num1;
            }
            
            //            printf("num1=%f", num1);
            
            index += 4;
            num1 = *(float*)(_buffer+index);
            modeB.startRadiusVar = num1;
            //            printf("num1=%f", num1);
            
            index += 4;
            num1 = *(float*)(_buffer+index);
            
            if (!_configName.empty())
            {
                modeB.endRadius = static_cast<int>(num1);
            }
            else
            {
                modeB.endRadius = num1;
            }
            
            //            printf("num1=%f", num1);
            
            index += 4;
            num1 = *(float*)(_buffer+index);
            if (num1>0.0)
            {
                modeB.endRadiusVar = num1;
            }
            else
            {
                modeB.endRadiusVar = 0.0f;
            }
            //            printf("num1=%f", num1);
            
            index += 4;
            num1 = *(float*)(_buffer+index);
            
            if (!_configName.empty())
            {
                modeB.rotatePerSecond = static_cast<int>(num1);
            }
            else
            {
                modeB.rotatePerSecond = num1;
            }
            
            
            
            index += 4;
            num1 = *(float*)(_buffer+index);
            modeB.rotatePerSecondVar = num1;
            //            printf("num1=%f", num1);
            
            index += 4;
        }
        
        if (_emitterMode == ParticleSystem::Mode::GRAVITY)
            index += 24;
        
        // life span
        //        m_fLife = dictionary->valueForKey("particleLifespan")->floatValue();
        //        m_fLifeVar = dictionary->valueForKey("particleLifespanVariance")->floatValue();
        //
        //        // emission Rate
        //        m_fEmissionRate = m_uTotalParticles / m_fLife;
        //
        //        //don't get the internal texture if a batchNode is used
        //        if (!m_pBatchNode && m_initTextureByPlist)
        
        
        
        //        particleLifespan
        num1 = *(float*)(_buffer+index);
        _life = num1;
        //        printf("num1=%f", num1);
        
        //        particleLifespanVariance
        index += 4;
        num1 = *(float*)(_buffer+index);
        _lifeVar = num1;
        //        printf("num1=%f", num1);
        
        // emission Rate
        _emissionRate = _totalParticles / _life;
        
        
        //      textureFileName
        index += 4;
        num = *(uint16_t*)(_buffer+index);
        num = CC_SWAP_INT16_LITTLE_TO_HOST(num);
        //        printf("num=%d", num);
        //        num1 = *(float*)(_buffer+index);
        //        printf("-------num1------%f", num1);
        index += 2;
        char tmp[512];
        memset(tmp, 0, sizeof(tmp));
        memcpy(tmp, buffer+index, num);
        //        printf("tmp==%s", tmp);
        
        textureName = std::string(tmp);
        
        
        if (!_batchNode && m_initTextureByPlist)
        {
            // Set a compatible default for the alpha transfer
            _opacityModifyRGB = false;
            
            size_t rPos = textureName.rfind('/');
            
            if (rPos != string::npos)
            {
                string textureDir = textureName.substr(0, rPos + 1);
                
                if (dirname != NULL && textureDir != dirname)
                {
                    textureName = textureName.substr(rPos+1);
                    _texturefileName = textureName;
                    textureName = string(dirname) + textureName;
                }
            }
            else
            {
                if (dirname != NULL && !textureName.empty())
                {
                    _texturefileName = textureName;
                    textureName = string(dirname) + textureName;
                }
            }
            
            Texture2D *tex = NULL;
            
            if (textureName.length() > 0)
            {
                // set not pop-up message box when load image failed
                bool bNotify = FileUtils::sharedFileUtils()->isPopupNotify();
                FileUtils::sharedFileUtils()->setPopupNotify(false);
                tex = TextureCache::sharedTextureCache()->addImage(textureName.c_str());
                // reset the value of UIImage notify
                FileUtils::sharedFileUtils()->setPopupNotify(bNotify);
            }
            
            if(tex)
            {
                setTexture(tex);
            }
            
        }
    }
    
    return textureName;
}
bool ParticleSystem::initWithDictionary(ValueMap& dictionary)
{
    return initWithDictionary(dictionary, "");
}

bool ParticleSystem::initWithDictionary(ValueMap& dictionary, const std::string& dirname)
{
    bool ret = false;
    unsigned char *buffer = nullptr;
    unsigned char *deflated = nullptr;
    Image *image = nullptr;
    do
    {
        int maxParticles = dictionary["maxParticles"].asInt();
        // self, not super
        if(this->initWithTotalParticles(maxParticles))
        {
            // Emitter name in particle designer 2.0
            _configName = dictionary["configName"].asString();
            
            // angle
            _angle = dictionary["angle"].asFloat();
            _angleVar = dictionary["angleVariance"].asFloat();
            
            // duration
            _duration = dictionary["duration"].asFloat();
            
            // blend function
            if (!_configName.empty())
            {
                _blendFunc.src = dictionary["blendFuncSource"].asFloat();
            }
            else
            {
                _blendFunc.src = dictionary["blendFuncSource"].asInt();
            }
            _blendFunc.dst = dictionary["blendFuncDestination"].asInt();
            
            // color
            _startColor.r = dictionary["startColorRed"].asFloat();
            _startColor.g = dictionary["startColorGreen"].asFloat();
            _startColor.b = dictionary["startColorBlue"].asFloat();
            _startColor.a = dictionary["startColorAlpha"].asFloat();
            
            _startColorVar.r = dictionary["startColorVarianceRed"].asFloat();
            _startColorVar.g = dictionary["startColorVarianceGreen"].asFloat();
            _startColorVar.b = dictionary["startColorVarianceBlue"].asFloat();
            _startColorVar.a = dictionary["startColorVarianceAlpha"].asFloat();
            
            _endColor.r = dictionary["finishColorRed"].asFloat();
            _endColor.g = dictionary["finishColorGreen"].asFloat();
            _endColor.b = dictionary["finishColorBlue"].asFloat();
            _endColor.a = dictionary["finishColorAlpha"].asFloat();
            
            _endColorVar.r = dictionary["finishColorVarianceRed"].asFloat();
            _endColorVar.g = dictionary["finishColorVarianceGreen"].asFloat();
            _endColorVar.b = dictionary["finishColorVarianceBlue"].asFloat();
            _endColorVar.a = dictionary["finishColorVarianceAlpha"].asFloat();
            
            // particle size
            _startSize = dictionary["startParticleSize"].asFloat();
            _startSizeVar = dictionary["startParticleSizeVariance"].asFloat();
            _endSize = dictionary["finishParticleSize"].asFloat();
            _endSizeVar = dictionary["finishParticleSizeVariance"].asFloat();
            
            // position
            float x = dictionary["sourcePositionx"].asFloat();
            float y = dictionary["sourcePositiony"].asFloat();
            this->setPosition(x,y);
            _posVar.x = dictionary["sourcePositionVariancex"].asFloat();
            _posVar.y = dictionary["sourcePositionVariancey"].asFloat();
            
            // Spinning
            _startSpin = dictionary["rotationStart"].asFloat();
            _startSpinVar = dictionary["rotationStartVariance"].asFloat();
            _endSpin= dictionary["rotationEnd"].asFloat();
            _endSpinVar= dictionary["rotationEndVariance"].asFloat();
            
            _emitterMode = (Mode) dictionary["emitterType"].asInt();
            
            // Mode A: Gravity + tangential accel + radial accel
            if (_emitterMode == Mode::GRAVITY)
            {
                // gravity
                modeA.gravity.x = dictionary["gravityx"].asFloat();
                modeA.gravity.y = dictionary["gravityy"].asFloat();
                
                // speed
                modeA.speed = dictionary["speed"].asFloat();
                modeA.speedVar = dictionary["speedVariance"].asFloat();
                
                // radial acceleration
                modeA.radialAccel = dictionary["radialAcceleration"].asFloat();
                modeA.radialAccelVar = dictionary["radialAccelVariance"].asFloat();
                
                // tangential acceleration
                modeA.tangentialAccel = dictionary["tangentialAcceleration"].asFloat();
                modeA.tangentialAccelVar = dictionary["tangentialAccelVariance"].asFloat();
                
                // rotation is dir
                modeA.rotationIsDir = dictionary["rotationIsDir"].asBool();
            }
            
            // or Mode B: radius movement
            else if (_emitterMode == Mode::RADIUS)
            {
                if (!_configName.empty())
                {
                    modeB.startRadius = dictionary["maxRadius"].asInt();
                }
                else
                {
                    modeB.startRadius = dictionary["maxRadius"].asFloat();
                }
                modeB.startRadiusVar = dictionary["maxRadiusVariance"].asFloat();
                if (!_configName.empty())
                {
                    modeB.endRadius = dictionary["minRadius"].asInt();
                }
                else
                {
                    modeB.endRadius = dictionary["minRadius"].asFloat();
                }
                
                if (dictionary.find("minRadiusVariance") != dictionary.end())
                {
                    modeB.endRadiusVar = dictionary["minRadiusVariance"].asFloat();
                }
                else
                {
                    modeB.endRadiusVar = 0.0f;
                }
                
                if (!_configName.empty())
                {
                    modeB.rotatePerSecond = dictionary["rotatePerSecond"].asInt();
                }
                else
                {
                    modeB.rotatePerSecond = dictionary["rotatePerSecond"].asFloat();
                }
                modeB.rotatePerSecondVar = dictionary["rotatePerSecondVariance"].asFloat();
                
            } else {
                CCASSERT( false, "Invalid emitterType in config file");
                CC_BREAK_IF(true);
            }
            
            // life span
            _life = dictionary["particleLifespan"].asFloat();
            _lifeVar = dictionary["particleLifespanVariance"].asFloat();
            
            // emission Rate
            _emissionRate = _totalParticles / _life;
            
            //don't get the internal texture if a batchNode is used
            if (!_batchNode&&m_initTextureByPlist)
            {
                // Set a compatible default for the alpha transfer
                _opacityModifyRGB = false;
                
                // texture
                // Try to get the texture from the cache
                std::string textureName = dictionary["textureFileName"].asString();
                
                size_t rPos = textureName.rfind('/');
                
                if (rPos != string::npos)
                {
                    string textureDir = textureName.substr(0, rPos + 1);
                    
                    if (!dirname.empty() && textureDir != dirname)
                    {
                        textureName = textureName.substr(rPos+1);
                        // Added by ChenFei 2014-12-26 V3.2 supports
                        _texturefileName = textureName;
                        textureName = dirname + textureName;
                    }
                }
                else if (!dirname.empty() && !textureName.empty())
                {
                    // Added by ChenFei 2014-12-26 V3.2 supports
                    _texturefileName = textureName;
                    textureName = dirname + textureName;
                }
                
                Texture2D *tex = nullptr;
                
                if (!textureName.empty())
                {
                    // set not pop-up message box when load image failed
                    bool notify = FileUtils::getInstance()->isPopupNotify();
                    FileUtils::getInstance()->setPopupNotify(false);
                    Director::getInstance()->getTextureCache()->setUseDefaultTexture(false);
                    tex = Director::getInstance()->getTextureCache()->addImage(textureName);
                    Director::getInstance()->getTextureCache()->setUseDefaultTexture(true);
                    // reset the value of UIImage notify
                    FileUtils::getInstance()->setPopupNotify(notify);
                }
                
                if (tex)
                {
                    setTexture(tex);
                }
                else if( dictionary.find("textureImageData") != dictionary.end() )
                {
                    std::string textureData = dictionary.at("textureImageData").asString();
                    CCASSERT(!textureData.empty(), "textureData can't be empty!");
                    
                    auto dataLen = textureData.size();
                    if (dataLen != 0)
                    {
                        // if it fails, try to get it from the base64-gzipped data
                        int decodeLen = base64Decode((unsigned char*)textureData.c_str(), (unsigned int)dataLen, &buffer);
                        CCASSERT( buffer != nullptr, "CCParticleSystem: error decoding textureImageData");
                        CC_BREAK_IF(!buffer);
                        
                        ssize_t deflatedLen = ZipUtils::inflateMemory(buffer, decodeLen, &deflated);
                        CCASSERT( deflated != nullptr, "CCParticleSystem: error ungzipping textureImageData");
                        CC_BREAK_IF(!deflated);
                        
                        // For android, we should retain it in VolatileTexture::addImage which invoked in Director::getInstance()->getTextureCache()->addUIImage()
                        image = new (std::nothrow) Image();
                        bool isOK = image->initWithImageData(deflated, deflatedLen);
                        CCASSERT(isOK, "CCParticleSystem: error init image with Data");
                        CC_BREAK_IF(!isOK);
                        
                        setTexture(Director::getInstance()->getTextureCache()->addImage(image, textureName.c_str()));
                        
                        image->release();
                    }
                }
                
                _yCoordFlipped = dictionary.find("yCoordFlipped") == dictionary.end() ? 1 : dictionary.at("yCoordFlipped").asInt();
                
                if( !this->_texture)
                    CCLOGWARN("cocos2d: Warning: ParticleSystemQuad system without a texture");
            }
            ret = true;
        }
    } while (0);
    free(buffer);
    free(deflated);
    return ret;
}

bool ParticleSystem::initWithTotalParticles(int numberOfParticles)
{
    _totalParticles = numberOfParticles;
    
    _particleData.release();
    
    if( !_particleData.init(_totalParticles) )
    {
        CCLOG("Particle system: not enough memory");
        this->release();
        return false;
    }
    _allocatedParticles = numberOfParticles;
    
    if (_batchNode)
    {
        for (int i = 0; i < _totalParticles; i++)
        {
            _particleData.atlasIndex[i] = i;
        }
    }
    // default, active
    _isActive = true;
    
    // default blend function
    _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
    
    // default movement type;
    _positionType = PositionType::FREE;
    
    // by default be in mode A:
    _emitterMode = Mode::GRAVITY;
    
    // default: modulate
    // FIXME:: not used
    //    colorModulate = YES;
    
    _isAutoRemoveOnFinish = false;
    
    // Optimization: compile updateParticle method
    //updateParticleSel = @selector(updateQuadWithParticle:newPosition:);
    //updateParticleImp = (CC_UPDATE_PARTICLE_IMP) [self methodForSelector:updateParticleSel];
    //for batchNode
    _transformSystemDirty = false;
    
    return true;
}

ParticleSystem::~ParticleSystem()
{
    // Since the scheduler retains the "target (in this case the ParticleSystem)
    // it is not needed to call "unscheduleUpdate" here. In fact, it will be called in "cleanup"
    //unscheduleUpdate();
    _particleData.release();
    CC_SAFE_RELEASE(_texture);
    creationCounter--;
    //lixu 20160125 get particle status
    if(Director::getInstance()->isDisplayStats())
    {
        creationCounter++;
//        Director::getInstance()->setGameStatusString(StringUtils::format("Particle system count %lu current pars:%d",creationCounter,_particleCount));
    }
}

void ParticleSystem::addParticles(int count)
{
    uint32_t RANDSEED = rand();
    
    int start = _particleCount;
    _particleCount += count;
    
    //life
    for (int i = start; i < _particleCount ; ++i)
    {
        float theLife = _life + _lifeVar * RANDOM_M11(&RANDSEED);
        _particleData.timeToLive[i] = MAX(0, theLife);
    }
    
    //position
    for (int i = start; i < _particleCount; ++i)
    {
        _particleData.posx[i] = _sourcePosition.x + _posVar.x * RANDOM_M11(&RANDSEED);
    }
    
    for (int i = start; i < _particleCount; ++i)
    {
        _particleData.posy[i] = _sourcePosition.y + _posVar.y * RANDOM_M11(&RANDSEED);
    }
    
    //color
#define SET_COLOR(c, b, v)\
for (int i = start; i < _particleCount; ++i)\
{\
c[i] = clampf( b + v * RANDOM_M11(&RANDSEED) , 0 , 1 );\
}
    
    SET_COLOR(_particleData.colorR, _startColor.r, _startColorVar.r);
    SET_COLOR(_particleData.colorG, _startColor.g, _startColorVar.g);
    SET_COLOR(_particleData.colorB, _startColor.b, _startColorVar.b);
    SET_COLOR(_particleData.colorA, _startColor.a, _startColorVar.a);
    
    SET_COLOR(_particleData.deltaColorR, _endColor.r, _endColorVar.r);
    SET_COLOR(_particleData.deltaColorG, _endColor.g, _endColorVar.g);
    SET_COLOR(_particleData.deltaColorB, _endColor.b, _endColorVar.b);
    SET_COLOR(_particleData.deltaColorA, _endColor.a, _endColorVar.a);
    
#define SET_DELTA_COLOR(c, dc)\
for (int i = start; i < _particleCount; ++i)\
{\
dc[i] = (dc[i] - c[i]) / _particleData.timeToLive[i];\
}
    
    SET_DELTA_COLOR(_particleData.colorR, _particleData.deltaColorR);
    SET_DELTA_COLOR(_particleData.colorG, _particleData.deltaColorG);
    SET_DELTA_COLOR(_particleData.colorB, _particleData.deltaColorB);
    SET_DELTA_COLOR(_particleData.colorA, _particleData.deltaColorA);
    
    //size
    for (int i = start; i < _particleCount; ++i)
    {
        _particleData.size[i] = _startSize + _startSizeVar * RANDOM_M11(&RANDSEED);
        _particleData.size[i] = MAX(0, _particleData.size[i]);
    }
    
    if (_endSize != START_SIZE_EQUAL_TO_END_SIZE)
    {
        for (int i = start; i < _particleCount; ++i)
        {
            float endSize = _endSize + _endSizeVar * RANDOM_M11(&RANDSEED);
            endSize = MAX(0, endSize);
            _particleData.deltaSize[i] = (endSize - _particleData.size[i]) / _particleData.timeToLive[i];
        }
    }
    else
    {
        for (int i = start; i < _particleCount; ++i)
        {
            _particleData.deltaSize[i] = 0.0f;
        }
    }
    
    // rotation
    for (int i = start; i < _particleCount; ++i)
    {
        _particleData.rotation[i] = _startSpin + _startSpinVar * RANDOM_M11(&RANDSEED);
    }
    for (int i = start; i < _particleCount; ++i)
    {
        float endA = _endSpin + _endSpinVar * RANDOM_M11(&RANDSEED);
        _particleData.deltaRotation[i] = (endA - _particleData.rotation[i]) / _particleData.timeToLive[i];
    }
    
    // position
    Vec2 pos;
    if (_positionType == PositionType::FREE)
    {
        pos = this->convertToWorldSpace(Vec2::ZERO);
    }
    else if (_positionType == PositionType::RELATIVE)
    {
        pos = _position;
    }
    for (int i = start; i < _particleCount; ++i)
    {
        _particleData.startPosX[i] = pos.x;
    }
    for (int i = start; i < _particleCount; ++i)
    {
        _particleData.startPosY[i] = pos.y;
    }
    
    // Mode Gravity: A
    if (_emitterMode == Mode::GRAVITY)
    {
        
        // radial accel
        for (int i = start; i < _particleCount; ++i)
        {
            _particleData.modeA.radialAccel[i] = modeA.radialAccel + modeA.radialAccelVar * RANDOM_M11(&RANDSEED);
        }
        
        // tangential accel
        for (int i = start; i < _particleCount; ++i)
        {
            _particleData.modeA.tangentialAccel[i] = modeA.tangentialAccel + modeA.tangentialAccelVar * RANDOM_M11(&RANDSEED);
        }
        
        // rotation is dir
        if( modeA.rotationIsDir )
        {
            for (int i = start; i < _particleCount; ++i)
            {
                float a = CC_DEGREES_TO_RADIANS( _angle + _angleVar * RANDOM_M11(&RANDSEED) );
                Vec2 v(cosf( a ), sinf( a ));
                float s = modeA.speed + modeA.speedVar * RANDOM_M11(&RANDSEED);
                Vec2 dir = v * s;
                _particleData.modeA.dirX[i] = dir.x;//v * s ;
                _particleData.modeA.dirY[i] = dir.y;
                _particleData.rotation[i] = -CC_RADIANS_TO_DEGREES(dir.getAngle());
            }
        }
        else
        {
            for (int i = start; i < _particleCount; ++i)
            {
                float a = CC_DEGREES_TO_RADIANS( _angle + _angleVar * RANDOM_M11(&RANDSEED) );
                Vec2 v(cosf( a ), sinf( a ));
                float s = modeA.speed + modeA.speedVar * RANDOM_M11(&RANDSEED);
                Vec2 dir = v * s;
                _particleData.modeA.dirX[i] = dir.x;//v * s ;
                _particleData.modeA.dirY[i] = dir.y;
            }
        }
        
    }
    
    // Mode Radius: B
    else
    {
        //Need to check by Jacky
        // Set the default diameter of the particle from the source position
        for (int i = start; i < _particleCount; ++i)
        {
            _particleData.modeB.radius[i] = modeB.startRadius + modeB.startRadiusVar * RANDOM_M11(&RANDSEED);
        }
        
        for (int i = start; i < _particleCount; ++i)
        {
            _particleData.modeB.angle[i] = CC_DEGREES_TO_RADIANS( _angle + _angleVar * RANDOM_M11(&RANDSEED));
        }
        
        for (int i = start; i < _particleCount; ++i)
        {
            _particleData.modeB.degreesPerSecond[i] = CC_DEGREES_TO_RADIANS(modeB.rotatePerSecond + modeB.rotatePerSecondVar * RANDOM_M11(&RANDSEED));
        }
        
        if(modeB.endRadius == START_RADIUS_EQUAL_TO_END_RADIUS)
        {
            for (int i = start; i < _particleCount; ++i)
            {
                _particleData.modeB.deltaRadius[i] = 0.0f;
            }
        }
        else
        {
            for (int i = start; i < _particleCount; ++i)
            {
                float endRadius = modeB.endRadius + modeB.endRadiusVar * RANDOM_M11(&RANDSEED);
                _particleData.modeB.deltaRadius[i] = (endRadius - _particleData.modeB.radius[i]) / _particleData.timeToLive[i];
            }
        }
    }
}

void ParticleSystem::onEnter()
{
#if CC_ENABLE_SCRIPT_BINDING
    if (_scriptType == kScriptTypeJavascript)
    {
        if (ScriptEngineManager::sendNodeEventToJSExtended(this, kNodeOnEnter))
            return;
    }
#endif
    
    Node::onEnter();
    
    // update after action in run!
    this->scheduleUpdateWithPriority(1);
}

void ParticleSystem::onExit()
{
#if CC_ENABLE_SCRIPT_BINDING
    if (_scriptType == kScriptTypeJavascript)
    {
        if (ScriptEngineManager::sendNodeEventToJSExtended(this, kNodeOnExit))
            return;
    }
#endif
    
    this->unscheduleUpdate();
    Node::onExit();
}

void ParticleSystem::stopSystem()
{
    _isActive = false;
    _elapsed = _duration;
    _emitCounter = 0;
}

void ParticleSystem::resetSystem()
{
    _isActive = true;
    _elapsed = 0;
    for (int i = 0; i < _particleCount; ++i)
    {
        _particleData.timeToLive[i] = 0.0f;
    }
}

bool ParticleSystem::isFull()
{
    return (_particleCount == _totalParticles);
}

// ParticleSystem - MainLoop
void ParticleSystem::update(float dt)
{
    THREAD_PERF_NODE("ParticleSystem::update");
    CC_PROFILER_START_CATEGORY(kProfilerCategoryParticles , "CCParticleSystem - update");
    
    if (_isActive && _emissionRate)
    {
        float rate = 1.0f / _emissionRate;
        //issue #1201, prevent bursts of particles, due to too high emitCounter
        if (_particleCount < _totalParticles)
        {
            _emitCounter += dt;
            if (_emitCounter < 0.f)
                _emitCounter = 0.f;
        }
        
        int emitCount = MIN(_totalParticles - _particleCount, _emitCounter / rate);
        addParticles(emitCount);
        _emitCounter -= rate * emitCount;
        
        _elapsed += dt;
        if (_elapsed < 0.f)
            _elapsed = 0.f;
        if (_duration != DURATION_INFINITY && _duration < _elapsed)
        {
            this->stopSystem();
        }
    }
    
    {
        for (int i = 0; i < _particleCount; ++i)
        {
            _particleData.timeToLive[i] -= dt;
        }
        
        for (int i = 0; i < _particleCount; ++i)
        {
            if (_particleData.timeToLive[i] <= 0.0f)
            {
                int j = _particleCount - 1;
                while (j > 0 && _particleData.timeToLive[j] <= 0)
                {
                    _particleCount--;
                    j--;
                }
                _particleData.copyParticle(i, _particleCount - 1);
                if (_batchNode)
                {
                    //disable the switched particle
                    int currentIndex = _particleData.atlasIndex[i];
                    _batchNode->disableParticle(_atlasIndex + currentIndex);
                    //switch indexes
                    _particleData.atlasIndex[_particleCount - 1] = currentIndex;
                }
                --_particleCount;
                if( _duration != DURATION_INFINITY && _particleCount == 0 && _isAutoRemoveOnFinish )
                {
                    this->unscheduleUpdate();
                    _parent->removeChild(this, true);
                    return;
                }
            }
        }
        
        if (_emitterMode == Mode::GRAVITY)
        {
            for (int i = 0 ; i < _particleCount; ++i)
            {
                particle_point tmp, radial = {0.0f, 0.0f}, tangential;
                
                // radial acceleration
                if (_particleData.posx[i] || _particleData.posy[i])
                {
                    nomalize_point(_particleData.posx[i], _particleData.posy[i], &radial);
                }
                tangential = radial;
                radial.x *= _particleData.modeA.radialAccel[i];
                radial.y *= _particleData.modeA.radialAccel[i];
                
                // tangential acceleration
                std::swap(tangential.x, tangential.y);
                tangential.x *= - _particleData.modeA.tangentialAccel[i];
                tangential.y *= _particleData.modeA.tangentialAccel[i];
                
                // (gravity + radial + tangential) * dt
                tmp.x = radial.x + tangential.x + modeA.gravity.x;
                tmp.y = radial.y + tangential.y + modeA.gravity.y;
                tmp.x *= dt;
                tmp.y *= dt;
                
                _particleData.modeA.dirX[i] += tmp.x;
                _particleData.modeA.dirY[i] += tmp.y;
                
                // this is cocos2d-x v3.0
                // if (_configName.length()>0 && _yCoordFlipped != -1)
                
                // this is cocos2d-x v3.0
                tmp.x = _particleData.modeA.dirX[i] * dt * _yCoordFlipped;
                tmp.y = _particleData.modeA.dirY[i] * dt * _yCoordFlipped;
                _particleData.posx[i] += tmp.x;
                _particleData.posy[i] += tmp.y;
            }
        }
        else
        {
            for (int i = 0; i < _particleCount; ++i)
            {
                _particleData.modeB.angle[i] += _particleData.modeB.degreesPerSecond[i] * dt;
            }
            
            for (int i = 0; i < _particleCount; ++i)
            {
                _particleData.modeB.radius[i] += _particleData.modeB.deltaRadius[i] * dt;
            }
            
            for (int i = 0; i < _particleCount; ++i)
            {
                _particleData.posx[i] = - cosf(_particleData.modeB.angle[i]) * _particleData.modeB.radius[i];
            }
            for (int i = 0; i < _particleCount; ++i)
            {
                _particleData.posy[i] = - sinf(_particleData.modeB.angle[i]) * _particleData.modeB.radius[i] * _yCoordFlipped;
            }
        }
        
        //color r,g,b,a
        for (int i = 0 ; i < _particleCount; ++i)
        {
            _particleData.colorR[i] += _particleData.deltaColorR[i] * dt;
        }
        
        for (int i = 0 ; i < _particleCount; ++i)
        {
            _particleData.colorG[i] += _particleData.deltaColorG[i] * dt;
        }
        
        for (int i = 0 ; i < _particleCount; ++i)
        {
            _particleData.colorB[i] += _particleData.deltaColorB[i] * dt;
        }
        
        for (int i = 0 ; i < _particleCount; ++i)
        {
            _particleData.colorA[i] += _particleData.deltaColorA[i] * dt;
        }
        //size
        for (int i = 0 ; i < _particleCount; ++i)
        {
            _particleData.size[i] += (_particleData.deltaSize[i] * dt);
            _particleData.size[i] = MAX(0, _particleData.size[i]);
        }
        //angle
        for (int i = 0 ; i < _particleCount; ++i)
        {
            _particleData.rotation[i] += _particleData.deltaRotation[i] * dt;
        }
        
        updateParticleQuads();
        _transformSystemDirty = false;
    }
    
    // only update gl buffer when visible
    if (_visible && ! _batchNode)
    {
        postStep();
    }
    
    CC_PROFILER_STOP_CATEGORY(kProfilerCategoryParticles , "CCParticleSystem - update");
}

void ParticleSystem::updateWithNoTime(void)
{
    this->update(0.0f);
}

void ParticleSystem::updateParticleQuads()
{
    //should be overridden
}

void ParticleSystem::postStep()
{
    // should be overridden
}

// ParticleSystem - Texture protocol
void ParticleSystem::setTexture(Texture2D* var)
{
    if (_texture != var)
    {
        CC_SAFE_RETAIN(var);
        CC_SAFE_RELEASE(_texture);
        _texture = var;
        // Added by ChenFei 2014-12-26 V3.2 supports
        if(_texture && _texture->hasAlphaName())
        {
            // shader state
            setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_ETC1_A_NO_MVP));
        }
        else
        {
            // shader state
            setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));

        }
        updateBlendFunc();
    }
}

void ParticleSystem::updateBlendFunc()
{
    CCASSERT(! _batchNode, "Can't change blending functions when the particle is being batched");
    
    if(_texture)
    {
        bool premultiplied = _texture->hasPremultipliedAlpha();
        
        _opacityModifyRGB = false;
        
        if( _texture && ( _blendFunc.src == CC_BLEND_SRC && _blendFunc.dst == CC_BLEND_DST ) )
        {
            if( premultiplied )
            {
                _opacityModifyRGB = true;
            }
            else
            {
                _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
            }
        }
    }
}

Texture2D * ParticleSystem::getTexture() const
{
    return _texture;
}

// ParticleSystem - Additive Blending
void ParticleSystem::setBlendAdditive(bool additive)
{
    if( additive )
    {
        _blendFunc = BlendFunc::ADDITIVE;
    }
    else
    {
        if( _texture && ! _texture->hasPremultipliedAlpha() )
            _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
        else
            _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
    }
}

bool ParticleSystem::isBlendAdditive() const
{
    return( _blendFunc.src == GL_SRC_ALPHA && _blendFunc.dst == GL_ONE);
}

// ParticleSystem - Properties of Gravity Mode
void ParticleSystem::setTangentialAccel(float t)
{
    CCASSERT( _emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    modeA.tangentialAccel = t;
}

float ParticleSystem::getTangentialAccel() const
{
    CCASSERT( _emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    return modeA.tangentialAccel;
}

void ParticleSystem::setTangentialAccelVar(float t)
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    modeA.tangentialAccelVar = t;
}

float ParticleSystem::getTangentialAccelVar() const
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    return modeA.tangentialAccelVar;
}

void ParticleSystem::setRadialAccel(float t)
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    modeA.radialAccel = t;
}

float ParticleSystem::getRadialAccel() const
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    return modeA.radialAccel;
}

void ParticleSystem::setRadialAccelVar(float t)
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    modeA.radialAccelVar = t;
}

float ParticleSystem::getRadialAccelVar() const
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    return modeA.radialAccelVar;
}

void ParticleSystem::setRotationIsDir(bool t)
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    modeA.rotationIsDir = t;
}

bool ParticleSystem::getRotationIsDir() const
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    return modeA.rotationIsDir;
}

void ParticleSystem::setGravity(const Vec2& g)
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    modeA.gravity = g;
}

const Vec2& ParticleSystem::getGravity()
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    return modeA.gravity;
}

void ParticleSystem::setSpeed(float speed)
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    modeA.speed = speed;
}

float ParticleSystem::getSpeed() const
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    return modeA.speed;
}

void ParticleSystem::setSpeedVar(float speedVar)
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    modeA.speedVar = speedVar;
}

float ParticleSystem::getSpeedVar() const
{
    CCASSERT(_emitterMode == Mode::GRAVITY, "Particle Mode should be Gravity");
    return modeA.speedVar;
}

// ParticleSystem - Properties of Radius Mode
void ParticleSystem::setStartRadius(float startRadius)
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    modeB.startRadius = startRadius;
}

float ParticleSystem::getStartRadius() const
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    return modeB.startRadius;
}

void ParticleSystem::setStartRadiusVar(float startRadiusVar)
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    modeB.startRadiusVar = startRadiusVar;
}

float ParticleSystem::getStartRadiusVar() const
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    return modeB.startRadiusVar;
}

void ParticleSystem::setEndRadius(float endRadius)
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    modeB.endRadius = endRadius;
}

float ParticleSystem::getEndRadius() const
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    return modeB.endRadius;
}

void ParticleSystem::setEndRadiusVar(float endRadiusVar)
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    modeB.endRadiusVar = endRadiusVar;
}

float ParticleSystem::getEndRadiusVar() const
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    return modeB.endRadiusVar;
}

void ParticleSystem::setRotatePerSecond(float degrees)
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    modeB.rotatePerSecond = degrees;
}

float ParticleSystem::getRotatePerSecond() const
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    return modeB.rotatePerSecond;
}

void ParticleSystem::setRotatePerSecondVar(float degrees)
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    modeB.rotatePerSecondVar = degrees;
}

float ParticleSystem::getRotatePerSecondVar() const
{
    CCASSERT(_emitterMode == Mode::RADIUS, "Particle Mode should be Radius");
    return modeB.rotatePerSecondVar;
}

bool ParticleSystem::isActive() const
{
    return _isActive;
}

int ParticleSystem::getTotalParticles() const
{
    return _totalParticles;
}

void ParticleSystem::setTotalParticles(int var)
{
    CCASSERT( var <= _allocatedParticles, "Particle: resizing particle array only supported for quads");
    _totalParticles = var;
}

const BlendFunc& ParticleSystem::getBlendFunc() const
{
    return _blendFunc;
}

void ParticleSystem::setBlendFunc(const BlendFunc &blendFunc)
{
    if( _blendFunc.src != blendFunc.src || _blendFunc.dst != blendFunc.dst ) {
        _blendFunc = blendFunc;
        this->updateBlendFunc();
    }
}

bool ParticleSystem::isAutoRemoveOnFinish() const
{
    return _isAutoRemoveOnFinish;
}

void ParticleSystem::setAutoRemoveOnFinish(bool var)
{
    _isAutoRemoveOnFinish = var;
}


// ParticleSystem - methods for batchNode rendering

ParticleBatchNode* ParticleSystem::getBatchNode(void) const
{
    return _batchNode;
}

void ParticleSystem::setBatchNode(ParticleBatchNode* batchNode)
{
    if( _batchNode != batchNode ) {
        
        _batchNode = batchNode; // weak reference
        
        if( batchNode ) {
            //each particle needs a unique index
            for (int i = 0; i < _totalParticles; i++)
            {
                _particleData.atlasIndex[i] = i;
            }
        }
    }
}

//don't use a transform matrix, this is faster
void ParticleSystem::setScale(float s)
{
    _transformSystemDirty = true;
    Node::setScale(s);
}

void ParticleSystem::setRotation(float newRotation)
{
    _transformSystemDirty = true;
    Node::setRotation(newRotation);
}

void ParticleSystem::setScaleX(float newScaleX)
{
    _transformSystemDirty = true;
    Node::setScaleX(newScaleX);
}

void ParticleSystem::setScaleY(float newScaleY)
{
    _transformSystemDirty = true;
    Node::setScaleY(newScaleY);
}


NS_CC_END
