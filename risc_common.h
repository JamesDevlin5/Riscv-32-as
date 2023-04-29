#ifndef RISC_COMMON_H
#define RISC_COMMON_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instructions.h"

extern int yylineno;
void yyerror(char *s, ...);

/*
 * Whether the immediate value can fit in 12 bits (one `li` instruction)
 */
bool is_small(int numval);

int get_offset(size_t instr_posn, char *label);

int get_addr(char *label);

/* An individual label, defining a jump/branch target location */
struct label {
    char *name;
    size_t posn;
};

/* Linked-list of all labels in a parse */
struct label_list {
    struct label *lab;
    struct label_list *next;
};

/* Adds a parsed label name to the label list */
void add_label(char *name);

/* Prints all labels scanned & their byte position */
void print_labels();

/* Linked-list of all instructions in a parse */
struct pre_instr_list {
    struct pre_instruction *instr;
    struct pre_instr_list *next;
};

struct post_instr_list {
    struct post_instruction *instr;
    struct post_instr_list *next;
};

/* Parser State */
struct parser_state {
    // Byte position of parser in regards to output
    size_t byte_posn;
    // List of all labels in the parse
    struct label_list *labels;
    // List of all instructions in the parse
    union {
        struct pre_instr_list *pre_instrs;
        struct post_instr_list *post_instrs;
    } instrs;
};

void init_parser_state();

void add_r_instr(enum yytokentype op, int dst, int rs1, int rs2);
void add_i_instr(enum yytokentype op, int dst, int rs1, int imm);
void add_s_instr(enum yytokentype op, int rs1, int rs2, int imm);
void add_b_instr(enum yytokentype op, int rs1, int rs2, char *label);
void add_u_instr(enum yytokentype op, int dst, int imm);
void add_j_instr(int rd, char *label);
void add_la_instr(int rd, char *symbol);

void add_post_instr(struct post_instruction *instr);

/*
 * Fixes the list of instructions, such that:
 * 1. the list is reversed (the LR parser will give the instructions in reverse-order)
 * 2. any branch or jump target labels are filled in (not la)
 */
void fix_instrs();

/* Free all the items in the parser state */
void free_parser_state();

void pr_instrs();

unsigned int emit(struct post_instruction *instr);

#endif
