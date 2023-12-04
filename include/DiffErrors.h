/**
 * @file
 * @brief All differentiator errors
*/
#ifndef DIFF_ERRORS_H
#define DIFF_ERRORS_H

enum diffErrorCode {
    NO_DIFF_ERRORS
};

void print_diff_error(diffErrorCode error);

void print_diff_error_message(diffErrorCode error, FILE* stream);

#endif