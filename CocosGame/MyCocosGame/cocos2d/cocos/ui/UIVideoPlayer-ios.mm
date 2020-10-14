/****************************************************************************
 Copyright (c) 2014 Chukong Technologies Inc.
 
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

#include "UIVideoPlayer.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS

using namespace cocos2d::experimental::ui;
//-------------------------------------------------------------------------------------

#include "platform/ios/CCEAGLView-ios.h"
#import <MediaPlayer/MediaPlayer.h>
#import <AVKit/AVKit.h>
#include "base/CCDirector.h"
#include "platform/CCFileUtils.h"
@interface  UIVideoViewWrapperIosBase : NSObject

- (void) setFrame:(int) left :(int) top :(int) width :(int) height;
- (void) setURL:(int) videoSource :(std::string&) videoUrl;
- (void) play;
- (void) pause;
- (void) resume;
- (void) stop;
- (void) seekTo:(float) sec;
- (void) setVisible:(bool) visible;
- (void) setKeepRatioEnabled:(bool) enabled;
- (void) setFullScreenEnabled:(bool) enabled;
- (bool) isFullScreenEnabled;
- (long) duration;
- (long) currentTime;
-(id) init:(void*) videoPlayer;

-(void) videoFinished:(NSNotification*) notification;

@end

@implementation UIVideoViewWrapperIosBase
{
    
}

- (void) setFrame:(int) left :(int) top :(int) width :(int) height
{
    assert(0);
}
- (void) setURL:(int) videoSource :(std::string&) videoUrl
{
    assert(0);
}
- (void) play
{
    assert(0);
}

- (void) pause
{
    assert(0);
}

- (void) resume
{
    assert(0);
}
- (void) stop
{
    assert (0);
}
- (void) seekTo:(float) sec
{
    assert (0);
}
- (void) setVisible:(bool) visible
{
    assert (0);
}
- (void) setKeepRatioEnabled:(bool) enabled
{
    assert (0);
}
- (void) setFullScreenEnabled:(bool) enabled
{
    assert (0);
}
- (bool) isFullScreenEnabled;
{
    assert (0);
    return false;
}
- (long) duration
{
    assert (0);
    return false;
}
- (long) currentTime
{
    assert (0);
    return 0;
}
-(id) init:(void*) videoPlayer
{
    assert (0);
    return nullptr;
}

-(void) videoFinished:(NSNotification*) notification
{
    assert (0);
    return;
}
@end


@interface UIVideoViewWrapperIosOld : UIVideoViewWrapperIosBase

@property (strong,nonatomic) MPMoviePlayerController * moviePlayer;

@end

@implementation UIVideoViewWrapperIosOld
{
    int _left;
    int _top;
    int _width;
    int _height;
    bool _keepRatioEnabled;
    
    VideoPlayer* _videoPlayer;
}

-(id)init:(void*)videoPlayer
{
    if (self = [super init]) {
        self.moviePlayer = nullptr;
        _videoPlayer = (VideoPlayer*)videoPlayer;
        _keepRatioEnabled = false;
    }
    
    return self;
}

-(void) dealloc
{
    auto view = cocos2d::Director::getInstance()->getOpenGLView();
    auto eaglview = (CCEAGLView *) view->getEAGLView();
    eaglview.backgroundColor = [UIColor blackColor];
    
    if (self.moviePlayer != nullptr) {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:MPMoviePlayerPlaybackDidFinishNotification object:self.moviePlayer];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:MPMoviePlayerPlaybackStateDidChangeNotification object:self.moviePlayer];
        
        [self.moviePlayer stop];
        [self.moviePlayer.view removeFromSuperview];
        self.moviePlayer = nullptr;
        _videoPlayer = nullptr;
    }
    [super dealloc];
}

-(void) setFrame:(int)left :(int)top :(int)width :(int)height
{
    _left = left;
    _width = width;
    _top = top;
    _height = height;
    if (self.moviePlayer != nullptr) {
        [self.moviePlayer.view setFrame:CGRectMake(left, top, width, height)];
    }
}

-(void) setFullScreenEnabled:(bool) enabled
{
    if (self.moviePlayer != nullptr) {
        [self.moviePlayer setFullscreen:enabled animated:(true)];
    }
}

-(bool) isFullScreenEnabled
{
    if (self.moviePlayer != nullptr) {
        return [self.moviePlayer isFullscreen];
    }
    
    return false;
}

-(long) duration {
    long ret = 0L;
    if (self.moviePlayer != nullptr) {
        ret = (long)([self.moviePlayer duration]);
        if( ret < 0 ) {
            ret = 0L;
        }
    }
    return ret;
}

-(long) currentTime {
    long ret = 0L;
    if (self.moviePlayer != nullptr) {
        ret = (long)([self.moviePlayer currentPlaybackTime]);
        if( ret < 0 ) {
            ret = 0L;
        }
    }
    return ret;
}

-(void) setURL:(int)videoSource :(std::string &)videoUrl
{
    if (self.moviePlayer != nullptr) {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:MPMoviePlayerPlaybackDidFinishNotification object:self.moviePlayer];
        [[NSNotificationCenter defaultCenter] removeObserver:self name:MPMoviePlayerPlaybackStateDidChangeNotification object:self.moviePlayer];
        
        [self.moviePlayer stop];
        [self.moviePlayer.view removeFromSuperview];
        self.moviePlayer = nullptr;
    }
    
    if (videoSource == 1) {
        self.moviePlayer = [[[MPMoviePlayerController alloc] init] autorelease];
        self.moviePlayer.movieSourceType = MPMovieSourceTypeStreaming;
        [self.moviePlayer setContentURL:[NSURL URLWithString:@(videoUrl.c_str())]];
    } else {
        self.moviePlayer = [[[MPMoviePlayerController alloc] initWithContentURL:[NSURL fileURLWithPath:@(videoUrl.c_str())]] autorelease];
        self.moviePlayer.movieSourceType = MPMovieSourceTypeFile;
    }
    self.moviePlayer.allowsAirPlay = true;
    self.moviePlayer.controlStyle = MPMovieControlStyleNone;
    self.moviePlayer.view.userInteractionEnabled = true;
    
    auto clearColor = [UIColor clearColor];
    self.moviePlayer.backgroundView.backgroundColor = clearColor;
    self.moviePlayer.view.backgroundColor = clearColor;
    for (UIView * subView in self.moviePlayer.view.subviews) {
        subView.backgroundColor = clearColor;
    }
    
    if (_keepRatioEnabled) {
        self.moviePlayer.scalingMode = MPMovieScalingModeAspectFit;
    } else {
        self.moviePlayer.scalingMode = MPMovieScalingModeFill;
    }
    
    auto view = cocos2d::Director::getInstance()->getOpenGLView();
    auto eaglview = (CCEAGLView *) view->getEAGLView();
    //    [eaglview addSubview:self.moviePlayer.view];
    eaglview.backgroundColor = clearColor;
    
    [[eaglview superview] addSubview:self.moviePlayer.view];
    [[eaglview superview] sendSubviewToBack:self.moviePlayer.view];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(videoFinished:) name:MPMoviePlayerPlaybackDidFinishNotification object:self.moviePlayer];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playStateChange) name:MPMoviePlayerPlaybackStateDidChangeNotification object:self.moviePlayer];
}

-(void) videoFinished:(NSNotification *)notification
{
    if(_videoPlayer != nullptr)
    {
        if([self.moviePlayer playbackState] != MPMoviePlaybackStateStopped)
        {
            _videoPlayer->onPlayEvent((int)VideoPlayer::EventType::COMPLETED);
        }
    }
}

-(void) playStateChange
{
    MPMoviePlaybackState state = [self.moviePlayer playbackState];
    switch (state) {
        case MPMoviePlaybackStatePaused:
            _videoPlayer->onPlayEvent((int)VideoPlayer::EventType::PAUSED);
            break;
        case MPMoviePlaybackStateStopped:
            _videoPlayer->onPlayEvent((int)VideoPlayer::EventType::STOPPED);
            break;
        case MPMoviePlaybackStatePlaying:
            _videoPlayer->onPlayEvent((int)VideoPlayer::EventType::PLAYING);
            break;
        case MPMoviePlaybackStateInterrupted:
            break;
        case MPMoviePlaybackStateSeekingBackward:
            break;
        case MPMoviePlaybackStateSeekingForward:
            break;
        default:
            break;
    }
}

-(void) seekTo:(float)sec
{
    if (self.moviePlayer != NULL) {
        [self.moviePlayer setCurrentPlaybackTime:(sec)];
    }
}

-(void) setVisible:(bool)visible
{
    if (self.moviePlayer != NULL) {
        [self.moviePlayer.view setHidden:!visible];
    }
}

-(void) setKeepRatioEnabled:(bool)enabled
{
    _keepRatioEnabled = enabled;
    if (self.moviePlayer != NULL) {
        if (enabled) {
            self.moviePlayer.scalingMode = MPMovieScalingModeAspectFit;
        } else {
            self.moviePlayer.scalingMode = MPMovieScalingModeFill;
        }
    }
}

-(void) play
{
    if (self.moviePlayer != NULL) {
        [self.moviePlayer.view setFrame:CGRectMake(_left, _top, _width, _height)];
        [self.moviePlayer play];
    }
}

-(void) pause
{
    if (self.moviePlayer != NULL) {
        [self.moviePlayer pause];
    }
}

-(void) resume
{
    if (self.moviePlayer != NULL) {
        if([self.moviePlayer playbackState] == MPMoviePlaybackStatePaused)
        {
            [self.moviePlayer play];
        }
    }
}

-(void) stop
{
    if (self.moviePlayer != NULL) {
        [self.moviePlayer stop];
    }
}

@end
//------------------------------------------------------------------------------------------------------------
//New Player Using AVPlayerController


@interface UIVideoViewWrapperIosNew : UIVideoViewWrapperIosBase

//@property (strong,nonatomic) AVPlayerViewController * moviePlayer;
@property (strong, nonatomic) AVPlayerLayer* movieLayer;

@end

@implementation UIVideoViewWrapperIosNew
{
    int _left;
    int _top;
    int _width;
    int _height;
    bool _keepRatioEnabled;
    
    VideoPlayer* _videoPlayer;
}

-(id)init:(void*)videoPlayer
{
    if (self = [super init]) {
        self.movieLayer = nullptr;
        _videoPlayer = (VideoPlayer*)videoPlayer;
        _keepRatioEnabled = false;
    }
    
    return self;
}


-(void) dealloc
{
    auto view = cocos2d::Director::getInstance()->getOpenGLView();
    auto eaglview = (CCEAGLView *) view->getEAGLView();
    eaglview.backgroundColor = [UIColor blackColor];
    
    if (self.movieLayer != nullptr && self.movieLayer.player) {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:AVPlayerItemDidPlayToEndTimeNotification
                                                      object:self.movieLayer.player.currentItem];
        
        [self.movieLayer.player seekToTime:CMTimeMake(0, 1)];
        [self.movieLayer.player pause];
        
        if( self.movieLayer.superlayer != nullptr)
        {
            UIView * view = (UIView*)(self.movieLayer.superlayer.delegate);
            if(view)
                [view removeFromSuperview];
        }
        [self.movieLayer removeFromSuperlayer];
        self.movieLayer = nullptr;
        _videoPlayer = nullptr;
    }
    [super dealloc];
}

-(void) setFrame:(int)left :(int)top :(int)width :(int)height
{
    _left = left;
    _width = width;
    _top = top;
    _height = height;
    if (self.movieLayer != nullptr) {
        [self.movieLayer setFrame:CGRectMake(left, top, width, height)];
    }
}

-(void) setFullScreenEnabled:(bool) enabled
{
    //    if (self.moviePlayer != nullptr) {
    //        self.moviePlayer.entersFullScreenWhenPlaybackBegins = true;
    //        self.moviePlayer.exitsFullScreenWhenPlaybackEnds = true;
    //    }
}

-(bool) isFullScreenEnabled
{
    if (self.movieLayer != nullptr) {
        //        return [self.moviePlayer isFullscreen];
    }
    
    return false;
}

-(long) duration {
    long ret = 0L;
    if (self.movieLayer != nullptr) {
        ret = (long)CMTimeGetSeconds(([self.movieLayer.player.currentItem duration]));
        if( ret < 0 ) {
            ret = 0L;
        }
    }
    return ret;
}

-(long) currentTime {
    long ret = 0L;
    if (self.movieLayer != nullptr) {
        ret = (long)CMTimeGetSeconds(([self.movieLayer.player.currentItem currentTime]));
        if( ret < 0 ) {
            ret = 0L;
        }
    }
    return ret;
}


-(void) setURL:(int)videoSource :(std::string &)videoUrl
{
    if (self.movieLayer != nullptr) {
        [[NSNotificationCenter defaultCenter] removeObserver:self name:AVPlayerItemDidPlayToEndTimeNotification
                                                      object:self.movieLayer.player.currentItem];
        if(self.movieLayer.player != nullptr)
            [self.movieLayer.player pause];
        UIView* view = (UIView*)self.movieLayer.delegate;
        if(view)
            [view removeFromSuperview];
        self.movieLayer = nullptr;
    }
    
    if(videoSource== 0)
    {
        NSString* path = [NSString stringWithCString:videoUrl.c_str()
                                            encoding:NSUTF8StringEncoding];
        NSURL* url = [[NSURL alloc] initFileURLWithPath:path];
        AVAsset *asset = [AVURLAsset URLAssetWithURL:url options:nil];
        AVPlayerItem *anItem = [AVPlayerItem playerItemWithAsset:asset];
        AVPlayer* player  = [AVPlayer playerWithPlayerItem:anItem];
        self.movieLayer = [AVPlayerLayer playerLayerWithPlayer:player];
    }
    else
    {
        AVPlayer* player = [AVPlayer playerWithURL:[NSURL URLWithString:[NSString stringWithCString:videoUrl.c_str() encoding:NSUTF8StringEncoding]]];
        self.movieLayer = [AVPlayerLayer playerLayerWithPlayer:player];
    }
    
    
    auto clearColor = [UIColor clearColor];
    self.movieLayer.backgroundColor = [clearColor CGColor];
    
    if (_keepRatioEnabled) {
        self.movieLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
    } else {
        self.movieLayer.videoGravity = AVLayerVideoGravityResize;
    }
    
    auto view = cocos2d::Director::getInstance()->getOpenGLView();
    auto eaglview = (CCEAGLView *) view->getEAGLView();
    
    eaglview.backgroundColor = clearColor;
    UIView *newView = [[UIView alloc] initWithFrame:eaglview.bounds];
    [newView.layer addSublayer:self.movieLayer];
    newView.userInteractionEnabled = false;
    [[eaglview superview] addSubview:newView];
    [[eaglview superview] sendSubviewToBack:newView];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(videoFinished:)
                                                 name:AVPlayerItemDidPlayToEndTimeNotification object:self
     .movieLayer.player.currentItem];
    [self.movieLayer.player addObserver:self forKeyPath:@"rate" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:self.movieLayer.player];
    
    [self.movieLayer.player addObserver:self forKeyPath:@"status" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld context:self.movieLayer.player];
    
}

-(void) videoFinished:(NSNotification *)notification
{
    if(_videoPlayer != nullptr)
    {
        //        if([self.movieLayer.player timeControlStatus] != MPMoviePlaybackStateStopped)
        //        {
        _videoPlayer->onPlayEvent((int)VideoPlayer::EventType::COMPLETED);
        //        }
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context {
    //todo 暂时视频播放不需要复杂功能， 后面需要的时候再添加
    if (context == (self.movieLayer.player)) {
        if([keyPath isEqualToString:@"status"])
        {
            if (self.movieLayer.player.status == AVPlayerStatusFailed) {
                //                NSLog(@"AVPlayer Failed");
            } else if (self.movieLayer.player.status == AVPlayerStatusReadyToPlay) {
                //                NSLog(@"AVPlayer Ready to Play");
            } else if (self.movieLayer.player.status == AVPlayerStatusUnknown) {
                //                NSLog(@"AVPlayer Unknown");
            }
        }
        
        //        NSLog(@"AVPlayer change %@, %@, %@", keyPath, [change valueForKey:@"old"], [change valueForKey:@"new"]);
        return;
    }
    
    [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    return;
}

-(void) playStateChange
{
    //    MPMoviePlaybackState state = [self.moviePlayer playbackState];
    //    switch (state) {
    //        case MPMoviePlaybackStatePaused:
    //            _videoPlayer->onPlayEvent((int)VideoPlayer::EventType::PAUSED);
    //            break;
    //        case MPMoviePlaybackStateStopped:
    //            _videoPlayer->onPlayEvent((int)VideoPlayer::EventType::STOPPED);
    //            break;
    //        case MPMoviePlaybackStatePlaying:
    //            _videoPlayer->onPlayEvent((int)VideoPlayer::EventType::PLAYING);
    //            break;
    //        case MPMoviePlaybackStateInterrupted:
    //            break;
    //        case MPMoviePlaybackStateSeekingBackward:
    //            break;
    //        case MPMoviePlaybackStateSeekingForward:
    //            break;
    //        default:
    //            break;
    //    }
}

-(void) seekTo:(float)sec
{
    if (self.movieLayer != NULL) {
        [self.movieLayer.player seekToTime:CMTimeMakeWithSeconds(sec, [self.movieLayer.player currentTime].timescale)];
    }
}

-(void) setVisible:(bool)visible
{
    if (self.movieLayer != NULL) {
        [self.movieLayer setHidden:!visible];
    }
}

-(void) setKeepRatioEnabled:(bool)enabled
{
    _keepRatioEnabled = enabled;
    if (self.movieLayer != NULL) {
        if (enabled) {
            self.movieLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
        } else {
            self.movieLayer.videoGravity = AVLayerVideoGravityResize;
        }
    }
}

-(void) play
{
    if (self.movieLayer != NULL) {
        [self.movieLayer setFrame:CGRectMake(_left, _top, _width, _height)];
        [self.movieLayer.player play];
    }
}

-(void) pause
{
    if (self.movieLayer != NULL) {
        [self.movieLayer.player pause];
    }
}

-(void) resume
{
    if (self.movieLayer != NULL) {
        if([self.movieLayer.player rate] == 0.0)
        {
            [self.movieLayer.player play];
        }
    }
}

-(void) stop
{
    if (self.movieLayer != NULL) {
        [self.movieLayer.player seekToTime:CMTimeMake(0, 1)];
        [self.movieLayer.player pause];
    }
}



@end

VideoPlayer::VideoPlayer()
: _videoPlayerIndex(-1)
, _eventCallback(nullptr)
, _fullScreenEnabled(false)
, _fullScreenDirty(false)
, _keepAspectRatioEnabled(false)
, _isPlaying(false)
{
    if(@available(iOS 13.0, *))
    {
        _videoView = [[UIVideoViewWrapperIosNew alloc] init:this];
    }
    else
    {
        _videoView =[[UIVideoViewWrapperIosOld alloc] init:this];
    }
#if CC_VIDEOPLAYER_DEBUG_DRAW
    _debugDrawNode = DrawNode::create();
    addChild(_debugDrawNode);
#endif
}

VideoPlayer::~VideoPlayer()
{
    if(_videoView)
    {
        [((UIVideoViewWrapperIosBase*)_videoView) dealloc];
    }
}

void VideoPlayer::setFileName(const std::string& fileName)
{
    _videoURL = FileUtils::getInstance()->fullPathForFilename(fileName);
    _videoSource = VideoPlayer::Source::FILENAME;
    [((UIVideoViewWrapperIosBase*)_videoView) setURL:(int)_videoSource :_videoURL];
}

void VideoPlayer::setURL(const std::string& videoUrl)
{
    _videoURL = videoUrl;
    _videoSource = VideoPlayer::Source::URL;
    [((UIVideoViewWrapperIosBase*)_videoView) setURL:(int)_videoSource :_videoURL];
}

void VideoPlayer::draw(Renderer* renderer, const Mat4 &transform, uint32_t flags)
{
    cocos2d::ui::Widget::draw(renderer,transform,flags);
    
    if (flags & FLAGS_TRANSFORM_DIRTY)
    {
        auto directorInstance = Director::getInstance();
        auto glView = directorInstance->getOpenGLView();
        auto frameSize = glView->getFrameSize();
        auto scaleFactor = [static_cast<CCEAGLView *>(glView->getEAGLView()) contentScaleFactor];
        
        auto winSize = directorInstance->getWinSize();
        
        auto leftBottom = convertToWorldSpace(Vec2::ZERO);
        auto rightTop = convertToWorldSpace(Vec2(_contentSize.width,_contentSize.height));
        
        auto uiLeft = (frameSize.width / 2 + (leftBottom.x - winSize.width / 2 ) * glView->getScaleX()) / scaleFactor;
        auto uiTop = (frameSize.height /2 - (rightTop.y - winSize.height / 2) * glView->getScaleY()) / scaleFactor;
        
        [((UIVideoViewWrapperIosBase*)_videoView) setFrame :uiLeft :uiTop
                                                           :(rightTop.x - leftBottom.x) * glView->getScaleX() / scaleFactor
                                                           :( (rightTop.y - leftBottom.y) * glView->getScaleY()/scaleFactor)];
    }
    
#if CC_VIDEOPLAYER_DEBUG_DRAW
    _debugDrawNode->clear();
    auto size = getContentSize();
    Point vertices[4]=
    {
        Point::ZERO,
        Point(size.width, 0),
        Point(size.width, size.height),
        Point(0, size.height)
    };
    _debugDrawNode->drawPoly(vertices, 4, true, Color4F(1.0, 1.0, 1.0, 1.0));
#endif
}

bool VideoPlayer::isFullScreenEnabled()const
{
    return [((UIVideoViewWrapperIosBase*)_videoView) isFullScreenEnabled];
}

void VideoPlayer::setFullScreenEnabled(bool enabled)
{
    [((UIVideoViewWrapperIosBase*)_videoView) setFullScreenEnabled:enabled];
}

void VideoPlayer::setKeepAspectRatioEnabled(bool enable)
{
    if (_keepAspectRatioEnabled != enable)
    {
        _keepAspectRatioEnabled = enable;
        [((UIVideoViewWrapperIosBase*)_videoView) setKeepRatioEnabled:enable];
    }
}

void VideoPlayer::play()
{
    if (! _videoURL.empty())
    {
        [((UIVideoViewWrapperIosBase*)_videoView) play];
    }
}

void VideoPlayer::pause()
{
    if (! _videoURL.empty())
    {
        [((UIVideoViewWrapperIosBase*)_videoView) pause];
    }
}

void VideoPlayer::resume()
{
    if (! _videoURL.empty())
    {
        [((UIVideoViewWrapperIosBase*)_videoView) resume];
    }
}

void VideoPlayer::stop()
{
    if (! _videoURL.empty())
    {
        [((UIVideoViewWrapperIosBase*)_videoView) stop];
    }
}

void VideoPlayer::seekTo(float sec)
{
    if (! _videoURL.empty())
    {
        [((UIVideoViewWrapperIosBase*)_videoView) seekTo:sec];
    }
}

bool VideoPlayer::isPlaying() const
{
    return _isPlaying;
}

long VideoPlayer::getDuration() {
    return [((UIVideoViewWrapperIosBase*)_videoView) duration];
}
long VideoPlayer::getCurrentTime() {
    return [((UIVideoViewWrapperIosBase*)_videoView) currentTime];
}

void VideoPlayer::setVisible(bool visible)
{
    cocos2d::ui::Widget::setVisible(visible);
    
    if (! _videoURL.empty())
    {
        [((UIVideoViewWrapperIosBase*)_videoView) setVisible:visible];
    }
}

void VideoPlayer::addEventListener(const VideoPlayer::ccVideoPlayerCallback& callback)
{
    _eventCallback = callback;
}

void VideoPlayer::onPlayEvent(int event)
{
    if (event == (int)VideoPlayer::EventType::PLAYING) {
        _isPlaying = true;
    } else {
        _isPlaying = false;
    }
    
    if (_eventCallback)
    {
        _eventCallback(this, (VideoPlayer::EventType)event);
    }
}

cocos2d::ui::Widget* VideoPlayer::createCloneInstance()
{
    return VideoPlayer::create();
}

void VideoPlayer::copySpecialProperties(Widget *widget)
{
    VideoPlayer* videoPlayer = dynamic_cast<VideoPlayer*>(widget);
    if (videoPlayer)
    {
        _isPlaying = videoPlayer->_isPlaying;
        _fullScreenEnabled = videoPlayer->_fullScreenEnabled;
        _fullScreenDirty = videoPlayer->_fullScreenDirty;
        _videoURL = videoPlayer->_videoURL;
        _keepAspectRatioEnabled = videoPlayer->_keepAspectRatioEnabled;
        _videoSource = videoPlayer->_videoSource;
        _videoPlayerIndex = videoPlayer->_videoPlayerIndex;
        _eventCallback = videoPlayer->_eventCallback;
        _videoView = videoPlayer->_videoView;
    }
}

#endif
