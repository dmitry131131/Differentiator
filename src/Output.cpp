#include <stdio.h>
#include <assert.h>

#include "Output.h"
#include "Diff.h"

static diffErrorCode print_expression_recursive(TreeSegment* segment, FILE* stream);

diffErrorCode print_expression(TreeData* tree, FILE* stream)
{
    assert(tree);
    assert(stream);

    diffErrorCode error = NO_DIFF_ERRORS;

    return error;
}

static diffErrorCode print_expression_recursive(TreeSegment* segment, FILE* stream)
{
    assert(segment);
    assert(stream);
    diffErrorCode error = NO_DIFF_ERRORS;

    switch (segment->type)
    {
    case DOUBLE_SEGMENT_DATA:
        
        break;

    case OP_CODE_SEGMENT_DATA:

        break;

    case VAR_SEGMENT_DATA:

        break;

    case TEXT_SEGMENT_DATA:
    case NO_TYPE_SEGMENT_DATA:
    default:
        error = BAD_TREE_SEGMENT;
        break;
    }

    return error;
}