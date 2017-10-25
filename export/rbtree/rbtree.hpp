/*! rbtree.hpp */

#ifndef _RBTREE_RBTREE_HPP_
#define _RBTREE_RBTREE_HPP_

#include <rbtree/exports.h>

#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>

namespace containers
{

template<class Data, class Comp = std::less<Data>, class Alloc = std::allocator<Data>>
class rbtree
{
  public:
    rbtree() : m_root(), m_nil(), m_alloc()
    {
        m_nil = create_nil();
    }

    ~rbtree()
    {
        destroy_nil(m_nil);
    }
  private:
    enum NodeColor
    {
        BLACK = 0,
        RED = 1
    };

    struct Node
    {
        Data m_data;
        NodeColor m_color : 1;
        uintptr_t m_parent : sizeof(void*)*8-1;
        Node* m_left;
        Node* m_right;

        NodeColor color() const
        {
            return m_color;
        }
        Node* parent() const
        {
            return static_cast<Node*>(m_parent);
        }
        Node* right() const
        {
            return m_right;
        }
        Node* left() const
        {
            return m_left;
        }
        void set_color(NodeColor c)
        {
            m_color = c;
        }
        void set_parent(Node* n)
        {
            m_parent = n;
        }
        void set_right(Node* n)
        {
            m_right = n;
        }
        void set_left(Node* n)
        {
            m_left = n;
        }
    };

    using MyAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using MyAllocTraits = std::allocator_traits<MyAlloc>;

    Node* m_root;
    Node* m_nil;
    MyAlloc m_alloc;

    Node* create_nil()
    {
        auto node = MyAllocTraits::allocate(m_alloc, 1);
        assert(static_cast<uintptr_t>(node) & 1 == 0);
        node->m_color = BLACK;
        node->m_parent = 0;
        node->m_left = node->m_right = nullptr;
        return node;
    }

    void destroy_nil(Node* node)
    {
        if (!node) return;
        MyAllocTraits::deallocate(m_alloc, node, 1);
    }

    Node* create_node(Data&& d)
    {
        auto node = MyAllocTraits::allocate(m_alloc, 1);
        assert(static_cast<uintptr_t>(node) & 1 == 0);
        ::new (static_cast<void*>(&node->m_data)) Data(std::forward<Data>(d));
        node->m_color = RED;
        node->m_parent = 0;
        node->m_left = node->m_right = nullptr;
        return node;
    }

    void destroy_node(Node* node)
    {
        if (!node) return;
        node->data.~Data();
        MyAllocTraits::deallocate(m_alloc, node, 1);
    }
};

} // namespace containers

#endif // _RBTREE_RBTREE_HPP_
