/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
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

#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_MAC

#include "platform/CCDevice.h"
#include <Foundation/Foundation.h>
#include <Cocoa/Cocoa.h>
#include <string>
#include "base/ccTypes.h"

NS_CC_BEGIN

int Device::getDPI()
{
    //TODO: return correct DPI
    return 160;
}

void Device::setAccelerometerEnabled(bool isEnabled)
{

}

void Device::setAccelerometerInterval(float interval)
{

}

#if 0


typedef struct
{
    int height;
    int width;
    bool        hasAlpha;
    bool        isPremultipliedAlpha;
    unsigned char*  data;
} tImageInfo;

static bool _initWithString(const char * text, Device::TextAlign align, const char * fontName, int size, tImageInfo* info, const Color3B* fontColor, int fontAlpha)
{
    bool ret = false;
    
    CCASSERT(text, "Invalid pText");
    CCASSERT(info, "Invalid pInfo");
    
    do {
        NSString * string  = [NSString stringWithUTF8String:text];
        NSString * fntName = [NSString stringWithUTF8String:fontName];
        
        fntName = [[fntName lastPathComponent] stringByDeletingPathExtension];
        
        // font
        NSFont *font = [[NSFontManager sharedFontManager]
                        fontWithFamily:fntName
                        traits:NSUnboldFontMask | NSUnitalicFontMask
                        weight:0
                        size:size];
        
        if (font == nil) {
            font = [[NSFontManager sharedFontManager]
                    fontWithFamily:@"Arial"
                    traits:NSUnboldFontMask | NSUnitalicFontMask
                    weight:0
                    size:size];
        }
        CC_BREAK_IF(!font);
        
        // color
        NSColor* foregroundColor;
        if (fontColor) {
            foregroundColor = [NSColor colorWithDeviceRed:fontColor->r/255.0 green:fontColor->g/255.0 blue:fontColor->b/255.0 alpha:fontAlpha/255.0];
        } else {
            foregroundColor = [NSColor whiteColor];
        }
        
        
        // alignment, linebreak
        unsigned horiFlag = (int)align & 0x0f;
        unsigned vertFlag = ((int)align >> 4) & 0x0f;
        NSTextAlignment textAlign = (2 == horiFlag) ? NSRightTextAlignment
        : (3 == horiFlag) ? NSCenterTextAlignment
        : NSLeftTextAlignment;
        
        NSMutableParagraphStyle *paragraphStyle = [[[NSMutableParagraphStyle alloc] init] autorelease];
        [paragraphStyle setParagraphStyle:[NSParagraphStyle defaultParagraphStyle]];
        [paragraphStyle setLineBreakMode:NSLineBreakByCharWrapping];
        [paragraphStyle setAlignment:textAlign];
        
        // attribute
        NSDictionary* tokenAttributesDict = [NSDictionary dictionaryWithObjectsAndKeys:
                                             foregroundColor,NSForegroundColorAttributeName,
                                             font, NSFontAttributeName,
                                             paragraphStyle, NSParagraphStyleAttributeName, nil];
        
        // linebreak
        if (info->width > 0) {
            if ([string sizeWithAttributes:tokenAttributesDict].width > info->width) {
                NSMutableString *lineBreak = [[[NSMutableString alloc] init] autorelease];
                NSUInteger length = [string length];
                NSRange range = NSMakeRange(0, 1);
                NSSize textSize;
                NSUInteger lastBreakLocation = 0;
                NSUInteger insertCount = 0;
                for (NSUInteger i = 0; i < length; i++) {
                    range.location = i;
                    NSString *character = [string substringWithRange:range];
                    [lineBreak appendString:character];
                    if ([@"!?.,-= " rangeOfString:character].location != NSNotFound) {
                        lastBreakLocation = i + insertCount;
                    }
                    textSize = [lineBreak sizeWithAttributes:tokenAttributesDict];
                    if(info->height > 0 && (int)textSize.height > info->height)
                        break;
                    if ((int)textSize.width > info->width) {
                        if(lastBreakLocation > 0) {
                            [lineBreak insertString:@"\r" atIndex:lastBreakLocation];
                            lastBreakLocation = 0;
                        }
                        else {
                            [lineBreak insertString:@"\r" atIndex:[lineBreak length] - 1];
                        }
                        insertCount += 1;
                    }
                }
                
                string = lineBreak;
            }
        }
        
        NSAttributedString *stringWithAttributes =[[[NSAttributedString alloc] initWithString:string
                                                                                   attributes:tokenAttributesDict] autorelease];
        
        NSSize realDimensions = [stringWithAttributes size];
        // Mac crashes if the width or height is 0
        CC_BREAK_IF(realDimensions.width <= 0 || realDimensions.height <= 0);
        
        CGSize dimensions = CGSizeMake(info->width, info->height);
        if(dimensions.width <= 0.f) {
            dimensions.width = realDimensions.width;
        }
        if (dimensions.height <= 0.f) {
            dimensions.height = realDimensions.height;
        }
        
        NSInteger POTWide = dimensions.width;
        NSInteger POTHigh = dimensions.height;
        unsigned char* data = nullptr;
        
        //Alignment
        CGFloat xPadding = 0;
        switch (textAlign) {
            case NSLeftTextAlignment: xPadding = 0; break;
            case NSCenterTextAlignment: xPadding = (dimensions.width-realDimensions.width)/2.0f; break;
            case NSRightTextAlignment: xPadding = dimensions.width-realDimensions.width; break;
            default: break;
        }
        
        CGFloat yPadding = 0.f;
        switch (vertFlag) {
                // align to top
            case 1: yPadding = dimensions.height - realDimensions.height; break;
                // align to bottom
            case 2: yPadding = 0.f; break;
                // align to center
            case 3: yPadding = (dimensions.height - realDimensions.height) / 2.0f; break;
            default: break;
        }
        
        NSRect textRect = NSMakeRect(xPadding, POTHigh - dimensions.height + yPadding, realDimensions.width, realDimensions.height);
        //Disable antialias
        
        [[NSGraphicsContext currentContext] setShouldAntialias:NO];
        
        NSImage *image = [[NSImage alloc] initWithSize:NSMakeSize(POTWide, POTHigh)];
        
        [image lockFocus];
        
        // patch for mac retina display and lableTTF
        [[NSAffineTransform transform] set];
        
        //[stringWithAttributes drawAtPoint:NSMakePoint(xPadding, offsetY)]; // draw at offset position
        [stringWithAttributes drawInRect:textRect];
        //[stringWithAttributes drawInRect:textRect withAttributes:tokenAttributesDict];
        NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect (0.0f, 0.0f, POTWide, POTHigh)];
        [image unlockFocus];
        
        data = (unsigned char*) [bitmap bitmapData];  //Use the same buffer to improve the performance.
        
        NSUInteger textureSize = POTWide*POTHigh*4;
        
        unsigned char* dataNew = (unsigned char*)malloc(sizeof(unsigned char) * textureSize);
        if (dataNew) {
            memcpy(dataNew, data, textureSize);
            // output params
            info->width = static_cast<int>(POTWide);
            info->height = static_cast<int>(POTHigh);
            info->data = dataNew;
            info->hasAlpha = true;
            info->isPremultipliedAlpha = true;
            ret = true;
        }
        [bitmap release];
        [image release];
    } while (0);
    return ret;
}

Data Device::getTextureDataForText(const char * text, const FontDefinition& textDefinition, TextAlign align, int &width, int &height, bool& hasPremultipliedAlpha)
{
    Data ret;
    do {
        tImageInfo info = {0};
        info.width = textDefinition._dimensions.width;
        info.height = textDefinition._dimensions.height;
        
        if (! _initWithString(text, align, textDefinition._fontName.c_str(), textDefinition._fontSize, &info, &textDefinition._fontFillColor, textDefinition._fontAlpha))
        {
            break;
        }
        height = (short)info.height;
        width = (short)info.width;
        ret.fastSet(info.data,width * height * 4);
        hasPremultipliedAlpha = true;
    } while (0);
    
    return ret;
}


#else


typedef struct
{
    unsigned int height;
    unsigned int width;
    bool         isPremultipliedAlpha;
    bool         hasShadow;
    CGSize       shadowOffset;
    float        shadowBlur;
    float        shadowOpacity;
    bool         hasStroke;
    float        strokeColorR;
    float        strokeColorG;
    float        strokeColorB;
    float        strokeColorA;
    float        strokeSize;
    float        tintColorR;
    float        tintColorG;
    float        tintColorB;
    float        tintColorA;
    int          lineBreak;
    int          underLine;
    int          bold;
    int          italic;
    int          strike;
    unsigned char*  data;
    
} tImageInfo;

static bool s_isIOS7OrHigher = true;

static inline void lazyCheckIOS7()
{
    static bool isInited = false;
    if (!isInited)
    {
//        s_isIOS7OrHigher = [[[UIDevice currentDevice] systemVersion] compare:@"7.0" options:NSNumericSearch] != NSOrderedAscending;
        isInited = true;
    }
}

static CGSize _calculateStringSize(NSString *str, id font, CGSize *constrainSize)
{
    CGSize textRect = CGSizeZero;
    textRect.width = constrainSize->width > 0 ? constrainSize->width
    : 0x7fffffff;
    textRect.height = constrainSize->height > 0 ? constrainSize->height
    : 0x7fffffff;
    
    CGSize dim;
    if(s_isIOS7OrHigher){
        NSDictionary *attibutes = @{NSFontAttributeName:font};
        dim = [str boundingRectWithSize:textRect options:(NSStringDrawingOptions)(NSStringDrawingUsesLineFragmentOrigin) attributes:attibutes context:nil].size;
    }
    else {
//        assert(0);
//        dim = [str sizeWithFont:font constrainedToSize:textRect];
    }
    
    dim.width = ceilf(dim.width);
    dim.height = ceilf(dim.height);
    
    return dim;
}

// refer Image::ETextAlign
#define ALIGN_TOP    1
#define ALIGN_CENTER 3
#define ALIGN_BOTTOM 2


static bool _initWithString(const char * text, cocos2d::Device::TextAlign align, const char * fontName, int size, tImageInfo* info)
{
    // lazy check whether it is iOS7 device
    lazyCheckIOS7();
    
    if(info->lineBreak==(int)CCLineBreakMode::CCLineBreakByTruncatingTail){
        CCLOG("msg: %s", text);
    }
    
    bool bRet = false;
    do
    {
        CC_BREAK_IF(! text || ! info);
        
        NSString * str          = [NSString stringWithUTF8String:text];
        NSString * fntName      = [NSString stringWithUTF8String:fontName];
        
        CGSize dim, constrainSize;
        
        constrainSize.width     = info->width;
        constrainSize.height    = info->height;
        
        // On iOS custom fonts must be listed beforehand in the App info.plist (in order to be usable) and referenced only the by the font family name itself when
        // calling [UIFont fontWithName]. Therefore even if the developer adds 'SomeFont.ttf' or 'fonts/SomeFont.ttf' to the App .plist, the font must
        // be referenced as 'SomeFont' when calling [UIFont fontWithName]. Hence we strip out the folder path components and the extension here in order to get just
        // the font family name itself. This stripping step is required especially for references to user fonts stored in CCB files; CCB files appear to store
        // the '.ttf' extensions when referring to custom fonts.
        fntName = [[fntName lastPathComponent] stringByDeletingPathExtension];
        
        // create the font
        NSFont* font = [NSFont systemFontOfSize:size];//[NSFont fontWithName:fntName size:size];
        
        if (font)
        {
            dim = _calculateStringSize(str, font, &constrainSize);
        }
        else
        {
            if (!font)
            {
                font = [NSFont systemFontOfSize:size];
            }
            
            if (font)
            {
                dim = _calculateStringSize(str, font, &constrainSize);
            }
        }
        
        CC_BREAK_IF(! font);
        
        // compute start point
        int startH = 0;
        if (constrainSize.height > dim.height)
        {
            // vertical alignment
            unsigned int vAlignment = ((int)align >> 4) & 0x0F;
            if (vAlignment == ALIGN_TOP)
            {
                startH = 0;
            }
            else if (vAlignment == ALIGN_CENTER)
            {
                startH = (constrainSize.height - dim.height) / 2;
            }
            else
            {
                startH = constrainSize.height - dim.height;
            }
        }
        
        // adjust text rect
        if (constrainSize.width > 0 && constrainSize.width > dim.width)
        {
            dim.width = constrainSize.width;
        }
        if (constrainSize.height > 0 && constrainSize.height > dim.height)
        {
            dim.height = constrainSize.height;
        }
        
        
        // compute the padding needed by shadow and stroke
        float shadowStrokePaddingX = 0.0f;
        float shadowStrokePaddingY = 0.0f;
        
        if ( info->hasStroke )
        {
            shadowStrokePaddingX = ceilf(info->strokeSize);
            shadowStrokePaddingY = ceilf(info->strokeSize);
        }
        
        // add the padding (this could be 0 if no shadow and no stroke)
        dim.width  += shadowStrokePaddingX*2;
        dim.height += shadowStrokePaddingY*2;
        
        
        unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * (int)(dim.width * dim.height * 4));
        memset(data, 0, (int)(dim.width * dim.height * 4));
        
        // draw text
        CGColorSpaceRef colorSpace  = CGColorSpaceCreateDeviceRGB();
        CGContextRef context        = CGBitmapContextCreate(data,
                                                            dim.width,
                                                            dim.height,
                                                            8,
                                                            (int)(dim.width) * 4,
                                                            colorSpace,
                                                            kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
        if (!context)
        {
            CGColorSpaceRelease(colorSpace);
            CC_SAFE_FREE(data);
            break;
        }
        
        // text color
        CGContextSetRGBFillColor(context, info->tintColorR, info->tintColorG, info->tintColorB, info->tintColorA);
        // move Y rendering to the top of the image
        CGContextTranslateCTM(context, 0.0f, (dim.height - shadowStrokePaddingY) );
        CGContextScaleCTM(context, 1.0f, -1.0f); //NOTE: NSString draws in UIKit referential i.e. renders upside-down compared to CGBitmapContext referential
        
        [NSGraphicsContext saveGraphicsState];
        NSGraphicsContext * nscg = [NSGraphicsContext graphicsContextWithGraphicsPort:context flipped:YES];
        [NSGraphicsContext setCurrentContext:nscg];
        
        // store the current context
//        UIGraphicsPushContext(context);
        
        // measure text size with specified font and determine the rectangle to draw text in
        unsigned uHoriFlag = (int)align & 0x0f;
        NSTextAlignment nsAlign = (2 == uHoriFlag) ? NSTextAlignmentRight
        : (3 == uHoriFlag) ? NSTextAlignmentCenter
        : NSTextAlignmentLeft;
        
        
        CGColorSpaceRelease(colorSpace);
        
        // compute the rect used for rendering the text
        // based on wether shadows or stroke are enabled
        
        float textOriginX  = 0;
        float textOrigingY = startH;
        
        float textWidth    = dim.width;
        float textHeight   = dim.height;
        
        CGRect rect = CGRectMake(textOriginX, textOrigingY, textWidth, textHeight);
        
        CGContextSetShouldSubpixelQuantizeFonts(context, false);
        
        CGContextBeginTransparencyLayerWithRect(context, rect, NULL);
        
        
        NSMutableParagraphStyle *paragraphStyle = [[NSMutableParagraphStyle alloc] init];
        paragraphStyle.alignment = nsAlign;
        paragraphStyle.lineBreakMode = (NSLineBreakMode)info->lineBreak;
        
        if ( info->hasStroke )
        {
            CGContextSetTextDrawingMode(context, kCGTextStroke);

            NSColor* strokeColor = [NSColor colorWithRed:info->strokeColorR
                                                   green:info->strokeColorG
                                                    blue:info->strokeColorB
                                                   alpha:info->strokeColorA];
            
            NSMutableDictionary* attrDict = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                             font, NSFontAttributeName,
                                             strokeColor, NSForegroundColorAttributeName,
                                             paragraphStyle, NSParagraphStyleAttributeName,
                                             
                                             strokeColor, NSStrokeColorAttributeName,
                                             [NSNumber numberWithFloat: info->strokeSize / size * 100 ], NSStrokeWidthAttributeName,
                                             nil];
            
            if (info->underLine > 0)
            {
                [attrDict setObject:strokeColor forKey:NSUnderlineColorAttributeName];
                [attrDict setObject:@(NSUnderlineStyleSingle) forKey:NSUnderlineStyleAttributeName];
            }
            
            if (info->strike > 0)
            {
                [attrDict setObject:strokeColor forKey:NSStrikethroughColorAttributeName];
                [attrDict setObject:@(NSUnderlineStyleSingle) forKey:NSStrikethroughStyleAttributeName];
            }
            
            if (info->bold > 0)
            {
                [attrDict setObject:@(0.1) forKey:NSExpansionAttributeName];
            }
            
            if (info->italic > 0)
            {
                [attrDict setObject:@(0.4) forKey:NSObliquenessAttributeName];
            }
            
            [str drawInRect:rect withAttributes:attrDict];

        }
        
        CGContextSetTextDrawingMode(context, kCGTextFill);
        
        // actually draw the text in the context
        
        NSColor* tintColor = [NSColor colorWithRed:info->tintColorR
                                               green:info->tintColorG
                                                blue:info->tintColorB
                                               alpha:info->tintColorA];
        
        
        NSMutableDictionary* attrDict = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                  font, NSFontAttributeName,
                                  tintColor, NSForegroundColorAttributeName,
                                  paragraphStyle, NSParagraphStyleAttributeName,
                                  nil];
        
        if (info->underLine > 0)
        {
            [attrDict setObject:tintColor forKey:NSUnderlineColorAttributeName];
            [attrDict setObject:@(NSUnderlineStyleSingle) forKey:NSUnderlineStyleAttributeName];
        }
        
        if (info->strike > 0)
        {
            [attrDict setObject:tintColor forKey:NSStrikethroughColorAttributeName];
            [attrDict setObject:@(NSUnderlineStyleSingle) forKey:NSStrikethroughStyleAttributeName];
        }
        
        if (info->bold > 0)
        {
            [attrDict setObject:@(0.1) forKey:NSExpansionAttributeName];
        }
        
        if (info->italic > 0)
        {
            [attrDict setObject:@(0.4) forKey:NSObliquenessAttributeName];
        }

        [str drawInRect:rect withAttributes:attrDict];
                
        
        CGContextEndTransparencyLayer(context);
        
        // pop the context
//        UIGraphicsPopContext();
        [NSGraphicsContext restoreGraphicsState];
        
        // release the context
        CGContextRelease(context);
        
        // output params
        info->data                 = data;
        info->isPremultipliedAlpha = true;
        info->width                = dim.width;
        info->height               = dim.height;
        bRet                        = true;
        
    } while (0);
    
    return bRet;
}


Data Device::getTextureDataForText(const char * text, const FontDefinition& textDefinition, TextAlign align, int &width, int &height, bool& hasPremultipliedAlpha)
{
    Data ret;
    
    do {
        tImageInfo info = {0};
        info.width                  = textDefinition._dimensions.width;
        info.height                 = textDefinition._dimensions.height;
        info.hasShadow              = textDefinition._shadow._shadowEnabled;
        info.shadowOffset.width     = textDefinition._shadow._shadowOffset.width;
        info.shadowOffset.height    = textDefinition._shadow._shadowOffset.height;
        info.shadowBlur             = textDefinition._shadow._shadowBlur;
        info.shadowOpacity          = textDefinition._shadow._shadowOpacity;
        info.hasStroke              = textDefinition._stroke._strokeEnabled;
        info.strokeColorR           = textDefinition._stroke._strokeColor.r / 255.0f;
        info.strokeColorG           = textDefinition._stroke._strokeColor.g / 255.0f;
        info.strokeColorB           = textDefinition._stroke._strokeColor.b / 255.0f;
        info.strokeColorA           = textDefinition._stroke._strokeAlpha / 255.0f;
        info.strokeSize             = textDefinition._stroke._strokeSize;
        info.tintColorR             = textDefinition._fontFillColor.r / 255.0f;
        info.tintColorG             = textDefinition._fontFillColor.g / 255.0f;
        info.tintColorB             = textDefinition._fontFillColor.b / 255.0f;
        info.tintColorA             = textDefinition._fontAlpha / 255.0f;
        info.lineBreak              = (int)textDefinition._lineBreak;
        info.underLine              = (int)textDefinition._underLine;
        info.strike                 = (int)textDefinition._strikeLine;
        info.bold                   = (int)textDefinition._bold;
        info.italic                 = (int)textDefinition._italic;
        
        if (! _initWithString(text, align, textDefinition._fontName.c_str(), textDefinition._fontSize, &info))
        {
            break;
        }
        height = (short)info.height;
        width = (short)info.width;
        ret.fastSet(info.data,width * height * 4);
        hasPremultipliedAlpha = true;
    } while (0);
    
    return ret;
}

#endif

void Device::setKeepScreenOn(bool value)
{
}

NS_CC_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_MAC
