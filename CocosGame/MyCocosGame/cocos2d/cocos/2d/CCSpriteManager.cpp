
#include "CCSpriteManager.hpp"

NS_CC_BEGIN

static CCSpriteManager *_sharedSpriteManager = nullptr;


CCSpriteManager* CCSpriteManager::getInstance()
{
    if (! _sharedSpriteManager)
    {
        _sharedSpriteManager = new (std::nothrow) CCSpriteManager();
    }
    
    return _sharedSpriteManager;
}

void CCSpriteManager::destroyInstance()
{
    CC_SAFE_RELEASE_NULL(_sharedSpriteManager);
}

void CCSpriteManager::addSprite(Sprite* spr)
{
    all_sprs_.insert(spr);
}

void CCSpriteManager::removeSprite(Sprite* spr)
{
    all_sprs_.erase(spr);
}
    
void CCSpriteManager::updateSpriteFrame(SpriteFrame* sf)
{
    for (auto ite = all_sprs_.begin(); ite != all_sprs_.end(); ++ite)
    {
        Sprite* spr = (*ite);
        if (spr->getSpriteFrame2() == sf)
        {
            spr->setSpriteFrame(sf);
        }
    }
}


NS_CC_END

