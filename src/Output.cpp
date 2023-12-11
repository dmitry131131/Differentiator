#include <stdio.h>
#include <assert.h>

#include "Diff.h"

static diffErrorCode print_expression_recursive(TreeSegment* segment, FILE* stream);

static diffErrorCode print_command_by_opcode(OpCodes code, FILE* stream);

diffErrorCode print_expression(TreeData* tree, FILE* stream)
{
    assert(tree);
    assert(stream);

    diffErrorCode error = NO_DIFF_ERRORS;

    error = print_expression_recursive(tree->root, stream);

    return error;
}

static diffErrorCode print_expression_recursive(TreeSegment* segment, FILE* stream)
{
    assert(segment);
    assert(stream);
    diffErrorCode error = NO_DIFF_ERRORS;
    if (segment->left)
    {
        if ((error = print_expression_recursive(segment->left, stream)))
        {
            return error;
        }
    }

    switch (segment->type)
    {
    case DOUBLE_SEGMENT_DATA:
        fprintf(stream, "%.2lf", segment->data.D_number);
        break;

    case OP_CODE_SEGMENT_DATA:
        if ((error = print_command_by_opcode(segment->data.Op_code, stream)))
        {
            return error;
        }
        break;

    case VAR_SEGMENT_DATA:
        fprintf(stream, "x");
        break;

    case TEXT_SEGMENT_DATA:
    case NO_TYPE_SEGMENT_DATA:
    default:
        error = BAD_TREE_SEGMENT;
        return error;
        break;
    }

    if (segment->right)
    {
        if ((error = print_expression_recursive(segment->right, stream)))
        {
            return error;
        }
    }

    return error;
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

    case OBR:
    case CBR:
    default:
        break;
    }

    return NO_DIFF_ERRORS;
}