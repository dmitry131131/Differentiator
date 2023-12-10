#include <stdio.h>

#include "Diff.h"
#include "Parser.h"

int main()
{
    TreeData tree = {};
    diffErrorCode error = NO_DIFF_ERRORS;

    if ((error = read_diff_from_file("hh.txt", &tree)))
    {
        print_diff_error(error);
    }

    tree_dump(&tree);

    printf("Answer: %lf\n", solve_tree(&tree, &error));

    tree_dtor(&tree);
    return 0;
}