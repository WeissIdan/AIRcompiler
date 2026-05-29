#!/bin/sh

# Find this shell script's directory - DO NOT DELETE
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Compile the lex and yacc files - Change if needed
lex AIRcompiler.l
yacc -d AIRcompiler.y
cc y.tab.c -o compiler -ll -Ly
./compiler < input1.txt > output1.txt
./compiler < input2.txt > output2.txt
./compiler < input3.txt > output3.txt
./compiler < input4.txt > output4.txt