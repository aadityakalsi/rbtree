/*! rbtree.hpp */

#ifndef _RBTREE_RBTREE_HPP_
#define _RBTREE_RBTREE_HPP_

#include <rbtree/exports.h>

#include <iostream>
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
            remove_nodes_under(m_root);
            m_root = nullptr;
        }
    }

    bool contains(Data const& x)
    {
        Node* n = nullptr;
        auto p = find_lb(x, n);
        return n != nullptr;
    }

    bool insert(Data const& x)
    {
        Node* n = nullptr;
        auto p = find_parent(x, n);
        if (n) return false;
        n = create_node(x);
        if (!p) {
            m_root = n;
        } else {
            n->set_parent(p);
            if (m_comp(x, p->data())) {
                p->set_left(n);
            } else {
                p->set_right(n);
            }
        }
        insert_rebalance(n);
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
            auto ptr = (uintptr_t)m_parent;
            return (Node*)ptr;
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
            m_parent = ((uintptr_t)n & ~uintptr_t(1));
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

    bool is_equal(Data const& a, Data const& b) const
    {
        return !m_comp(a, b) && !m_comp(b, a);
    }

    Node* create_nil()
    {
        auto node = MyAllocTraits::allocate(m_alloc, 1);
        assert((((uintptr_t)node) & 1) == 0);
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

    Node* create_post(Node* node)
    {
        ++m_size;
        node->set_color(RED);
        node->set_left(m_nil);
        node->set_right(m_nil);
        return node;
    }

    Node* create_node(Data const& d)
    {
        auto node = create_nil();
        try {
            ::new (static_cast<void*>(&node->m_data)) Data(d);
        } catch(...) {
            destroy_nil(node);
            throw;
        }
        return create_post(node);
    }

    Node* create_node(Data&& d)
    {
        auto node = create_nil();
        try {
            ::new (static_cast<void*>(&node->m_data)) Data(std::forward<Data>(d));
        } catch(...) {
            destroy_nil(node);
            throw;
        }
        return create_post(node);
    }

    void destroy_node(Node* node)
    {
        if (!node) return;
        node->m_data.~Data();
        --m_size;
        destroy_nil(node);
    }

    // actions
    void rotate_left(Node* n)
    {
        assert(n != m_nil);
        auto nnew = n->right();
        assert(nnew != m_nil);

        // rotate
        n->set_right(nnew->left());
        nnew->set_left(n);
        if (n->right()) {
            n->right()->set_parent(n);
        }

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
        assert(n != m_nil);
        auto nnew = n->left();
        assert(nnew != m_nil);

        // rotate
        n->set_left(nnew->right());
        nnew->set_right(n);
        if (n->left() != m_nil) {
            n->left()->set_parent(n);
        }

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

    Node* find_lb(Data const& x, Node*& next) const
    {
        Node* p = nullptr;
        Node* n = m_root;
        if (!n) return nullptr;
        auto const NIL = m_nil;
        while (n != NIL) {
            if (!m_comp(n->data(), x)) {
                p = n;
                n = n->left();
            } else {
                n = n->right();
            }
        }
        next = (p == nullptr || p == NIL) ? nullptr : m_comp(x, p->data()) ? nullptr : p;
        return p;
    }

    Node* find_parent(Data const& x, Node*& next) const
    {
        Node* p = nullptr;
        Node* n = m_root;
        auto const NIL = m_nil;
        if (!n) return nullptr;
        bool lt = true;
        while (n != NIL) {
            p = n;
            lt = m_comp(x, n->data());
            n = lt ? n->left() : n->right();
        }
        if (lt) {
            while (p != nullptr && p->parent() != nullptr && p->parent()->left() == p) {
                p = p->parent();
            }
            if (p != nullptr && p->parent() != nullptr && is_equal(p->parent()->data(), x)) {
                p = p->grandparent();
            }
        }
        if (p && is_equal(p->data(), x)) {
            p = p->parent();
        }
        if (!p) {
            next = is_equal(m_root->data(), x) ? m_root : nullptr;
        } else {
            if (p->left() != NIL && is_equal(p->left()->data(), x)) {
                next = p->left();
            } else if (p->right() != NIL && is_equal(p->right()->data(), x)) {
                next = p->right();
            } else {
                next = nullptr;
            }
        }
        return p;
    }

  public:
    void print(std::ostream& strm = std::cerr)
    {
        std::unique_ptr<Node*[]> buf(new Node*[m_size]);
        std::unique_ptr<int[]> lvls(new int[m_size]);
        auto const NIL = m_nil;
        std::size_t idx = 0;
        std::size_t end = 1;
        std::size_t cur = 0;
        buf[0] = m_root;
        lvls[0] = 1;
        while (1) {
            auto n = buf[idx++];
            if (n->left() != NIL) {
                buf[end++] = n->left();
            }
            if (n->right() != NIL) {
                buf[end++] = n->right();
            }
            if (end >= m_size) {
                break;
            }
            if (idx == lvls[cur]) {
                lvls[++cur] = end;
            }
        }
        lvls[++cur] = end;
        idx = 0;
        cur = 0;
        while (idx < m_size) {
            auto num = lvls[cur++];
            strm << "lvl " << cur << " ";
            for (int i = idx; i < num; ++i) {
                strm << (buf[i]->color() == RED ? " r " : " b ") << buf[i]->data();
            }
            strm << "\n";
            idx = num;
        }
    }

  private:
    void remove_nodes_under(Node* n)
    {
        std::unique_ptr<Node*[]> buf(new Node*[m_size]);
        std::size_t idx = 0;
        std::size_t end = 1;
        auto const NIL = m_nil;
        buf[0] = m_root;
        while (1) {
            auto n = buf[idx++];
            if (n->left() != NIL) {
                buf[end++] = n->left();
            }
            if (n->right() != NIL) {
                buf[end++] = n->right();
            }
            if (end >= m_size) {
                break;
            }
        }
        idx = 0;
        auto const sz = m_size;
        while (idx < sz) {
            destroy_node(buf[idx++]);
        }
    }

    void insert_rebalance(Node* node)
    {
        auto parent = node->parent();
        if (parent == nullptr) {
            // node is the new root
            node->set_color(BLACK);
            return;
        }
        if (parent->color() == BLACK) {
            // nothing to do.. balanced
            return;
        }
        // parent is red
        auto grandparent = parent->parent();
        auto uncle = parent->sibling();
        //if (!uncle || uncle == m_nil) return;
        if (!uncle) return;
        if (uncle->color() == RED) {
            // If both the parent P and the uncle U are red, then both of them can be
            // repainted black and the grandparent G becomes red to maintain property:
            // "all paths from any given node to its leaf nodes contain the same number
            //  of black nodes"
            // Since any path through the parent or uncle must pass through the grandparent
            // the number of black nodes on these paths has not changed. However, the grandparent
            // G may now violate property: "the root is black" if it is the root or property:
            // "both children of every red node are black" if it has a red parent.
            parent->set_color(BLACK);
            uncle->set_color(BLACK);
            grandparent->set_color(RED);
            insert_rebalance(grandparent);
            return;
        }
        // The parent P is red but the uncle U is black. The ultimate goal will be to rotate the parent
        // node into the grandparent position, but this will not work if the current node is on the "inside"
        // of the subtree under G (i.e., if N is the left child of the right child of the grandparent or the
        // right child of the left child of the grandparent). In this case, a left rotation on P that
        // switches the roles of the current node N and its parent P can be performed. The rotation causes
        // some paths to pass through the node N where they did not before. It also causes some paths not to
        // pass through the node P where they did before. However, both of these nodes are red, so property:
        // "all paths from any given node to its leaf nodes contain the same number of black nodes" is not
        // violated by the rotation. After this step has been completed, property "both children of every red
        // node are black" is still violated, but now we can resolve this by continuing to step 2.
        if (node == grandparent->left()->right()) {
            rotate_left(parent);
            node = node->left();
        } else if (node == grandparent->right()->left()) {
            rotate_right(parent);
            node = node->right();
        }
        // The current node N is now certain to be on the "outside" of the subtree under G (left of left child
        // or right of right child). In this case, a right rotation on G is performed; the result is a tree
        // where the former parent P is now the parent of both the current node N and the former grandparent G.
        // G is known to be black, since its former child P could not have been red without violating property
        // "red nodes have only black children". Once the colors of P and G are switched, the resulting tree
        // satisfies that property. Property "all paths from any given node to its leaf nodes contain the same
        // number of black nodes" also remains satisfied, since all paths that went through any of these three
        // nodes went through G before, and now they all go through P.
        parent = node->parent();
        grandparent = node->grandparent();
        if (!parent || !grandparent) return;
        if (node == parent->left()) {
            rotate_right(grandparent);
        } else {
            rotate_left(grandparent);
        }
        parent->set_color(BLACK);
        grandparent->set_color(RED);
    }
};

} // namespace containers

#endif // _RBTREE_RBTREE_HPP_
