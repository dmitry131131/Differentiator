#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "Diff.h"

diffErrorCode read_diff_from_file(const char* filename, TreeData* tree)
{
    assert(filename);
    assert(tree);

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

    DiffToken* token_array = (DiffToken*) calloc(buffer.customSize, sizeof(DiffToken));
    diff_tokenizer(&token_array, &buffer);

    if (buffer_dtor(&buffer))
    {
        return DTOR_BUFFER_ERROR;
    }

    printf("%lf\n", token_array[0].data.num);

    free(token_array);

    return NO_DIFF_ERRORS;
}

diffErrorCode diff_tokenizer(DiffToken** token_array, outputBuffer* buffer)
{
    assert(token_array);
    assert(buffer);
    diffErrorCode error = NO_DIFF_ERRORS;

    size_t count = 0;
    while (buffer->customBuffer[buffer->bufferPointer] != '\0')
    {
        if (isdigit(buffer->customBuffer[buffer->bufferPointer]))
        {
            (token_array[count])->type     = NUM;
            (token_array[count])->position = buffer->bufferPointer;

            int digit_len = 0;
            sscanf(buffer->customBuffer +buffer->bufferPointer, "%lf%n", &((token_array[count])->data.num), &digit_len);
            buffer->bufferPointer += (size_t) digit_len;
                
            count++;
        }
        else if (isalpha(buffer->customBuffer[buffer->bufferPointer]))
        {
            if ((error = read_text_command(buffer, token_array[count])))
            {
                return error;
            }

            count++;
        }
        else if (ispunct(buffer->customBuffer[buffer->bufferPointer]))
        {
            if ((error = read_punct_command(buffer, token_array[count])))
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
    while(isalpha(buffer->customBuffer[buffer->bufferPointer]))
    {
        cmd[len] = buffer->customBuffer[buffer->bufferPointer];
        len++;
        (buffer->bufferPointer)++;
    }
    cmd[len] = '\0';

    if (!strcmp(cmd, "sin"))
    {
        token->data.op = SIN;
        token->type    = OP;
    }
    else if (!strcmp(cmd, "cos"))
    {
        token->data.op = COS;
        token->type    = OP;
    }
    else if (!strcmp(cmd, "tan"))
    {
        token->data.op = TAN;
        token->type    = OP;
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
