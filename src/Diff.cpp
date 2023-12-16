#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "Diff.h"

static double solve_tree_recursive(const TreeSegment* segment, diffErrorCode* error);

static double solve_op_segment(const TreeSegment* segment, const double left, const double right);

static diffErrorCode take_derivative_by_opcode(const TreeSegment* src, TreeSegment** dest);

static diffErrorCode take_derivative_recursive(const TreeSegment* src, TreeSegment** dest);

double solve_tree(const TreeData* tree, diffErrorCode* error)
{
    assert(tree);
    return solve_tree_recursive(tree->root, error);
}

static double solve_tree_recursive(const TreeSegment* segment, diffErrorCode* error)
{
    assert(segment);

    double l_val  = NAN;
    double r_val  = NAN;
    double answer = NAN;
    switch (segment->type)
    {
    case DOUBLE_SEGMENT_DATA:
        return segment->data.D_number;
        break;

    case OP_CODE_SEGMENT_DATA:
        if (!segment->left)
        {
            if (error) *error = BAD_TREE_SEGMENT;
            return NAN;
        }

        l_val = solve_tree_recursive(segment->left,  error);
        
        if (segment->right)
        {
            r_val = solve_tree_recursive(segment->right, error);
        }
        
        answer = solve_op_segment(segment, l_val, r_val);

        if (isnan(answer))
        {
            if (error) *error = BAD_TREE_COMPOSITION;
        }

        return answer;

        break;

    case VAR_SEGMENT_DATA:
        return NAN;             // при этом никикий ошибки нет, просто внутри дерева переменная и её нельзя вычислить
        break;

    case TEXT_SEGMENT_DATA:
    case NO_TYPE_SEGMENT_DATA:
    default:
        if (error) *error = BAD_TREE_SEGMENT;
        return NAN;
        break;
    }

}

static double solve_op_segment(const TreeSegment* segment, const double left, const double right)
{
    assert(segment);
    switch (segment->data.Op_code)
    {
    case PLUS:
        return left + right;
        
    case MINUS:
        return left - right;
        
    case MUL:
        return left * right;

    case DIV:
        return left / right;

    case SIN:
        return sin(left);

    case COS:
        return cos(left);

    case TAN:
        return tan(left);

    case POW:
        return pow(left, right);

    case NONE:
    case OBR:
    case CBR:
    default:
        return NAN;
        break;
    }
}

diffErrorCode take_derivative(TreeData* input, TreeData* output)
{
    assert(input);
    assert(output);
    diffErrorCode error = NO_DIFF_ERRORS;

    if ((error = take_derivative_recursive(input->root, &(output->root))))
    {
        return error;
    }

    return error;
}

#define CREATE_DOUBLE_SEG(ptr, val) do{                                 \
    SegmentData data = {};                                              \
    data.D_number = val;                                                \
    (ptr) = CreateNode(DOUBLE_SEGMENT_DATA, data, nullptr, nullptr);    \
    (ptr)->weight = 0;                                                  \
}while(0)

#define CREATE_OP_CODE_SEG(ptr, val, weight_) do{                       \
    SegmentData data = {};                                              \
    data.Op_code = val;                                                 \
    (ptr) = CreateNode(OP_CODE_SEGMENT_DATA, data, nullptr, nullptr);   \
    (ptr)->weight = weight_;                                            \
}while(0)

static diffErrorCode take_derivative_recursive(const TreeSegment* src, TreeSegment** dest)
{
    assert(src);
    assert(dest);
    diffErrorCode error = NO_DIFF_ERRORS;

    switch (src->type)
    {
    case DOUBLE_SEGMENT_DATA:
        CREATE_DOUBLE_SEG(*dest, 0);
        break;
    
    case VAR_SEGMENT_DATA:
        CREATE_DOUBLE_SEG(*dest, 1);
        break;

    case OP_CODE_SEGMENT_DATA:
        if ((error = take_derivative_by_opcode(src, dest)))
        {
            return error;
        }
        break;

    case NO_TYPE_SEGMENT_DATA:
    case TEXT_SEGMENT_DATA:
    default:
        break;
    }

    return error;
}

static diffErrorCode take_derivative_by_opcode(const TreeSegment* src, TreeSegment** dest)
{
    assert(src);
    assert(dest);
    diffErrorCode error = NO_DIFF_ERRORS;

    switch (src->data.Op_code)
    {
    case PLUS:
        CREATE_OP_CODE_SEG(*dest, PLUS, 3);
        if ((error = take_derivative_recursive(src->left, &((*dest)->left))))
        {
            return error;
        }
        if ((error = take_derivative_recursive(src->right, &((*dest)->right))))
        {
            return error;
        }
        break;
    case MINUS:
        CREATE_OP_CODE_SEG(*dest, MINUS, 3);
        if ((error = take_derivative_recursive(src->left, &((*dest)->left))))
        {
            return error;
        }
        if ((error = take_derivative_recursive(src->right, &((*dest)->right))))
        {
            return error;
        }
        break;
    case MUL:
        CREATE_OP_CODE_SEG(*dest, PLUS, 3);
        CREATE_OP_CODE_SEG((*dest)->left, MUL, 2);
        if ((error = take_derivative_recursive(src->left, &(((*dest)->left)->left))))
        {
            return error;
        }
        if (copy_subtree(src->right, &(((*dest)->left)->right)))
        {
            return COPY_SUBTREE_ERROR;
        }

        CREATE_OP_CODE_SEG((*dest)->right, MUL, 2);
        if ((error = take_derivative_recursive(src->right, &(((*dest)->right)->right))))
        {
            return error;
        }
        if (copy_subtree(src->left, &(((*dest)->right)->left)))
        {
            return COPY_SUBTREE_ERROR;
        }

        break;
    default:
        break;
    }

    return error;
}

#undef CREATE_OP_CODE_SEG
#undef CREATE_DOUBLE_SEG
