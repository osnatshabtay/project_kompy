lex scanner.l
yacc -d parser.y
cc -o parser y.tab.c -ll -Ly
./parser < test_un_dec_var.t
# ./parser < test_un_dec_func.t
# ./parser < test_bad_ret_val.t
# ./parser < test_just_else.t