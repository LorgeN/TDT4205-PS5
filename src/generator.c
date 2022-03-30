#include <vslc.h>

/**Generate table of strings in a rodata section. */
void generate_stringtable(void);
/**Declare global variables in a bss section */
void generate_global_variables(void);
/**Generate function entry code
 * @param function symbol table entry of function */
void generate_function(symbol_t *function);
/**Generate code for a node in the AST, to be called recursively from
 * generate_function
 * @param node root node of current code block */
static void generate_node(node_t *node);
/**Initializes program (already implemented) */
void generate_main(symbol_t *first);

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static const char *record[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

void generate_program(void) {
    /* TODO: Emit assembly instructions for functions, function calls,
     * print statements and expressions.
     * The provided function 'generate_main' creates a program entry point
     * for the function symbol it is given as argument.
     */

    generate_stringtable();
    generate_global_variables();
    // TODO: Implement
    // - Generate string table
    // - Declare global variables
    // - Generate code for all functions
    // - Generate main (function already implemented) by assigning either the
    //   function named main or the first function of the source file if no
    //   main exists.
}

void generate_stringtable(void) {
    /* These can be used to emit numbers, strings and a run-time
     * error msg. from main
     */
    puts(".section .rodata");
    puts(".intout:\n\t.asciz \"\%ld \"");
    puts(".strout:\n\t.asciz \"\%s \"");
    puts(".errout:\n\t.asciz \"Wrong number of arguments\"");

    for (size_t i = 0; i < stringc; i++) {
        printf("STR%d:\n\t.asciz %s\n", i, string_list[i]);
    }
}

void generate_global_variables(void) {
    puts(".section .bss");
    puts(".align 8");

    size_t n_globals = tlhash_size(global_names);
    symbol_t **global_list = malloc(sizeof(symbol_t *) * n_globals);
    tlhash_values(global_names, (void **) global_list);

    symbol_t *node;
    for (size_t i = 0; i < n_globals; i++) {
        node = global_list[i];
        if (node->type != SYM_GLOBAL_VAR) {
            continue;
        }

        printf(".%s:\n", node->name);
    }

    puts(".section .text");
    for (size_t i = 0; i < n_globals; i++) {
        node = global_list[i];
        if (node->type != SYM_FUNCTION) {
            continue;
        }

        generate_function(node);
    }
}

void generate_function(symbol_t *function) {
    printf(".globl %s\n", function->name);
    printf("%s:\n", function->name);
    puts("\tpushq %rbp");
    puts("\tmovq %rsp, %rbp");

    size_t paramc = MIN(6, function->nparms);
    for (int param = 0; param < paramc; param++) {
        printf("\tpushq %s\n", record[param]);
    } 

    if (paramc % 2 != 0) {
        puts("\tsubq $8, %rsp");
    }

    generate_node(function->node);
}

void generate_node(node_t *node) {
    // TODO: Generate code corresponding to node
}

/**Generates the main function with argument parsing and calling of our
 * main function (first, if no function is named main)
 * @param first Symbol table entry of our main function */
void generate_main(symbol_t *first) {
    puts(".globl main");
    puts(".section .text");
    puts("main:");
    puts("\tpushq   %rbp");
    puts("\tmovq    %rsp, %rbp");

    printf("\tsubq\t$1,%%rdi\n");
    printf("\tcmpq\t$%zu,%%rdi\n", first->nparms);
    printf("\tjne\tABORT\n");
    printf("\tcmpq\t$0,%%rdi\n");
    printf("\tjz\tSKIP_ARGS\n");

    printf("\tmovq\t%%rdi,%%rcx\n");
    printf("\taddq $%zu, %%rsi\n", 8 * first->nparms);
    printf("PARSE_ARGV:\n");
    printf("\tpushq %%rcx\n");
    printf("\tpushq %%rsi\n");

    printf("\tmovq\t(%%rsi),%%rdi\n");
    printf("\tmovq\t$0,%%rsi\n");
    printf("\tmovq\t$10,%%rdx\n");
    printf("\tcall\tstrtol\n");

    /*  Now a new argument is an integer in rax */

    printf("\tpopq %%rsi\n");
    printf("\tpopq %%rcx\n");
    printf("\tpushq %%rax\n");
    printf("\tsubq $8, %%rsi\n");
    printf("\tloop PARSE_ARGV\n");

    /* Now the arguments are in order on stack */
    for (int arg = 0; arg < MIN(6, first->nparms); arg++)
        printf("\tpopq\t%s\n", record[arg]);

    printf("SKIP_ARGS:\n");
    printf("\tcall\t_%s\n", first->name);
    printf("\tjmp\tEND\n");
    printf("ABORT:\n");
    printf("\tmovq\t$.errout, %%rdi\n");
    printf("\tcall puts\n");

    printf("END:\n");
    puts("\tmovq    %rax, %rdi");
    puts("\tcall    exit");
}
