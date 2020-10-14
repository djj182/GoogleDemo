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
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS

#import "CCDirectorCaller-ios.h"

#import <Foundation/Foundation.h>
#import <OpenGLES/EAGL.h>

#import "base/CCDirector.h"
#import "CCEAGLView-ios.h"
#include "base/CCGlobalLock.h"

static id s_sharedDirectorCaller;

@interface NSObject(CADisplayLink)
+(id) displayLinkWithTarget: (id)arg1 selector:(SEL)arg2;
-(void) addToRunLoop: (id)arg1 forMode: (id)arg2;
-(void) setFrameInterval: (NSInteger)interval;
-(void) invalidate;
@end

@implementation CCDirectorCaller

@synthesize interval;

extern dispatch_queue_t BackRenderLoopQueue;
extern pthread_t MAIN_THREAD_ID;

+(id) sharedDirectorCaller
{
    if (s_sharedDirectorCaller == nil)
    {
        s_sharedDirectorCaller = [[CCDirectorCaller alloc] init];
    }
    
    return s_sharedDirectorCaller;
}

+(void) destroy
{
    [s_sharedDirectorCaller stopMainLoop];
    [s_sharedDirectorCaller release];
    s_sharedDirectorCaller = nil;
}

- (instancetype)init
{
    if (self = [super init])
    {
        isAppActive = [UIApplication sharedApplication].applicationState == UIApplicationStateActive;
        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
        [nc addObserver:self selector:@selector(appDidBecomeActive) name:UIApplicationDidBecomeActiveNotification object:nil];
        [nc addObserver:self selector:@selector(appDidBecomeInactive) name:UIApplicationWillResignActiveNotification object:nil];
        
        self.interval = 1;
    }
    return self;
}

- (void)appDidBecomeActive
{
    isAppActive = YES;
}

- (void)appDidBecomeInactive
{
    isAppActive = NO;
}


-(void) dealloc
{
    [displayLink release];
    [super dealloc];
}

-(void) startMainLoop
{
        // Director::setAnimationInterval() is called, we should invalidate it first
    [self stopMainLoop];
    
    displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(doCaller:)];
    [displayLink setFrameInterval: self.interval];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

-(void) stopMainLoop
{
    [displayLink invalidate];
    displayLink = nil;
}

-(void) runInBackThread:(const std::function<void ()>)func
{
    dispatch_async(BackRenderLoopQueue, ^{
        if (func) {
            func();
        }
    });
}

-(void) runInMainThread:(const std::function<void ()>)func
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (func) {
            func();
        }
    });
}

-(void) setAnimationInterval:(double)intervalNew
{
    // Director::setAnimationInterval() is called, we should invalidate it first
    [self stopMainLoop];
        
    self.interval = 60.0 * intervalNew;
        
    displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(doCaller:)];
    [displayLink setFrameInterval: self.interval];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}
                      
-(void) doCaller: (id) sender
{
    if(isAppActive)
    {
        cocos2d::Director* director = cocos2d::Director::getInstance();
        EAGLContext* cocos2dxContext = [(CCEAGLView*)director->getOpenGLView()->getEAGLView() context];
        if (cocos2dxContext != [EAGLContext currentContext])
            glFlush();
        
        [EAGLContext setCurrentContext: cocos2dxContext];
        // CADisplayLink 默认使用的是vsync，垂直同步是60fps，
        // 为了降低CPU能耗，所以这里简单处理一下。之前是写了一些代码进行了限帧处理，但是感觉这样更方便。
        // 目前应该是51帧左右，因为如此以来不一定卡住整秒，所以49~53来回震荡。
        static int frames = 0;
        if (++frames == 7)
        {
            frames = 0;
            return;
        }
        
        director->mainLoop();
    }
}

@end

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS

