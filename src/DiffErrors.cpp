#include <stdio.h>

#include "Color_output.h"
#include "DiffErrors.h"

void print_diff_error(diffErrorCode error)
{
    print_diff_error_message(error, stderr);
}

void print_diff_error_message(diffErrorCode error, FILE* stream)
{
    color_fprintf(stream, COLOR_RED, STYLE_BOLD, "Differentiator error: ");

    #define CHECK_CODE(code, message)               \
        case code:                                  \
            fprintf(stream, message);               \
            break;                                  \

    switch (error)
    {
        case NO_DIFF_ERRORS:
            break;

            CHECK_CODE(DIFF_FILE_OPEN_ERROR,    "Can't open input file!\n")
            CHECK_CODE(READ_FROM_FILE_ERROR,    "Error in reading file!\n")
            CHECK_CODE(WRONG_DIFF_SYNTAX,       "Wrong syntax!\n")
            CHECK_CODE(DTOR_BUFFER_ERROR,       "Buffer dtor error!\n")
            CHECK_CODE(BAD_TREE_SEGMENT,        "Bad tree segment(bad type of value)!\n")
            CHECK_CODE(BAD_TREE_COMPOSITION,    "Bad tree composition(can't solve)!\n")

    default:
        fprintf(stream, "Unknown error!\n");
        break;
    }
    #undef CHECK_CODE
}