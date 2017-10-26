/*! rtbree.cpp */

#include <rbtree/rbtree.hpp>

namespace containers
{

bool _rbtree_ops::_verify_rb_alt(_rbtree_node_base* n)
{
    if (!n) return true;
    if (n->color() == _RED) {
        if (n->left() && n->left()->color() == _RED) return false;
        if (n->right() && n->right()->color() == _RED) return false;
    }
    return _verify_rb_alt(n->left()) && _verify_rb_alt(n->right());
}

bool _rbtree_ops::_verify_black_ht(_rbtree_node_base* n, size_t& ht)
{
    if (!n) {
        ht = 0;
        return true;
    }
    size_t lh = 0, rh = 0;
    bool lv = _verify_black_ht(n->left(), lh);
    bool rv = _verify_black_ht(n->right(), rh);
    if (!lv || !rv || lh != rh) return false;
    ht = lh + (n->color() == _BLACK);
    return true;
}

} // namespace containers
