/**
 * @file 
 * @brief All differentiator functions
*/
#ifndef DIFF_H
#define DIFF_H

#include "DataBuffer.h"
#include "DiffErrors.h"
#include "Tree.h"

double solve_tree(const TreeData* tree, diffErrorCode* error);

#endif