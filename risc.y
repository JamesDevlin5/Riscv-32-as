
%{
    #include "risc.tab.h"
    #include "risc_common.h"

    // Arg parsing
    #include <unistd.h>
    extern bool DEBUG;
%}

%start start

%union {
    int numval;
    int regnum;
    char *name;
    enum yytokentype op;
}

%token <regnum> REG
%token <numval> NUMBER
%token <name> LABEL

%type <numval> imm

%type <op> r_ins
%type <op> i_ins
%type <op> b_ins
%type <op> u_ins
%type <op> load_ins
%type <op> store_ins

%token ADD SUB ADDI MUL MULH AND OR XOR ANDI ORI XORI
%token SLL SRL SRA SLLI SRLI SRAI SLT SLTU SLTI SLTIU
%token SEQZ SNEZ SLTZ SGTZ SGT SGTU
%token BEQ BNE BLT BGT BLTU BGTU BGEU BLEU BLE BGE
%token BEQZ BNEZ BLEZ BGEZ BLTZ BGTZ
%token JAL JALR J JR ECALL CALL RET
%token LB LH LW LBU LHU LUI LI LA SB SH SW
%token MV NOT NEG AUIPC NOP EOL

%%

r_ins: ADD { $$ = ADD; }
     | SUB { $$ = SUB; }
     | MUL { $$ = MUL; }
     | MULH { $$ = MULH; }
     | AND { $$ = AND; }
     | OR { $$ = OR; }
     | XOR { $$ = XOR; }
     | SLL { $$ = SLL; }
     | SRL { $$ = SRL; }
     | SRA { $$ = SRA; }
     | SLT { $$ = SLT; }
     | SLTU { $$ = SLTU; }
     ;

i_ins: ADDI { $$ = ADDI; }
     | ANDI { $$ = ANDI; }
     | ORI { $$ = ORI; }
     | XORI { $$ = XORI; }
     | SLLI { $$ = SLLI; }
     | SRLI { $$ = SRLI; }
     | SRAI { $$ = SRAI; }
     | SLTI { $$ = SLTI; }
     | SLTIU { $$ = SLTIU; }
     ;

b_ins: BEQ { $$ = BEQ; }
     | BNE { $$ = BNE; }
     | BLT { $$ = BLT; }
     | BGE { $$ = BGE; }
     | BLTU { $$ = BLTU; }
     | BGEU { $$ = BGEU; }
     ;

u_ins: LUI { $$ = LUI; }
     | AUIPC { $$ = AUIPC; }
     ;

load_ins: LB { $$ = LB; }
        | LH { $$ = LH; }
        | LW { $$ = LW; }
        | LBU { $$ = LBU; }
        | LHU { $$ = LHU; }
        ;

store_ins: SB { $$ = SB; }
         | SH { $$ = SH; }
         | SW { $$ = SW; }
         ;

stmt: r_ins REG ',' REG ',' REG { add_r_instr($1, $2, $4, $6); }
    | i_ins REG ',' REG ',' imm { add_i_instr($1, $2, $4, $6); }
    | JALR REG ',' REG ',' imm { add_i_instr(JALR, $2, $4, $6); }
    | b_ins REG ',' REG ',' LABEL { add_b_instr($1, $2, $4, $6); }
    | JAL REG ',' LABEL { add_j_instr($2, $4); }
    | u_ins REG ',' imm { add_u_instr($1, $2, $4); }
    | load_ins REG ',' imm '(' REG ')'  { add_i_instr($1, $2, $6, $4); }
    | store_ins REG ',' imm '(' REG ')' { add_s_instr($1, $6, $2, $4); }
    | LI REG ',' imm {
        if (!is_small($4)) {
            /*
             * lui (imm[31:12] + imm[11])
             * rd := rd + imm[11:0]
             */
            int upp_imm = ($4 >> 12) + (($4 & 0x80) >> 11);
            int low_imm = ($4 & 0xFFF);
            add_u_instr(LUI, $2, upp_imm);
            add_i_instr(ADDI, $2, $2, low_imm);
        }
        else {
            /* rd := 0 + small_imm */
            add_i_instr(ADDI, $2, 0, $4);
        }
    }
    | LA REG ',' LABEL { add_la_instr($2, $4); }
    | NOT REG ',' REG {
        /* xori rd, rs1, -1
         * bit-wise investion (one's complement) of rs1
         */
        add_i_instr(XORI, $2, $4, -1);
    }
    | NEG  REG ',' REG {
        /* sub rd, x0, rs1
         * negative value (two's complement) of rs1
         */
        add_r_instr(SUB, $2, 0, $4);
    }
    | SGT REG ',' REG ',' REG {
        /* P-Type: rs1 > rs2 => rs2 < rs1 */
        add_r_instr(SLT, $2, $6, $4);
    }
    | SGTU REG ',' REG ',' REG {
        /* P-Type: rs1 > rs2 => rs2 < rs1 */
        add_r_instr(SLTU, $2, $6, $4);
    }
    | SEQZ REG ',' REG {
        /* sltiu rd, rs1, 1 */
        add_i_instr(SLTIU, $2, $4, 1);
    }
    | SNEZ REG ',' REG {
        /* sltu rd, x0, rs1 */
        add_r_instr(SLTU, $2, 0, $4);
    }
    | SLTZ REG ',' REG {
        /* slt rd, rs1, x0 */
        add_r_instr(SLT, $2, $4, 0);
    }
    | SGTZ REG ',' REG {
        /* slt rd, x0, rs1 */
        add_r_instr(SLT, $2, 0, $4);
    }
    | BEQZ REG ',' LABEL {
        /* beq rs1, x0, label */
        add_b_instr(BEQ, $2, 0, $4);
    }
    | BNEZ REG ',' LABEL {
        /* bne rs1, x0, label */
        add_b_instr(BNE, $2, 0, $4);
    }
    | BGT REG ',' REG ',' LABEL   {
        /* P-Type: rs1 > rs2 => rs2 < rs1 */
        add_b_instr(BLT, $4, $2, $6);
    }
    | BLTZ REG ',' LABEL {
        /* blt rs1, x0, label */
        add_b_instr(BLT, $2, 0, $4);
    }
    | BGTZ REG ',' LABEL {
        /* blt x0, rs1, label */
        add_b_instr(BLT, 0, $2, $4);
    }
    | BLE REG ',' REG ',' LABEL   {
        /* P-Type: rs1 <= rs2 => rs2 >= rs1 */
        add_b_instr(BGE, $4, $2, $6);
    }
    | BGEZ REG ',' LABEL {
        /* bge rs1, x0, label */
        add_b_instr(BGE, $2, 0, $4);
    }
    | BLEZ REG ',' LABEL {
        /* bge x0, rs1, label */
        add_b_instr(BGE, 0, $2, $4);
    }
    | BGTU REG ',' REG ',' LABEL {
        /* bltu rs2, rs1, label */
        add_b_instr(BLTU, $4, $2, $6);
    }
    | BLEU REG ',' REG ',' LABEL {
        /* bgeu rs2, rs1, label */
        add_b_instr(BGEU, $4, $2, $6);
    }
    | JALR REG {
        /* jalr ra, 0(rs1) */
        add_i_instr(JALR, 1, $2, 0);
    }
    | J LABEL    {
        /* jal x0, label */
        add_j_instr(0, $2);
    }
    | JAL LABEL  {
        /* jal ra, label */
        add_j_instr(1, $2);
    }
    | JR REG {
        /* jalr x0, 0(rs1) */
        add_i_instr(JALR, 0, $2, 0);
    }
    | ECALL { /* TODO */ }
    | CALL LABEL {
        /* auipc ra, (offset[31:12] + offset[11])
         * jalr ra, offset[11:0](ra)
         */
        // TODO
        add_u_instr(AUIPC, 1, 0);
        add_i_instr(JALR, 1, 1, 0);
    }
    | RET {
        /* jalr x0, 0(ra) */
        add_i_instr(JALR, 0, 1, 0);
    }
    | MV REG ',' REG {
        /* addi rd, rs1, 0 */
        add_i_instr(ADDI, $2, $4, 0);
    }
    | NOP {
        /* addi x0, x0, 0 */
        add_i_instr(ADDI, 0, 0, 0);
    }
    ;

imm: '+' NUMBER { $$ = $2; }
   | '-' NUMBER { $$ = - $2; }
   | NUMBER     { $$ = $1; }
   ;

labeldef: LABEL ':' { add_label($1); }
        ;

start:                  /* Empty */
     | start labeldef { /* Definition of a label */ }
     | start stmt EOL { /* Read instruction(s, if p-type, possibly) */ }
     | start EOL      { /* Blank Line, or after labeldef(s) */ }
     ;

%%

int main(int argc, char *argv[]) {
    // Parse CLI args
    int opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
            case 'v':
                DEBUG = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [-v] [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    init_parser_state();

    yyparse();

    fix_instrs();
    pr_instrs();

    free_parser_state();

    return 0;
}
