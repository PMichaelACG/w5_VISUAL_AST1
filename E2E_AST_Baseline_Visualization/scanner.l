/* QuectoC AST Baseline: return token kinds and leaf pointers to Bison. */
%option noyywrap noinput nounput never-interactive nodefault
%{
#include <stdio.h>
#include "parser.tab.h"

#include "parse_context.h"
static int leaf(const char *kind, int token);
%}
%%
"let"                   { return KW_LET; }
"int"                   { return KW_INT; }
"print"                 { return KW_PRINT; }
[A-Za-z_][A-Za-z0-9_]*    { return leaf("Identifier", IDENTIFIER); }
[0-9]+                  { return leaf("Integer", INT_LITERAL); }
"="                     { return ASSIGN; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }
";"                     { return SEMICOLON; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"//"[^\n]*              { /* Skip a line comment. */ }
[ \t\r\n]+              { /* Skip whitespace. */ }
<<EOF>>                 { return 0; }
. {
    fprintf(stderr, "LEX_ERROR unexpected character: %s\n", yytext);
    parse_context.had_error = 1;
    return LEX_ERROR;
}
%%

static int leaf(const char *kind, int token) {
    yylval.node = ast_new(&parse_context.tree, kind, yytext);
    if (!yylval.node) {
        parse_context.out_of_memory = 1;
        return LEX_ERROR;
    }
    return token;
}
