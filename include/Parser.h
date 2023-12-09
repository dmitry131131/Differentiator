/**
 * @file
 * @brief Parser functions
*/
#ifndef PARSER_H
#define PARSER_H

const size_t MAX_TEXT_LEN = 250;

enum TokenType {
    NO_TYPE,
    NUM,
    VAR,
    OP
};

union TokenData {
    double num;
    OpCodes op;
    int var;
};

struct DiffToken {
    TokenData data;
    TokenType type;
    size_t position;
};

diffErrorCode read_text_command(outputBuffer* buffer, DiffToken* token);

diffErrorCode read_punct_command(outputBuffer* buffer, DiffToken* token);

diffErrorCode diff_tokenizer(DiffToken** token_array, outputBuffer* buffer);

diffErrorCode read_diff_from_file(const char* filename, TreeData* tree);

#endif