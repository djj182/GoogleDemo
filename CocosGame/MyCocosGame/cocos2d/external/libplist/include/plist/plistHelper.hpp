//
//  plistHelper.hpp
//  cocos2d_libs
//
//  Created by liusiyang on 2018/4/25.
//  一些为了方便plist的帮助函数
//  暂时写的比较简易，其实可以考虑和Value接口兼容的，但是目前看没有太大意义
//  期待未来的你们来补充完善！！
//

#ifndef plistHelper_h
#define plistHelper_h

#include "plist.h"

class PlistNode;

// PlistNode数组
typedef std::vector<PlistNode> PlistNodeVector;

class PlistNode
{
public:
    PlistNode(plist_t n) : node_(n)
    {
    }

    /** Gets as an integer value. Will convert to integer if possible, or will trigger assert error. */
    int asInt() const {
        if (node_) {
            uint64_t v;
            plist_get_uint_val(node_, &v);
            return (int) v;
        }
        return 0;
    }
    
    /** Gets as a float value. Will convert to float if possible, or will trigger assert error. */
    float asFloat() const {
        if (node_) {
            double v;
            plist_get_real_val(node_, &v);
            return (float) v;
        }
        return 0.f;
    }
    
    /** Gets as a double value. Will convert to double if possible, or will trigger assert error. */
    double asDouble() const {
        if (node_) {
            double v;
            plist_get_real_val(node_, &v);
            return v;
        }
        return 0.f;
    }
    
    /** Gets as a bool value. Will convert to bool if possible, or will trigger assert error. */
    bool asBool() const {
        if (node_) {
            uint8_t v;
            plist_get_bool_val(node_, &v);
            return v;
        }
        return false;
    }
    
    /** Gets as a string value. Will convert to string if possible, or will trigger assert error. */
    std::string asString() const {
        std::string ret;
        if (node_) {
            char* v;
            plist_get_string_val(node_, &v);
            if (v) {
                ret = v;
                free(v);
            }
        }
        return ret;
    }
    
    // 等同
    bool asValueVector(PlistNodeVector& nodeV)
    {
        if (!PLIST_IS_ARRAY(node_)) {
            return false;
        }
        
        uint32_t c = plist_array_get_size(node_);
        for (int i=0;i<c;++i)
        {
            plist_t node = plist_array_get_item(node_, i);
            nodeV.push_back(PlistNode(node));
        }
        return true;
    }

    
    PlistNode operator[](const char* key)
    {
        assert(key);
        if (node_)
        {
            plist_t sub_node = plist_dict_get_item(node_, key);
            return PlistNode(sub_node);
        }
        
        CCLOG("not found sub node %s", key);
        return PlistNode(nullptr);
    }

    // 是否为null
    inline bool isNull() const {
        return node_ == nullptr;
    }
    
    // 是否有某个key
    bool hasKey(const char* key)
    {
        assert(key);
        if (node_)
        {
            plist_t sub_node = plist_dict_get_item(node_, key);
            return sub_node ? true : false;
        }
        
        return false;
    }
    
private:
    plist_t node_;
};


// 简单的自动释放处理
class PlistAutoFree
{
public:
    PlistAutoFree(plist_t n) : node_(n) {}
    ~PlistAutoFree() {
        plist_free(node_);
    }
    
    plist_t node_;
};

#endif /* plistHelper_h */
