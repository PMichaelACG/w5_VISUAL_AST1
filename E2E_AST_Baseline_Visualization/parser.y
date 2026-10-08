/* QuectoC AST construction. The actions build data and do not execute it. */
%require "3.8"
%code requires {
#include "ast.h"
}
%{
#include "parse_context.h"
#include <stdio.h>
int yylex(void);
void yyerror(const char *message);

static AstNode *one(const char *kind, const char *text,
                    const char *role, AstNode *child) {
    AstNode *node = ast_new(&parse_context.tree, kind, text);
    if (!node || !ast_add_child(node, role, child)) return NULL;
    return node;
}
static AstNode *two(const char *kind, const char *text,
                    const char *r1, AstNode *a, const char *r2, AstNode *b) {
    AstNode *node = one(kind, text, r1, a);
    if (!node || !ast_add_child(node, r2, b)) return NULL;
    return node;
}
%}
%union { AstNode *node; }
%token <node> IDENTIFIER INT_LITERAL
%token KW_LET KW_INT KW_PRINT
%token ASSIGN PLUS MINUS SEMICOLON LPAREN RPAREN LEX_ERROR
%start program
/* The Ast allocation list owns every node, including discarded token values.
 * No %destructor frees nodes: main.c destroys the entire Ast on every exit. */
%%
program:
    KW_LET IDENTIFIER ASSIGN INT_LITERAL SEMICOLON {
        AstNode *decl = two("Declaration", "let", "name", $2,
                            "initializer", $4);
        if (!decl) YYNOMEM;
        parse_context.root = one("Program", NULL, "statement", decl);
        if (!parse_context.root) YYNOMEM;
    }
;
%%
void yyerror(const char *message) {
    if (!parse_context.had_error && !parse_context.out_of_memory)
        fprintf(stderr, "PARSER_ERROR %s\n", message);
    parse_context.had_error = 1;
}
