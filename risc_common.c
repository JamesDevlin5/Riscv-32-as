#include "risc_common.h"

struct parser_state ps;
bool DEBUG;

void yyerror(char *s, ...) {
    va_list args;
    va_start(args, s);

    fprintf(stderr, s, args);
    fprintf(stderr, "\n");
}

bool is_small(int numval) {
    int MAX_SMALL_VAL = (1 << 12);
    return numval < MAX_SMALL_VAL;
}

int get_offset(size_t instr_posn, char *label) {
    int label_posn = get_addr(label);
    return (int) (instr_posn - label_posn) * INSTR_SIZE;
}

int get_addr(char *label) {
    struct label_list *curr = ps.labels;
    while (curr) {
        if (strcmp(label, curr->lab->name) == 0) {
            return (int) curr->lab->posn;
        }
        curr = curr->next;
    }
    yyerror("could not find label %s", label);
    return 0;
}

void add_label(char *name) {
    struct label *new_label = malloc(sizeof(struct label));
    new_label->name = name;
    new_label->posn = ps.byte_posn;

    struct label_list *new_list = malloc(sizeof(struct label_list));
    new_list->lab = new_label;
    new_list->next = ps.labels;
    ps.labels = new_list;
}

void init_parser_state() {
    ps.byte_posn = 0;
    ps.labels = NULL;
    ps.instrs.pre_instrs = NULL;
}

void print_labels() {
    struct label_list *curr = ps.labels;
    if (!curr) {
        printf("    <none>\n");
    }
    else {
        while (curr) {
            printf("(%08zu): %s\n", curr->lab->posn, curr->lab->name);
            curr = curr->next;
        }
    }
}

void free_parser_state() {
    struct post_instr_list *tmp_instrs;
    while (ps.instrs.post_instrs) {
        free(ps.instrs.post_instrs->instr);
        tmp_instrs = ps.instrs.post_instrs->next;
        free(ps.instrs.post_instrs);
        ps.instrs.post_instrs = tmp_instrs;
    }

    struct label_list *tmp_labels;
    while (ps.labels) {
        free(ps.labels->lab->name);
        tmp_labels = ps.labels->next;
        free(ps.labels);
        ps.labels = tmp_labels;
    }
}

static void add_instr_to_ps(struct pre_instruction *new_instr) {
    struct pre_instr_list *new_list = malloc(sizeof(struct pre_instr_list));
    new_list->instr = new_instr;
    new_list->next = ps.instrs.pre_instrs;
    ps.instrs.pre_instrs = new_list;
}

void add_r_instr(enum yytokentype op, int dst, int rs1, int rs2) {
    struct pre_instruction *new_instr = malloc(sizeof(struct pre_instruction));
    new_instr->class = R_TY;
    new_instr->instr.r.op = op;
    new_instr->instr.r.dst = dst;
    new_instr->instr.r.rs1 = rs1;
    new_instr->instr.r.rs2 = rs2;

    add_instr_to_ps(new_instr);
    ps.byte_posn += 1;
}

void add_i_instr(enum yytokentype op, int dst, int rs1, int imm) {
    struct pre_instruction *new_instr = malloc(sizeof(struct pre_instruction));
    new_instr->class = I_TY;
    new_instr->instr.i.op = op;
    new_instr->instr.i.dst = dst;
    new_instr->instr.i.rs1 = rs1;
    new_instr->instr.i.imm = imm;

    add_instr_to_ps(new_instr);
    ps.byte_posn += 1;
}

void add_s_instr(enum yytokentype op, int rs1, int rs2, int imm) {
    struct pre_instruction *new_instr = malloc(sizeof(struct pre_instruction));
    new_instr->class = S_TY;
    new_instr->instr.s.op = op;
    new_instr->instr.s.rs1 = rs1;
    new_instr->instr.s.rs2 = rs2;
    new_instr->instr.s.imm = imm;

    add_instr_to_ps(new_instr);
    ps.byte_posn += 1;
}

void add_b_instr(enum yytokentype op, int rs1, int rs2, char *label) {
    struct pre_instruction *new_instr = malloc(sizeof(struct pre_instruction));
    new_instr->class = B_TY;
    new_instr->instr.b.op = op;
    new_instr->instr.b.rs1 = rs1;
    new_instr->instr.b.rs2 = rs2;
    new_instr->instr.b.label = label;

    add_instr_to_ps(new_instr);
    ps.byte_posn += 1;
}

void add_u_instr(enum yytokentype op, int dst, int imm) {
    struct pre_instruction *new_instr = malloc(sizeof(struct pre_instruction));
    new_instr->class = U_TY;
    new_instr->instr.u.op = op;
    new_instr->instr.u.dst = dst;
    new_instr->instr.u.imm = imm;

    add_instr_to_ps(new_instr);
    ps.byte_posn += 1;
}

void add_j_instr(int rd, char *label) {
    struct pre_instruction *new_instr = malloc(sizeof(struct pre_instruction));
    new_instr->class = J_TY;
    new_instr->instr.j.rd = rd;
    new_instr->instr.j.label = label;
    new_instr->instr.j.posn = ps.byte_posn;

    add_instr_to_ps(new_instr);
    ps.byte_posn += 1;
}

void add_la_instr(int rd, char *symbol) {
    struct pre_instruction *new_instr = malloc(sizeof(struct pre_instruction));
    new_instr->class = LA_TY;
    new_instr->instr.la.rd = rd;
    new_instr->instr.la.symbol = symbol;

    add_instr_to_ps(new_instr);
    /* NOTE: `la` will expand into one or two instructions (`lui` & `li`) */
    ps.byte_posn += 2;
}

void add_post_instr(struct post_instruction *instr) {
    struct post_instr_list *new_list = malloc(sizeof(struct post_instr_list));
    new_list->instr = instr;
    new_list->next = ps.instrs.post_instrs;
    ps.instrs.post_instrs = new_list;
}

void fix_instrs() {
    struct pre_instr_list *curr = ps.instrs.pre_instrs;
    struct pre_instr_list *next = NULL;

    ps.instrs.post_instrs = NULL;

    while (curr) {
        struct post_instruction *new_instr = malloc(sizeof(struct post_instruction));
        new_instr->class = curr->instr->class;
        switch (curr->instr->class) {
            case R_TY:
                new_instr->instr.r = curr->instr->instr.r;
                add_post_instr(new_instr);
                break;
            case I_TY:
                new_instr->instr.i = curr->instr->instr.i;
                add_post_instr(new_instr);
                break;
            case S_TY:
                new_instr->instr.s = curr->instr->instr.s;
                add_post_instr(new_instr);
                break;
            case B_TY:
                struct b_pre_instr old_b = curr->instr->instr.b;
                struct b_post_instr new_b = {
                    .op = old_b.op,
                    .rs1 = old_b.rs1,
                    .rs2 = old_b.rs2,
                    .offset = get_offset(old_b.posn, old_b.label)};
                free(old_b.label);
                new_instr->instr.b = new_b;
                add_post_instr(new_instr);
                break;
            case U_TY:
                new_instr->instr.u = curr->instr->instr.u;
                add_post_instr(new_instr);
                break;
            case J_TY:
                struct j_pre_instr old_j = curr->instr->instr.j;
                struct j_post_instr new_j = {
                    .rd = old_j.rd,
                    .offset = get_offset(old_j.posn, old_j.label)};
                free(old_j.label);
                new_instr->instr.j = new_j;
                add_post_instr(new_instr);
                break;
            case LA_TY:
                struct post_instruction *upp_la = malloc(sizeof(struct post_instruction));
                struct post_instruction *low_la = malloc(sizeof(struct post_instruction));
                int address = get_addr(curr->instr->instr.la.symbol);
                int upp_imm = (address >> 12) + ((address & 0x80) >> 11);
                int low_imm = (address & 0x8F);
                upp_la->class = U_TY;
                upp_la->instr.u.op = LUI;
                upp_la->instr.u.dst = curr->instr->instr.la.rd;
                upp_la->instr.u.imm = upp_imm;
                add_post_instr(upp_la);
                low_la->class = I_TY;
                low_la->instr.i.op = ADDI;
                low_la->instr.i.dst = curr->instr->instr.la.rd;
                low_la->instr.i.rs1 = 0;
                low_la->instr.i.imm = low_imm;
                add_post_instr(low_la);
                free(curr->instr->instr.la.symbol);
                break;
        }
        // switch values & advance ptr
        next = curr->next;
        free(curr->instr);
        free(curr);
        curr = next;
    }
}

unsigned int emit(struct post_instruction *instr) {
    unsigned int result = 0;
    int opcode = 0;
    int fn3 = 0;
    int fn7 = 0;
    int imm = 0;

    switch (instr->class) {
        case R_TY:
            struct r_instr r = instr->instr.r;
            opcode = 0x33;

            switch (r.op) {
                case ADD:
                    fn3 = 0;
                    fn7 = 0;
                    break;
                case SUB:
                    fn3 = 0;
                    fn7 = 0x20;
                    break;
                case MUL:
                    fn3 = 0;
                    fn7 = 1;
                    break;
                case MULH:
                    fn3 = 1;
                    fn7 = 1;
                    break;
                case AND:
                    fn3 = 7;
                    fn7 = 0;
                    break;
                case OR:
                    fn3 = 6;
                    fn7 = 0;
                    break;
                case XOR:
                    fn3 = 4;
                    fn7 = 0;
                    break;
                case SLL:
                    fn3 = 1;
                    fn7 = 0;
                    break;
                case SRL:
                    fn3 = 5;
                    fn7 = 0;
                    break;
                case SRA:
                    fn3 = 5;
                    fn7 = 0x20;
                    break;
                case SLT:
                    fn3 = 2;
                    fn7 = 0;
                    break;
                case SLTU:
                    fn3 = 3;
                    fn7 = 0;
                    break;
                default:
                    yyerror("attempting to parse non r-type as r-type");
            }
            result = opcode;
            result |= r.dst << 7;
            result |= fn3 << 12;
            result |= r.rs1 << 15;
            result |= r.rs2 << 20;
            result |= fn7 << 25;
            return result;
        case I_TY:
            struct i_instr i = instr->instr.i;
            opcode = 0x13;

            switch (i.op) {
                case ADDI:
                    fn3 = 0;
                    break;
                case ANDI:
                    fn3 = 7;
                    break;
                case ORI:
                    fn3 = 6;
                    break;
                case XORI:
                    fn3 = 4;
                    break;
                case SLLI:
                    fn3 = 1;
                    break;
                case SRLI:
                    fn3 = 5;
                    break;
                case SRAI:
                    fn3 = 5;
                    imm = 0x400;
                    break;
                default:
                    yyerror("attempting to parse non i-type as i-type");
            }
            imm += i.imm;
            result = opcode;
            result |= i.dst << 7;
            result |= fn3 << 12;
            result |= i.rs1 << 15;
            result |= imm << 20;
            return result;
        case S_TY:
            struct s_instr s = instr->instr.s;
            opcode = 0x23;

            switch (s.op) {
                case SB:
                    fn3 = 0;
                    break;
                case SH:
                    fn3 = 1;
                    break;
                case SW:
                    fn3 = 2;
                    break;
                default:
                    yyerror("attempting to parse non s-type as s-type");
            }
            result = opcode;
            result |= (s.imm & 0x1F) << 7;
            result |= fn3 << 12;
            result |= s.rs1 << 15;
            result |= s.rs2 << 20;
            result |= (s.imm & 0xFE) << 25;
            return result;
        case B_TY:
            struct b_post_instr b = instr->instr.b;
            opcode = 0x63;

            switch (b.op) {
                case BEQ:
                    fn3 = 0;
                    break;
                case BNE:
                    fn3 = 1;
                    break;
                case BLT:
                    fn3 = 4;
                    break;
                case BGE:
                    fn3 = 5;
                    break;
                case BLTU:
                    fn3 = 6;
                    break;
                case BGEU:
                    fn3 = 7;
                    break;
                default:
                    yyerror("attempting to parse non b-type as b-type");
            }
            int low_imm = (b.offset & 0x1E) | ((b.offset & 0x800) >> 11);
            int upp_imm = ((b.offset & 0x7E0) >> 5) | ((b.offset & 0x1000) >> 6);
            result = opcode;
            result |= low_imm << 7;
            result |= fn3 << 12;
            result |= b.rs1 << 15;
            result |= b.rs2 << 20;
            result |= upp_imm << 25;
            return result;
        case U_TY:
            struct u_instr u = instr->instr.u;
            switch (u.op) {
                case LUI:
                    opcode = 0x37;
                    break;
                case AUIPC:
                    opcode = 0x17;
                    break;
                default:
                    yyerror("attempting to parse non u-type as s-type");
            }
            result = opcode;
            result |= u.dst << 7;
            // NOTE: the immediate is NOT shifted in U-Type
            result |= u.imm << 12;
            return result;
        case J_TY:
            struct j_post_instr j = instr->instr.j;
            opcode = 0x6F;
            // FIXME: immediate value
            imm = j.offset;
            result = opcode;
            result |= j.rd << 7;
            result |= imm << 12;
            return result;
        default:
            yyerror("attempting to parse unrecognized instruction type");
            return 0;
    }
}

void pr_instrs() {
    struct post_instr_list *list = ps.instrs.post_instrs;
    if (DEBUG) {
        printf("Labels:\n");
        print_labels();
        printf("\n");
    }
    int i = 1;
    while (list) {
        int out = emit(list->instr);
        if (DEBUG) {
            printf("(%4d): %08x\n", i, out);
        } else {
            printf("%08x\n", out);
        }
        list = list->next;
        i++;
    }
}
