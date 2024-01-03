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

diffErrorCode read_diff_from_file(const char* filename, TreeData* tree);
TreeSegment* CreateNode(SegmemtType type, SegmentData data, TreeSegment* left, TreeSegment* right);

#endif