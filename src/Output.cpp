#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "Diff.h"
#include "config.h"

const size_t phrase_count = 10;
const char* phrase_array[] = {
            "Очевидно что:\n",
            "Из чего следует:\n",
            "Легко показать, что:\n",
            "По виду выражения понятно:\n",
            "По методу Дедекинда:\n",
            "Исходя из простейших математических правил:\n",
            "По велению богов матанализа:\n",
            "Легко увидеть, что:\n",
            "Легко заметить, что:\n",
            "Оценим следующее выражение:\n"
};

enum PrintCommandMode {
    CONSOLE,
    LATEX
};

static diffErrorCode print_expression_recursive(TreeSegment* segment, FILE* stream, size_t count = 0);
static diffErrorCode print_command_by_opcode(OpCodes code, FILE* stream, PrintCommandMode mode);
static bool is_single_command(OpCodes code);
static size_t get_subtree_depth(TreeSegment* segment);
static bool is_more(const double first, const double second);

//-------------------------------------------------------------------------------------------------//

#define BRACKET(brack) do {                                                                                                     \
    if (segment->parent)                                                                                                        \
    {                                                                                                                           \
        if (segment->parent->weight < segment->weight || (segment->parent->weight == segment->weight && segment->weight == 1))  \
        {                                                                                                                       \
            fprintf(stream, brack);                                                                                             \
        }                                                                                                                       \
    }                                                                                                                           \
}while(0)

//#################################################################################################//
//--------------------------------> Default output functions <-------------------------------------//
//#################################################################################################//

diffErrorCode print_expression(TreeData* tree, FILE* stream)
{
    assert(tree);
    assert(stream);

    diffErrorCode error = NO_DIFF_ERRORS;

    error = print_expression_recursive(tree->root, stream);

    fprintf(stream, "\n");

    return error;
}

static diffErrorCode print_expression_recursive(TreeSegment* segment, FILE* stream, size_t count)
{
    assert(segment);
    assert(stream);

    diffErrorCode error = NO_DIFF_ERRORS;
    
    BRACKET("(");

    switch ((int) segment->type)
    {
    case DOUBLE_SEGMENT_DATA:
        if (is_more(0, segment->data.D_number))
        {
            fprintf(stream, "(%.2lf)", segment->data.D_number);
        }
        else
        {
            fprintf(stream, "%.2lf", segment->data.D_number);
        }
        break;

    case VAR_SEGMENT_DATA:
        fprintf(stream, "x");
        break;

    case OP_CODE_SEGMENT_DATA:
        if (is_single_command(segment->data.Op_code))
        {
            if ((error = print_command_by_opcode(segment->data.Op_code, stream, CONSOLE)))
            {
                return error;
            }
            fprintf(stream, "(");
            if ((error = print_expression_recursive(segment->left, stream, count)))
            {
                return error;
            }
            fprintf(stream, ")");
        }
        else
        {
            if (!segment->left || !segment->right)
            {
                return BAD_TREE_SEGMENT;
            }

            if ((error = print_expression_recursive(segment->left, stream, count)))
            {
                return error;
            }
            if ((error = print_command_by_opcode(segment->data.Op_code, stream, CONSOLE)))
            {
                return error;
            }
            if ((error = print_expression_recursive(segment->right, stream, count)))
            {
                return error;
            }

        }
        break;

    case TEXT_SEGMENT_DATA:
    case NO_TYPE_SEGMENT_DATA:
    default:
        error = BAD_TREE_SEGMENT;
        return error;
        break;
    }

    BRACKET(")");

    return error;
}

//#################################################################################################//
//---------------------------------> Latex output functions <--------------------------------------//
//#################################################################################################//

diffErrorCode print_expression_to_latex(TreeData* tree, FILE* stream)
{
    assert(tree);
    assert(stream);
    diffErrorCode error = NO_DIFF_ERRORS;

    write_latex_header(stream);
    fprintf(stream, "\\[\n");

    error = print_expression_to_latex_recursive(tree->root, stream);

    fprintf(stream, "\n\\]\n");
    write_latex_footer(stream);

    return error;
}

diffErrorCode print_expression_to_latex_recursive(const TreeSegment* segment, FILE* stream, size_t count)
{
    assert(segment);
    assert(stream);

    diffErrorCode error = NO_DIFF_ERRORS;
    size_t first_rename  = 0;
    size_t second_rename = 0;

    BRACKET("(");

    switch ((int) segment->type)
    {
    case DOUBLE_SEGMENT_DATA:
        if (is_more(0, segment->data.D_number))
        {
            fprintf(stream, "(%.2lf)", segment->data.D_number);
        }
        else
        {
            fprintf(stream, "%.2lf", segment->data.D_number);
        }
        break;

    case VAR_SEGMENT_DATA:
        fprintf(stream, "x");
        break;

    case OP_CODE_SEGMENT_DATA:
        if (is_single_command(segment->data.Op_code))
        {
            if ((error = print_command_by_opcode(segment->data.Op_code, stream, LATEX)))
            {
                return error;
            }
            fprintf(stream, "(");
            if ((error = print_expression_to_latex_recursive(segment->left, stream)))
            {
                return error;
            }
            fprintf(stream, ")");
        }
        else
        {
            if (!segment->left || !segment->right)
            {
                return BAD_TREE_SEGMENT;
            }

            if (segment->data.Op_code == DIV)
            {
                if ((error = print_command_by_opcode(segment->data.Op_code, stream, LATEX)))
                {
                    return error;
                }
                fprintf(stream, "{");
            }

            size_t depth = get_subtree_depth(segment->left);
            if (depth > MAX_LINE_DEPTH)
            {
                count++;
                fprintf(stream, "A_%lu", count);
                first_rename = count;
            }
            else
            {
                if ((error = print_expression_to_latex_recursive(segment->left, stream, count)))
                {
                    return error;
                }
            }
            
            if (segment->data.Op_code == DIV)
            {
                fprintf(stream, "} ");
            }

            if (segment->data.Op_code != DIV)
            {
                if ((error = print_command_by_opcode(segment->data.Op_code, stream, LATEX)))
                {
                    return error;
                }
            }

            if (segment->data.Op_code == DIV || segment->data.Op_code == POW)
            {
                fprintf(stream, "{");
            }

            depth = get_subtree_depth(segment->right);
            if (depth > MAX_LINE_DEPTH)
            {
                count++;
                fprintf(stream, "A_%lu", count);
                second_rename = count;
            }
            else
            {
                if ((error = print_expression_to_latex_recursive(segment->right, stream, count)))
                {
                    return error;
                }
            }

            if (segment->data.Op_code == DIV || segment->data.Op_code == POW)
            {
                fprintf(stream, "} ");
            }
        }
        break;

    case TEXT_SEGMENT_DATA:
    case NO_TYPE_SEGMENT_DATA:
    default:
        error = BAD_TREE_SEGMENT;
        return error;
        break;
    }

    BRACKET(")");

    if (first_rename)
    {
        fprintf(stream, "\n\\\\ A_%lu = ", first_rename);
        if ((error = print_expression_to_latex_recursive(segment->left, stream, count)))
        {
            return error;
        }
    }
    if (second_rename)
    {
        fprintf(stream, "\n\\\\ A_%lu = ", second_rename);
        if ((error = print_expression_to_latex_recursive(segment->right, stream, count)))
        {
            return error;
        }
    }

    return error;
}

#undef BRACKET

diffErrorCode write_latex_header(FILE* stream)
{
    assert(stream);

    fprintf(stream,     "\\documentclass{article}\n"
                        "\\usepackage{graphicx} %% Required for inserting images\n"
                        "\\usepackage[T2A]{fontenc}\n"
                        "\\usepackage{amsfonts}\n"
                        "\\usepackage{mathtools}\n"
                        "\\title{Отчёт}\n"
                        "\\date{September 2023}\n"
                        "\\begin{document}\n"
                        "\\maketitle\n"
                        "\\section{Introduction}\n");

    return NO_DIFF_ERRORS;
}

diffErrorCode write_latex_footer(FILE* stream)
{
    assert(stream);

    fprintf(stream, "\\end{document}\n");

    return NO_DIFF_ERRORS;
}

//#################################################################################################//
//------------------------------------> Shared functions <-----------------------------------------//
//#################################################################################################//

diffErrorCode random_phrase(FILE* stream)
{
    assert(stream);
    size_t phrase_index = (size_t) (rand() % 10);
    if (phrase_index <= phrase_count)
    {
        fprintf(stream, "%s", phrase_array[phrase_index]);
    }
    return NO_DIFF_ERRORS;
}

static bool is_single_command(OpCodes code)
{
    bool res = false;
    switch (code)
    {
    case PLUS:
        break;
    case MINUS:
        break;
    case MUL:
        break;
    case DIV:
        break;
    case POW:
        break;

    case SIN:
    case COS:
    case TAN:
    case LN:
        res = true;
        break;

    case OBR:
    case CBR:
    case NONE:
    
    default:
        break;
    }
    return res;
}

static diffErrorCode print_command_by_opcode(OpCodes code, FILE* stream, PrintCommandMode mode)
{
    assert(stream);
    switch (code)
    {
    case NONE:
        fprintf(stream, "NONE");
        break;
    case PLUS:
        fprintf(stream, "+");
        break;
    case MINUS:
        fprintf(stream, "-");
        break;
    case MUL:
        if (mode == CONSOLE)
            fprintf(stream, "*");
        else 
            fprintf(stream, " \\cdot ");
        break;
    case DIV:
        if (mode == CONSOLE)
            fprintf(stream, "/");
        else
            fprintf(stream, " \\frac");
        break;
    case SIN:
        fprintf(stream, "sin");
        break;
    case COS:
        fprintf(stream, "cos");
        break;
    case TAN:
        fprintf(stream, "tan");
        break;
    case POW:
        fprintf(stream, "^");
        break;
    case LN:
        fprintf(stream, "ln");
        break;

    case OBR:
    case CBR:
    default:
        break;
    }

    return NO_DIFF_ERRORS;
}

static size_t get_subtree_depth(TreeSegment* segment)
{
    assert(segment);

    size_t max_depth = 0, left_depth = 0, right_depth = 0;

    if (segment->left)
    {
        left_depth = get_subtree_depth(segment->left);
    }
    if (segment->right)
    {
        right_depth = get_subtree_depth(segment->right);
    }

    if (!segment->left && !segment->right)
    {
        return 1;
    }

    max_depth = 1 + ((left_depth >= right_depth) ? left_depth : right_depth);
    return max_depth;
}

static bool is_more(const double first, const double second)
{
    return first - second >= epsilon;
}