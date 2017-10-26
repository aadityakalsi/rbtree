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

enum _rbnode_color
{
    _BLACK = 0,
    _RED = 1
};

struct _rbtree_node_base
{
    uintptr_t m_parent_color;
    _rbtree_node_base* m_left;
    _rbtree_node_base* m_right;
    // access
    _rbnode_color color() const
    {
        return static_cast<_rbnode_color>(m_parent_color & uintptr_t(1));
    }
    _rbtree_node_base* parent() const
    {
        return (_rbtree_node_base*)(m_parent_color & ~uintptr_t(1));
    }
    void set_color(_rbnode_color c)
    {
        auto par_col = m_parent_color & ~uintptr_t(1);
        par_col |= uintptr_t((c == _RED) ? 1 : 0);
        m_parent_color = par_col;
    }
    void set_parent(_rbtree_node_base* n)
    {
        auto n_int = (uintptr_t)n;
        n_int |= (m_parent_color & uintptr_t(1));
        m_parent_color = n_int;
    }
    _rbtree_node_base* right() const
    {
        return m_right;
    }
    _rbtree_node_base* left() const
    {
        return m_left;
    }
    void set_right(_rbtree_node_base* n)
    {
        m_right = n;
    }
    void set_left(_rbtree_node_base* n)
    {
        m_left = n;
    }
    // relatives
    _rbtree_node_base* grandparent() const
    {
        auto p = parent();
        return p ? p->parent() : nullptr;
    }
    _rbtree_node_base* sibling() const
    {
        auto p = parent();
        if (!p) return p;
        return p->left() == this ? p->right() : p->left();
    }
    _rbtree_node_base* uncle() const
    {
        auto p = parent();
        auto g = p ? p->parent() : nullptr;
        if (!g) return g;
        return p == g->left() ? g->right() : g->left();
    }
};

template<class Data>
struct _rbtree_node : public _rbtree_node_base
{
    Data m_data;
    // access
    Data const& data() const
    {
        return m_data;
    }
    _rbtree_node* parent() const
    {
        return static_cast<_rbtree_node*>(_rbtree_node_base::parent());
    }
    _rbtree_node* right() const
    {
        return static_cast<_rbtree_node*>(_rbtree_node_base::right());
    }
    _rbtree_node* left() const
    {
        return static_cast<_rbtree_node*>(_rbtree_node_base::left());
    }
    _rbtree_node* grandparent() const
    {
        return static_cast<_rbtree_node*>(_rbtree_node_base::grandparent());
    }
    _rbtree_node* sibling() const
    {
        return static_cast<_rbtree_node*>(_rbtree_node_base::sibling());
    }
    _rbtree_node* uncle() const
    {
        return static_cast<_rbtree_node*>(_rbtree_node_base::uncle());
    }
};

class RBTREE_API _rbtree_ops
{
  public:
    // diagnostic
    static bool _verify_rb_alt(_rbtree_node_base* n);
    static bool _verify_black_ht(_rbtree_node_base* n, size_t& ht);
  private:
    // tree operations
    static void rotate_left(_rbtree_node_base* n, _rbtree_node_base** root)
    {
        assert(n != nullptr);
        auto nnew = n->right();
        assert(nnew != nullptr);

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
            *root = nnew;
        }
        nnew->set_parent(parent);
        n->set_parent(nnew);
    }

    static void rotate_right(_rbtree_node_base* n, _rbtree_node_base** root)
    {
        assert(n != nullptr);
        auto nnew = n->left();
        assert(nnew != nullptr);

        // rotate
        n->set_left(nnew->right());
        nnew->set_right(n);
        if (n->left()) {
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
            *root = nnew;
        }
        nnew->set_parent(parent);
        n->set_parent(nnew);
    }
  public:
    static bool insert_rebalance(_rbtree_node_base* node, _rbtree_node_base** root)
    {
        auto parent = node->parent();
        if (parent == nullptr) {
            node->set_color(_BLACK);
            return false;
        }
        if (parent->color() == _BLACK) return false;
        auto grandparent = parent->parent();
        if (!grandparent) return false;
        auto uncle = parent->sibling();
        if (uncle && uncle->color() == _RED) {
            // If both the parent P and the uncle U are _RED, then both of them can be
            // repainted _BLACK and the grandparent G becomes _RED to maintain property:
            // "all paths from any given node to its leaf nodes contain the same number
            //  of _BLACK nodes"
            // Since any path through the parent or uncle must pass through the grandparent
            // the number of _BLACK nodes on these paths has not changed. However, the grandparent
            // G may now violate property: "the root is _BLACK" if it is the root or property:
            // "both children of every _RED node are _BLACK" if it has a _RED parent.
            parent->set_color(_BLACK);
            uncle->set_color(_BLACK);
            grandparent->set_color(_RED);
            return true;
        }
        // The parent P is _RED but the uncle U is _BLACK. The ultimate goal will be to rotate the parent
        // node into the grandparent position, but this will not work if the current node is on the "inside"
        // of the subtree under G (i.e., if N is the left child of the right child of the grandparent or the
        // right child of the left child of the grandparent). In this case, a left rotation on P that
        // switches the roles of the current node N and its parent P can be performed. The rotation causes
        // some paths to pass through the node N where they did not before. It also causes some paths not to
        // pass through the node P where they did before. However, both of these nodes are _RED, so property:
        // "all paths from any given node to its leaf nodes contain the same number of _BLACK nodes" is not
        // violated by the rotation. After this step has been completed, property "both children of every _RED
        // node are _BLACK" is still violated, but now we can resolve this by continuing to step 2.
        if (grandparent->left() && node == grandparent->left()->right()) {
            rotate_left(parent, root);
            node = node->left();
        } else if (grandparent->right() && node == grandparent->right()->left()) {
            rotate_right(parent, root);
            node = node->right();
        }
        // The current node N is now certain to be on the "outside" of the subtree under G (left of left child
        // or right of right child). In this case, a right rotation on G is performed; the result is a tree
        // where the former parent P is now the parent of both the current node N and the former grandparent G.
        // G is known to be _BLACK, since its former child P could not have been _RED without violating property
        // "_RED nodes have only _BLACK children". Once the colors of P and G are switched, the resulting tree
        // satisfies that property. Property "all paths from any given node to its leaf nodes contain the same
        // number of _BLACK nodes" also remains satisfied, since all paths that went through any of these three
        // nodes went through G before, and now they all go through P.
        parent = node->parent();
        grandparent = node->grandparent();
        if (!parent || !grandparent) return false;
        if (node == parent->left()) {
            rotate_right(grandparent, root);
        } else {
            rotate_left(grandparent, root);
        }
        parent->set_color(_BLACK);
        grandparent->set_color(_RED);
        return false;
    }
};

template<class Data, class Alloc>
using _rbtree_base_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<_rbtree_node<Data>>;

template<class Data, class Alloc>
struct _rbtree_base : public _rbtree_base_alloc<Data, Alloc>
{
  protected:
    using _alloc = _rbtree_base_alloc<Data, Alloc>;
    using _alloc_traits = std::allocator_traits<_alloc>;

    using _node = _rbtree_node<Data>;

    _node* m_root;
    std::size_t m_size;

  public:
    void clear()
    {
        if (m_root) {
            remove_nodes_under(m_root);
            m_root = nullptr;
        }
    }

    std::size_t size() const
    {
        return m_size;
    }

    bool empty() const
    {
        return m_size == 0;
    }

  protected:
    _rbtree_base() : m_root(nullptr), m_size(0)
    { }

    ~_rbtree_base()
    { clear(); }

    // node creation and deletion
    _node* _create_node_common()
    {
        auto node = _alloc_traits::allocate(*this, 1);
        assert((((uintptr_t)node) & 1) == 0);
        node->m_parent_color = 0;
        node->m_left = node->m_right = nullptr;
        return node;
    }

    void _destroy_node_common(_node* node)
    {
        if (!node) return;
        _alloc_traits::deallocate(*this, node, 1);
    }

    _node* _post_create_node(_node* node)
    {
        ++m_size;
        node->set_color(_RED);
        node->set_left(nullptr);
        node->set_right(nullptr);
        return node;
    }

    _node* create_node(Data const& d)
    {
        auto node = _create_node_common();
        try {
            ::new (static_cast<void*>(&node->m_data)) Data(d);
        } catch(...) {
            _destroy_node_common(node);
            throw;
        }
        return _post_create_node(node);
    }

    _node* create_node(Data&& d)
    {
        auto node = _create_node_common();
        try {
            ::new (static_cast<void*>(&node->m_data)) Data(std::forward<Data>(d));
        } catch(...) {
            _destroy_node_common(node);
            throw;
        }
        return _post_create_node(node);
    }

    void destroy_node(_node* node)
    {
        if (!node) return;
        node->m_data.~Data();
        --m_size;
        _destroy_node_common(node);
    }

  private:
    static void _print_node(_node* n, int lvl)
    {
        if (!n) return;
        for (int i = 0; i < lvl; ++i) {
            std::cerr << "  ";
        }
        std::cerr << "|-> (" << (n->color() == _RED ? "r" : "b") << ") " << n->data() << "\n";
        _print_node(n->left(), lvl + 1);
        _print_node(n->right(), lvl + 1);
    }
  public:
    void print() const
    {
        std::cerr << "\n";
        _print_node(m_root, 0);
    }

  protected:
    void remove_nodes_under(_node* n)
    {
        std::unique_ptr<_node*[]> buf(new _node*[m_size]);
        std::size_t idx = 0;
        std::size_t end = 1;
        buf[0] = m_root;
        while (1) {
            auto n = buf[idx++];
            if (n->left() != nullptr) {
                buf[end++] = n->left();
            }
            if (n->right() != nullptr) {
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

    bool verify() const
    {
        if (!m_root) return true;
        bool const rbalt = _rbtree_ops::_verify_rb_alt(m_root);
        size_t lh = 0, rh = 0;
        bool lv = _rbtree_ops::_verify_black_ht(m_root->left(), lh);
        bool rv = _rbtree_ops::_verify_black_ht(m_root->right(), rh);
        bool const ret = lv && rv && (lh == rh);
        if (!ret) {
            print();
        }
        return ret;
    }
};

template<class Data, class Comp = std::less<Data>, class Alloc = std::allocator<Data>>
class rbtree : public _rbtree_base<Data, Alloc>
{
  private:
    using _node = typename _rbtree_base<Data, Alloc>::_node;
  public:
    bool contains(Data const& d) const
    {
        _node* n;
        find_lb(d, n);
        return n != nullptr;
    }

    bool insert(Data const& d)
    {
        assert(this->verify());
        _node* n = nullptr;
        auto p = find_parent(d, n);
        if (n) return false;
        n = this->create_node(d);
        if (!p) {
            this->m_root = n;
        } else {
            n->set_parent(p);
            if (m_comp(d, p->data())) {
                p->set_left(n);
            } else {
                p->set_right(n);
            }
        }
        while (n && _rbtree_ops::insert_rebalance(n, (_rbtree_node_base**)&this->m_root)) {
            n = n->grandparent();
        }
        assert(this->verify());
        return true;
    }

  private:
    Comp m_comp;

    bool is_equal(Data const& a, Data const& b) const
    {
        return !m_comp(a, b) && !m_comp(b, a);
    }

    _node* find_lb(Data const& x, _node*& next) const
    {
        _node* p = nullptr;
        _node* n = this->m_root;
        while (n != nullptr) {
            if (!this->m_comp(n->data(), x)) {
                p = n;
                n = n->left();
            } else {
                n = n->right();
            }
        }
        next = (p == nullptr) ? nullptr : this->m_comp(x, p->data()) ? nullptr : p;
        return p;
    }

    _node* find_parent(Data const& x, _node*& next) const
    {
        _node* p = nullptr;
        _node* n = this->m_root;
        bool lt = true;
        while (n != nullptr) {
            p = n;
            lt = this->m_comp(x, n->data());
            n = lt ? n->left() : n->right();
        }
        if (lt) {
            if (p != nullptr && p->parent() != nullptr && this->is_equal(p->parent()->data(), x)) {
                p = p->grandparent();
            }
        }
        if (p && this->is_equal(p->data(), x)) {
            p = p->parent();
        }
        if (!p) {
            if (this->m_root) {
                next = this->is_equal(this->m_root->data(), x) ? this->m_root : nullptr;
            }
        } else {
            if (p->left()  && this->is_equal(p->left()->data(), x)) {
                next = p->left();
            } else if (p->right() && this->is_equal(p->right()->data(), x)) {
                next = p->right();
            } else {
                next = nullptr;
            }
        }
        return p;
    }
};

} // namespace containers

#endif // _RBTREE_RBTREE_HPP_
