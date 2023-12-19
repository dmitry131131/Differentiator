/**
 * @file 
 * @brief tree simplify functions
*/
#ifndef SIMPLIFY_H
#define SIMPLIFY_H

const double epsilon = 0.00001;

diffErrorCode simplify_tree(TreeData* tree, FILE* stream);

#endif