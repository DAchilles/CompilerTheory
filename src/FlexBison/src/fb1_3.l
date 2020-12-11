%%
"+"     {printf("PLUS\n"); }
"-"     {printf("MINUS\n"); }
"*"     {printf("TIMES\n"); }
"/"     {printf("DIVIDE\n"); }
"|"     {printf("ABS\n"); }
[0-9]+  {printf("NUMBER %s\n", yytext); }
\n      {printf("NELINE\n"); }
[\t]    { }
.       {printf("Mystery character %s\n", yytesxt); }
%%