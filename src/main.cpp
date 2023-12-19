#include <stdio.h>

#include "Diff.h"
#include "Simplify.h"

int main()
{   
    #define RETURN(code) do{        \
        tree_dtor(&tree);           \
        return code;                \
    }while(0)

    TreeData tree = {};
    diffErrorCode error = NO_DIFF_ERRORS;

    if ((error = read_diff_from_file("hh.txt", &tree)))
    {
        print_diff_error(error);
        RETURN(0);
    }

    // tree_dump(&tree);

    printf("Answer: %lf\n", solve_tree(&tree, &error));

    print_expression(&tree, stdout);

    TreeData tree2 = {};

    take_derivative(&tree, &tree2);
    
    print_expression_to_latex(&tree, stdout);
    tree_dump(&tree2);

    print_expression(&tree2, stdout);

    tree_dtor(&tree2);
    tree_dtor(&tree);
    return 0;

    #undef RETURN
}