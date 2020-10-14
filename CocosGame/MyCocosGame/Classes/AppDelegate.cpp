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

#include "AppDelegate.h"
#include "HelloWorldScene.h"

// #define USE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#endif

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);


bool g_fix_ar_direction = true;
bool g_label_render_optimize = false;

///
float g_textureContrast = 1.0f;//设置全局纹理对比度的值
float g_textureSaturation = 1.0f;//设置全局纹理饱和度的值
bool g_isApplicationExit = false;
bool g_isWestMode = false;//是否为西部大陆模式
bool g_isChangeLanguage = false;//是否切换多语言
int  g_reConnectTimes = 0; //这个地方用作本次游戏内断线重连的次数.
#if COCOS2D_DEBUG > 0
bool g_isSmallInit = true; // 拆包登陆
#else
bool g_isSmallInit = false; // 拆包登陆
#endif


// 省略一部分
bool isWarzGame(){
    return true;
}

bool Common_IsObbOk(){
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    return false;
#else
    return false;
#endif
}

bool Common_labelAutoFixSizeCheck()
{
    return true;
}

std::string Common_getSDCardObbPath(){
    return "";
}

std::string Common_getOBBDownPath(){
    return "";
}

bool Common_isTHLang(){
    return false;
}

bool Common_canBeSupportedWithBmpFont(){
//    if (isWarzGame()){
//        return Warz_canBeSupportedWithBmpFont();
//    } else {
//        return canBeSupportedWithBmpFont();//canBeSupportedWithBmpFont();
//    }
    return false;//canBeSupportedWithBmpFont();
}
unsigned long Common_getPopupWindowInstanceCount(){
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    //    if (isWarzGame()){
    //        return Warz_getPopupWindowInstanceCount();
    //    } else {
    //        return Warz_getPopupWindowInstanceCount();//getPopupWindowInstanceCount();
    //    }
    return 0;
#else
    return 0;
#endif
}

extern void printBackTraceTolog(const char* data);
void Common_printBackTraceTolog( const char* data ){
    if (isWarzGame()){
//        return printBackTraceTolog(data);
    } else {
    }
}

void* operator new(std::size_t sz) {
    void* ptr = std::malloc(sz);
    std::memset(ptr, 0, sz);
    return ptr;
}

void* operator new[](std::size_t sz) {
    void* ptr = std::malloc(sz);
    std::memset(ptr, 0, sz);
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    std::free(ptr);
}


// openGL打点
void openGLPrint(const char* fn, int line, int error)
{
    return;
}

// 添加关键日志 - 条目多了话，弹出前面的。
// 添加fabric日志
void addFabricLog(const char* fmt, ...)
{
}

// cocos底层错误返回
void cocosErrorLog(const char* log)
{
    return;
}

bool g_bQuitGame = false;
bool IsQuitGame()
{
    return g_bQuitGame;
}

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate() 
{
#if USE_AUDIO_ENGINE
    AudioEngine::end();
#endif
}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,  
// don't modify or remove this function
static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        glview = GLViewImpl::createWithRect("MyCocosGame", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
#else
        glview = GLViewImpl::create("MyCocosGame");
#endif
        director->setOpenGLView(glview);
    }

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60);

    // Set the design resolution
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::NO_BORDER);
    auto frameSize = glview->getFrameSize();
    // if the frame's height is larger than the height of medium size.
    if (frameSize.height > mediumResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(largeResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is larger than the height of small size.
    else if (frameSize.height > smallResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is smaller than the height of medium size.
    else
    {        
        director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
    }

    register_all_packages();

    // create a scene. it's an autorelease object
    auto scene = HelloWorld::createScene();

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#endif
}
