#ifndef RISC_INSTRS_H
#define RISC_INSTRS_H

#include <stddef.h>

#include "risc.tab.h"

/* Size of each Instruction, in words */
#define INSTR_SIZE 4

enum instr_class {
    R_TY,
    I_TY,
    S_TY,
    B_TY,
    U_TY,
    J_TY,
    /* Special handling of `load-address` as we may see the target label after parsing */
    LA_TY
};

struct r_instr {
    enum yytokentype op;
    int dst;
    int rs1;
    int rs2;
};

struct i_instr {
    enum yytokentype op;
    int dst;
    int rs1;
    int imm;
};

struct s_instr {
    enum yytokentype op;
    int rs1;
    int rs2;
    int imm;
};

/* Pre-parsing the label offset */
struct b_pre_instr {
    enum yytokentype op;
    int rs1;
    int rs2;
    char *label;
    size_t posn;
};

/* Post-parsing the label offset */
struct b_post_instr {
    enum yytokentype op;
    int rs1;
    int rs2;
    int offset;
};

struct u_instr {
    enum yytokentype op;
    int dst;
    int imm;
};

/* Pre-parsing the label offset */
struct j_pre_instr {
    int rd;
    char *label;
    size_t posn;
};

/* Post-parsing the label offset */
struct j_post_instr {
    int rd;
    int offset;
};

struct la_pre_instr {
    int rd;
    char *symbol;
};

struct la_post_instr {
    int rd;
    int offset;
};

/* Pre-parsing any label offsets */
struct pre_instruction {
    enum instr_class class;
    union {
        struct r_instr r;
        struct i_instr i;
        struct s_instr s;
        struct b_pre_instr b;
        struct u_instr u;
        struct j_pre_instr j;
        struct la_pre_instr la;
    } instr;
};

/* Post-parsing any label offsets */
struct post_instruction {
    enum instr_class class;
    union {
        struct r_instr r;
        struct i_instr i;
        struct s_instr s;
        struct b_post_instr b;
        struct u_instr u;
        struct j_post_instr j;
        struct la_post_instr la;
    } instr;
};

#endif
