/**
 * @file 
 * @brief All output functions
*/
#ifndef OUTPUT_H
#define OUTPUT_H

diffErrorCode print_expression(TreeData* tree, FILE* stream);

diffErrorCode print_expression_to_latex(TreeData* tree, FILE* stream);

diffErrorCode print_expression_to_latex_recursive(const TreeSegment* segment, FILE* stream);

diffErrorCode write_latex_header(FILE* stream);

diffErrorCode write_latex_footer(FILE* stream);

#endif