#include "CCLabelBMFontLoader.h"

using namespace cocos2d;

namespace cocosbuilder {

#define PROPERTY_COLOR "color"
#define PROPERTY_OPACITY "opacity"
#define PROPERTY_BLENDFUNC "blendFunc"
#define PROPERTY_FNTFILE "fntFile"
#define PROPERTY_STRING "string"

void LabelBMFontLoader::onHandlePropTypeColor3(Node * pNode, Node * pParent, const char * pPropertyName, Color3B pColor3B, CCBReader * ccbReader) {
    if(strcmp(pPropertyName, PROPERTY_COLOR) == 0) {
        ((LabelBMFont *)pNode)->setColor(pColor3B); //modify by owner - caibird
    } else {
        NodeLoader::onHandlePropTypeColor3(pNode, pParent, pPropertyName, pColor3B, ccbReader);
    }
}

void LabelBMFontLoader::onHandlePropTypeByte(Node * pNode, Node * pParent, const char * pPropertyName, unsigned char pByte, CCBReader * ccbReader) {
    if(strcmp(pPropertyName, PROPERTY_OPACITY) == 0) {
        ((LabelBMFont *)pNode)->setOpacity(pByte); //modify by owner - caibird
    } else {
        NodeLoader::onHandlePropTypeByte(pNode, pParent, pPropertyName, pByte, ccbReader);
    }
}

void LabelBMFontLoader::onHandlePropTypeBlendFunc(Node * pNode, Node * pParent, const char * pPropertyName, BlendFunc pBlendFunc, CCBReader * ccbReader) {
    if(strcmp(pPropertyName, PROPERTY_BLENDFUNC) == 0) {
        ((LabelBMFont *)pNode)->setBlendFunc(pBlendFunc); //modify by owner - caibird
    } else {
        NodeLoader::onHandlePropTypeBlendFunc(pNode, pParent, pPropertyName, pBlendFunc, ccbReader);
    }
}

void LabelBMFontLoader::onHandlePropTypeFntFile(Node * pNode, Node * pParent, const char * pPropertyName, const char* pFntFile, CCBReader * ccbReader) {
    if(strcmp(pPropertyName, PROPERTY_FNTFILE) == 0) {
      //  ((LabelBMFont *)pNode)->setBMFontFilePath(pFntFile);
        ((LabelBMFont *)pNode)->setFntFile(pFntFile); //modify by owner - caibird
        
    } else {
        NodeLoader::onHandlePropTypeFntFile(pNode, pParent, pPropertyName, pFntFile, ccbReader);
    }
}

void LabelBMFontLoader::onHandlePropTypeText(Node * pNode, Node * pParent, const char * pPropertyName, const char* pText, CCBReader * ccbReader) {
    if(strcmp(pPropertyName, PROPERTY_STRING) == 0) {
//        ((LabelBMFont *)pNode)->setString(pText);
    } else {
        NodeLoader::onHandlePropTypeText(pNode, pParent, pPropertyName, pText, ccbReader);
    }
}

}
