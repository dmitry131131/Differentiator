#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "DataBuffer.h"
#include "DiffErrors.h"
#include "Tree.h"
#include "Parser.h"

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

    token_array.Array = (DiffToken*) calloc(buffer.customSize, sizeof(DiffToken));
    
    if ((error = diff_tokenizer(&token_array, &buffer)))
    {
        RETURN(error);
    }

    printf("%d\n", token_array.Array[2].data.op);
    
    RETURN(NO_DIFF_ERRORS);

    #undef RETURN
}

diffErrorCode diff_tokenizer(tokenArray* token_array, outputBuffer* buffer)
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

    return NO_DIFF_ERRORS;
}

diffErrorCode read_punct_command(outputBuffer* buffer, DiffToken* token)
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
    
    default:
        return WRONG_DIFF_SYNTAX;
        break;
    }

    (buffer->bufferPointer)++;
    return NO_DIFF_ERRORS;
}

diffErrorCode read_text_command(outputBuffer* buffer, DiffToken* token)
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
/*
diffErrorCode getG(TreeData* tree, DiffToken** token_array)
{
    assert(tree);
    assert(token_array);

    diffErrorCode error = NO_DIFF_ERRORS;

    TreeSegment* val = getE(token_array);


    return error;
}

TreeSegment* getE(DiffToken** token_array)
{
    int val = getT(data);
    while ((data->s)[data->p] == '+' || (data->s)[data->p] == '-')
    {
        int op = (data->s)[data->p];
        (data->p)++;
        int val2 = getT(data);
        switch (op)
        {
        case '+':
            val += val2;
            break;
        case '-':
            val -= val2;
            break;
        
        default:
            printf("getE error!");
            assert(0);
            break;
        }
    }   

    return val;
}
*/