
%{
    #include <string.h>
    #include <stdlib.h>
    #include "risc.tab.h"
    #include "risc_common.h"
%}

%option noyywrap nodefault nounput yylineno

DIGIT        [0-9]
HEXDIGIT     [0-9aAbBcCdDeEfF]
BINDIGIT     [01]
LABELHEAD    [a-zA-Z0-9.]
LABELBODY    [-_a-zA-Z0-9.]
WS           [ \t\r]

%%

"#".*     ; /* Eat comments to newline */
{WS}+     ; /* Eat whitespace */

"+" |
"-" |
"," |
":" |
"(" |
")"      { return yytext[0]; } /* Pass through */

"add"    { return ADD; }
"sub"    { return SUB; }
"addi"   { return ADDI; }
"mul"    { return MUL; }
"mulh"   { return MULH; }
"and"    { return AND; }
"or"     { return OR; }
"xor"    { return XOR; }
"andi"   { return ANDI; }
"ori"    { return ORI; }
"xori"   { return XORI; }
"not"    { return NOT; }
"neg"    { return NEG; }
"sll"    { return SLL; }
"srl"    { return SRL; }
"sra"    { return SRA; }
"slli"   { return SLLI; }
"srli"   { return SRLI; }
"srai"   { return SRAI; }
"slt"    { return SLT; }
"sltu"   { return SLTU; }
"slti"   { return SLTI; }
"sltiu"  { return SLTIU; }
"seqz"   { return SEQZ; }
"snez"   { return SNEZ; }
"sltz"   { return SLTZ; }
"sgtz"   { return SGTZ; }
"beq"    { return BEQ; }
"bne"    { return BNE; }
"blt"    { return BLT; }
"bge"    { return BGE; }
"bltu"   { return BLTU; }
"bgeu"   { return BGEU; }
"bgt"    { return BGT; }
"ble"    { return BLE; }
"beqz"   { return BEQZ; }
"bnez"   { return BNEZ; }
"blez"   { return BLEZ; }
"bgez"   { return BGEZ; }
"bltz"   { return BLTZ; }
"bgtz"   { return BGTZ; }
"jal"    { return JAL; }
"jalr"   { return JALR; }
"j"      { return J; }
"ecall"  { return ECALL; }
"call"   { return CALL; }
"ret"    { return RET; }
"lb"     { return LB; }
"lh"     { return LH; }
"lw"     { return LW; }
"lbu"    { return LBU; }
"lhu"    { return LHU; }
"lui"    { return LUI; }
"auipc"  { return AUIPC; }
"li"     { return LI; }
"la"     { return LA; }
"sb"     { return SB; }
"sh"     { return SH; }
"sw"     { return SW; }
"mv"     { return MV; }
"nop"    { return NOP; }

"zero" |
"x0"     { yylval.regnum = 0; return REG; }
"ra" |
"x1"     { yylval.regnum = 1; return REG; }
"sp" |
"x2"     { yylval.regnum = 2; return REG; }
"gp" |
"x3"     { yylval.regnum = 3; return REG; }
"tp" |
"x4"     { yylval.regnum = 4; return REG; }
"t0" |
"x5"     { yylval.regnum = 5; return REG; }
"t1" |
"x6"     { yylval.regnum = 6; return REG; }
"t2" |
"x7"     { yylval.regnum = 7; return REG; }
"s0" |
"fp" |
"x8"     { yylval.regnum = 8; return REG; }
"s1" |
"x9"     { yylval.regnum = 9; return REG; }
"a0" |
"x10"    { yylval.regnum = 10; return REG; }
"a1" |
"x11"    { yylval.regnum = 11; return REG; }
"a2" |
"x12"    { yylval.regnum = 12; return REG; }
"a3" |
"x13"    { yylval.regnum = 13; return REG; }
"a4" |
"x14"    { yylval.regnum = 14; return REG; }
"a5" |
"x15"    { yylval.regnum = 15; return REG; }
"a6" |
"x16"    { yylval.regnum = 16; return REG; }
"a7" |
"x17"    { yylval.regnum = 17; return REG; }
"s2" |
"x18"    { yylval.regnum = 18; return REG; }
"s3" |
"x19"    { yylval.regnum = 19; return REG; }
"s4" |
"x20"    { yylval.regnum = 20; return REG; }
"s5" |
"x21"    { yylval.regnum = 21; return REG; }
"s6" |
"x22"    { yylval.regnum = 22; return REG; }
"s7" |
"x23"    { yylval.regnum = 23; return REG; }
"s8" |
"x24"    { yylval.regnum = 24; return REG; }
"s9" |
"x25"    { yylval.regnum = 25; return REG; }
"s10" |
"x26"    { yylval.regnum = 26; return REG; }
"s11" |
"x27"    { yylval.regnum = 27; return REG; }
"t3" |
"x28"    { yylval.regnum = 28; return REG; }
"t4" |
"x29"    { yylval.regnum = 29; return REG; }
"t5" |
"x30"    { yylval.regnum = 30; return REG; }
"t6" |
"x31"    { yylval.regnum = 31; return REG; }

{DIGIT}+ {
    yylval.numval = atoi(yytext);
    return NUMBER;
}

"'"."'" {
    /* Int-value of single char */
    yylval.numval = yytext[1];
    return NUMBER;
}

"0"[xX]{HEXDIGIT}+ {
    /* Hexadecimal integer literal */
    long int val = strtol(yytext + 2, NULL, 16);
    // TODO: check that value is within range
    yylval.numval = (int) val;
    return NUMBER;
}

"0"[bB]{BINDIGIT}+ {
    /* Binary integer literal */
    long int val = strtol(yytext + 2, NULL, 2);
    // TODO: check that value is within range
    yylval.numval = (int) val;
    return NUMBER;
}

{LABELHEAD}{LABELBODY}* {
    yylval.name = strdup(yytext);
    return LABEL;
}

\n       { return EOL; }

.        { yyerror("lexer unrecognized char (%c)", *yytext); }

