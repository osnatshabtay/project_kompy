lex scanner.l
yacc -d parser.y
cc -o parser y.tab.c -ll -Ly
./parser < test2.t