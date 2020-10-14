/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2013 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2015 Chukong Technologies Inc.

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

// cocos2d includes
#include "base/CCDirector.h"

// standard includes
#include <string>

#include "2d/CCDrawingPrimitives.h"
#include "2d/CCSpriteFrameCache.h"
#include "platform/CCFileUtils.h"

#include "2d/CCActionManager.h"
#include "2d/CCFontFNT.h"
#include "2d/CCFontAtlasCache.h"
#include "2d/CCAnimationCache.h"
#include "2d/CCTransition.h"
#include "2d/CCFontFreeType.h"
#include "2d/CCLabelAtlas.h"
#include "renderer/CCGLProgramCache.h"
#include "renderer/CCGLProgramStateCache.h"
#include "renderer/CCTextureCache.h"
#include "renderer/ccGLStateCache.h"
#include "renderer/CCRenderer.h"
#include "renderer/CCRenderState.h"
#include "renderer/CCFrameBuffer.h"
#include "2d/CCCamera.h"
#include "base/CCUserDefault.h"
#include "base/ccFPSImages.h"
#include "base/CCScheduler.h"
#include "base/ccMacros.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventCustom.h"
#include "base/CCConsole.h"
#include "base/CCAutoreleasePool.h"
#include "base/CCConfiguration.h"
#include "base/CCAsyncTaskPool.h"
#include "platform/CCApplication.h"
#include "base/CCGlobalLock.h"
//#include "platform/CCGLViewImpl.h"
#include "platform/CCPThread.h"   //modify by owner - caibird
#if CC_ENABLE_SCRIPT_BINDING
#include "CCScriptSupport.h"
#endif



// #include "AssetsManagerEx.h"

//extern void addFabricLog(const char* fmt, ...);

/**
 Position of the FPS
 
 Default: 0,0 (bottom-left corner)
 */
#ifndef CC_DIRECTOR_STATS_POSITION
#define CC_DIRECTOR_STATS_POSITION Director::getInstance()->getVisibleOrigin()
#endif // CC_DIRECTOR_STATS_POSITION

// added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
extern unsigned long Common_getPopupWindowInstanceCount();
std::unordered_map<std::string, long> methodCall_perfomance;
bool isLogMethodCallPerformance = false;

#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS

using namespace std;

pthread_t MAIN_THREAD_ID;
pthread_t BACK_PROCESS_THREAD_ID;
bool g_isInLoadingScene = false;
bool g_isInDataParsing = false;
bool g_isLoadingSceneInit = false;
static bool sDirector_invalid = false;
std::chrono::high_resolution_clock::time_point g_lockStartTime = std::chrono::high_resolution_clock::now();
bool g_hasCreateMutex = false;

#if COCOS2D_DEBUG > 0
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, "cocos2d-x debug info2",__VA_ARGS__)
#else
#define  LOGD(...) printf(__VA_ARGS__)
#endif
#else
#define  LOGD(...)
#endif

NS_CC_BEGIN

Autolock::Autolock( pthread_mutex_t& lock):m_lock(lock)
{
    if( g_hasCreateMutex )
    pthread_mutex_lock(&m_lock);
}
    
Autolock::~Autolock()
{
    if( g_hasCreateMutex )
    pthread_mutex_unlock(&m_lock);
}

// FIXME: it should be a Director ivar. Move it there once support for multiple directors is added

// singleton stuff
static DisplayLinkDirector *s_SharedDirector = nullptr;
static CCPThread *s_SharedThread = NULL;  // -- added by Zouc
#define kDefaultFPS        60  // 60 frames per second
extern const char* cocos2dVersion(void);

const char *Director::EVENT_PROJECTION_CHANGED = "director_projection_changed";
const char *Director::EVENT_AFTER_DRAW = "director_after_draw";
const char *Director::EVENT_AFTER_VISIT = "director_after_visit";
const char *Director::EVENT_BEFORE_UPDATE = "director_before_update";
const char *Director::EVENT_AFTER_UPDATE = "director_after_update";
const char *Director::EVENT_RESET = "director_reset";

pthread_mutex_t                   g_platmMutex;
pthread_mutexattr_t           g_platmMutexAttr;

pthread_mutex_t                   g_platmMutex_paused_list;

Director* Director::getInstance()
{
    if (!s_SharedDirector)
    {
        MAIN_THREAD_ID = pthread_self();
        s_SharedDirector = new (std::nothrow) DisplayLinkDirector();
        
        CCLOG("MAIN_THREAD_ID = %08x", MAIN_THREAD_ID);
        LOGD("MAIN_THREAD_ID 12123 = %08x", MAIN_THREAD_ID);
        
        CCASSERT(s_SharedDirector, "FATAL: Not enough memory");
        s_SharedDirector->init();
    }

    return s_SharedDirector;
}

Director::Director()
: _isStatusLabelUpdated(true)
,_invalid(false)
{
}

bool Director::init(void)
{
    int ret;
    if(( ret = pthread_mutexattr_init(&g_platmMutexAttr)) != 0){
        CCAssert(0, "pthread_mutexattr_init FAIL");
    }
    
#ifdef _PERFORMANCE
    CProfileManager::createThreadPerf();
    CProfileManager::getThreadPerf();
#endif

    pthread_mutexattr_settype(&g_platmMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&g_platmMutex, &g_platmMutexAttr);
    pthread_mutex_init(&g_platmMutex_paused_list, &g_platmMutexAttr);
    g_hasCreateMutex = true;
    setDefaultValues();
    m_visitFlag = true;   //modify by owner - caibird
    _internalState = 0;
    _drawSimple = false;
    _disableAutoReleasePool = false;
    _glsync_func = nullptr;
    // scenes
    _runningScene = nullptr;
    _nextScene = nullptr;

    _notificationNode = nullptr;

    _scenesStack.reserve(15);
    
    sDirector_invalid = false;

    // Stat labels added by Tao Cheng
    initStatLabels();
    _lastUpdate = new struct timeval;

    // paused ?
    _paused = false;

    // purge ?
    _purgeDirectorInNextLoop = false;
    
    // restart ?
    _restartDirectorInNextLoop = false;

    _winSizeInPoints = Size::ZERO;

    _openGLView = nullptr;
    _defaultFBO = nullptr;
    
    _contentScaleFactor = 1.0f;
    
    gettimeofday(&m_lastAutoReleaseTime, nullptr);
    _console = new (std::nothrow) Console;

    // scheduler
    _scheduler = new (std::nothrow) Scheduler();
    // action manager
    _actionManager = new (std::nothrow) ActionManager();
    _scheduler->scheduleUpdate(_actionManager, Scheduler::PRIORITY_SYSTEM, false);

    _eventDispatcher = new (std::nothrow) EventDispatcher();
    _eventAfterDraw = new (std::nothrow) EventCustom(EVENT_AFTER_DRAW);
    _eventAfterDraw->setUserData(this);
    _eventAfterVisit = new (std::nothrow) EventCustom(EVENT_AFTER_VISIT);
    _eventAfterVisit->setUserData(this);
    _eventBeforeUpdate = new (std::nothrow) EventCustom(EVENT_BEFORE_UPDATE);
    _eventBeforeUpdate->setUserData(this);
    _eventAfterUpdate = new (std::nothrow) EventCustom(EVENT_AFTER_UPDATE);
    _eventAfterUpdate->setUserData(this);
    _eventProjectionChanged = new (std::nothrow) EventCustom(EVENT_PROJECTION_CHANGED);
    _eventProjectionChanged->setUserData(this);
    _eventResetDirector = new (std::nothrow) EventCustom(EVENT_RESET);
    //init TextureCache
    initTextureCache();
    initMatrixStack();

    _renderer = new (std::nothrow) Renderer;
    RenderState::initialize();

    return true;
}

Director::~Director(void)
{
    CCLOGINFO("deallocing Director: %p", this);

    CC_SAFE_RELEASE(_runningScene);
    CC_SAFE_RELEASE(_notificationNode);
    CC_SAFE_RELEASE(_scheduler);
    CC_SAFE_RELEASE(_actionManager);
    CC_SAFE_DELETE(_defaultFBO);
    
    delete _eventBeforeUpdate;
    delete _eventAfterUpdate;
    delete _eventAfterDraw;
    delete _eventAfterVisit;
    delete _eventProjectionChanged;
    delete _eventResetDirector;

    delete _renderer;

    delete _console;


    CC_SAFE_RELEASE(_eventDispatcher);

    releaseStatLabels();

    // delete _lastUpdate
    CC_SAFE_DELETE(_lastUpdate);

    Configuration::destroyInstance();

    s_SharedDirector = nullptr;
    g_hasCreateMutex = false;
    pthread_mutexattr_destroy( &g_platmMutexAttr);
    pthread_mutex_destroy(&g_platmMutex);
    pthread_mutex_destroy(&g_platmMutex_paused_list);
    
#ifdef _PERFORMANCE
    CProfileManager::destroyThreadPerf();
#endif
}

void Director::setDefaultValues(void)
{
    Configuration *conf = Configuration::getInstance();

    // default FPS
    double fps = conf->getValue("cocos2d.x.fps", Value(kDefaultFPS)).asDouble();
    _oldAnimationInterval = _animationInterval = 1.0 / fps;

    // Display FPS
    _displayStats = conf->getValue("cocos2d.x.display_fps", Value(false)).asBool();

    // GL projection
    std::string projection = conf->getValue("cocos2d.x.gl.projection", Value("3d")).asString();
    if (projection == "3d")
        _projection = Projection::_3D;
    else if (projection == "2d")
        _projection = Projection::_2D;
    else if (projection == "custom")
        _projection = Projection::CUSTOM;
    else
        CCASSERT(false, "Invalid projection value");

    // Default pixel format for PNG images with alpha
    std::string pixel_format = conf->getValue("cocos2d.x.texture.pixel_format_for_png", Value("rgba8888")).asString();
    if (pixel_format == "rgba8888")
        Texture2D::setDefaultAlphaPixelFormat(Texture2D::PixelFormat::RGBA8888);
    else if(pixel_format == "rgba4444")
        Texture2D::setDefaultAlphaPixelFormat(Texture2D::PixelFormat::RGBA4444);
    else if(pixel_format == "rgba5551")
        Texture2D::setDefaultAlphaPixelFormat(Texture2D::PixelFormat::RGB5A1);

    // PVR v2 has alpha premultiplied ?
    bool pvr_alpha_premultipled = conf->getValue("cocos2d.x.texture.pvrv2_has_alpha_premultiplied", Value(false)).asBool();
    Image::setPVRImagesHavePremultipliedAlpha(pvr_alpha_premultipled);
}

void Director::setGLDefaultValues()
{
    // This method SHOULD be called only after openGLView_ was initialized
    CCASSERT(_openGLView, "opengl view should not be null");

    setAlphaBlending(true);
    setDepthTest(false);
    setProjection(_projection);
}

// Draw the Scene
void Director::drawScene()
{
    THREAD_PERF_NODE_FUNC();
    
    if(!m_visitFlag){    //modify by owner - caibird
        return;
    }
    // calculate "global" dt
    calculateDeltaTime();
    
    if (_openGLView)
    {
        _openGLView->pollEvents();
    }
    
    //tick before glClear: issue #533
    if (! _paused)
    {
        _eventDispatcher->dispatchEvent(_eventBeforeUpdate);
        _scheduler->update(_deltaTime);
        _eventDispatcher->dispatchEvent(_eventAfterUpdate);
    }

    {
        THREAD_PERF_NODE("drawScene - clear");
        _renderer->clear();
    }
    {
        THREAD_PERF_NODE("drawScene - clearAllFBOs");
        experimental::FrameBuffer::clearAllFBOs();
    }
    
    /* to avoid flickr, nextScene MUST be here: after tick and before draw.
     * FIXME: Which bug is this one. It seems that it can't be reproduced with v0.9
     */
    if (_nextScene)
    {
        setNextScene();
    }

    pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    
    if (_runningScene)
    {
#if (CC_USE_PHYSICS || (CC_USE_3D_PHYSICS && CC_ENABLE_BULLET_INTEGRATION) || CC_USE_NAVMESH)
        _runningScene->stepPhysicsAndNavigation(_deltaTime);
#endif
        //clear draw stats
        _renderer->clearDrawStats();
        
        //render the scene
        _runningScene->render(_renderer);
        
        _eventDispatcher->dispatchEvent(_eventAfterVisit);
    }

    // draw the notifications node
    if (_notificationNode)
    { 
        _notificationNode->visit(_renderer, Mat4::IDENTITY, 0);
        _renderer->render(); 
    }

    if (_displayStats)
    { 
        showStats();
        _renderer->render(); 
    } 

    _eventDispatcher->dispatchEvent(_eventAfterDraw);

    popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

    _totalFrames++;

    // swap buffers
    if (_openGLView)
    {
        THREAD_PERF_NODE("drawScene - swapBuffers");
        _openGLView->swapBuffers();
    }

    if (_displayStats)
    {
        calculateMPF();
    }
}

void Director::calculateDeltaTime()
{
    THREAD_PERF_NODE_FUNC();
    
    struct timeval now;

    if (gettimeofday(&now, nullptr) != 0)
    {
        CCLOG("error in gettimeofday");
        _deltaTime = 0;
        return;
    }

    // new delta time. Re-fixed issue #1277
    if (_nextDeltaTimeZero)
    {
        _deltaTime = 0;
        _nextDeltaTimeZero = false;
    }
    else
    {
        _deltaTime = (now.tv_sec - _lastUpdate->tv_sec) + (now.tv_usec - _lastUpdate->tv_usec) / 1000000.0f;
        _deltaTime = MAX(0, _deltaTime);
    }

#if COCOS2D_DEBUG
    // If we are debugging our code, prevent big delta time
    if (_deltaTime > 0.2f)
    {
        _deltaTime = 0.2f;
//        _deltaTime = 1 / 60.0f;
    }
#endif

    *_lastUpdate = now;
}
float Director::getDeltaTime() const
{
    return _deltaTime;
}
void Director::setOpenGLView(GLView *openGLView)
{
    CCASSERT(openGLView, "opengl view should not be null");

    if (_openGLView != openGLView)
    {
        // Configuration. Gather GPU info
        Configuration *conf = Configuration::getInstance();
        conf->gatherGPUInfo();
        CCLOG("%s\n",conf->getInfo().c_str());

        if(_openGLView)
            _openGLView->release();
        _openGLView = openGLView;
        _openGLView->retain();

        // set size
        _winSizeInPoints = _openGLView->getDesignResolutionSize();

        _isStatusLabelUpdated = true;

        if (_openGLView)
        {
            setGLDefaultValues();
        }

        _renderer->initGLView();

        CHECK_GL_ERROR_DEBUG();

        if (_eventDispatcher)
        {
            _eventDispatcher->setEnabled(true);
        }
        
        _defaultFBO = experimental::FrameBuffer::getOrCreateDefaultFBO(_openGLView);
        _defaultFBO->retain();
    }
}

TextureCache* Director::getTextureCache() const
{
    return _textureCache;
}

void Director::initTextureCache()
{
#ifdef EMSCRIPTEN
    _textureCache = new (std::nothrow) TextureCacheEmscripten();
#else
    _textureCache = new (std::nothrow) TextureCache();
#endif // EMSCRIPTEN
}

void Director::destroyTextureCache()
{
    if (_textureCache)
    {
        _textureCache->waitForQuit();
        CC_SAFE_RELEASE_NULL(_textureCache);
    }
}

void Director::setViewport()
{
    if (_openGLView)
    {
        _openGLView->setViewPortInPoints(0, 0, _winSizeInPoints.width, _winSizeInPoints.height);
    }
}

void Director::setNextDeltaTimeZero(bool nextDeltaTimeZero)
{
    _nextDeltaTimeZero = nextDeltaTimeZero;
}

//
// FIXME TODO
// Matrix code MUST NOT be part of the Director
// MUST BE moved outide.
// Why the Director must have this code ?
//
void Director::initMatrixStack()
{
    while (!_modelViewMatrixStack.empty())
    {
        _modelViewMatrixStack.pop();
    }
    
    while (!_projectionMatrixStack.empty())
    {
        _projectionMatrixStack.pop();
    }
    
    while (!_textureMatrixStack.empty())
    {
        _textureMatrixStack.pop();
    }
    
    _modelViewMatrixStack.push(Mat4::IDENTITY);
    _projectionMatrixStack.push(Mat4::IDENTITY);
    _textureMatrixStack.push(Mat4::IDENTITY);
}

void Director::resetMatrixStack()
{
    initMatrixStack();
}

void Director::popMatrix(MATRIX_STACK_TYPE type)
{
    if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type)
    {
        _modelViewMatrixStack.pop();
    }
    else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type)
    {
        _projectionMatrixStack.pop();
    }
    else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type)
    {
        _textureMatrixStack.pop();
    }
    else
    {
        CCASSERT(false, "unknow matrix stack type");
    }
}

void Director::loadIdentityMatrix(MATRIX_STACK_TYPE type)
{
    if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type)
    {
        _modelViewMatrixStack.top() = Mat4::IDENTITY;
    }
    else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type)
    {
        _projectionMatrixStack.top() = Mat4::IDENTITY;
    }
    else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type)
    {
        _textureMatrixStack.top() = Mat4::IDENTITY;
    }
    else
    {
        CCASSERT(false, "unknow matrix stack type");
    }
}

void Director::loadMatrix(MATRIX_STACK_TYPE type, const Mat4& mat)
{
    if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type)
    {
        _modelViewMatrixStack.top() = mat;
    }
    else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type)
    {
        _projectionMatrixStack.top() = mat;
    }
    else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type)
    {
        _textureMatrixStack.top() = mat;
    }
    else
    {
        CCASSERT(false, "unknow matrix stack type");
    }
}

void Director::multiplyMatrix(MATRIX_STACK_TYPE type, const Mat4& mat)
{
    if(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW == type)
    {
        _modelViewMatrixStack.top() *= mat;
    }
    else if(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION == type)
    {
        _projectionMatrixStack.top() *= mat;
    }
    else if(MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE == type)
    {
        _textureMatrixStack.top() *= mat;
    }
    else
    {
        CCASSERT(false, "unknow matrix stack type");
    }
}

void Director::pushMatrix(MATRIX_STACK_TYPE type)
{
    if(type == MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW)
    {
        _modelViewMatrixStack.push(_modelViewMatrixStack.top());
    }
    else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION)
    {
        _projectionMatrixStack.push(_projectionMatrixStack.top());
    }
    else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE)
    {
        _textureMatrixStack.push(_textureMatrixStack.top());
    }
    else
    {
        CCASSERT(false, "unknow matrix stack type");
    }
}

const Mat4& Director::getMatrix(MATRIX_STACK_TYPE type)
{
    if(type == MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW)
    {
        return _modelViewMatrixStack.top();
    }
    else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION)
    {
        return _projectionMatrixStack.top();
    }
    else if(type == MATRIX_STACK_TYPE::MATRIX_STACK_TEXTURE)
    {
        return _textureMatrixStack.top();
    }

    CCASSERT(false, "unknow matrix stack type, will return modelview matrix instead");
    return  _modelViewMatrixStack.top();
}

void Director::setProjection(Projection projection)
{
    Size size = _winSizeInPoints;

    setViewport();

    switch (projection)
    {
        case Projection::_2D:
        {
            loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

            Mat4 orthoMatrix;
            Mat4::createOrthographicOffCenter(0, size.width, 0, size.height, -1024, 1024, &orthoMatrix);
            multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, orthoMatrix);
            loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            break;
        }
            
        case Projection::_3D:
        {
            float zeye = this->getZEye();

            Mat4 matrixPerspective, matrixLookup;

            loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);

            // issue #1334
            Mat4::createPerspective(60, (GLfloat)size.width/size.height, 10, zeye+size.height/2, &matrixPerspective);

            multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, matrixPerspective);

            Vec3 eye(size.width/2, size.height/2, zeye), center(size.width/2, size.height/2, 0.0f), up(0.0f, 1.0f, 0.0f);
            Mat4::createLookAt(eye, center, up, &matrixLookup);
            multiplyMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION, matrixLookup);
            
            loadIdentityMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
            break;
        }

        case Projection::CUSTOM:
            // Projection Delegate is no longer needed
            // since the event "PROJECTION CHANGED" is emitted
            break;

        default:
            CCLOG("cocos2d: Director: unrecognized projection");
            break;
    }

    _projection = projection;
    GL::setProjectionMatrixDirty();

    _eventDispatcher->dispatchEvent(_eventProjectionChanged);
}

void Director::purgeCachedData(void)
{
    FontFNT::purgeCachedData();
    FontAtlasCache::purgeCachedData();

    if (s_SharedDirector->getOpenGLView())
    {
        SpriteFrameCache::getInstance()->removeUnusedSpriteFrames();
        _textureCache->removeUnusedTextures();

        // Note: some tests such as ActionsTest are leaking refcounted textures
        // There should be no test textures left in the cache
        log("%s\n", _textureCache->getCachedTextureInfo().c_str());
    }
    FileUtils::getInstance()->purgeCachedEntries();
    
    // add by im30!
    Sprite3DMaterial::releaseBuiltInMaterial();
    Sprite3DMaterial::releaseCachedMaterial();
    GLProgramCache::destroyInstance();
    
    return;
}

float Director::getZEye(void) const
{
    //3d模型在不同手机上大小不一致 所以让z轴保持固定
    //return (_winSizeInPoints.height / 1.1566f);
    return (_openGLView->getDesignResolutionSize().height /1.1566f);
}

void Director::setAlphaBlending(bool on)
{
    if (on)
    {
        GL::blendFunc(CC_BLEND_SRC, CC_BLEND_DST);
    }
    else
    {
        GL::blendFunc(GL_ONE, GL_ZERO);
    }

    CHECK_GL_ERROR_DEBUG();
}

void Director::setDepthTest(bool on)
{
    _renderer->setDepthTest(on);
}

void Director::setClearColor(const Color4F& clearColor)
{
    _renderer->setClearColor(clearColor);
    auto defaultFBO = experimental::FrameBuffer::getOrCreateDefaultFBO(_openGLView);
    
    if(defaultFBO) defaultFBO->setClearColor(clearColor);
}

static void GLToClipTransform(Mat4 *transformOut)
{
    if(nullptr == transformOut) return;
    
    Director* director = Director::getInstance();
    CCASSERT(nullptr != director, "Director is null when seting matrix stack");

    auto projection = director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_PROJECTION);
    auto modelview = director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    *transformOut = projection * modelview;
}

Vec2 Director::convertToGL(const Vec2& uiPoint)
{
    Mat4 transform;
    GLToClipTransform(&transform);

    Mat4 transformInv = transform.getInversed();

    // Calculate z=0 using -> transform*[0, 0, 0, 1]/w
    float zClip = transform.m[14]/transform.m[15];

    Size glSize = _openGLView->getDesignResolutionSize();
    Vec4 clipCoord(2.0f*uiPoint.x/glSize.width - 1.0f, 1.0f - 2.0f*uiPoint.y/glSize.height, zClip, 1);

    Vec4 glCoord;
    //transformInv.transformPoint(clipCoord, &glCoord);
    transformInv.transformVector(clipCoord, &glCoord);
    float factor = 1.0/glCoord.w;
    return Vec2(glCoord.x * factor, glCoord.y * factor);
}

Vec2 Director::convertToUI(const Vec2& glPoint)
{
    Mat4 transform;
    GLToClipTransform(&transform);

    Vec4 clipCoord;
    // Need to calculate the zero depth from the transform.
    Vec4 glCoord(glPoint.x, glPoint.y, 0.0, 1);
    transform.transformVector(glCoord, &clipCoord);

	/*
	BUG-FIX #5506

	a = (Vx, Vy, Vz, 1)
	b = (a×M)T
	Out = 1 ⁄ bw(bx, by, bz)
	*/
	
	clipCoord.x = clipCoord.x / clipCoord.w;
	clipCoord.y = clipCoord.y / clipCoord.w;
	clipCoord.z = clipCoord.z / clipCoord.w;

    Size glSize = _openGLView->getDesignResolutionSize();
    float factor = 1.0/glCoord.w;
    return Vec2(glSize.width*(clipCoord.x*0.5 + 0.5) * factor, glSize.height*(-clipCoord.y*0.5 + 0.5) * factor);
}

const Size& Director::getWinSize(void) const
{
    return _winSizeInPoints;
}

// cocos builder底层的很多适配使用的是getWinSize()，很土，这里为了避免大量修改，使用一个小技巧吧
void Director::_setWinSize(Size s)
{
//    assert(s == _openGLView->getDesignResolutionSize());
    _winSizeInPoints = s;
}

Size Director::getWinSizeInPixels() const
{
    return Size(_winSizeInPoints.width * _contentScaleFactor, _winSizeInPoints.height * _contentScaleFactor);
}

Size Director::getVisibleSize() const
{
    if (_openGLView)
    {
        return _openGLView->getVisibleSize();
    }
    else
    {
        return Size::ZERO;
    }
}

Vec2 Director::getVisibleOrigin() const
{
    if (_openGLView)
    {
        return _openGLView->getVisibleOrigin();
    }
    else
    {
        return Vec2::ZERO;
    }
}

// scene management

void Director::runWithScene(Scene *scene)
{
    CCASSERT(scene != nullptr, "This command can only be used to start the Director. There is already a scene present.");
    CCASSERT(_runningScene == nullptr, "_runningScene should be null");

    pushScene(scene);
    startAnimation();
}

void Director::replaceScene(Scene *scene)
{
    //CCASSERT(_runningScene, "Use runWithScene: instead to start the director");
    CCASSERT(scene != nullptr, "the scene should not be null");
    
    if (_runningScene == nullptr) {
        runWithScene(scene);
        return;
    }
    
    if (scene == _nextScene)
        return;
    
    if (_nextScene)
    {
        if (_nextScene->isRunning())
        {
            _nextScene->onExit();
        }
        _nextScene->cleanup();
        _nextScene = nullptr;
    }

    ssize_t index = _scenesStack.size();

    _sendCleanupToScene = true;
    _scenesStack.replace(index - 1, scene);

    _nextScene = scene;
}

void Director::pushScene(Scene *scene)
{
    CCASSERT(scene, "the scene should not null");

    _sendCleanupToScene = false;

    _scenesStack.pushBack(scene);
    _nextScene = scene;
}

void Director::popScene(void)
{
    CCASSERT(_runningScene != nullptr, "running scene should not null");

    _scenesStack.popBack();
    ssize_t c = _scenesStack.size();

    if (c == 0)
    {
        end();
    }
    else
    {
        _sendCleanupToScene = true;
        _nextScene = _scenesStack.at(c - 1);
    }
}

void Director::popToRootScene(void)
{
    popToSceneStackLevel(1);
}

void Director::popToSceneStackLevel(int level)
{
    CCASSERT(_runningScene != nullptr, "A running Scene is needed");
    ssize_t c = _scenesStack.size();

    // level 0? -> end
    if (level == 0)
    {
        end();
        return;
    }

    // current level or lower -> nothing
    if (level >= c)
        return;

    auto fisrtOnStackScene = _scenesStack.back();
    if (fisrtOnStackScene == _runningScene)
    {
        _scenesStack.popBack();
        --c;
    }

    // pop stack until reaching desired level
    while (c > level)
    {
        auto current = _scenesStack.back();

        if (current->isRunning())
        {
            current->onExit();
        }

        current->cleanup();
        _scenesStack.popBack();
        --c;
    }

    _nextScene = _scenesStack.back();

    // cleanup running scene
    _sendCleanupToScene = true;
}

void Director::end()
{
    _purgeDirectorInNextLoop = true;
}

void Director::restart()
{
    _restartDirectorInNextLoop = true;
}

void Director::reset()
{    
    if (_runningScene)
    {
        _runningScene->onExit();
        _runningScene->cleanup();
        _runningScene->release();
    }
    
    _runningScene = nullptr;
    _nextScene = nullptr;

    _eventDispatcher->dispatchEvent(_eventResetDirector);
    
    // cleanup scheduler
    getScheduler()->unscheduleAll();
    
    // Remove all events
    if (_eventDispatcher)
    {
        _eventDispatcher->removeAllEventListeners();
    }
    
    // remove all objects, but don't release it.
    // runWithScene might be executed after 'end'.
    _scenesStack.clear();
    
    stopAnimation();
    
    CC_SAFE_RELEASE_NULL(_notificationNode);
    releaseStatLabels();
    
    // purge bitmap cache
    FontFNT::purgeCachedData();
    
    // FontFreeType::shutdownFreeType();
    
    // purge all managed caches
    
#if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif _MSC_VER >= 1400 //vs 2005 or higher
#pragma warning (push)
#pragma warning (disable: 4996)
#endif
//it will crash clang static analyzer so hide it if __clang_analyzer__ defined
#ifndef __clang_analyzer__
    DrawPrimitives::free();
#endif
#if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#elif _MSC_VER >= 1400 //vs 2005 or higher
#pragma warning (pop)
#endif
    AnimationCache::destroyInstance();
    SpriteFrameCache::destroyInstance();
    GLProgramCache::destroyInstance();
    GLProgramStateCache::destroyInstance();
    FileUtils::destroyInstance();
    AsyncTaskPool::destoryInstance();
    
    // cocos2d-x specific data structures
    UserDefault::destroyInstance();
    
    GL::invalidateStateCache();

    RenderState::finalize();
    
    destroyTextureCache();
}

void Director::purgeDirector()
{
//    addFabricLog("purgeDirector begin!!!");

    reset();

    CHECK_GL_ERROR_DEBUG();
    
    // OpenGL view
    if (_openGLView)
    {
        _openGLView->end();
        _openGLView = nullptr;
    }

    // delete Director
    release();
//    addFabricLog("purgeDirector end!!!");
}

void Director::restartDirector()
{
    reset();
    
    // RenderState need to be reinitialized
    RenderState::initialize();

    // Texture cache need to be reinitialized
    initTextureCache();
    
    // Reschedule for action manager
    getScheduler()->scheduleUpdate(getActionManager(), Scheduler::PRIORITY_SYSTEM, false);
    
    // release the objects
    PoolManager::getInstance()->getCurrentPool()->clear();
    
    // Real restart in script level
#if CC_ENABLE_SCRIPT_BINDING
    ScriptEvent scriptEvent(kRestartGame, NULL);
    ScriptEngineManager::getInstance()->getScriptEngine()->sendEvent(&scriptEvent);
#endif
}

void Director::setNextScene()
{
    bool runningIsTransition = dynamic_cast<TransitionScene*>(_runningScene) != nullptr;
    bool newIsTransition = dynamic_cast<TransitionScene*>(_nextScene) != nullptr;

    // If it is not a transition, call onExit/cleanup
     if (! newIsTransition)
     {
         if (_runningScene)
         {
             _runningScene->onExitTransitionDidStart();
             _runningScene->onExit();
         }
 
         // issue #709. the root node (scene) should receive the cleanup message too
         // otherwise it might be leaked.
         if (_sendCleanupToScene && _runningScene)
         {
             _runningScene->cleanup();
         }
     }

    if (_runningScene)
    {
        _runningScene->release();
    }
    _runningScene = _nextScene;
    _nextScene->retain();
    _nextScene = nullptr;

    if ((! runningIsTransition) && _runningScene)
    {
        _runningScene->onEnter();
        _runningScene->onEnterTransitionDidFinish();
    }
}

void Director::pause()
{
    CCLOG("Director::pause()");
    if (_paused)
    {
        return;
    }

    _oldAnimationInterval = _animationInterval;

    // when paused, don't consume CPU
    setAnimationInterval(1 / 1.0);
    _paused = true;
}

void Director::resume()
{
    CCLOG("Director::resume()");
    if (! _paused)
    {
        return;
    }

    setAnimationInterval(_oldAnimationInterval);

    _paused = false;
    _deltaTime = 0;
    // fix issue #3509, skip one fps to avoid incorrect time calculation.
    setNextDeltaTimeZero(true);
}

int cmp(const std::pair<std::string, long>& x, const std::pair<std::string, long>& y) {
    return x.second > y.second;
}
std::vector<std::pair<std::string,long> > tVector;
extern uint32_t TheCounterTextureStr;
// display the FPS using a LabelAtlas
// updates the FPS every frame
void Director::showStats()
{
    if (_isStatusLabelUpdated)
    {
        createStatsLabel();
        _isStatusLabelUpdated = false;
    }

    const Mat4& identity = Mat4::IDENTITY;
    static unsigned long prevCalls = 0;
    static unsigned long prevVerts = 0;
    static float prevDeltaTime  = 0.016f; // 60FPS
    static const float FPS_FILTER = 0.10f;
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    static unsigned long refInstanceCount = 0;
    static unsigned long nodeInstanceCount = 0;
    static unsigned long labelInstanceCount = 0;
    static unsigned long texture2DInstanceCount = 0;
    static unsigned long popupWindowInstanceCount = 0;
#endif //#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    
    if (!_displayStats)
    {
        return;
    }
    
    char buffer[30];
    if (_FPSLabel)
    {
        float dt = _deltaTime * FPS_FILTER + (1-FPS_FILTER) * prevDeltaTime;
        prevDeltaTime = dt;
        _frameRate = 1/dt;
        _accumDt += _deltaTime;

        // Probably we don't need this anymore since
        // the framerate is using a low-pass filter
        // to make the FPS stable
        if (_accumDt > CC_DIRECTOR_STATS_INTERVAL)
        {
            sprintf(buffer, "FPS         : %.1f / %.3f", _frameRate, _secondsPerFrame);
            _FPSLabel->setString(buffer);
            _accumDt = 0;
        }

    }
    
    if(_drawnBatchesLabel)
    {
        auto currentCalls = (unsigned long)_renderer->getDrawnBatches();
        if( currentCalls != prevCalls ) {
            sprintf(buffer, "GL calls    : %lu", currentCalls);
            _drawnBatchesLabel->setString(buffer);
            prevCalls = currentCalls;
        }
    }
    if(_drawnVerticesLabel)
    {
        auto currentVerts = (unsigned long)_renderer->getDrawnVertices();
        if( currentVerts != prevVerts) {
            sprintf(buffer, "GL verts    : %lu", currentVerts);
            _drawnVerticesLabel->setString(buffer);
            prevVerts = currentVerts;
        }
    }
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    unsigned long currentObjectCount = 0;
    if(_refStatLabel)
    {
        currentObjectCount = Ref::getInstanceCount();
        if(refInstanceCount != currentObjectCount)
        {
            sprintf(buffer, "Refs        : %lu", currentObjectCount);
            _refStatLabel->setString(buffer);
            refInstanceCount = currentObjectCount;
        }
    }
    if(_nodeStatLabel)
    {
        currentObjectCount = Node::getInstanceCount();
        if(nodeInstanceCount != currentObjectCount)
        {
            sprintf(buffer, "Nodes       : %lu", currentObjectCount);
            _nodeStatLabel->setString(buffer);
            nodeInstanceCount = currentObjectCount;
        }
    }
    if(_labelStatLabel)
    {
        currentObjectCount = Label::getInstanceCount();
        if(labelInstanceCount != currentObjectCount)
        {
            sprintf(buffer, "Labels      : %lu", currentObjectCount);
            _labelStatLabel->setString(buffer);
            labelInstanceCount = currentObjectCount;
        }
    }
    if(_texture2DStatLabel)
    {
        currentObjectCount = Texture2D::getInstanceCount();
        if(texture2DInstanceCount != currentObjectCount)
        {
            sprintf(buffer, "Textures(2D): %lu / %lu", currentObjectCount, (unsigned long)TheCounterTextureStr);
            _texture2DStatLabel->setString(buffer);
            texture2DInstanceCount = currentObjectCount;
        }
    }
    if(_popupWindowStatLabel) {
        currentObjectCount = Common_getPopupWindowInstanceCount();
        if(popupWindowInstanceCount != currentObjectCount)
        {
            sprintf(buffer, "PopupWindows: %lu", currentObjectCount);
            _popupWindowStatLabel->setString(buffer);
            popupWindowInstanceCount = currentObjectCount;
        }
    }
    if(_stpLabel)
    {
        tVector.clear();
        for (std::unordered_map<std::string, long>::iterator curr = methodCall_perfomance.begin(); curr != methodCall_perfomance.end(); curr++)
        {
            tVector.push_back(std::make_pair(curr->first, curr->second));
        }
        std::sort(tVector.begin(), tVector.end(), cmp);
        
        size_t len = tVector.size();
        len = len > 3 ? 3 : len;
        if(isLogMethodCallPerformance) CCLOG("Perf: Begin");
        for (size_t i = 0; i < len ; ++i ) {
            char tempBuffer[100] = {0};
            sprintf(tempBuffer, "%04ld %s", tVector.at(i).second, tVector.at(i).first.c_str());
            if( i == 0 )_stpLabel->setString(tempBuffer);
            if( i == 1 )_stpLabel2->setString(tempBuffer);
            if( i == 2 )_stpLabel3->setString(tempBuffer);
            if(isLogMethodCallPerformance && tVector.at(i).second > 1000) CCLOG("Perf: %s", tempBuffer);
        }
        if(isLogMethodCallPerformance) CCLOG("Perf: End");
    }
    
    if(_refStatLabel) { _refStatLabel->visit(_renderer, identity, 0); }
    if(_nodeStatLabel) { _nodeStatLabel->visit(_renderer, identity, 0); }
    if(_labelStatLabel) { _labelStatLabel->visit(_renderer, identity, 0); }
    if(_texture2DStatLabel) { _texture2DStatLabel->visit(_renderer, identity, 0); }
    if(_popupWindowStatLabel) { _popupWindowStatLabel->visit(_renderer, identity, 0); }
    if(_stpLabel)   { _stpLabel->visit(_renderer, identity, 0); }
    if(_stpLabel2)   { _stpLabel2->visit(_renderer, identity, 0); }
    if(_stpLabel3)   { _stpLabel3->visit(_renderer, identity, 0); }
#endif //#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    
    if(_drawnBatchesLabel) { _drawnBatchesLabel->visit(_renderer, identity, 0); }
    if(_drawnVerticesLabel) { _drawnVerticesLabel->visit(_renderer, identity, 0); }
    if(_FPSLabel) { _FPSLabel->visit(_renderer, identity, 0); }
}

void Director::calculateMPF()
{
    static float prevSecondsPerFrame = 0;
    static const float MPF_FILTER = 0.10f;

    struct timeval now;
    gettimeofday(&now, nullptr);
    
    _secondsPerFrame = (now.tv_sec - _lastUpdate->tv_sec) + (now.tv_usec - _lastUpdate->tv_usec) / 1000000.0f;

    _secondsPerFrame = _secondsPerFrame * MPF_FILTER + (1-MPF_FILTER) * prevSecondsPerFrame;
    prevSecondsPerFrame = _secondsPerFrame;
}

// returns the FPS image data pointer and len
void Director::getFPSImageData(unsigned char** datapointer, ssize_t* length)
{
    // FIXME: fixed me if it should be used 
    *datapointer = cc_fps_images_png;
    *length = cc_fps_images_len();
}

void Director::createStatsLabel()
{
    Texture2D *texture = nullptr;
    std::string fpsString = "00.0";
    std::string drawBatchString = "000";
    std::string drawVerticesString = "00000";
    
// added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    std::string refStatString = "Refs        : 0";
    std::string nodeStatString = "Nodes       : 0";
    std::string texture2DStatString = "Textures(2D): 0";
    std::string labelStatString = "Labels      : 0";
    std::string popupWindowStatString = "PopupWindows: 0";
    std::string scheduleTimerPerf = "ST: 0";
#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS

    if (_FPSLabel)
    {
        fpsString = _FPSLabel->getString();
        drawBatchString = _drawnBatchesLabel->getString();
        drawVerticesString = _drawnVerticesLabel->getString();
        
        releaseStatLabels();
        _textureCache->removeTextureForKey("/cc_fps_images");
        FileUtils::getInstance()->purgeCachedEntries();
    }

    Texture2D::PixelFormat currentFormat = Texture2D::getDefaultAlphaPixelFormat();
    Texture2D::setDefaultAlphaPixelFormat(Texture2D::PixelFormat::RGBA4444);
    unsigned char *data = nullptr;
    ssize_t dataLength = 0;
    getFPSImageData(&data, &dataLength);

    Image* image = new (std::nothrow) Image();
    bool isOK = image->initWithImageData(data, dataLength);
    if (! isOK) {
        CCLOGERROR("%s", "Fails: init fps_images");
        return;
    }

    texture = _textureCache->addImage(image, "/cc_fps_images");
    CC_SAFE_RELEASE(image);

    /*
     We want to use an image which is stored in the file named ccFPSImage.c 
     for any design resolutions and all resource resolutions. 
     
     To achieve this, we need to ignore 'contentScaleFactor' in 'AtlasNode' and 'LabelAtlas'.
     So I added a new method called 'setIgnoreContentScaleFactor' for 'AtlasNode',
     this is not exposed to game developers, it's only used for displaying FPS now.
     */
    float scaleFactor = 1 / CC_CONTENT_SCALE_FACTOR();
    Texture2D::setDefaultAlphaPixelFormat(currentFormat);
    const int height_spacing = 22 / CC_CONTENT_SCALE_FACTOR();

    _FPSLabel = createStatLabel(fpsString, texture, scaleFactor);
    _drawnBatchesLabel = createStatLabel(drawBatchString, texture, scaleFactor);
    _drawnVerticesLabel = createStatLabel(drawVerticesString, texture, scaleFactor);
    
    // added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    _refStatLabel = createStatLabel(refStatString, texture, scaleFactor);
    _nodeStatLabel = createStatLabel(nodeStatString, texture, scaleFactor);
    _labelStatLabel = createStatLabel(labelStatString, texture, scaleFactor);
    _texture2DStatLabel = createStatLabel(texture2DStatString, texture, scaleFactor);
    _popupWindowStatLabel = createStatLabel(popupWindowStatString, texture, scaleFactor);
    _stpLabel = createStatLabel(scheduleTimerPerf, texture, scaleFactor);
    _stpLabel2 = createStatLabel(scheduleTimerPerf, texture, scaleFactor);
    _stpLabel3 = createStatLabel(scheduleTimerPerf, texture, scaleFactor);
    
    _stpLabel->setPosition(Vec2(0, height_spacing*21) + CC_DIRECTOR_STATS_POSITION);
    _stpLabel2->setPosition(Vec2(0, height_spacing*20) + CC_DIRECTOR_STATS_POSITION);
    _stpLabel3->setPosition(Vec2(0, height_spacing*19) + CC_DIRECTOR_STATS_POSITION);
    _popupWindowStatLabel->setPosition(Vec2(0, height_spacing*18) + CC_DIRECTOR_STATS_POSITION);
    _texture2DStatLabel->setPosition(Vec2(0, height_spacing*17) + CC_DIRECTOR_STATS_POSITION);
    _labelStatLabel->setPosition(Vec2(0, height_spacing*16) + CC_DIRECTOR_STATS_POSITION);
    _nodeStatLabel->setPosition(Vec2(0, height_spacing*15) + CC_DIRECTOR_STATS_POSITION);
    _refStatLabel->setPosition(Vec2(0, height_spacing*14) + CC_DIRECTOR_STATS_POSITION);
#endif //#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    
    _drawnVerticesLabel->setPosition(Vec2(0, height_spacing*12) + CC_DIRECTOR_STATS_POSITION);
    _drawnBatchesLabel->setPosition(Vec2(0, height_spacing*11) + CC_DIRECTOR_STATS_POSITION);
    _FPSLabel->setPosition(Vec2(0, height_spacing*10)+CC_DIRECTOR_STATS_POSITION);
}

LabelAtlas * Director::createStatLabel(std::string &initString, cocos2d::Texture2D *texture, float scale) // added by Tao Cheng
{
    LabelAtlas * label = LabelAtlas::create();
    label->retain();
    label->setIgnoreContentScaleFactor(true);
    label->initWithString(initString, texture, 12, 32, '.');
    label->setScale(scale);
    label->setColor(Color3B(255,255,0));
    
    return label;
}

void Director::setContentScaleFactor(float scaleFactor)
{
    if (scaleFactor != _contentScaleFactor)
    {
        _contentScaleFactor = scaleFactor;
        _isStatusLabelUpdated = true;
    }
}

void Director::setNotificationNode(Node *node)
{
    CC_SAFE_RELEASE(_notificationNode);
    _notificationNode = node;
    CC_SAFE_RETAIN(_notificationNode);
}

void Director::setScheduler(Scheduler* scheduler)
{
    if (_scheduler != scheduler)
    {
        CC_SAFE_RETAIN(scheduler);
        CC_SAFE_RELEASE(_scheduler);
        _scheduler = scheduler;
    }
}

void Director::setActionManager(ActionManager* actionManager)
{
    if (_actionManager != actionManager)
    {
        CC_SAFE_RETAIN(actionManager);
        CC_SAFE_RELEASE(_actionManager);
        _actionManager = actionManager;
    }    
}

void Director::setEventDispatcher(EventDispatcher* dispatcher)
{
    if (_eventDispatcher != dispatcher)
    {
        CC_SAFE_RETAIN(dispatcher);
        CC_SAFE_RELEASE(_eventDispatcher);
        _eventDispatcher = dispatcher;
    }
}

void Director::runInMainThread(const std::function<void ()> func, float delay, bool repeat)
{
    if (_scheduler) {
        auto callback = [func](float){
            if (func) {
                func();
            }
        };
        static int counter = 0;
        std::string key = "DirectorMainThread" + Value(++counter).asString();
        _scheduler->schedule(callback, this, 0.0, repeat, delay, false, key);
    }
}

CCPThread* Director::sharedMainThread(void) {     //modify by owner - caibird
    if (!s_SharedThread) {
        s_SharedThread = new CCPThread("MainUIThread");
    }
    return s_SharedThread;
}

void Director::setVisitFlag(bool b){
    if( b == m_visitFlag) {
        return;
    }
    m_visitFlag = b;
    if(m_visitFlag) {
        CCNotificationCenter::sharedNotificationCenter()->postNotification("msg_native_on_resume_in_app");
    }
    else {
        CCNotificationCenter::sharedNotificationCenter()->postNotification("msg_native_on_pause_in_app");
    }
}

void Director::initStatLabels() // added by Tao Cheng
{
    _accumDt = 0.0f;
    _frameRate = 0.0f;
    _totalFrames = 0;
    _secondsPerFrame = 1.0f;
    
    _FPSLabel = nullptr;
    _drawnBatchesLabel = nullptr;
    _drawnVerticesLabel = nullptr;
    
    // added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    _refStatLabel = nullptr;
    _nodeStatLabel = nullptr;
    _labelStatLabel = nullptr;
    _texture2DStatLabel = nullptr;
    _popupWindowStatLabel = nullptr;
#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
}
void Director::releaseStatLabels() // added by Tao Cheng 2016/4
{
    CC_SAFE_RELEASE_NULL(_FPSLabel);
    CC_SAFE_RELEASE_NULL(_drawnVerticesLabel);
    CC_SAFE_RELEASE_NULL(_drawnBatchesLabel);
    
// added by Tao Cheng
#ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
    CC_SAFE_RELEASE_NULL(_refStatLabel);
    CC_SAFE_RELEASE_NULL(_nodeStatLabel);
    CC_SAFE_RELEASE_NULL(_labelStatLabel);
    CC_SAFE_RELEASE_NULL(_texture2DStatLabel);
    CC_SAFE_RELEASE_NULL(_popupWindowStatLabel);
#endif // #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
}

/***************************************************
* implementation of DisplayLinkDirector
**************************************************/

// should we implement 4 types of director ??
// I think DisplayLinkDirector is enough
// so we now only support DisplayLinkDirector
void DisplayLinkDirector::startAnimation()
{
    if (gettimeofday(_lastUpdate, nullptr) != 0)
    {
        CCLOG("cocos2d: DisplayLinkDirector: Error on gettimeofday");
    }

    _invalid = false;
    sDirector_invalid = false;

#ifndef WP8_SHADER_COMPILER
    Application::getInstance()->setAnimationInterval(_animationInterval);
#endif

    // fix issue #3509, skip one fps to avoid incorrect time calculation.
    setNextDeltaTimeZero(true);
}

void DisplayLinkDirector::mainLoop()
{
    cocos2d::Autolock lock(g_platmMutex);
    
    if (1)
    {
        THREAD_PERF_NODE("mainLoop");
        
        this->doBackSyncThread();
        
        if (_purgeDirectorInNextLoop)
        {
            _purgeDirectorInNextLoop = false;
            purgeDirector();
        }
        else if (_restartDirectorInNextLoop)
        {
            _restartDirectorInNextLoop = false;
            restartDirector();
        }
        else if (! _invalid && !Application::isDeviceReallyInvalid())
        {
            drawScene();
            
            if( !_disableAutoReleasePool ){
                // release the objects
                PoolManager::getInstance()->getCurrentPool()->clear();
            }
            
            _internalState = 0;
    //        // release the objects
    //        PoolManager::getInstance()->getCurrentPool()->clear();
    #ifdef CC_SHOW_STATISTICS_OBJECT_COUNTS
            methodCall_perfomance.clear();
    #endif
            
            if (s_SharedThread) {     //modify by owner - caibird
                CCPThread::processMessage(s_SharedThread);
            }
        }
    //    if (s_SharedThread) {     //modify by owner - caibird
    //        CCPThread::processMessage(s_SharedThread);
    //    }
    }
    
#ifdef _PERFORMANCE
    ///////////////////////////////////////////
    // 性能统计分析
    static double lastTime = utils::gettime();
    auto profileOne = CProfileManager::getThreadPerf();
    if (profileOne)
    {
        profileOne->IncrementFrameCounter();
        
        // 每秒保存一下节点信息
        double dCurTime = utils::gettime();
        if (dCurTime - lastTime >= 1.f)
        {
            lastTime = lastTime + 1;

            std::string path = CCFileUtils::sharedFileUtils()->getWritablePath();
            path += "game.prf";
            profileOne->SaveDataFrame( path.c_str(),NULL,0,NULL);
            profileOne->Reset();
        }
    }
#endif
}

void DisplayLinkDirector::stopAnimation()
{
    _invalid = true;
    sDirector_invalid = true;
}

void DisplayLinkDirector::setAnimationInterval(double interval)
{
    _animationInterval = interval;
    if (! _invalid)
    {
        stopAnimation();
        startAnimation();
    }    
}

void Director::runInBackThread(const std::function<void()> func) {
    
    Application::getInstance()->runInBackThread([this,func]{
        if (func) {
            BACK_PROCESS_THREAD_ID = pthread_self();
            _disableAutoReleasePool = true;
            _drawSimple = true;
            func();

            Director::sharedDirector()->runSyncOnGLThread([this] {
                clearPausedList();
                _drawSimple = false;
                _disableAutoReleasePool = false;
//                PoolManager::getInstance()->getCurrentPool()->clear();
            });
        }
    });
}


void Director::addGLProcInMainThread(cocos2d::Ref *target, std::function<void ()> func) {
    if (MAIN_THREAD_ID == pthread_self()) {
        if (func) {
            func();
        }
        return;
    }
    _glProcess[target] = func;
}

void Director::removeGLProcInMainThread(cocos2d::Ref *target) {
    _glProcess.erase(target);
}

void Director::runSyncOnGLThread(std::function<void ()> func, bool isDraw) {
    
    if (MAIN_THREAD_ID == pthread_self()) {
        if (func) {
            func();
        }
        return;
    }
    
    CCLOG("Director::runSyncOnGLThread on other thread begin.");
//    std::condition_variable _sleepCondition;
//    std::mutex signalMutex;
//    std::unique_lock<std::mutex> signal(signalMutex);
    _glsync_func = [this,func]{
        if (func) {
            func();
        }
//        _sleepCondition.notify_one();
    };
    
    GlobalLock::unlockAllClassTypesMutex();

    cocos2d::GlobalLock::lockMainThread();
    
    CCLOG("Director::runSyncOnGLThread on other thread 22222222.");
    
    this->_internalState = 1;
    //    this->runInGLThread(_glsync_func);
    Application::getInstance()->runInGLThread([this]{
        doBackSyncThread();
    });
    
    cocos2d::GlobalLock::unlockMainThread();
    
    while (_internalState == 1) {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
    
    GlobalLock::lockAllClassTypesMutex();

    this->_internalState = 0;
    _glsync_func = nullptr;
}

void Director::runInGLThread(const std::function<void ()> func) {
    Application::getInstance()->runInGLThread(func);
}

bool Director::isHavePriorityWorking() {
    return _internalState == 1 || _drawSimple;
}

void Director::doBackSyncThread() {
    
    if (sDirector_invalid) {
        return;
    }
    
    if (_internalState == 1) {
        if (_glsync_func) {
            _glsync_func();
            _glsync_func = nullptr;
        }
    }
}

void Director::doDrawEvent(int status) {
}

void Director::addPausedList(cocos2d::Ref *ref) {
    cocos2d::Autolock lock(g_platmMutex_paused_list);
    _managedPausedObjectArray.push_back(ref);
}

void Director::clearPausedList() {
    cocos2d::Autolock lock(g_platmMutex_paused_list);
    std::vector<Ref*> releasings;
    releasings.swap(_managedPausedObjectArray);
    for (const auto &obj : releasings)
    {
        obj->setObjPaused(false);
    }
}

void Director::removePausedList(cocos2d::Ref *ref) {
    cocos2d::Autolock lock(g_platmMutex_paused_list);
    auto iter = std::find(_managedPausedObjectArray.begin(), _managedPausedObjectArray.end(), ref);

    if( iter != _managedPausedObjectArray.end() && (*iter) != nullptr) {
        _managedPausedObjectArray.erase(iter);
    }
    
}

NS_CC_END

