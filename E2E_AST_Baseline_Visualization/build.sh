#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
export LC_ALL=C
mkdir -p build
flags=(-std=c11 -Wall -Wextra -Werror -pedantic -g -I. -Ibuild)
if [[ ${SANITIZE:-0} == 1 ]]; then
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie)
fi
bison -Wall -Werror -d --report=state -o build/parser.tab.c parser.y
flex -o build/lex.yy.c scanner.l
parser=(ast.c build/parser.tab.c build/lex.yy.c)
"${CC:-gcc}" "${flags[@]}" -D_POSIX_C_SOURCE=200809L \
    "${parser[@]}" ast_dot.c main.c -o build/quectoc-ast
"${CC:-gcc}" "${flags[@]}" ast.c tests/test_ast.c -o build/test-ast
"${CC:-gcc}" "${flags[@]}" -D_POSIX_C_SOURCE=200809L \
    "${parser[@]}" tests/test_parser_ast.c -o build/test-parser-ast
"${CC:-gcc}" "${flags[@]}" ast.c ast_dot.c tests/test_dot.c -o build/test-dot
