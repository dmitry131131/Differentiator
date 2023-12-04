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

    default:
        fprintf(stream, "Unknown error!\n");
        break;
    }
    #undef CHECK_CODE
}