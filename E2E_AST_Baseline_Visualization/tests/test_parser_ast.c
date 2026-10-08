/* Inspect the parser's actual in-memory result before cleanup. */
#include "parse_context.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

ParseContext parse_context = {0};
extern FILE *yyin;
int yyparse(void);
int yylex_destroy(void);

static void check(const char *source, const char *name, const char *value) {
    parse_context = (ParseContext){0};
    yyin = tmpfile();
    assert(yyin);
    assert(fputs(source, yyin) >= 0);
    rewind(yyin);
    assert(yyparse() == 0);
    assert(!parse_context.had_error && !parse_context.out_of_memory);
    AstNode *program = parse_context.root;
    assert(program && strcmp(program->kind, "Program") == 0);
    assert(program->text == NULL && program->parent == NULL);
    assert(program->role == NULL && program->next_sibling == NULL);
    AstNode *decl = program->first_child;
    assert(decl && program->last_child == decl);
    assert(strcmp(decl->kind, "Declaration") == 0);
    assert(strcmp(decl->text, "let") == 0);
    assert(strcmp(decl->role, "statement") == 0);
    assert(decl->parent == program && decl->next_sibling == NULL);
    AstNode *id = decl->first_child;
    assert(id && strcmp(id->kind, "Identifier") == 0);
    assert(strcmp(id->text, name) == 0 && strcmp(id->role, "name") == 0);
    AstNode *literal = id->next_sibling;
    assert(literal && decl->last_child == literal);
    assert(strcmp(literal->kind, "Integer") == 0);
    assert(strcmp(literal->text, value) == 0);
    assert(strcmp(literal->role, "initializer") == 0);
    assert(id->parent == decl && literal->parent == decl);
    assert(id->first_child == NULL && id->last_child == NULL);
    assert(literal->first_child == NULL && literal->last_child == NULL);
    assert(literal->next_sibling == NULL);
    size_t count = 0;
    for (AstNode *n = parse_context.tree.allocations; n; n = n->allocation_next) {
        assert(n->owner == &parse_context.tree);
        ++count;
    }
    assert(count == 4);
    FILE *input = yyin;
    assert(yylex_destroy() == 0);
    assert(fclose(input) == 0);
    ast_destroy(&parse_context.tree);
    parse_context.root = NULL;
    assert(parse_context.tree.allocations == NULL);
}

int main(void) {
    check("let total = 12;", "total", "12");
    check("let count=007;", "count", "007");
    check("let _x9=0;", "_x9", "0");
    check("let big=9999999999999999999999999999999999999999;",
          "big", "9999999999999999999999999999999999999999");
    check("// lead\r\nlet a // name\n= 1; // end", "a", "1");
    check("\tlet\tN_2 =\n0007;\n", "N_2", "0007");
    puts("PASS 6 direct in-memory parser AST cases");
    return 0;
}
