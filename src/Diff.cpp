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

    case LN:
        if (left <= 0)
        {
            return NAN;
        }
        return log(left);

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

#define COPY_SEG(S, D) do{                                              \
    if (copy_subtree((S), (D)))                                         \
    {                                                                   \
        return COPY_SUBTREE_ERROR;                                      \
    }                                                                   \
}while(0)

#define TAKE_DIR(S, D) do{                                              \
    if ((error = take_derivative_recursive((S), (D))))                  \
    {                                                                   \
        return error;                                                   \
    }                                                                   \
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

            TAKE_DIR(src->left, &((*dest)->left));
            TAKE_DIR(src->right, &((*dest)->right));

        break;
    case MINUS:
        CREATE_OP_CODE_SEG(*dest, MINUS, 3);

            TAKE_DIR(src->left, &((*dest)->left));
            TAKE_DIR(src->right, &((*dest)->right));

        break;
    case MUL:
        CREATE_OP_CODE_SEG(*dest, PLUS, 3);

            CREATE_OP_CODE_SEG((*dest)->left, MUL, 2);
                TAKE_DIR(src->left, &(((*dest)->left)->left));
                COPY_SEG(src->right, &(((*dest)->left)->right));

            CREATE_OP_CODE_SEG((*dest)->right, MUL, 2);
                TAKE_DIR(src->right, &(((*dest)->right)->right));
                COPY_SEG(src->left, &(((*dest)->right)->left));

        break;
    case DIV:
        CREATE_OP_CODE_SEG(*dest, DIV, 2);

            CREATE_OP_CODE_SEG((*dest)->left, MINUS, 3);

                CREATE_OP_CODE_SEG(((*dest)->left)->left, MUL, 2);
                    TAKE_DIR(src->left, &((((*dest)->left)->left)->left));
                    COPY_SEG(src->right, &((((*dest)->left)->left)->right));

                CREATE_OP_CODE_SEG(((*dest)->left)->right, MUL, 2);
                    TAKE_DIR(src->right, &((((*dest)->left)->right)->right));
                    COPY_SEG(src->left, &((((*dest)->left)->right)->left));

            CREATE_OP_CODE_SEG((*dest)->right, POW, 1);
                COPY_SEG(src->right, &(((*dest)->right)->left));
                CREATE_DOUBLE_SEG(((*dest)->right)->right, 2);

        break;
    case SIN:
        CREATE_OP_CODE_SEG(*dest, MUL, 2);

            CREATE_OP_CODE_SEG((*dest)->left, COS, 0);
                COPY_SEG(src->left, &(((*dest)->left)->left));

            TAKE_DIR(src->left, &((*dest)->right));

        break;
    case COS:
        CREATE_OP_CODE_SEG(*dest, MUL, 2);

            CREATE_OP_CODE_SEG((*dest)->left, MUL, 2);
                CREATE_OP_CODE_SEG(((*dest)->left)->left, SIN, 0);
                    COPY_SEG(src->left, &((((*dest)->left)->left)->left));
                CREATE_DOUBLE_SEG(((*dest)->left)->right, -1);

            TAKE_DIR(src->left, &((*dest)->right));

        break;
    case TAN:
        CREATE_OP_CODE_SEG(*dest, DIV, 2);

            CREATE_DOUBLE_SEG((*dest)->left, 1);

            CREATE_OP_CODE_SEG((*dest)->right, POW, 1);
                CREATE_OP_CODE_SEG(((*dest)->right)->left, COS, 0);
                    COPY_SEG(src->left, &((((*dest)->right)->left)->left));
                CREATE_DOUBLE_SEG(((*dest)->right)->right, 2);

        break;
    case LN:
        CREATE_OP_CODE_SEG(*dest, DIV, 2);

            TAKE_DIR(src->left, &((*dest)->left));
            COPY_SEG(src->left, &((*dest)->right));

        break;
    case POW:  //FIXME how to simplify tree
        CREATE_OP_CODE_SEG(*dest, MUL, 2);

            COPY_SEG(src, &((*dest)->left));

            CREATE_OP_CODE_SEG((*dest)->right, PLUS, 3);
                CREATE_OP_CODE_SEG(((*dest)->right)->left, MUL, 2);

                    CREATE_OP_CODE_SEG((((*dest)->right)->left)->left, DIV, 2);
                        TAKE_DIR(src->left, &(((((*dest)->right)->left)->left)->left));
                        COPY_SEG(src->left, &(((((*dest)->right)->left)->left)->right));

                    COPY_SEG(src->right, &((((*dest)->right)->left)->right));

                CREATE_OP_CODE_SEG(((*dest)->right)->right, MUL, 2);

                    CREATE_OP_CODE_SEG((((*dest)->right)->right)->left, LN, 0);
                        COPY_SEG(src->left, &(((((*dest)->right)->right)->left)->left));

                    TAKE_DIR(src->right, &(((((*dest)->right)->right)->left)->right));

        break;
    
    case OBR:
    case CBR:
    case NONE:
    default:
        break;
    }

    return error;
}

#undef TAKE_DIR
#undef COPY_SEG
#undef CREATE_OP_CODE_SEG
#undef CREATE_DOUBLE_SEG
