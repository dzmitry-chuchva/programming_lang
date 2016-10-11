/*! \file          lex.l
 *  \author        Чучва Дмитрий
 *  \date          07.12.2006
 *  \brief         файл лексического анализатора, определяющий токены языка
 *
 *  Данный файл передается лексическому анализатору flex для генерации лексического сканера
 *  токенов для синтаксического анализатора. Синтаксический анализатор вызывает генерируемую
 *  лексическим сканером функцию yylex каждый раз, когда ему необходимо получить очередной
 *  токен, исходя из грамматики.
 */
 
%{

    #include "symbol.h"
    #include "code3.h"
    #include "parse.h"
    #include "stdlib.h"
    #include "math.h"
        
    extern int lineno;					//!< номер строки в исходном файле
    extern char syn_errflag;			//!< флаг синтаксической ошибки

%}

DIGIT    [0-9]
ID       [a-z_][a-z0-9_]*
STRING   \"[^\n"'`]+\"
COMMENT1 \/\*[^(\*\/)]*\*\/
COMMENT2 \/\/[^\n]*\n

%option noyywrap

%%

"int"                   { yylval.sym = lookup(yytext); return TOK_INTEGER_TYPE; }
"float"                 { yylval.sym = lookup(yytext); return TOK_FLOAT_TYPE; }
"return"                { return TOK_RETURN_WORD; }
"read"                  { return TOK_READ_WORD; }
"print"                 { return TOK_PRINT_WORD; }
"println"               { return TOK_PRINTLN_WORD; }
"do"                    { return TOK_DO_WORD; }
"while"                 { return TOK_WHILE_WORD; }
"for"                   { return TOK_FOR_WORD; }
"if"                    { return TOK_IF_WORD; }
"else"                  { return TOK_ELSE_WORD; }


{STRING}                {
                            symbol *symb;
                            
                            symb = install_string(yytext);
                            yylval.sym = symb;
                            return TOK_STRING;
                        }


{DIGIT}+                {
                            yylval.expr.val.ival = atoi(yytext);
                            yylval.expr.type = TYPE_INT;
                            return TOK_INTEGER_VAL;
                        }
{DIGIT}+"."{DIGIT}*     {
                            yylval.expr.val.fval = (float)atof(yytext);
                            yylval.expr.type = TYPE_FLOAT;
                            return TOK_FLOATING_VAL;
                        }

{ID}                    { 
                            symbol *symb;
                            
                            if ((symb = lookup(yytext)) == NULL)
                                symb = install(yytext,0);
                            yylval.sym = symb;
                            return TOK_IDENTIFIER; 
                        }

;                       { return TOK_SEMICOLON; }
,                       { return TOK_COMA; }
=                       { return TOK_ASSIGN; }


"("                     { return TOK_LBRACKET; }
")"                     { return TOK_RBRACKET; }
"{"                     { return TOK_LFBRACKET; }       // context_level++(--) is in parse.y after '(',')' in function rules
"}"                     { return TOK_RFBRACKET; }

"+"                     { return TOK_PLUS; }
"++"                    { return TOK_PLUSPLUS; }
"--"                    { return TOK_MINUSMINUS; }
"-"                     { return TOK_MINUS; }
"*"                     { return TOK_MULT; }
"/"                     { return TOK_DIV; }
"^"                     { return TOK_POW; }
"%"                     { return TOK_MOD; }
"=="                    { return TOK_EQ; }
">"                     { return TOK_GT; }
"<"                     { return TOK_LW; }
">="                    { return TOK_GE; }
"<="                    { return TOK_LE; }
"||"                    { return TOK_OR; }
"&&"                    { return TOK_AND; }
"^^"                    { return TOK_XOR; }
"!"                     { return TOK_NOT; }
"!="                    { return TOK_NEQ; }

\n                      { lineno++; }
{COMMENT1}              {}
{COMMENT2}				{ lineno++; }
[ \t\r]+                {}
.                       {}

%%

//! выводит информацию о синтаксической ошибке
/*!
 *  Эта функция вызывается синтаксическим анализатором при обнаружении синтаксической ошибки.
 *  \param s сформированная синтаксическим анализатором строка с сообщением об ошибке
 */
 void yyerror(char *s)
{
    printf("%d: %s near %s\n",lineno,s,yytext);
    syn_errflag = 1;
}