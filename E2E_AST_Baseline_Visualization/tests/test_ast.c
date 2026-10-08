#include "ast.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    Ast tree = {0}, other = {0};
    char kind[] = "Identifier", value[] = "x", role[] = "left";
    AstNode *root = ast_new(&tree, "Program", NULL);
    AstNode *a = ast_new(&tree, kind, value);
    AstNode *b = ast_new(&tree, "Integer", "007");
    AstNode *foreign = ast_new(&other, "Identifier", "z");
    assert(root && a && b && foreign);
    kind[0] = 'Z'; value[0] = 'y';
    assert(strcmp(a->kind, "Identifier") == 0);
    assert(strcmp(a->text, "x") == 0);
    assert(a->id != b->id);
    assert(ast_add_child(root, role, a));
    role[0] = 'z';
    assert(strcmp(a->role, "left") == 0);
    assert(ast_add_child(root, "right", b));
    assert(root->first_child == a && a->next_sibling == b);
    assert(root->last_child == b && b->next_sibling == NULL);
    assert(a->parent == root && b->parent == root);
    assert(!ast_add_child(root, "again", a));
    assert(!ast_add_child(a, "cycle", root));
    assert(!ast_add_child(foreign, "self", foreign));
    assert(!ast_add_child(root, "foreign", foreign));
    assert(!ast_add_child(NULL, "null", b));
    assert(!ast_add_child(root, "null", NULL));
    assert(!ast_add_child(root, "", foreign));
    assert(ast_new(NULL, "Bad", NULL) == NULL);
    assert(ast_new(&tree, NULL, NULL) == NULL);
    /* Unattached allocations must be reclaimed too. */
    assert(ast_new(&tree, "Unattached", "partial construction"));
    ast_destroy(&tree);
    ast_destroy(&other);
    assert(tree.allocations == NULL && tree.next_id == 0);
    ast_destroy(&tree);
    ast_destroy(NULL);
    /* Cleanup uses the allocation list, not recursive tree traversal. */
    AstNode *tail = ast_new(&tree, "Root", NULL);
    assert(tail);
    for (int i = 0; i < 5000; ++i) {
        AstNode *child = ast_new(&tree, "Child", NULL);
        assert(child && ast_add_child(tail, "child", child));
        tail = child;
    }
    ast_destroy(&tree);
    puts("PASS AST allocation, copying, linking and cleanup");
    return 0;
}
