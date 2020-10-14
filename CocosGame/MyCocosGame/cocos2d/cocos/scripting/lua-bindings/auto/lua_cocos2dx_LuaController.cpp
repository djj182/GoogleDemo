#include "lua_cocos2dx_LuaController.hpp"
#include "cocos2d.h"
#include "CCProtectedNode.h"
#include "CCAsyncTaskPool.h"
#include "SpritePolygonCache.h"
#include "tolua_fix.h"
#include "LuaBasicConversions.h"
/* method: getInstance of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getInstance00
static int tolua_Cocos2d_LuaController_getInstance00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        {
            LuaController* tolua_ret = (LuaController*)  LuaController::getInstance();
            tolua_pushusertype(tolua_S,(void*)tolua_ret,"LuaController");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getInstance'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: doResourceByCommonIndex of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_doResourceByCommonIndex00
static int tolua_Cocos2d_LuaController_doResourceByCommonIndex00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isboolean(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        int commonIndex = ((int)  tolua_tonumber(tolua_S,2,0));
        bool isLoad = ((bool)  tolua_toboolean(tolua_S,3,0));
        {
            LuaController::doResourceByCommonIndex(commonIndex,isLoad);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'doResourceByCommonIndex'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getResourceNameByType of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getResourceNameByType00
static int tolua_Cocos2d_LuaController_getResourceNameByType00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        int itemid = ((int)  tolua_tonumber(tolua_S,2,0));
        {
            std::string tolua_ret = (std::string)  LuaController::getResourceNameByType(itemid);
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getResourceNameByType'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: createParticleForLua of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_createParticleForLua00
static int tolua_Cocos2d_LuaController_createParticleForLua00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        const char* name = ((const char*)  tolua_tostring(tolua_S,2,0));
        {
            CCParticleSystemQuad* tolua_ret = (CCParticleSystemQuad*)  LuaController::createParticleForLua(name);
            int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
            int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCParticleSystemQuad");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'createParticleForLua'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getLang of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getLang00
static int tolua_Cocos2d_LuaController_getLang00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string msg = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        {
            std::string tolua_ret = (std::string)  LuaController::getLang(msg);
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
            tolua_pushcppstring(tolua_S,(const char*)msg);
        }
    }
    return 2;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getLang'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getLang1 of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getLang100
static int tolua_Cocos2d_LuaController_getLang100(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string msg = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        std::string param = ((std::string)  tolua_tocppstring(tolua_S,3,0));
        {
            std::string tolua_ret = (std::string)  LuaController::getLang1(msg,param);
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
            tolua_pushcppstring(tolua_S,(const char*)msg);
            tolua_pushcppstring(tolua_S,(const char*)param);
        }
    }
    return 3;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getLang1'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getDollarString of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getDollarString00
static int tolua_Cocos2d_LuaController_getDollarString00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        string dollar = ((string)  tolua_tocppstring(tolua_S,2,0));
        string productId = ((string)  tolua_tocppstring(tolua_S,3,0));
        {
            std::string tolua_ret = (std::string)  LuaController::getDollarString(dollar,productId);
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
            tolua_pushcppstring(tolua_S,(const char*)dollar);
            tolua_pushcppstring(tolua_S,(const char*)productId);
        }
    }
    return 3;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getDollarString'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getCMDLang of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getCMDLang00
static int tolua_Cocos2d_LuaController_getCMDLang00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string msg = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        {
            std::string tolua_ret = (std::string)  LuaController::getCMDLang(msg);
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
            tolua_pushcppstring(tolua_S,(const char*)msg);
        }
    }
    return 2;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getCMDLang'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: addButtonLight of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_addButtonLight00
static int tolua_Cocos2d_LuaController_addButtonLight00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"CCNode",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        CCNode* node = ((CCNode*)  tolua_tousertype(tolua_S,2,0));
        {
            LuaController::addButtonLight(node);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'addButtonLight'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: addItemIcon of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_addItemIcon00
static int tolua_Cocos2d_LuaController_addItemIcon00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"CCNode",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isusertype(tolua_S,4,"CCLabelTTF",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        CCNode* node = ((CCNode*)  tolua_tousertype(tolua_S,2,0));
        std::string itemid = ((std::string)  tolua_tocppstring(tolua_S,3,0));
        CCLabelTTF* lable = ((CCLabelTTF*)  tolua_tousertype(tolua_S,4,0));
        {
            LuaController::addItemIcon(node,itemid,lable);
            tolua_pushcppstring(tolua_S,(const char*)itemid);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'addItemIcon'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: addIconByType of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_addIconByType00
static int tolua_Cocos2d_LuaController_addIconByType00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"CCNode",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isusertype(tolua_S,4,"CCLabelTTF",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,7,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        CCNode* node = ((CCNode*)  tolua_tousertype(tolua_S,2,0));
        std::string itemid = ((std::string)  tolua_tocppstring(tolua_S,3,0));
        CCLabelTTF* lable = ((CCLabelTTF*)  tolua_tousertype(tolua_S,4,0));
        int type = ((int)  tolua_tonumber(tolua_S,5,0));
        int size = ((int)  tolua_tonumber(tolua_S,6,0));
        {
            LuaController::addIconByType(node,itemid,lable,type,size);
            tolua_pushcppstring(tolua_S,(const char*)itemid);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'addIconByType'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getWorldTime of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getWorldTime00
static int tolua_Cocos2d_LuaController_getWorldTime00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        {
            int tolua_ret = (int)  LuaController::getWorldTime();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getWorldTime'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSECLang of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getSECLang00
static int tolua_Cocos2d_LuaController_getSECLang00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        int time = ((int)  tolua_tonumber(tolua_S,2,0));
        {
            std::string tolua_ret = (std::string)  LuaController::getSECLang(time);
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getSECLang'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: callPaymentToFriend of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_callPaymentToFriend00
static int tolua_Cocos2d_LuaController_callPaymentToFriend00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string itemid = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        std::string touid = ((std::string)  tolua_tocppstring(tolua_S,3,0));
        std::string toName = ((std::string)  tolua_tocppstring(tolua_S,4,0));
        {
            LuaController::callPaymentToFriend(itemid,touid,toName);
            tolua_pushcppstring(tolua_S,(const char*)itemid);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'callPaymentToFriend'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: callPayment of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_callPayment00
static int tolua_Cocos2d_LuaController_callPayment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string itemid = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        {
            LuaController::callPayment(itemid);
            tolua_pushcppstring(tolua_S,(const char*)itemid);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'callPayment'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: removeAllPopup of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_removeAllPopup00
static int tolua_Cocos2d_LuaController_removeAllPopup00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        {
            LuaController::removeAllPopup();
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'removeAllPopup'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: showDetailPopup of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_showDetailPopup00
static int tolua_Cocos2d_LuaController_showDetailPopup00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string itemid = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        {
            LuaController::showDetailPopup(itemid);
            tolua_pushcppstring(tolua_S,(const char*)itemid);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'showDetailPopup'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: checkSkeletonFile of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_checkSkeletonFile00
static int tolua_Cocos2d_LuaController_checkSkeletonFile00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        const char* fileName = ((const char*)  tolua_tostring(tolua_S,2,0));
        {
            bool tolua_ret = (bool)  LuaController::checkSkeletonFile(fileName);
            tolua_pushboolean(tolua_S,(bool)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'checkSkeletonFile'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: addSkeletonAnimation of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_addSkeletonAnimation00
static int tolua_Cocos2d_LuaController_addSkeletonAnimation00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"CCNode",0,&tolua_err) ||
        !tolua_isstring(tolua_S,3,0,&tolua_err) ||
        !tolua_isstring(tolua_S,4,0,&tolua_err) ||
        !tolua_isstring(tolua_S,5,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,6,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,7,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        CCNode* node = ((CCNode*)  tolua_tousertype(tolua_S,2,0));
        const char* skeletonDataFile = ((const char*)  tolua_tostring(tolua_S,3,0));
        const char* atlasFile = ((const char*)  tolua_tostring(tolua_S,4,0));
        const char* animation = ((const char*)  tolua_tostring(tolua_S,5,0));
        float scale = ((float)  tolua_tonumber(tolua_S,6,0));
        {
            LuaController::addSkeletonAnimation(node,skeletonDataFile,atlasFile,animation,scale);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'addSkeletonAnimation'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getStringLength of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getStringLength00
static int tolua_Cocos2d_LuaController_getStringLength00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string str = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        {
            int tolua_ret = (int)  LuaController::getStringLength(str);
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
            tolua_pushcppstring(tolua_S,(const char*)str);
        }
    }
    return 2;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getStringLength'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getUTF8SubString of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getUTF8SubString00
static int tolua_Cocos2d_LuaController_getUTF8SubString00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string str = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        int start = ((int)  tolua_tonumber(tolua_S,3,0));
        int end = ((int)  tolua_tonumber(tolua_S,4,0));
        {
            std::string tolua_ret = (std::string)  LuaController::getUTF8SubString(str,start,end);
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
            tolua_pushcppstring(tolua_S,(const char*)str);
        }
    }
    return 2;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getUTF8SubString'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: toSelectUser of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_toSelectUser00
static int tolua_Cocos2d_LuaController_toSelectUser00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isboolean(tolua_S,3,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string itemid = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        bool removeAllPop = ((bool)  tolua_toboolean(tolua_S,3,0));
        int backTo = ((int)  tolua_tonumber(tolua_S,4,0));
        {
            LuaController::toSelectUser(itemid,removeAllPop,backTo);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'toSelectUser'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPlatform of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getPlatform00
static int tolua_Cocos2d_LuaController_getPlatform00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        {
            std::string tolua_ret = (std::string)  LuaController::getPlatform();
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getPlatform'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getLanguage of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getLanguage00
static int tolua_Cocos2d_LuaController_getLanguage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        {
            std::string tolua_ret = (std::string)  LuaController::getLanguage();
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getLanguage'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSaleViewContainer of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getSaleViewContainer00
static int tolua_Cocos2d_LuaController_getSaleViewContainer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getSaleViewContainer'", NULL);
#endif
        {
            CCNode* tolua_ret = (CCNode*)  self->getSaleViewContainer();
            int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
            int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCNode");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getSaleViewContainer'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getAdvCellContainer of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getAdvCellContainer00
static int tolua_Cocos2d_LuaController_getAdvCellContainer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getAdvCellContainer'", NULL);
#endif
        {
            CCNode* tolua_ret = (CCNode*)  self->getAdvCellContainer();
            int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
            int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCNode");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getAdvCellContainer'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getAdvCell of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getAdvCell00
static int tolua_Cocos2d_LuaController_getAdvCell00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getAdvCell'", NULL);
#endif
        {
            CCNode* tolua_ret = (CCNode*)  self->getAdvCell();
            int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
            int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCNode");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getAdvCell'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getIconContainer of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getIconContainer00
static int tolua_Cocos2d_LuaController_getIconContainer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getIconContainer'", NULL);
#endif
        {
            CCNode* tolua_ret = (CCNode*)  self->getIconContainer();
            int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
            int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCNode");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getIconContainer'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: closeChatNotice of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_closeChatNotice00
static int tolua_Cocos2d_LuaController_closeChatNotice00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'closeChatNotice'", NULL);
#endif
        {
            self->closeChatNotice();
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'closeChatNotice'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: sendCMD of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_sendCMD00
static int tolua_Cocos2d_LuaController_sendCMD00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isusertype(tolua_S,3,"CCDictionary",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
        std::string cmdName = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        CCDictionary* dict = ((CCDictionary*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'sendCMD'", NULL);
#endif
        {
            self->sendCMD(cmdName,dict);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'sendCMD'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: backCMD of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_backCMD00
static int tolua_Cocos2d_LuaController_backCMD00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isusertype(tolua_S,3,"CCDictionary",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
        std::string cmdName = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        CCDictionary* dict = ((CCDictionary*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'backCMD'", NULL);
#endif
        {
            self->backCMD(cmdName,dict);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'backCMD'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getCMDParams of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getCMDParams00
static int tolua_Cocos2d_LuaController_getCMDParams00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
        std::string cmdName = ((std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCMDParams'", NULL);
#endif
        {
            CCDictionary* tolua_ret = (CCDictionary*)  self->getCMDParams(cmdName);
            int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
            int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCDictionary");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getCMDParams'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getChatNoticeContainer of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getChatNoticeContainer00
static int tolua_Cocos2d_LuaController_getChatNoticeContainer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getChatNoticeContainer'", NULL);
#endif
        {
            CCNode* tolua_ret = (CCNode*)  self->getChatNoticeContainer();
            int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
            int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCNode");
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getChatNoticeContainer'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: removeLastPopup of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_removeLastPopup00
static int tolua_Cocos2d_LuaController_removeLastPopup00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        {
            LuaController::removeLastPopup();
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'removeLastPopup'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getTimeStamp of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getTimeStamp00
static int tolua_Cocos2d_LuaController_getTimeStamp00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        {
            int tolua_ret = (int)  LuaController::getTimeStamp();
            tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getTimeStamp'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: playEffects of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_playEffects00
static int tolua_Cocos2d_LuaController_playEffects00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string name = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        {
            LuaController::playEffects(name);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'playEffects'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTitleName of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_setTitleName00
static int tolua_Cocos2d_LuaController_setTitleName00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string _name = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        {
            LuaController::setTitleName(_name);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'setTitleName'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getActExcData of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getActExcData00
static int tolua_Cocos2d_LuaController_getActExcData00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getActExcData'", NULL);
#endif
        {
            self->getActExcData();
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getActExcData'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: showActRwdViewByActId of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_showActRwdViewByActId00
static int tolua_Cocos2d_LuaController_showActRwdViewByActId00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertype(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        LuaController* self = (LuaController*)  tolua_tousertype(tolua_S,1,0);
        std::string actId = ((std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
        if (!self) tolua_error(tolua_S,"invalid 'self' in function 'showActRwdViewByActId'", NULL);
#endif
        {
            self->showActRwdViewByActId(actId);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'showActRwdViewByActId'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: createGoodsIcon of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_createGoodsIcon00
static int tolua_Cocos2d_LuaController_createGoodsIcon00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string itemId = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        int size = ((int)  tolua_tonumber(tolua_S,3,0));
        {
            CCSprite* tolua_ret = (CCSprite*)  LuaController::createGoodsIcon(itemId,size);
            int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
            int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCSprite");
            tolua_pushcppstring(tolua_S,(const char*)itemId);
        }
    }
    return 2;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'createGoodsIcon'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: removeAllPopupView of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_removeAllPopupView00
static int tolua_Cocos2d_LuaController_removeAllPopupView00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,2,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        {
            LuaController::removeAllPopupView();
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'removeAllPopupView'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: openBagView of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_openBagView00
static int tolua_Cocos2d_LuaController_openBagView00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        int type = ((int)  tolua_tonumber(tolua_S,2,0));
        {
            LuaController::openBagView(type);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'openBagView'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getGoodsInfo of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getGoodsInfo00
static int tolua_Cocos2d_LuaController_getGoodsInfo00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string itemId = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        {
            CCDictionary* tolua_ret = (CCDictionary*)  LuaController::getGoodsInfo(itemId);
            int nID = (tolua_ret) ? (int)tolua_ret->m_uID : -1;
            int* pLuaID = (tolua_ret) ? &tolua_ret->m_nLuaID : NULL;
            toluafix_pushusertype_ccobject(tolua_S, nID, pLuaID, (void*)tolua_ret,"CCDictionary");
            tolua_pushcppstring(tolua_S,(const char*)itemId);
        }
    }
    return 2;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getGoodsInfo'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getObjectType of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_getObjectType00
static int tolua_Cocos2d_LuaController_getObjectType00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"CCObject",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        CCObject* obj = ((CCObject*)  tolua_tousertype(tolua_S,2,0));
        {
            std::string tolua_ret = (std::string)  LuaController::getObjectType(obj);
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'getObjectType'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: retReward of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_retReward00
static int tolua_Cocos2d_LuaController_retReward00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_isusertype(tolua_S,2,"CCArray",0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,3,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        CCArray* tolua_var_1 = ((CCArray*)  tolua_tousertype(tolua_S,2,0));
        {
            std::string tolua_ret = (std::string)  LuaController::retReward(tolua_var_1);
            tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
        }
    }
    return 1;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'retReward'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: flyHint of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_flyHint00
static int tolua_Cocos2d_LuaController_flyHint00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,4,0,&tolua_err) ||
        !tolua_isnoobj(tolua_S,5,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string icon = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        std::string titleText = ((std::string)  tolua_tocppstring(tolua_S,3,0));
        std::string context = ((std::string)  tolua_tocppstring(tolua_S,4,0));
        {
            LuaController::flyHint(icon,titleText,context);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'flyHint'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: showTipsView of class  LuaController */
#ifndef TOLUA_DISABLE_tolua_Cocos2d_LuaController_showTipsView00
static int tolua_Cocos2d_LuaController_showTipsView00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
    tolua_Error tolua_err;
    if (
        !tolua_isusertable(tolua_S,1,"LuaController",0,&tolua_err) ||
        !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
        !tolua_isnumber(tolua_S,3,1,&tolua_err) ||
        !tolua_isnoobj(tolua_S,4,&tolua_err)
        )
        goto tolua_lerror;
    else
#endif
    {
        std::string tolua_var_2 = ((std::string)  tolua_tocppstring(tolua_S,2,0));
        CCTextAlignment align = ((CCTextAlignment) (int)  tolua_tonumber(tolua_S,3,kCCTextAlignmentCenter));
        {
            LuaController::showTipsView(tolua_var_2,align);
        }
    }
    return 0;
#ifndef TOLUA_RELEASE
tolua_lerror:
    tolua_error(tolua_S,"#ferror in function 'showTipsView'.",&tolua_err);
    return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

TOLUA_API int register_all_LuaController (lua_State* tolua_S)
{
    tolua_cclass(tolua_S,"LuaController","LuaController","",NULL);
    tolua_beginmodule(tolua_S,"LuaController");
    tolua_function(tolua_S,"getInstance",tolua_Cocos2d_LuaController_getInstance00);
    tolua_function(tolua_S,"doResourceByCommonIndex",tolua_Cocos2d_LuaController_doResourceByCommonIndex00);
    tolua_function(tolua_S,"getResourceNameByType",tolua_Cocos2d_LuaController_getResourceNameByType00);
    tolua_function(tolua_S,"createParticleForLua",tolua_Cocos2d_LuaController_createParticleForLua00);
    tolua_function(tolua_S,"getLang",tolua_Cocos2d_LuaController_getLang00);
    tolua_function(tolua_S,"getLang1",tolua_Cocos2d_LuaController_getLang100);
    tolua_function(tolua_S,"getDollarString",tolua_Cocos2d_LuaController_getDollarString00);
    tolua_function(tolua_S,"getCMDLang",tolua_Cocos2d_LuaController_getCMDLang00);
    tolua_function(tolua_S,"addButtonLight",tolua_Cocos2d_LuaController_addButtonLight00);
    tolua_function(tolua_S,"addItemIcon",tolua_Cocos2d_LuaController_addItemIcon00);
    tolua_function(tolua_S,"addIconByType",tolua_Cocos2d_LuaController_addIconByType00);
    tolua_function(tolua_S,"getWorldTime",tolua_Cocos2d_LuaController_getWorldTime00);
    tolua_function(tolua_S,"getSECLang",tolua_Cocos2d_LuaController_getSECLang00);
    tolua_function(tolua_S,"callPaymentToFriend",tolua_Cocos2d_LuaController_callPaymentToFriend00);
    tolua_function(tolua_S,"callPayment",tolua_Cocos2d_LuaController_callPayment00);
    tolua_function(tolua_S,"removeAllPopup",tolua_Cocos2d_LuaController_removeAllPopup00);
    tolua_function(tolua_S,"showDetailPopup",tolua_Cocos2d_LuaController_showDetailPopup00);
    tolua_function(tolua_S,"checkSkeletonFile",tolua_Cocos2d_LuaController_checkSkeletonFile00);
    tolua_function(tolua_S,"addSkeletonAnimation",tolua_Cocos2d_LuaController_addSkeletonAnimation00);
    tolua_function(tolua_S,"getStringLength",tolua_Cocos2d_LuaController_getStringLength00);
    tolua_function(tolua_S,"getUTF8SubString",tolua_Cocos2d_LuaController_getUTF8SubString00);
    tolua_function(tolua_S,"toSelectUser",tolua_Cocos2d_LuaController_toSelectUser00);
    tolua_function(tolua_S,"getPlatform",tolua_Cocos2d_LuaController_getPlatform00);
    tolua_function(tolua_S,"getLanguage",tolua_Cocos2d_LuaController_getLanguage00);
    tolua_function(tolua_S,"getSaleViewContainer",tolua_Cocos2d_LuaController_getSaleViewContainer00);
    tolua_function(tolua_S,"getAdvCellContainer",tolua_Cocos2d_LuaController_getAdvCellContainer00);
    tolua_function(tolua_S,"getAdvCell",tolua_Cocos2d_LuaController_getAdvCell00);
    tolua_function(tolua_S,"getIconContainer",tolua_Cocos2d_LuaController_getIconContainer00);
    tolua_function(tolua_S,"closeChatNotice",tolua_Cocos2d_LuaController_closeChatNotice00);
    tolua_function(tolua_S,"sendCMD",tolua_Cocos2d_LuaController_sendCMD00);
    tolua_function(tolua_S,"backCMD",tolua_Cocos2d_LuaController_backCMD00);
    tolua_function(tolua_S,"getCMDParams",tolua_Cocos2d_LuaController_getCMDParams00);
    tolua_function(tolua_S,"getChatNoticeContainer",tolua_Cocos2d_LuaController_getChatNoticeContainer00);
    tolua_function(tolua_S,"removeLastPopup",tolua_Cocos2d_LuaController_removeLastPopup00);
    tolua_function(tolua_S,"getTimeStamp",tolua_Cocos2d_LuaController_getTimeStamp00);
    tolua_function(tolua_S,"playEffects",tolua_Cocos2d_LuaController_playEffects00);
    tolua_function(tolua_S,"setTitleName",tolua_Cocos2d_LuaController_setTitleName00);
    tolua_function(tolua_S,"getActExcData",tolua_Cocos2d_LuaController_getActExcData00);
    tolua_function(tolua_S,"showActRwdViewByActId",tolua_Cocos2d_LuaController_showActRwdViewByActId00);
    tolua_function(tolua_S,"createGoodsIcon",tolua_Cocos2d_LuaController_createGoodsIcon00);
    tolua_function(tolua_S,"removeAllPopupView",tolua_Cocos2d_LuaController_removeAllPopupView00);
    tolua_function(tolua_S,"openBagView",tolua_Cocos2d_LuaController_openBagView00);
    tolua_function(tolua_S,"getGoodsInfo",tolua_Cocos2d_LuaController_getGoodsInfo00);
    tolua_function(tolua_S,"getObjectType",tolua_Cocos2d_LuaController_getObjectType00);
    tolua_function(tolua_S,"retReward",tolua_Cocos2d_LuaController_retReward00);
    tolua_function(tolua_S,"flyHint",tolua_Cocos2d_LuaController_flyHint00);
    tolua_function(tolua_S,"showTipsView",tolua_Cocos2d_LuaController_showTipsView00);
    tolua_endmodule(tolua_S);
    return 1;
}
