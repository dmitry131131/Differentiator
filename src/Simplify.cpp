#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "Diff.h"
#include "Simplify.h"

static bool is_equal(const double first, const double second);

static bool simplify_tree_recursive(TreeSegment** segment, diffErrorCode* error);

static bool solve_simplify(TreeSegment** segment, diffErrorCode* error);

static bool mul_div_simplify(TreeSegment** segment, diffErrorCode* error);

static bool plus_minus_simplify(TreeSegment** segment, diffErrorCode* error);

static bool pow_simplify(TreeSegment** segment, diffErrorCode* error);

diffErrorCode simplify_tree(TreeData* tree)
{
    assert(tree);
    diffErrorCode error = NO_DIFF_ERRORS;

    simplify_tree_recursive(&(tree->root), &error);

    return error;
}

#define CHANGE_SUBTREE_TO_DOUBLE(segment_, val_) do{                                    \
    SegmentData data = {};                                                              \
    data.D_number = val_;                                                               \
    TreeSegment* new_seg = CreateNode(DOUBLE_SEGMENT_DATA, data, nullptr, nullptr);     \
    new_seg->parent = (*segment_)->parent;                                              \
    new_seg->weight = 0;                                                                \
                                                                                        \
    del_segment(*(segment_));                                                           \
    *segment_ = new_seg;                                                                \
}while(0)

#define CHANGE_SUBTREE_TO_SUBTREE(segment_, new_segment_) do{       \
    TreeSegment* new_seg = nullptr;                                 \
    copy_subtree(new_segment_, &new_seg);                           \
                                                                    \
    del_segment(*segment_);                                         \
    *segment_ = new_seg;                                            \
}while(0)   

static bool simplify_tree_recursive(TreeSegment** segment, diffErrorCode* error)
{
    assert(segment);

    bool is_simplufy = true;
    while (is_simplufy)
    {
        is_simplufy = false;

        if ((*segment)->left)
        {
            is_simplufy = simplify_tree_recursive(&((*segment)->left), error);
        }
        if ((*segment)->right)
        {
            is_simplufy = simplify_tree_recursive(&((*segment)->right), error);
        }

        is_simplufy = solve_simplify(segment, error);
        if (*error) return is_simplufy;
        if (is_simplufy) continue;

        is_simplufy = mul_div_simplify(segment, error);
        if (*error) return is_simplufy;
        if (is_simplufy) continue;

        is_simplufy = plus_minus_simplify(segment, error);
        if (*error) return is_simplufy;
        if (is_simplufy) continue;

        is_simplufy = pow_simplify(segment, error);
        if (*error) return is_simplufy;
        if (is_simplufy) continue;
    }

    return is_simplufy;
}

static bool solve_simplify(TreeSegment** segment, diffErrorCode* error)
{
    assert(segment);

    double simplify_res = solve_tree_recursive(*segment, error);
    if (*error) return false;

    if (!isnan(simplify_res) && ((*segment)->left || (*segment)->right))
    {
        CHANGE_SUBTREE_TO_DOUBLE(segment, simplify_res);
        return true;
    }

    return false;
}

static bool mul_div_simplify(TreeSegment** segment, diffErrorCode* error)
{
    assert(segment);
    if (!((*segment)->type == OP_CODE_SEGMENT_DATA && ((*segment)->data.Op_code == MUL || (*segment)->data.Op_code == DIV)))
    {
        return false;
    }

    if  (is_equal((*segment)->left->data.D_number, 0)  && (*segment)->left->type  == DOUBLE_SEGMENT_DATA
    && !(is_equal((*segment)->right->data.D_number, 0) && (*segment)->right->type == DOUBLE_SEGMENT_DATA))
    {
        CHANGE_SUBTREE_TO_DOUBLE(segment, 0);
        return true;
    }
    else if (is_equal((*segment)->right->data.D_number, 1) && (*segment)->right->type == DOUBLE_SEGMENT_DATA)
    {
        CHANGE_SUBTREE_TO_SUBTREE(segment, (*segment)->left);
        return true;
    }
    if ((*segment)->data.Op_code == MUL)
    {
        if (is_equal((*segment)->left->data.D_number, 1) && (*segment)->left->type == DOUBLE_SEGMENT_DATA)
        {
            CHANGE_SUBTREE_TO_SUBTREE(segment, (*segment)->right);
            return true;
        }
        else if (is_equal((*segment)->right->data.D_number, 0) && (*segment)->right->type == DOUBLE_SEGMENT_DATA)
        {
            CHANGE_SUBTREE_TO_DOUBLE(segment, 0);
            return true;
        }
    }
    
    return false;
}

static bool plus_minus_simplify(TreeSegment** segment, diffErrorCode* error)
{
    assert(segment);

    if (!((*segment)->type == OP_CODE_SEGMENT_DATA && ((*segment)->data.Op_code == PLUS || (*segment)->data.Op_code == MINUS)))
    {
        return false;
    }

    if (is_equal((*segment)->right->data.D_number, 0) && (*segment)->right->type == DOUBLE_SEGMENT_DATA)
    {
        CHANGE_SUBTREE_TO_SUBTREE(segment, (*segment)->left);
        return true;
    }
    if ((*segment)->data.Op_code == PLUS)
    {
        if (is_equal((*segment)->left->data.D_number, 0) && (*segment)->left->type == DOUBLE_SEGMENT_DATA)
        {
            CHANGE_SUBTREE_TO_SUBTREE(segment, (*segment)->right);
            return true;
        }
    }

    return false;
}

static bool pow_simplify(TreeSegment** segment, diffErrorCode* error)
{
    assert(segment);

    if (!((*segment)->type == OP_CODE_SEGMENT_DATA && (*segment)->data.Op_code == POW))
    {
        return false;
    }

    if ((*segment)->right->type == DOUBLE_SEGMENT_DATA && is_equal((*segment)->right->data.D_number, 0))
    {
        CHANGE_SUBTREE_TO_DOUBLE(segment, 1);
        return true;
    }
    else if ((*segment)->right->type == DOUBLE_SEGMENT_DATA && is_equal((*segment)->right->data.D_number, 1))
    {
        CHANGE_SUBTREE_TO_SUBTREE(segment, (*segment)->left);
        return true;
    }
    else if ((*segment)->left->type == DOUBLE_SEGMENT_DATA && is_equal((*segment)->left->data.D_number, 0))
    {
        CHANGE_SUBTREE_TO_DOUBLE(segment, 0);
        return true;
    }

    return false;
}

#undef CHANGE_SUBTREE_TO_SUBTREE
#undef CHANGE_SUBTREE_TO_DOUBLE

static bool is_equal(const double first, const double second)
{
    return fabs(first - second) <= epsilon;
}