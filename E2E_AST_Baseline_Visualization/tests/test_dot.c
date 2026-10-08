#include "ast_dot.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    Ast tree = {0};
    AstNode *root = ast_new(&tree, "Program", NULL);
    AstNode *a = ast_new(&tree, "String", "quote \"; slash \\; newline \n; tab \t; CR \r; control \001");
    AstNode *b = ast_new(&tree, "String", "\"]; evil [label=\"injected\"]; // \\N");
    assert(root && a && b);
    assert(ast_add_child(root, "first", a));
    assert(ast_add_child(root, "second", b));
    assert(ast_write_dot(NULL, root) == -1);
    assert(ast_write_dot(stdout, NULL) == -1);
    assert(ast_write_dot(stdout, root) == 0 && fflush(stdout) == 0);
    ast_destroy(&tree);
    return 0;
}
