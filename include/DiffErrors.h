/**
 * @file
 * @brief All differentiator errors
*/
#ifndef DIFF_ERRORS_H
#define DIFF_ERRORS_H

enum diffErrorCode {
    NO_DIFF_ERRORS,
    DIFF_FILE_OPEN_ERROR,
    READ_FROM_FILE_ERROR,
    WRONG_DIFF_SYNTAX,
    DTOR_BUFFER_ERROR,
    BAD_TREE_SEGMENT,
    BAD_TREE_COMPOSITION,
    COPY_SUBTREE_ERROR,
    REFERENCE_FILE_CREATING_ERROR
};

void print_diff_error(diffErrorCode error);

void print_diff_error_message(diffErrorCode error, FILE* stream);

#endif