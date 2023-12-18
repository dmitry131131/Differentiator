/**
 * @file 
 * @brief All differentiator functions
*/
#ifndef DIFF_H
#define DIFF_H

#include "DataBuffer.h"
#include "Tree.h"
#include "DiffErrors.h"
#include "Output.h"
#include "Parser.h"

double solve_tree(const TreeData* tree, diffErrorCode* error);

double solve_tree_recursive(const TreeSegment* segment, diffErrorCode* error);

diffErrorCode take_derivative(TreeData* input, TreeData* output);

#endif