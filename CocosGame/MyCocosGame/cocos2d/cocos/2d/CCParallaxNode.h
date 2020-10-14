/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
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
#ifndef __CCPARALLAX_NODE_H__
#define __CCPARALLAX_NODE_H__

#include "2d/CCNode.h"
/*#include "ccArray.h"*/

NS_CC_BEGIN

struct _ccArray;

/**
 * @addtogroup _2d
 * @{
 */

/** @class ParallaxNode
 * @brief ParallaxNode: A node that simulates a parallax scroller

The children will be moved faster / slower than the parent according the the parallax ratio.

*/
class CC_DLL ParallaxNode : public Node
{
public:
    /** Create a Parallax node. 
     *
     * @return An autoreleased ParallaxNode object.
     */
    static ParallaxNode * create();

    // prevents compiler warning: "Included function hides overloaded virtual functions"
    using Node::addChild;

    /** Adds a child to the container with a local z-order, parallax ratio and position offset.
     *
     * @param child A child node.
     * @param z Z order for drawing priority.
     * @param parallaxRatio A given parallax ratio.
     * @param positionOffset A given position offset.
     */
    void addChild(Node * child, int z, const Vec2& parallaxRatio, const Vec2& positionOffset);

    /** Sets an array of layers for the Parallax node.
     *
     * @param parallaxArray An array of layers for the Parallax node.
     */
    void setParallaxArray( struct _ccArray *parallaxArray) { _parallaxArray = parallaxArray; }
    /** Returns the array of layers of the Parallax node.
     *
     * @return An array of layers for the Parallax node.
     */
    struct _ccArray* getParallaxArray() { return _parallaxArray; }
    const struct _ccArray* getParallaxArray() const { return _parallaxArray; }

    //
    // Overrides
    //
    virtual void addChild(Node * child, int zOrder, int tag) override;
    virtual void addChild(Node * child, int zOrder, const std::string &name) override;
    virtual void removeChild(Node* child, bool cleanup) override;
    virtual void removeAllChildrenWithCleanup(bool cleanup) override;
    virtual void visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags) override;

CC_CONSTRUCTOR_ACCESS:
    /** Adds a child to the container with a z-order, a parallax ratio and a position offset
     It returns self, so you can chain several addChilds.
     @since v0.8
     * @js ctor
     */
    ParallaxNode();
    /**
     * @js NA
     * @lua NA
     */
    virtual ~ParallaxNode();

protected:
    Vec2 absolutePosition();

    Vec2    _lastPosition;
    struct _ccArray* _parallaxArray;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(ParallaxNode);
};

// end of _2d group
/// @}
class PointObject : public Ref
{
public:
    static PointObject * create(Vec2 ratio, Vec2 offset)
    {
        PointObject *ret = new (std::nothrow) PointObject();
        ret->initWithPoint(ratio, offset);
        ret->autorelease();
        return ret;
    }
    
    bool initWithPoint(Vec2 ratio, Vec2 offset)
    {
        _ratio = ratio;
        _offset = offset;
        _child = nullptr;
        return true;
    }
    
    inline const Vec2& getRatio() const { return _ratio; };
    inline void setRatio(const Vec2& ratio) { _ratio = ratio; };
    
    inline const Vec2& getOffset() const { return _offset; };
    inline void setOffset(const Vec2& offset) { _offset = offset; };
    
    inline Node* getChild() const { return _child; };
    inline void setChild(Node* child) { _child = child; };
    
private:
    Vec2 _ratio;
    Vec2 _offset;
    Node *_child; // weak ref
};
NS_CC_END

#endif //__CCPARALLAX_NODE_H__

