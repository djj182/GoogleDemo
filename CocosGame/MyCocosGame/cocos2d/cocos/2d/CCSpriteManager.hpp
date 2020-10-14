//
//  CCSpriteManager.hpp
//  libcocos2d Mac
//
//  Created by liusiyang on 2019/3/6.
//  主要是为了资源下载完毕之后的更新
//  本来是想用一个侵入式链表，但是感觉有点太C了，于是使用set吧

#ifndef CCSpriteManager_hpp
#define CCSpriteManager_hpp

#include "2d/CCSprite.h"
#include <set>

NS_CC_BEGIN

class CCSpriteManager : public Ref
{
public:
    static CCSpriteManager* getInstance();
    static void destroyInstance();
    
    void addSprite(Sprite* spr);
    void removeSprite(Sprite* spr);
  
    void updateSpriteFrame(SpriteFrame* sf);
    
private:
    std::set<Sprite*> all_sprs_;
};

NS_CC_END
    
#endif /* CCSpriteManager_hpp */
