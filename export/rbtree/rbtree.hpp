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
    rbtree() : m_root(), m_nil(), m_size(0), m_comp(), m_alloc()
    {
        m_nil = create_nil();
    }

    ~rbtree()
    {
        clear();
        destroy_nil(m_nil);
    }

    void clear()
    {
        if (m_root) {
            remove_node(m_root);
        }
    }

    bool contains(Data const& x)
    {
        Node* n = nullptr;
        find_parent(x, n);
        assert(n != nullptr || (!m_comp(n->data(), x) && !m_comp(x, n->data())));
        return n != nullptr;
    }

    bool insert(Data const& x)
    {
        Node* n = nullptr;
        auto p = find_parent(x, n);
        if (n) return false;
        n = create_node(x);
        ++m_size;
        if (!p) {
            m_root = n;
            return true;
        }
        n->set_parent(p);
        if (m_comp(x, p->data())) {
            p->set_left(n);
        } else {
            p->set_right(n);
        }
        return true;
    }

    std::size_t size() const
    {
        return m_size;
    }

    bool empty() const
    {
        return m_size == 0;
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
        // access
        Data const& data() const
        {
            return m_data;
        }
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
        // modify
        void set_color(NodeColor c)
        {
            m_color = c;
        }
        void set_parent(Node* n)
        {
            m_parent = (uintptr_t)n;
        }
        void set_right(Node* n)
        {
            m_right = n;
        }
        void set_left(Node* n)
        {
            m_left = n;
        }
        // relatives
        Node* grandparent() const
        {
            auto p = parent();
            return p ? p->parent() : nullptr;
        }
        Node* sibling() const
        {
            auto p = parent();
            if (!p) return p;
            return p->left() == this ? p->right() : p->left();
        }
        Node* uncle() const
        {
            auto p = parent();
            auto g = p ? p->parent() : nullptr;
            if (!g) return g;
            return p == g->left() ? g->right() : g->left();
        }
    };

    using MyAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
    using MyAllocTraits = std::allocator_traits<MyAlloc>;

    Node* m_root;
    Node* m_nil;
    std::size_t m_size;
    Comp m_comp;
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

    Node* create_node(Data const& x)
    {
        return create_node(Data(x));
    }

    Node* create_node(Data&& d)
    {
        auto node = create_nil();
        ::new (static_cast<void*>(&node->m_data)) Data(std::forward<Data>(d));
        node->set_color(RED);
        node->set_left(m_nil);
        node->set_right(m_nil);
        return node;
    }

    void destroy_node(Node* node)
    {
        if (!node) return;
        node->m_data.~Data();
        destroy_nil(node);
    }

    // actions
    void rotate_left(Node* n)
    {
        auto nnew = n->right();
        assert(nnew != m_nil);

        // rotate
        n->set_right(nnew->left());
        nnew->set_left(n);
        n->right()->set_parent(n);

        // handle parents
        auto parent = n->parent();
        if (parent) {
            if (parent->right() == n) {
                parent->set_right(nnew);
            } else {
                parent->set_left(nnew);
            }
        } else {
            m_root = nnew;
        }
        nnew->set_parent(parent);
        n->set_parent(nnew);
    }

    void rotate_right(Node* n)
    {
        auto nnew = n->left();
        assert(nnew != m_nil);

        // rotate
        n->set_left(nnew->right());
        nnew->set_right(n);
        n->left()->set_parent(n);

        // handle parents
        auto parent = n->parent();
        if (parent) {
            if (parent->right() == n) {
                parent->set_right(nnew);
            } else {
                parent->set_left(nnew);
            }
        } else {
            m_root = nnew;
        }
        nnew->set_parent(parent);
        n->set_parent(nnew);
    }

    Node* find_parent(Data const& x, Node*& next)
    {
        Node* p = nullptr;
        Node* n = m_root;
        if (!n) return nullptr;
        while (n != m_nil) {
            bool const is_less = m_comp(x, n->data());
            if (!m_comp(n->data(), x) && !is_less) {
                break;
            } else if (is_less) {
                p = n;
                n = n->left();
            } else {
                p = n;
                n = n->right();
            }
        }
        next = n == m_nil ? nullptr : n;
        return p;
    }

    void remove_node(Node* n)
    {
        if (n == m_nil) return;
        auto l = n->left();
        auto r = n->right();
        destroy_node(n);
        remove_node(l);
        remove_node(r);
    }
};

} // namespace containers

#endif // _RBTREE_RBTREE_HPP_
