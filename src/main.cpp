#include <stdio.h>

#include "Diff.h"

int main()
{
    TreeData tree = {};
    read_diff_from_file("hh.txt", &tree);
    return 0;
}