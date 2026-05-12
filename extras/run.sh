#!/bin/bash

# Find this shell script's directory - DO NOT DELETE
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Compile the lex and yacc files - Change if needed
lex AIRcompiler.l
yacc -d AIRcompiler.y
cc y.tab.c -o compiler -ll -Ly
./compiler < input2.txt > output.txt