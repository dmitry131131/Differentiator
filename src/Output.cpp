#include <stdio.h>
#include <assert.h>

#include "Diff.h"

static diffErrorCode print_expression_recursive(TreeSegment* segment, FILE* stream);

static diffErrorCode print_command_by_opcode(OpCodes code, FILE* stream);

static bool is_single_command(OpCodes code);

diffErrorCode print_expression(TreeData* tree, FILE* stream)
{
    assert(tree);
    assert(stream);

    diffErrorCode error = NO_DIFF_ERRORS;

    error = print_expression_recursive(tree->root, stream);

    fprintf(stream, "\n");

    return error;
}

static diffErrorCode print_expression_recursive(TreeSegment* segment, FILE* stream)
{
    assert(segment);
    assert(stream);

    #define BRACKET(brack) do {                             \
        if (segment->parent)                                \
        {                                                   \
            if (segment->parent->weight < segment->weight)  \
            {                                               \
                fprintf(stream, brack);                     \
            }                                               \
        }                                                   \
    }while(0)

    diffErrorCode error = NO_DIFF_ERRORS;
    BRACKET("(");

    switch (segment->type)
    {
    case DOUBLE_SEGMENT_DATA:
        fprintf(stream, "%.2lf", segment->data.D_number);
        break;

    case VAR_SEGMENT_DATA:
        fprintf(stream, "x");
        break;

    case OP_CODE_SEGMENT_DATA:
        if (is_single_command(segment->data.Op_code))
        {
            if ((error = print_command_by_opcode(segment->data.Op_code, stream)))
            {
                return error;
            }
            fprintf(stream, "(");
            if ((error = print_expression_recursive(segment->left, stream)))
            {
                return error;
            }
            fprintf(stream, ")");
        }
        else
        {
            if (segment->left)
            {
                if ((error = print_expression_recursive(segment->left, stream)))
                {
                    return error;
                }
            }

            if ((error = print_command_by_opcode(segment->data.Op_code, stream)))
            {
                return error;
            }

            if (segment->right)
            {
                if ((error = print_expression_recursive(segment->right, stream)))
                {
                    return error;
                }
            }
        }
        break;

    case TEXT_SEGMENT_DATA:
    case NO_TYPE_SEGMENT_DATA:
    default:
        error = BAD_TREE_SEGMENT;
        return error;
        break;
    }

    BRACKET(")");

    return error;

    #undef BRACKET
}

static bool is_single_command(OpCodes code)
{
    bool res = false;
    switch (code)
    {
    case PLUS:
        break;
    case MINUS:
        break;
    case MUL:
        break;
    case DIV:
        break;
    case POW:
        break;

    case SIN:
    case COS:
    case TAN:
        res = true;
        break;

    case OBR:
    case CBR:
    case NONE:
    
    default:
        break;
    }
    return res;
}

static diffErrorCode print_command_by_opcode(OpCodes code, FILE* stream)
{
    assert(stream);
    switch (code)
    {
    case NONE:
        fprintf(stream, "NONE");
        break;
    case PLUS:
        fprintf(stream, "+");
        break;
    case MINUS:
        fprintf(stream, "-");
        break;
    case MUL:
        fprintf(stream, "*");
        break;
    case DIV:
        fprintf(stream, "/");
        break;
    case SIN:
        fprintf(stream, "sin");
        break;
    case COS:
        fprintf(stream, "cos");
        break;
    case TAN:
        fprintf(stream, "tan");
        break;
    case POW:
        fprintf(stream, "^");
        break;

    case OBR:
    case CBR:
    default:
        break;
    }

    return NO_DIFF_ERRORS;
}