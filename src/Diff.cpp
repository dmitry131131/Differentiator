#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "Diff.h"

static double solve_tree_recursive(const TreeSegment* segment, diffErrorCode* error);

static double solve_op_segment(const TreeSegment* segment, const double left, const double right);

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

    case NONE:
    case OBR:
    case CBR:
    default:
        return NAN;
        break;
    }
}
