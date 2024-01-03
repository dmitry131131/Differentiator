#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "DataBuffer.h"
#include "DiffErrors.h"
#include "Tree.h"
#include "Parser.h"

// Tokenizer functions
static diffErrorCode read_text_command(outputBuffer* buffer, DiffToken* token);
static diffErrorCode read_punct_command(outputBuffer* buffer, DiffToken* token);
static diffErrorCode diff_tokenizer(tokenArray* token_array, outputBuffer* buffer);
// Recursive descent functions
static TreeSegment*  getId(tokenArray* token_array, diffErrorCode* error);
static TreeSegment*  getPow(tokenArray* token_array, diffErrorCode* error);
static TreeSegment*  getN(tokenArray* token_array, diffErrorCode* error);
static TreeSegment*  getP(tokenArray* token_array, diffErrorCode* error);
static TreeSegment*  getT(tokenArray* token_array, diffErrorCode* error);
static TreeSegment*  getE(tokenArray* token_array, diffErrorCode* error);
static diffErrorCode getG(TreeData* tree, tokenArray* token_array);

//------------------------------------------------------------------------------------//

// General function
diffErrorCode read_diff_from_file(const char* filename, TreeData* tree)
{
    assert(filename);
    assert(tree);

    #define RETURN(code) do{                \
        free(token_array.Array);            \
        if (buffer_dtor(&buffer))           \
        {                                   \
            return DTOR_BUFFER_ERROR;       \
        }                                   \
        return code;                        \
    }while(0)

    diffErrorCode error = NO_DIFF_ERRORS;

    FILE* file = fopen(filename, "r");
    if (!file)
    {
        return DIFF_FILE_OPEN_ERROR;
    }

    outputBuffer buffer = {};

    if (read_file_in_buffer(&buffer, file))
    {
        return READ_FROM_FILE_ERROR;
    }
    fclose(file);

    tokenArray token_array = {};

    token_array.Array = (DiffToken*) calloc(buffer.customSize + 2, sizeof(DiffToken));
    
    if ((error = diff_tokenizer(&token_array, &buffer)))
    {
        RETURN(error);
    }

    if ((error = getG(tree, &token_array)))
    {
        printf("In position: %lu\n", token_array.Array[token_array.Pointer].position);
        RETURN(error);
    }
    
    RETURN(NO_DIFF_ERRORS);

    #undef RETURN
}

//#################################################################################################//
//-----------------------------------> Tokenizer functions <---------------------------------------//
//#################################################################################################//

static diffErrorCode diff_tokenizer(tokenArray* token_array, outputBuffer* buffer)
{
    assert(token_array);
    assert(buffer);
    diffErrorCode error = NO_DIFF_ERRORS;

    size_t count = 0;
    while (buffer->customBuffer[buffer->bufferPointer] != '\0')
    {
        if (isdigit(buffer->customBuffer[buffer->bufferPointer]))
        {
            ((token_array->Array)[count]).type     = NUM;
            ((token_array->Array)[count]).position = buffer->bufferPointer;

            int digit_len = 0;
            sscanf(buffer->customBuffer +buffer->bufferPointer, "%lf%n", &(((token_array->Array)[count]).data.num), &digit_len);
            buffer->bufferPointer += (size_t) digit_len;

            count++;
        }
        else if (isalpha(buffer->customBuffer[buffer->bufferPointer]))
        {
            if ((error = read_text_command(buffer, &(token_array->Array)[count])))
            {
                return error;
            }

            count++;
        }
        else if (ispunct(buffer->customBuffer[buffer->bufferPointer]))
        {
            if ((error = read_punct_command(buffer, &(token_array->Array)[count])))
            {
                return error;
            }

            count++;
        }
        else
        {
            (buffer->bufferPointer)++;
        }
    }

    token_array->size = count;
    return NO_DIFF_ERRORS;
}

static diffErrorCode read_punct_command(outputBuffer* buffer, DiffToken* token)
{
    assert(buffer);
    assert(token);

    token->position = buffer->bufferPointer;

    switch (buffer->customBuffer[buffer->bufferPointer])
    {
    case '(':
        token->type = OP;
        token->data.op = OBR;
        break;
    case ')':
        token->type = OP;
        token->data.op = CBR;
        break;
    case '+':
        token->type = OP;
        token->data.op = PLUS;
        break;
    case '-':
        token->type = OP;
        token->data.op = MINUS;
        break;
    case '*':
        token->type = OP;
        token->data.op = MUL;
        break;
    case '/':
        token->type = OP;
        token->data.op = DIV;
        break;
    case '^':
        token->type = OP;
        token->data.op = POW;
        break;
    
    default:
        return WRONG_DIFF_SYNTAX;
        break;
    }

    (buffer->bufferPointer)++;
    return NO_DIFF_ERRORS;
}

static diffErrorCode read_text_command(outputBuffer* buffer, DiffToken* token)
{
    assert(buffer);
    assert(token);

    char cmd[MAX_TEXT_LEN] = {};
    size_t len = 0;

    token->position = buffer->bufferPointer;

    while(isalpha(buffer->customBuffer[buffer->bufferPointer]))
    {
        cmd[len] = buffer->customBuffer[buffer->bufferPointer];
        len++;
        (buffer->bufferPointer)++;
    }
    cmd[len] = '\0';

    if (!strcmp(cmd, "sin"))
    {
        token->type    = OP;
        token->data.op = SIN;
    }
    else if (!strcmp(cmd, "cos"))
    {
        token->type    = OP;
        token->data.op = COS;
    }
    else if (!strcmp(cmd, "tan"))
    {
        token->type    = OP;
        token->data.op = TAN;
    }
    else if (!strcmp(cmd, "ln"))
    {
        token->type    = OP;
        token->data.op = LN;
    }
    else if (!strcmp(cmd, "x"))
    {
        token->type     = VAR;
        token->data.var = 1;
    }
    else
    {
        return WRONG_DIFF_SYNTAX;
    }

    return NO_DIFF_ERRORS;
}

//#################################################################################################//
//--------------------------------> Recursive descent functions <----------------------------------//
//#################################################################################################//

static diffErrorCode getG(TreeData* tree, tokenArray* token_array)
{
    assert(tree);
    assert(token_array);

    diffErrorCode error = NO_DIFF_ERRORS;

    tree->root = getE(token_array, &error);

    return error;
}

static TreeSegment* getE(tokenArray* token_array, diffErrorCode* error)
{
    assert(token_array);
    assert(error);

    TreeSegment* val = getT(token_array, error);
    if (*error) return val;

    while ((token_array->Array)[token_array->Pointer].type == OP && 
    ((token_array->Array)[token_array->Pointer].data.op == PLUS || (token_array->Array)[token_array->Pointer].data.op == MINUS))
    {
        OpCodes op = (token_array->Array)[token_array->Pointer].data.op;
        (token_array->Pointer)++;

        TreeSegment* val2 = getT(token_array, error);
        if (*error)
        {
            del_segment(val);
            return val2;
        }

        SegmentData data = {};
        switch ((int) op)
        {
        case PLUS:
            data.Op_code = PLUS;
            val = CreateNode(OP_CODE_SEGMENT_DATA, data, val, val2);
            val2->parent      = val;
            val->left->parent = val;
            val->weight = 3;
            break;
        case MINUS:
            data.Op_code = MINUS;
            val = CreateNode(OP_CODE_SEGMENT_DATA, data, val, val2);
            val2->parent      = val;
            val->left->parent = val;
            val->weight = 3;
            break;
        
        default:
            *error = WRONG_DIFF_SYNTAX;
            del_segment(val);
            del_segment(val2);
            return NULL;
            break;
        }
    }   

    return val;
}

static TreeSegment* getT(tokenArray* token_array, diffErrorCode* error)
{
    TreeSegment* val = getPow(token_array, error);
    if (*error) return val;
    
    while ((token_array->Array)[token_array->Pointer].type == OP && 
    ((token_array->Array)[token_array->Pointer].data.op == MUL || (token_array->Array)[token_array->Pointer].data.op == DIV))
    {
        OpCodes op = (token_array->Array)[token_array->Pointer].data.op;
        (token_array->Pointer)++;

        TreeSegment* val2 = getPow(token_array, error);
        if (*error) 
        {
            del_segment(val);
            return val2;
        }

        SegmentData data = {};
        switch ((int) op)
        {
        case MUL:
            data.Op_code = MUL;
            val = CreateNode(OP_CODE_SEGMENT_DATA, data, val, val2);
            val2->parent      = val;
            val->left->parent = val;
            val->weight = 2;
            break;
        case DIV:
            data.Op_code = DIV;
            val = CreateNode(OP_CODE_SEGMENT_DATA, data, val, val2);
            val2->parent      = val;
            val->left->parent = val;
            val->weight = 2;
            break;
        
        default:
            *error = WRONG_DIFF_SYNTAX;
            del_segment(val);
            del_segment(val2);
            return NULL;
        }
    }

    return val;
}

static TreeSegment* getPow(tokenArray* token_array, diffErrorCode* error)
{
    TreeSegment* val = getP(token_array, error);
    if (*error) return val;

    if ((token_array->Array)[token_array->Pointer].type == OP && (token_array->Array)[token_array->Pointer].data.op == POW)
    {
        (token_array->Pointer)++;

        TreeSegment* val2 = getP(token_array, error);
        if (*error) 
        {
            del_segment(val);
            return val2;
        }

        SegmentData data = {};

        data.Op_code = POW;
        val = CreateNode(OP_CODE_SEGMENT_DATA, data, val, val2);
        val2->parent      = val;
        val->left->parent = val;
        val->weight = 1;
    }

    return val;
}

static TreeSegment* getP(tokenArray* token_array, diffErrorCode* error)
{
    TreeSegment* val = NULL;
    if ((token_array->Array)[token_array->Pointer].type == OP && (token_array->Array)[token_array->Pointer].data.op == OBR)
    {
        (token_array->Pointer)++;
        val = getE(token_array, error);
        if (*error) return val;

        if ((token_array->Array)[token_array->Pointer].data.op != CBR)
        {
            *error = WRONG_DIFF_SYNTAX;
            del_segment(val);
            return nullptr;
        }
        (token_array->Pointer)++;
        return val;
    }
    else if ((token_array->Array)[token_array->Pointer].type == NUM)
    {
        return getN(token_array, error);
    }
    else
    {
        return getId(token_array, error);
    }
}

static TreeSegment* getN(tokenArray* token_array, diffErrorCode* error)
{
    TreeSegment* val = NULL;

    if ((token_array->Array)[token_array->Pointer].type == NUM)
    {
        SegmentData data;
        data.D_number = (token_array->Array)[token_array->Pointer].data.num;
        val = CreateNode(DOUBLE_SEGMENT_DATA, data, nullptr, nullptr);
        val->weight = 0;
        (token_array->Pointer)++;
    }
    else
    {
        *error = WRONG_DIFF_SYNTAX;
    }

    return val;
}

static TreeSegment* getId(tokenArray* token_array, diffErrorCode* error)
{
    TreeSegment* val = NULL;

    if ((token_array->Array)[token_array->Pointer].type == VAR)
    {
        SegmentData data;
        data.Var = 1;
        val = CreateNode(VAR_SEGMENT_DATA, data, nullptr, nullptr);
        val->weight = 0;
    }
    else if ((token_array->Array)[token_array->Pointer].type == OP && (token_array->Array)[token_array->Pointer].data.op)
    {
        SegmentData data;
        data.Op_code = (token_array->Array)[token_array->Pointer].data.op;
        val = CreateNode(OP_CODE_SEGMENT_DATA, data, nullptr, nullptr);
        val->weight = 0;
        (token_array->Pointer)++;

        if ((token_array->Array)[token_array->Pointer].data.op != OBR)
        {
            if (error) *error = WRONG_DIFF_SYNTAX;
            del_segment(val);
            return nullptr;
        }
        (token_array->Pointer)++;
        val->left = getE(token_array, error);
        if ((token_array->Array)[token_array->Pointer].data.op != CBR)
        {
            if (error) *error = WRONG_DIFF_SYNTAX;
            del_segment(val);
            return nullptr;
        }
    }
    else 
    {
        *error = WRONG_DIFF_SYNTAX;
    }
    (token_array->Pointer)++;

    return val;
}

//#################################################################################################//
//------------------------------------> Shared functions <-----------------------------------------//
//#################################################################################################//

TreeSegment* CreateNode(SegmemtType type, SegmentData data, TreeSegment* left, TreeSegment* right)
{
    TreeSegment* seg = new_segment(type, sizeof(int));

    seg->data  = data;
    seg->left  = left;
    seg->right = right;

    return seg;
}