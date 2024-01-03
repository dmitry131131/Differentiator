#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "Diff.h"
#include "Simplify.h"

static double solve_op_segment(const TreeSegment* segment, const double left, const double right);
static diffErrorCode take_derivative_by_opcode(const TreeSegment* src, TreeSegment** dest, TreeSegment* par, FILE* stream);
static diffErrorCode take_derivative_recursive(const TreeSegment* src, TreeSegment** dest, TreeSegment* par, FILE* stream);

//-------------------------------------------------------------------------------------------------//

//#################################################################################################//
//----------------------------------> Solve tree functions <---------------------------------------//
//#################################################################################################//

double solve_tree(const TreeData* tree, diffErrorCode* error)
{
    assert(tree);
    return solve_tree_recursive(tree->root, error);
}

double solve_tree_recursive(const TreeSegment* segment, diffErrorCode* error)
{
    assert(segment);

    double l_val  = NAN;
    double r_val  = NAN;
    double answer = NAN;
    switch ((int) segment->type)
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

        if (isnan(l_val)) return NAN;
        
        if (segment->right)
        {
            r_val = solve_tree_recursive(segment->right, error);
        }

        if (isnan(r_val)) return NAN;
        
        answer = solve_op_segment(segment, l_val, r_val);

        if (isnan(answer))
        {
            if (error) *error = BAD_TREE_COMPOSITION;
        }

        return answer;

        break;

    case VAR_SEGMENT_DATA:
        if (error) *error = NO_DIFF_ERRORS;
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

//#################################################################################################//
//--------------------------------> Dirivative tree functions <------------------------------------//
//#################################################################################################//

diffErrorCode take_derivative(TreeData* input, TreeData* output)
{
    assert(input);
    assert(output);
    diffErrorCode error = NO_DIFF_ERRORS;

    FILE* file = fopen("ref.txt", "w");
    if (!file)
    {
        return REFERENCE_FILE_CREATING_ERROR;
    }

    write_latex_header(file);

    if ((error = take_derivative_recursive(input->root, &(output->root), nullptr, file)))
    {
        return error;
    }

    if ((error = simplify_tree(output, file)))
    {
        return error;
    }

    random_phrase(file);
    fprintf(file, "\\[(");                                    
    print_expression_to_latex_recursive(input->root, file);      
    fprintf(file, ")' = ");    
    print_expression_to_latex_recursive(output->root, file);
    fprintf(file, "\\]\n");

    write_latex_footer(file);

    return error;
}
//FIXME Проблема с parent кроется в этих макросах - нужно срочно решить
#define CREATE_DOUBLE_SEG(ptr, par, val) do{                            \
    SegmentData data = {};                                              \
    data.D_number = val;                                                \
    (ptr) = CreateNode(DOUBLE_SEGMENT_DATA, data, nullptr, nullptr);    \
    (ptr)->weight = 0;                                                  \
    (ptr)->parent = (par);                                              \
}while(0)

#define CREATE_OP_CODE_SEG(ptr, par, val, weight_) do{                  \
    SegmentData data = {};                                              \
    data.Op_code = val;                                                 \
    (ptr) = CreateNode(OP_CODE_SEGMENT_DATA, data, nullptr, nullptr);   \
    (ptr)->weight = weight_;                                            \
    (ptr)->parent = (par);                                              \
}while(0)

#define COPY_SEG(S, D, par) do{                                         \
    if (copy_subtree((S), (D)))                                         \
    {                                                                   \
        return COPY_SUBTREE_ERROR;                                      \
    }                                                                   \
    (*D)->parent = (par);                                               \
}while(0)

#define TAKE_DIR(S, D, par) do{                                         \
    if ((error = take_derivative_recursive((S), (D), par, stream)))     \
    {                                                                   \
        return error;                                                   \
    }                                                                   \
    (*D)->parent = (par);                                               \
}while(0)

static diffErrorCode take_derivative_recursive(const TreeSegment* src, TreeSegment** dest, TreeSegment* par, FILE* stream)
{
    assert(src);
    assert(dest);
    assert(stream);
    diffErrorCode error = NO_DIFF_ERRORS;

    switch ((int) src->type)
    {
    case DOUBLE_SEGMENT_DATA:
        CREATE_DOUBLE_SEG(*dest, par, 0);
        break;
    
    case VAR_SEGMENT_DATA:
        CREATE_DOUBLE_SEG(*dest, par, 1);
        break;

    case OP_CODE_SEGMENT_DATA:
        if ((error = take_derivative_by_opcode(src, dest, par, stream)))
        {
            return error;
        }
        break;

    case NO_TYPE_SEGMENT_DATA:
    case TEXT_SEGMENT_DATA:
    default:
        break;
    }

    random_phrase(stream);
    fprintf(stream, "\\[(");
    print_expression_to_latex_recursive(src, stream);
    fprintf(stream, ")' = ");
    print_expression_to_latex_recursive((*dest), stream);
    fprintf(stream, "\\]\n");

    return error;
}

static diffErrorCode take_derivative_by_opcode(const TreeSegment* src, TreeSegment** dest, TreeSegment* par, FILE* stream)
{
    assert(src);
    assert(dest);
    diffErrorCode error = NO_DIFF_ERRORS;
    double is_computable = NAN;

    switch (src->data.Op_code)
    {
    case PLUS:
        CREATE_OP_CODE_SEG(*dest, par, PLUS, 3);

            TAKE_DIR(src->left, &((*dest)->left), (*dest));
            TAKE_DIR(src->right, &((*dest)->right), (*dest));

        break;
    case MINUS:
        CREATE_OP_CODE_SEG(*dest, par, MINUS, 3);

            TAKE_DIR(src->left, &((*dest)->left), (*dest));
            TAKE_DIR(src->right, &((*dest)->right), (*dest));

        break;
    case MUL:
        CREATE_OP_CODE_SEG(*dest, par, PLUS, 3);

            CREATE_OP_CODE_SEG((*dest)->left, (*dest), MUL, 2);
                TAKE_DIR(src->left, &(((*dest)->left)->left), (*dest)->left);
                COPY_SEG(src->right, &(((*dest)->left)->right), (*dest)->left);

            CREATE_OP_CODE_SEG((*dest)->right, (*dest), MUL, 2);
                TAKE_DIR(src->right, &(((*dest)->right)->right), (*dest)->right);
                COPY_SEG(src->left, &(((*dest)->right)->left), (*dest)->right);

        break;
    case DIV:
        CREATE_OP_CODE_SEG(*dest, par, DIV, 2);

            CREATE_OP_CODE_SEG((*dest)->left, (*dest), MINUS, 3);

                CREATE_OP_CODE_SEG(((*dest)->left)->left, (*dest)->left, MUL, 2);
                    TAKE_DIR(src->left, &((((*dest)->left)->left)->left), ((*dest)->left)->left);
                    COPY_SEG(src->right, &((((*dest)->left)->left)->right), ((*dest)->left)->left);

                CREATE_OP_CODE_SEG(((*dest)->left)->right, (*dest)->left, MUL, 2);
                    TAKE_DIR(src->right, &((((*dest)->left)->right)->right), ((*dest)->left)->right);
                    COPY_SEG(src->left, &((((*dest)->left)->right)->left), ((*dest)->left)->right);

            CREATE_OP_CODE_SEG((*dest)->right, (*dest), POW, 1);
                COPY_SEG(src->right, &(((*dest)->right)->left), (*dest)->right);
                CREATE_DOUBLE_SEG(((*dest)->right)->right, (*dest)->right, 2);

        break;
    case SIN:
        CREATE_OP_CODE_SEG(*dest, par, MUL, 2);

            CREATE_OP_CODE_SEG((*dest)->left, (*dest), COS, 0);
                COPY_SEG(src->left, &(((*dest)->left)->left), (*dest)->left);

            TAKE_DIR(src->left, &((*dest)->right), (*dest));

        break;
    case COS:
        CREATE_OP_CODE_SEG(*dest, par, MUL, 2);

            CREATE_OP_CODE_SEG((*dest)->left, (*dest), MUL, 2);
                CREATE_OP_CODE_SEG(((*dest)->left)->left, (*dest)->left, SIN, 0);
                    COPY_SEG(src->left, &((((*dest)->left)->left)->left), ((*dest)->left)->left);
                CREATE_DOUBLE_SEG(((*dest)->left)->right, (*dest)->left, -1);

            TAKE_DIR(src->left, &((*dest)->right), (*dest));

        break;
    case TAN:
        CREATE_OP_CODE_SEG(*dest, par, DIV, 2);

            CREATE_DOUBLE_SEG((*dest)->left, (*dest), 1);

            CREATE_OP_CODE_SEG((*dest)->right, (*dest), POW, 1);
                CREATE_OP_CODE_SEG(((*dest)->right)->left, (*dest)->right, COS, 0);
                    COPY_SEG(src->left, &((((*dest)->right)->left)->left), ((*dest)->right)->left);
                CREATE_DOUBLE_SEG(((*dest)->right)->right, (*dest)->right, 2);

        break;
    case LN:
        CREATE_OP_CODE_SEG(*dest, par, DIV, 2);

            TAKE_DIR(src->left, &((*dest)->left), (*dest));
            COPY_SEG(src->left, &((*dest)->right), (*dest));

        break;
    case POW:  //FIXME how to simplify tree
        is_computable = solve_tree_recursive(src->right, &error);
        if (error) return error;
        if (!isnan(is_computable))
        {
            CREATE_OP_CODE_SEG(*dest, par, MUL, 2);
                COPY_SEG(src->right, &((*dest)->left), (*dest));

                CREATE_OP_CODE_SEG((*dest)->right, (*dest), MUL, 2);
                    CREATE_OP_CODE_SEG(((*dest)->right)->left, (*dest)->right, POW, 1);
                        COPY_SEG(src->left, &((*dest)->right->left->left), ((*dest)->right)->left);
                        CREATE_OP_CODE_SEG((*dest)->right->left->right, ((*dest)->right)->left, MINUS, 3);
                            COPY_SEG(src->right, &((*dest)->right->left->right->left), (*dest)->right->left->right);
                            CREATE_DOUBLE_SEG((*dest)->right->left->right->right, (*dest)->right->left->right, 1);
                    TAKE_DIR(src->left, &(((*dest)->right)->right), (*dest)->right);

            break;
        }
        is_computable = solve_tree_recursive(src->left, &error);
        if (!isnan(is_computable))
        {
            CREATE_OP_CODE_SEG(*dest, par, MUL, 2);
                CREATE_OP_CODE_SEG((*dest)->left, (*dest), LN, 0);
                    COPY_SEG(src->left, &((*dest)->left->left), (*dest)->left);

                CREATE_OP_CODE_SEG((*dest)->right, (*dest), MUL, 2);
                    COPY_SEG(src, &(((*dest)->right)->left), (*dest)->right);
                    TAKE_DIR(src->right, &(((*dest)->right)->right), (*dest)->right);

            break;
        }
        CREATE_OP_CODE_SEG(*dest, par, MUL, 2);

            COPY_SEG(src, &((*dest)->left), (*dest));

            CREATE_OP_CODE_SEG((*dest)->right, (*dest), PLUS, 3);
                CREATE_OP_CODE_SEG(((*dest)->right)->left, (*dest)->right, MUL, 2);

                    CREATE_OP_CODE_SEG((((*dest)->right)->left)->left, ((*dest)->right)->left, DIV, 2);
                        TAKE_DIR(src->left, &(((((*dest)->right)->left)->left)->left), (((*dest)->right)->left)->left);
                        COPY_SEG(src->left, &(((((*dest)->right)->left)->left)->right), (((*dest)->right)->left)->left);

                    COPY_SEG(src->right, &((((*dest)->right)->left)->right), ((*dest)->right)->left);

                CREATE_OP_CODE_SEG(((*dest)->right)->right, (*dest)->right, MUL, 2);

                    CREATE_OP_CODE_SEG((((*dest)->right)->right)->left, ((*dest)->right)->right, LN, 0);
                        COPY_SEG(src->left, &(((((*dest)->right)->right)->left)->left), (((*dest)->right)->right)->left);

                    TAKE_DIR(src->right, &((((*dest)->right)->right)->right), ((*dest)->right)->right);

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
