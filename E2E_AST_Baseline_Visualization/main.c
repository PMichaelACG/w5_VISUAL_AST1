/* Build: bash build.sh
 * Run: ./build/quectoc-ast examples/sample.qc
 * Input may be omitted or '-' for stdin.
 * Exit codes: 0 success, 1 lexical/syntax error, 2 I/O/resource/usage error.
 */
#include "parse_context.h"
#include "ast_dot.h"
#include <stdio.h>
#include <string.h>

ParseContext parse_context = {0};
extern FILE *yyin;
int yyparse(void);
int yylex_destroy(void);

int main(int argc, char **argv) {
    if (argc > 2) {
        fprintf(stderr, "usage: %s [source.qc|-]\n", argv[0]);
        return 2;
    }
    yyin = stdin;
    if (argc == 2 && strcmp(argv[1], "-") != 0) {
        yyin = fopen(argv[1], "rb");
        if (!yyin) { perror(argv[1]); return 2; }
    }
    int parsed = yyparse();
    int status = parsed == 2 || parse_context.out_of_memory ? 2 :
                 parsed != 0 || parse_context.had_error ? 1 : 0;
    if (ferror(yyin)) { fprintf(stderr, "input read failed\n"); status = 2; }
    if (yyin != stdin && fclose(yyin) != 0) {
        perror("closing input");
        status = 2;
    }
    /* A root alone does not prove acceptance of the whole input. */
    if (status == 0 && parse_context.root == NULL) {
        fprintf(stderr, "AST root missing\n");
        status = 2;
    }
    /* Export the existing tree before releasing its nodes. */
    if (status == 0 &&
        (ast_write_dot(stdout, parse_context.root) != 0 || fflush(stdout) != 0)) {
        fprintf(stderr, "DOT output failed\n");
        status = 2;
    }
    if (parse_context.out_of_memory)
        fprintf(stderr, "out of memory while creating an AST node\n");
    yylex_destroy();
    ast_destroy(&parse_context.tree);
    parse_context.root = NULL;
    return status;
}
