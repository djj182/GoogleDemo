/****************************************************************************
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

#include "editor-support/cocosbuilder/CCLabelTTFLoader.h"
#include "CCFileUtils.h"

using namespace cocos2d;

extern bool canUseThisFont(const char* fontName);

#define PROPERTY_COLOR "color"
#define PROPERTY_OPACITY "opacity"
#define PROPERTY_BLENDFUNC "blendFunc"
#define PROPERTY_FONTNAME "fontName"
#define PROPERTY_FONTSIZE "fontSize"
#define PROPERTY_HORIZONTALALIGNMENT "horizontalAlignment"
#define PROPERTY_VERTICALALIGNMENT "verticalAlignment"
#define PROPERTY_STRING "string"
#define PROPERTY_DIMENSIONS "dimensions"
#define PROPERTY_LOCALIZE "localize"

namespace cocosbuilder {

LabelTTFLoader::LabelTTFLoader()
{
    reset();
}

void LabelTTFLoader::reset()
{
    shadowEnable_ = false;
    shadowColor_ = Color3B::BLACK;
    shadowOpacity_ = 0xff;
    shadowOffset_ = Size(2,-2);
    shadowBlur_ = 0;
    
    outlineEnable_ = false;
    outlineColor_ = Color3B::BLACK;
    outlineOpacity_ = 0xff;
    outlineSize_ = 1.0;
}

void LabelTTFLoader::parseProperties(cocos2d::Node * pNode, cocos2d::Node * pParent, CCBReader * ccbReader)
{
    reset();
    
    NodeLoader::parseProperties(pNode, pParent, ccbReader);
    
    Label* pLabel = dynamic_cast<Label*>(pNode);
    if (pLabel == nullptr) {
        assert(0);
        return;
    }
    
    // ttf font
    if (!fontName_.empty() && canUseThisFont(fontName_.c_str()))
    {
        TTFConfig ttfConfig;
        ttfConfig.fontFilePath = fontName_;
        ttfConfig.fontSize = fontSize_;
        pLabel->setTTFConfig(ttfConfig);
    }
    else
    {
        pLabel->setSystemFontName(fontName_);
    }
    
    if (shadowEnable_)
    {
        pLabel->enableShadow(Color4B(shadowColor_, shadowOpacity_), shadowOffset_, shadowBlur_);
    }
    
    if (outlineEnable_)
    {
        pLabel->enableOutline(Color4B(outlineColor_, outlineOpacity_), outlineSize_);
    }
    
    return;
}

void LabelTTFLoader::onHandlePropTypeColor3(cocos2d::Node * pNode, cocos2d::Node * pParent, const char * pPropertyName, cocos2d::Color3B pCCColor3B, cocosbuilder::CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_COLOR) == 0) {
        ((Label*)pNode)->setColor(pCCColor3B);
    }
    else if(strcmp(pPropertyName, "shadowColor") == 0) {
        shadowColor_ = pCCColor3B;
    }
    else if(strcmp(pPropertyName, "outlineColor") == 0) {
        outlineColor_ = pCCColor3B;
    }
    else {
        NodeLoader::onHandlePropTypeColor3(pNode, pParent, pPropertyName, pCCColor3B, pCCBReader);
    }
}

void LabelTTFLoader::onHandlePropTypeByte(cocos2d::Node * pNode, cocos2d::Node * pParent, const char * pPropertyName, unsigned char pByte, cocosbuilder::CCBReader * pCCBReader)
{
    if (strcmp(pPropertyName, PROPERTY_OPACITY) == 0) {
        ((Label*)pNode)->setOpacity(pByte);
    }
    else if (strcmp(pPropertyName, "shadowOpacity") == 0) {
        shadowOpacity_ = pByte;
    }
    else if (strcmp(pPropertyName, "outlineOpacity") == 0) {
        outlineOpacity_ = pByte;
    }
    else {
        NodeLoader::onHandlePropTypeByte(pNode, pParent, pPropertyName, pByte, pCCBReader);
    }
}

void LabelTTFLoader::onHandlePropTypeBlendFunc(cocos2d::Node * pNode, cocos2d::Node * pParent, const char * pPropertyName, cocos2d::BlendFunc pCCBlendFunc, cocosbuilder::CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_BLENDFUNC) == 0) {
        //        ((CCLabelIF *)pNode)->setBlendFunc(pCCBlendFunc);
    } else {
        NodeLoader::onHandlePropTypeBlendFunc(pNode, pParent, pPropertyName, pCCBlendFunc, pCCBReader);
    }
}

void LabelTTFLoader::onHandlePropTypeFontTTF(cocos2d::Node * pNode, cocos2d::Node * pParent, const char * pPropertyName, const char * pFontTTF, cocosbuilder::CCBReader * pCCBReader) {
    if(strcmp(pPropertyName, PROPERTY_FONTNAME) == 0) {
        fontName_ = pFontTTF;
    } else {
        NodeLoader::onHandlePropTypeFontTTF(pNode, pParent, pPropertyName, pFontTTF, pCCBReader);
    }
}

void LabelTTFLoader::onHandlePropTypeText(cocos2d::Node * pNode, cocos2d::Node * pParent, const char * pPropertyName, const char * pText, cocosbuilder::CCBReader * pCCBReader)
{
    if(strcmp(pPropertyName, PROPERTY_STRING) == 0) {
        //        ((CCLabelIF *)pNode)->setString(pText);
    }
    else if (strcmp(pPropertyName, PROPERTY_LOCALIZE) == 0) {
        // 这个地方由界面生成代码直接设置吧。否则有语言变量的话，还得再设置一次
    }
    else {
        NodeLoader::onHandlePropTypeText(pNode, pParent, pPropertyName, pText, pCCBReader);
    }
}

void LabelTTFLoader::onHandlePropTypeFloat(cocos2d::Node * pNode, cocos2d::Node * pParent, const char* pPropertyName, float pFloat, CCBReader * ccbReader)
{
    if(strcmp(pPropertyName, "shadowBlur") == 0) {
        shadowBlur_ = pFloat;
    }
    else if(strcmp(pPropertyName, "lineSpacing") == 0) {
        ((Label*)pNode)->setLineSpacing(pFloat);
    }
    else if(strcmp(pPropertyName, "outlineSize") == 0) {
        outlineSize_ = pFloat;
    }
    else {
        NodeLoader::onHandlePropTypeFloat(pNode, pParent, pPropertyName, pFloat, ccbReader);
    }
}

void LabelTTFLoader::onHandlePropTypeFloatScale(cocos2d::Node * pNode, cocos2d::Node * pParent, const char * pPropertyName, float pFloatScale, cocosbuilder::CCBReader * pCCBReader)
{
    if(strcmp(pPropertyName, PROPERTY_FONTSIZE) == 0) {
        ((Label*)pNode)->setFontSize(pFloatScale);
        fontSize_ = pFloatScale;
    }
    else {
        NodeLoader::onHandlePropTypeFloatScale(pNode, pParent, pPropertyName, pFloatScale, pCCBReader);
    }
}

void LabelTTFLoader::onHandlePropTypeIntegerLabeled(cocos2d::Node * pNode, cocos2d::Node * pParent, const char * pPropertyName, int pIntegerLabeled, cocosbuilder::CCBReader * pCCBReader)
{
    if(strcmp(pPropertyName, PROPERTY_HORIZONTALALIGNMENT) == 0)
    {
        ((Label*)pNode)->setAlignment(cocos2d::TextHAlignment(pIntegerLabeled));
    }
    else if(strcmp(pPropertyName, PROPERTY_VERTICALALIGNMENT) == 0)
    {
        ((Label*)pNode)->setVerticalAlignment(cocos2d::TextVAlignment(pIntegerLabeled));
    }
    else {
        NodeLoader::onHandlePropTypeIntegerLabeled(pNode, pParent, pPropertyName, pIntegerLabeled, pCCBReader);
    }
    
    return;
}

void LabelTTFLoader::onHandlePropTypeSize(cocos2d::Node * pNode, cocos2d::Node * pParent, const char * pPropertyName, cocos2d::Size pSize, cocosbuilder::CCBReader * pCCBReader)
{
    if(strcmp(pPropertyName, PROPERTY_DIMENSIONS) == 0) {
        ((Label*)pNode)->setDimensions(pSize.width, pSize.height);
    } else {
        NodeLoader::onHandlePropTypeSize(pNode, pParent, pPropertyName, pSize, pCCBReader);
    }
}

void LabelTTFLoader::onHandlePropTypeCheck(cocos2d::Node * pNode, cocos2d::Node * pParent, const char* pPropertyName, bool pCheck, CCBReader * ccbReader)
{
    if(strcmp(pPropertyName, "shadowEnable") == 0) {
        shadowEnable_ = pCheck;
    }
    else if(strcmp(pPropertyName, "outlineEnable") == 0) {
        outlineEnable_ = pCheck;
    }
    else if(strcmp(pPropertyName, "bold") == 0) {
        ((Label*)pNode)->setBold(pCheck);
    }
    else if(strcmp(pPropertyName, "italic") == 0) {
        ((Label*)pNode)->setItalic(pCheck);
    }
    else {
        NodeLoader::onHandlePropTypeCheck(pNode, pParent, pPropertyName, pCheck, ccbReader);
    }
    return;
}

void LabelTTFLoader::onHandlePropTypeFloatXY(cocos2d::Node * pNode, cocos2d::Node * pParent, const char* pPropertyName, float * pFoatVar, CCBReader * ccbReader)
{
    if(strcmp(pPropertyName, "shadowOffset") == 0) {
        shadowOffset_ = Size(pFoatVar[0], pFoatVar[1]);
    }
    else {
        NodeLoader::onHandlePropTypeFloatXY(pNode, pParent, pPropertyName, pFoatVar, ccbReader);
    }
    return;
}

void LabelTTFLoader::onHandlePropTypeInteger(cocos2d::Node * pNode, cocos2d::Node * pParent, const char* pPropertyName, int pInteger, CCBReader * ccbReader)
{
    if(strcmp(pPropertyName, "underline") == 0)
    {
        ((Label*)pNode)->setUnderLine(pInteger);
    }
    else if(strcmp(pPropertyName, "strikeline") == 0)
    {
        ((Label*)pNode)->setStrikeLine(pInteger);
    }
    else
    {
        NodeLoader::onHandlePropTypeInteger(pNode, pParent, pPropertyName, pInteger, ccbReader);
    }
}
    
    
};

