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

struct tokenArray {
    DiffToken* Array;
    size_t Pointer;
    size_t size;
};

diffErrorCode read_text_command(outputBuffer* buffer, DiffToken* token);

diffErrorCode read_punct_command(outputBuffer* buffer, DiffToken* token);

diffErrorCode diff_tokenizer(tokenArray* token_array, outputBuffer* buffer);

diffErrorCode read_diff_from_file(const char* filename, TreeData* tree);

TreeSegment* CreateNode(SegmemtType type, SegmentData data, TreeSegment* left, TreeSegment* right);

TreeSegment* getId(tokenArray* token_array, diffErrorCode* error);

TreeSegment* getPow(tokenArray* token_array, diffErrorCode* error);

TreeSegment* getN(tokenArray* token_array, diffErrorCode* error);

TreeSegment* getP(tokenArray* token_array, diffErrorCode* error);

TreeSegment* getT(tokenArray* token_array, diffErrorCode* error);

TreeSegment* getE(tokenArray* token_array, diffErrorCode* error);

diffErrorCode getG(TreeData* tree, tokenArray* token_array);

#endif