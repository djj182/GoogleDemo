/****************************************************************************
 Copyright (c) 2013-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
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

#include "renderer/CCTrianglesCommand.h"
#include "renderer/ccGLStateCache.h"
#include "renderer/CCGLProgram.h"
#include "renderer/CCGLProgramState.h"
#include "xxhash.h"
#include "renderer/CCRenderer.h"
#include "base/CCDirector.h"
NS_CC_BEGIN

TrianglesCommand::TrianglesCommand()
:_materialID(0)
,_textureID(0)
// Added by ChenFei 2014-12-26 V3.2 supports
,_textureID2(0)
,_glProgramState(nullptr)
,_glProgram(nullptr)
,_blendType(BlendFunc::DISABLE)
,_isUseOrthographicProjecion(false)
{
    _type = RenderCommand::Type::TRIANGLES_COMMAND;
}

void TrianglesCommand::init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles,const Mat4& mv, uint32_t flags,bool isOrthoPro)
{
    CCASSERT(glProgramState, "Invalid GLProgramState");
    CCASSERT(glProgramState->getVertexAttribsFlags() == 0, "No custom attributes are supported in QuadCommand");

    RenderCommand::init(globalOrder, mv, flags);

    _triangles = triangles;
    if(_triangles.indexCount % 3 != 0)
    {
        int count = _triangles.indexCount;
        _triangles.indexCount = count / 3 * 3;
//        CCLOGERROR("Resize indexCount from %zd to %zd, size must be multiple times of 3", count, _triangles.indexCount);
    }
    _mv = mv;
    
    if( _textureID != textureID || _blendType.src != blendType.src || _blendType.dst != blendType.dst ||
       _glProgramState != glProgramState ||
       _glProgram != glProgramState->getGLProgram() ||
       _isUseOrthographicProjecion != isOrthoPro)
    {
        _textureID = textureID;
        _blendType = blendType;
        _glProgramState = glProgramState;
        _glProgram = glProgramState->getGLProgram();
        _isUseOrthographicProjecion = isOrthoPro;
        generateMaterialID();
    }
}

void TrianglesCommand::init(float globalOrder, GLuint textureID, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles,const Mat4& mv)
{
    init(globalOrder, textureID, glProgramState, blendType, triangles, mv, 0);
}

void TrianglesCommand::init(float globalOrder, Texture2D* texture, GLProgramState* glProgramState, BlendFunc blendType, const Triangles& triangles, const Mat4& mv, uint32_t flags)
{
//    init(globalOrder, texture->getName(), glProgramState, blendType, triangles, mv, 0);
    assert(0);
//    init(globalOrder, texture->getName(), glProgramState, blendType, triangles, mv, flags);
//    _alphaTextureID = texture->getAlphaTextureName();
}

TrianglesCommand::~TrianglesCommand()
{
}

void TrianglesCommand::generateMaterialID()
{
    // glProgramState is hashed because it contains:
    //  *  uniforms/values
    //  *  glProgram
    //
    // we safely can when the same glProgramState is being used then they share those states
    // if they don't have the same glProgramState, they might still have the same
    // uniforms/values and glProgram, but it would be too expensive to check the uniforms.
    struct {
        void* glProgramState;
        GLuint textureId;
        GLenum blendSrc;
        GLenum blendDst;
        bool   isOrthoPro;
        bool   enableDepthTest;
    } hashMe;

    // NOTE: Initialize hashMe struct to make the value of padding bytes be filled with zero.
    // It's important since XXH32 below will also consider the padding bytes which probably 
    // are set to random values by different compilers.
    memset(&hashMe, 0, sizeof(hashMe)); 

    hashMe.textureId = _textureID;
    hashMe.blendSrc = _blendType.src;
    hashMe.blendDst = _blendType.dst;
    hashMe.glProgramState = _glProgramState;
    hashMe.isOrthoPro = _isUseOrthographicProjecion;
    hashMe.enableDepthTest = _enableDepthTest;
    _materialID = XXH32((const void*)&hashMe, sizeof(hashMe), 0);
}

void TrianglesCommand::useMaterial() const
{
    if(_enableDepthTest)
    {
        glEnable(GL_DEPTH_TEST);
    }
    //Set texture
    GL::bindTexture2D(_textureID);
    // Added by ChenFei 2014-12-26 V3.2 supports
    if (_textureID2 > 0)
    {
        //        CCLOG("liudi bind quad, %d",_textureID2);
        GL::bindTexture2DN(1, _textureID2);
    }
    //set blend mode
    GL::blendFunc(_blendType.src, _blendType.dst);
    
    
    //因为默认是透视投影的 所以这里想用正交投影的话 _isUseOrthographicProjecion 为true
    if(_isUseOrthographicProjecion)
    {
        auto director = Director::getInstance();
        auto size = director->getWinSize();
        director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
        
        Mat4 orthoMatrix;
        Mat4::createOrthographicOffCenter(0, size.width, 0, size.height, -1024, 1024, &orthoMatrix);
        director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, orthoMatrix);
        
        _glProgramState->apply(_mv);
        
        director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    }
    else
    {
        _glProgramState->apply(_mv);
    }
}
//绘制完成的时候要关闭深度测试 以免影响其他cmd
void TrianglesCommand::endUseMaterial() const
{
    if(_enableDepthTest)
    {
        glDisable(GL_DEPTH_TEST);
    }
}

NS_CC_END
