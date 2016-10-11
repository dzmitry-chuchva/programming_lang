/*! \file          parse.y
 *  \author        „учва ƒмитрий
 *  \date          07.12.2006
 *  \brief         описание грамматики €зыка дл€ bison
 *
 * ƒанный файл задает описание грамматики дл€ генератора синтаксических анализаторов bison.
 * ѕо этому файлу будет построен синтаксический анализатор, который будет вызыватьс€ функцией
 * yyparse() данного файла.
 */
 
%{

/******************************************************************************/
/*                                                                            */
/*            Grammar for math-working languange MathC ver.0.1a               */
/*                          Written by Dmitry Chuchva                         */
/*                                                                            */
/******************************************************************************/

/* вариант €зыка 1 */
/* вариант целевого кода 3 */
/* вариант аналитического задани€ 20 */

#include "main.h"

//! \cond


%}

%union {
        symbol *sym;
        struct	{
					val_t val;
					int type;
				} expr;
		struct	{
					int type;
					symbol *place;
					code *cod;
					symbol *begin, *after;
				} instr;
}

%token <sym> TOK_IDENTIFIER 
%token <expr> TOK_INTEGER_VAL TOK_FLOATING_VAL
%token <sym> TOK_INTEGER_TYPE TOK_FLOAT_TYPE 
%token <sym> TOK_STRING
%token TOK_PLUS TOK_MINUS TOK_MULT TOK_DIV TOK_MOD TOK_POW
%token TOK_EQ TOK_GT TOK_LW TOK_GE TOK_LE TOK_NEQ TOK_OR TOK_AND TOK_XOR TOK_NOT
%token TOK_ASSIGN
%token TOK_SEMICOLON TOK_COMA TOK_LBRACKET TOK_RBRACKET TOK_LFBRACKET TOK_RFBRACKET
%token TOK_RETURN_WORD TOK_READ_WORD TOK_PRINT_WORD TOK_PRINTLN_WORD
%token TOK_FOR_WORD TOK_IF_WORD TOK_ELSE_WORD TOK_DO_WORD TOK_WHILE_WORD
%token TOK_PLUSPLUS TOK_MINUSMINUS /* only in for header */

%type <expr> const_expr
%type <instr> simple_expr logical_expr
//%type <instr> variable_declaration variable_decl_list variable_declaration_set variable_declarations
%type <sym> type_id variable_id
%type <instr> actual_arg_set actual_arg_list
%type <instr> statement statement_list simple_statement composite_statement complex_statement
%type <instr> if_statement while_statement do_statement for_statement
%type <instr> return_statement io_op assign_or_call_op empty_statement end_aoc_op
%type <instr> end_if for_end_expr for_end_expr_list for_end_expr_list_member
%type <instr> for_expr for_expr_list assign_expr
%type <instr> variable_list expr_list /* use only for io_op */


%right TOK_ASSIGN


%left TOK_OR TOK_AND TOK_XOR
%left TOK_EQ TOK_GT TOK_LW TOK_GE TOK_LE TOK_NEQ
%nonassoc TOK_NOT

%left TOK_MINUS TOK_PLUS
%left TOK_MULT TOK_DIV TOK_MOD
%left TOK_UNARY_MINUS TOK_UNARY_PLUS
%right TOK_POW

%%

dummy:          program
                ;
            

program:        block
				| program block
				| program error TOK_SEMICOLON { syn_errflag = 1; yyerrok; }
				| program error TOK_RFBRACKET { syn_errflag = 1; yyerrok; }
                ;

block:          type_id TOK_IDENTIFIER
                    { 
						curr_symbol = $2;
                        curr_type = $1->type;
                    }
                end_block 
                    {
						curr_symbol = NULL;
						curr_type = TYPE_UNKNOWN;
					}
				| type_id TOK_IDENTIFIER error TOK_SEMICOLON { syn_errflag = 1; yyerrok; }
                ;

end_block:      TOK_COMA
					{
						if (IS_DECLARED(curr_symbol))
						{
							static_error(ERROR,"identificator %s redeclarated",curr_symbol -> name);
							//YYERROR;
						}
						else
							DECLARE_VAR(curr_symbol,curr_type);
					}
				variable_decl_list TOK_SEMICOLON
				| TOK_COMA error TOK_SEMICOLON { syn_errflag = 1; yyerrok; }
				| TOK_LBRACKET
					{
						/* proc declared, but not defined yet, maybe will in this line */
//						context_level++;
						change_context(&(curr_symbol -> table));
						if (IS_DECLARED(curr_symbol))
						{
							if (IS_PROC(curr_symbol))
							{
								proc_definition = 1;
								param_index = 0;
							}
							else
							{
								static_error(ERROR,"identificator %s redeclarated",curr_symbol -> name);
								//YYERROR;
							}
						}
						else
						{
							DECLARE_PROC(curr_symbol,curr_type);
							proc_definition = 0;
						}
					}
				function_arg_list
					{
						if (proc_definition)
							if (param_index != count_args(curr_symbol))
							{
								static_error(ERROR,"count of formal parameters (%d) differs from declaration (%d)",param_index,count_args(curr_symbol));
								//YYERROR;
							}
					}
				TOK_RBRACKET end_function
					{
//						context_level--;
						restore_context();
					}
				| TOK_LBRACKET error TOK_RBRACKET end_function { syn_errflag = 1; yyerrok; }
                | TOK_ASSIGN const_expr
                    {
						if (IS_DECLARED(curr_symbol))
						{
							static_error(ERROR,"identificator %s redeclarated",curr_symbol -> name);
							//YYERROR;
						}
						else
						{
							DECLARE_VAR(curr_symbol,curr_type);
							
							// indecl definition - define var (this for msgs "use of undefined var")
							DEFINE(curr_symbol);
							
							if (TYPE_OF(curr_symbol) != $2.type)
								if (TYPE_OF(curr_symbol) == TYPE_INT)
									static_error(WARN,"truncating float to int in assignment to %s",curr_symbol -> name);
							
							if (TYPE_OF(curr_symbol) == TYPE_INT)
								valptr.val.ival = (int)VALUE($2);
							else
								valptr.val.fval = VALUE($2);
								
							// ATTENTION: using `table' field for saving value
							// in code generation, we will find all the TYPE_VAR & TYPE_DEFINED
							// for filling initialized data section
							curr_symbol -> table = valptr.ptr;
						}
                    } end_assign TOK_SEMICOLON
                | TOK_ASSIGN error TOK_SEMICOLON { syn_errflag = 1; yyerrok; }
                | TOK_SEMICOLON
					{
						if (IS_DECLARED(curr_symbol))
						{
							static_error(ERROR,"identificator %s redeclarated",curr_symbol -> name);
							//YYERROR;
						}
						else
							DECLARE_VAR(curr_symbol,curr_type);						
					}
				;

end_assign:     /* empty */
                | TOK_COMA variable_decl_list
                ;

variable_declarations:  variable_declarations variable_declaration_set TOK_SEMICOLON
                        | /* empty */
                        | variable_declarations error TOK_SEMICOLON { syn_errflag = 1; yyerrok; }
                        ;
                         
variable_declaration_set:       type_id
									{
										curr_type = $1->type;
									}
								variable_decl_list
									{
										curr_type = TYPE_UNKNOWN;
									}
                                ;

variable_decl_list:             variable_declaration
                                | variable_decl_list TOK_COMA variable_declaration
                                ;

variable_declaration:   variable_id
                            {
                                if (IS_DECLARED($1))
                                {
									if (!search_context($1 -> name))
									{
										temp = install($1 -> name,0);
										DECLARE_VAR(temp,curr_type);
									}
									else
									{
										static_error(ERROR,"identificator %s redeclarated",$1 -> name);
										//YYERROR;
									}
								}
								else
									DECLARE_VAR($1,curr_type);
                            }
                        | variable_id TOK_ASSIGN const_expr
                            { 
								if (IS_DECLARED($1))
								{
									if (!search_context($1 -> name))
									{
										temp = install($1 -> name,0);
										DECLARE_VAR(temp,curr_type);
										DEFINE(temp);
										
										if (TYPE_OF(temp) != $3.type)
											if (TYPE_OF(temp) == TYPE_INT)
												static_error(WARN,"truncating float to int in assignment to %s",$1 -> name);
																				
										if (TYPE_OF(temp) == TYPE_INT)
											valptr.val.ival = (int)VALUE($3);
										else
											valptr.val.fval = VALUE($3);
											
										// ATTENTION: using `table' field for saving value
										// in code generation, we will find all the TYPE_VAR & TYPE_DEFINED
										// for filling initialized data sections or initializing local vars
										temp -> table = valptr.ptr;
									}
									else
									{
										static_error(ERROR,"identificator %s redeclarated",$1 -> name);
										//YYERROR;
									}
								}
								else
								{
									DECLARE_VAR($1,curr_type);
									
									// indecl definition - define var (this for msgs "use of undefined var")
									DEFINE($1);
									
									if (TYPE_OF($1) != $3.type)
										if (TYPE_OF($1) == TYPE_INT)
											static_error(WARN,"truncating float to int in assignment to %s",$1 -> name);
																		
									if (TYPE_OF($1) == TYPE_INT)
										valptr.val.ival = (int)VALUE($3);
									else
										valptr.val.fval = VALUE($3);
										
									// ATTENTION: using `table' field for saving value
									// in code generation, we will find all the TYPE_VAR & TYPE_DEFINED
									// for filling initialized data sections or initializing local vars
									$1 -> table = valptr.ptr;
								}
                            }
                        ;

variable_id:    TOK_IDENTIFIER { $$ = $1; }
                ;

type_id:        TOK_INTEGER_TYPE { $$ = $1; }
                | TOK_FLOAT_TYPE { $$ = $1; }
                ;

end_function:   TOK_SEMICOLON
					{
						if (proc_definition)
						{
							static_error(ERROR,"function `%s' redeclarated",curr_symbol -> name);
							//YYERROR;
						}
					}
                | function_body
					{
						// deleting local symbols from symtable // - not needed after entering local symtables
						// delete_local(curr_symbol);
					}
                ;

function_arg_list:      /* empty */
                        | function_arg_list_set
                        ;

function_arg_list_set:  function_arg_list_member
                        | function_arg_list_set TOK_COMA function_arg_list_member
                        ;

function_arg_list_member:	type_id 
								{
									if (!proc_definition)
									{
										temp = global_table;
										global_table = NULL;
									}
								}
							variable_id
								{
									if (proc_definition)
									{
										param_index++;
										temp = lookup_arg(curr_symbol,param_index);
										if (temp)
										{
											if (strcmp(temp -> name,$3 -> name) != 0)
											{
												static_error(ERROR,"name of parameter (%s) differs from declaration (%s)",
															$3 -> name,temp -> name);
												//YYERROR;
											}
											else
												if (TYPE_OF(temp) != TYPE_OF($1))
												{
													static_error(ERROR,"type of parameter (%s) differs from declaration",
																temp -> name);
													//YYERROR;
												}
										}
									}
									else
									{
										DECLARE_ARG($3,$1 -> type);
										global_table = temp;
									}
								}
                                ;

function_body:  TOK_LFBRACKET
					{
						DEFINE(curr_symbol);
					}
				variable_declarations statement_list
					{
						// assign code to function
						add_code_entry(curr_symbol,$4.cod);
					}
				TOK_RFBRACKET
				| TOK_LFBRACKET error TOK_RFBRACKET { syn_errflag = 1; yyerrok; }
                ;

statement_list:         /* empty */
							{
								$$.place = NULL;
								$$.cod = NULL;
								$$.type = 0;
							}
                        | statement_list statement
							{
								$$.cod = merge_code($1.cod,$2.cod);
								$$.type = 0;
							}
						| statement_list error TOK_SEMICOLON { syn_errflag = 1; yyerrok; }
                        ;

statement:      simple_statement			/* default action works(rocks:) */
				| complex_statement
                | composite_statement
                ;

simple_statement:       assign_or_call_op
                        | io_op
                        | empty_statement
                        | return_statement
                        ;

assign_or_call_op:      TOK_IDENTIFIER
                            {
                                if (!IS_DECLARED($1))
								{
									static_error(ERROR,"%s: undeclarated identificator",$1 -> name);
									//YYERROR;
								}
                            } 
                        end_aoc_op TOK_SEMICOLON
							{
								$$.place = NULL;
								$$.cod = $3.cod;
								$$.type = 0;
							}
                        ;

end_aoc_op:             TOK_ASSIGN simple_expr
                            {
								if (IS_PROC($<sym>-1))
									{
										static_error(ERROR,"function name (`%s') cannot appear at the left side",$<sym>-1 -> name);
										//YYERROR;
									}
								else
								{
									if (TYPE_OF($<sym>-1) != $2.type)
										if (TYPE_OF($<sym>-1) == TYPE_INT)
											static_error(WARN,"truncating from float to int in assignment to %s",$<sym>-1 -> name);
									
									$$.cod = merge_code($2.cod,gen_code(OP_ASSIGN,$2.place,NULL,$<sym>-1,TYPE_OF($<sym>-1)));
									$$.place = NULL;
									$$.type = 0;
								}
                            }
                        | TOK_LBRACKET
                        	{
                        		if (!IS_PROC($<sym>-1))
                        		{
                        			static_error(ERROR,"`%s': not a function",$<sym>-1 -> name);
                        			//YYERROR;
                        		}
                        		if (call_function) // result of one func is an argument of another
                        		{
                        			fci_t frame = { call_function, param_index, param_count };
                        			
                        			fci_stack = push(fci_stack,&frame,sizeof(frame));
                        		}
								call_function = $<sym>-1;	
								param_count = count_args(call_function);
								param_index = 0;
							}
						actual_arg_set TOK_RBRACKET
							{
								if (IS_PROC($<sym>-1))
									if (param_index != param_count)
										{
											static_error(ERROR,"count of parameters of function `%s' isnot %d",$<sym>-1 -> name,param_index);
											//YYERROR;
										}
								
								$$.place = NULL;
								$$.cod = merge_code($3.cod,gen_code(OP_CALL,call_function,NULL,NULL,TYPE_OF($<sym>-1)));
								$$.type = 0;
		
								call_function = NULL;
								param_count = 0;
								param_index = 0;
								if (top(fci_stack))
								{
									fci_t *frame;
									
									frame = (fci_t *)top(fci_stack);
									call_function = frame -> call_function;
									param_index = frame -> param_index;
									param_count = frame -> param_count;									
								}
							}
						| TOK_LBRACKET error TOK_RBRACKET { syn_errflag = 1; yyerrok; }
                        ;

variable_list:  variable_id
                    {
                        if (!IS_DECLARED($1))
							{
								static_error(ERROR,"%s: undeclarated identificator",$1 -> name);
								//YYERROR;
							}
						else
						{
							if (IS_PROC($1))
								{
									static_error(ERROR,"using function name (`%s') as a variable's",$1 -> name);
									//YYERROR;
								}
							else
							{
								/* gen_code with TYPE_STRING because in this way we push a address of var, not a value */
								/* i dont want to make yet another symbol type, something, like TYPE_ADDR or else */
								$$.cod = gen_code(OP_PARAM,$1,NULL,NULL,TYPE_STRING);
								$$.cod = merge_code($$.cod,gen_code(OP_PARAM,NULL,NULL,NULL,TYPE_OF($1)));
							}
						}
						$$.type = 0;
						param_count++;
                    }
                | variable_list TOK_COMA variable_id
                    {
						if (!IS_DECLARED($3))
							{
								static_error(ERROR,"%s: undeclarated identificator",$3 -> name);
								//YYERROR;
							}
						else
						{
							if (IS_PROC($3))
								{
									static_error(ERROR,"using function name (`%s') as a variable's",$3 -> name);
									//YYERROR;
								}
							else
							{
								$$.cod = merge_code($1.cod,gen_code(OP_PARAM,$3,NULL,NULL,TYPE_STRING));
								$$.cod = merge_code($$.cod,gen_code(OP_PARAM,NULL,NULL,NULL,TYPE_OF($3)));
							}
						}
						$$.type = 0;
						param_count++;
                    }
                ;

empty_statement:        TOK_SEMICOLON
							{
								$$.cod = NULL;
								$$.place = NULL;
								$$.type = 0;
							}
                        ;

actual_arg_set:         /* empty */ { $$.place = NULL; $$.cod = NULL; $$.type = 0; }
                        | actual_arg_list
                        ;

actual_arg_list:        simple_expr
							{
								param_index++;
								temp = lookup_arg(call_function,param_index);
								if (temp)
								{
									if (TYPE_OF(temp) != $1.type)
										if (TYPE_OF(temp) == TYPE_INT)
											static_error(WARN,"truncating from float to int while type of formal parameter %d of function `%s' differs from type of actual parameter",
													param_index,call_function -> name);
									$$.place = NULL;
									$$.cod = merge_code($1.cod,gen_code(OP_PARAM,$1.place,NULL,NULL,TYPE_OF(temp)));
									$$.type = 0;
								}
							}
                        | actual_arg_list TOK_COMA simple_expr
                        	{
								param_index++;
								temp = lookup_arg(call_function,param_index);
								if (temp)
								{
									if (TYPE_OF(temp) != $3.type)
										if (TYPE_OF(temp) == TYPE_INT)
											static_error(WARN,"truncating from float to int while type of formal parameter %d of function `%s' differs from type of actual parameter",
													param_index,call_function -> name);
									$$.place = NULL;
									$$.cod = merge_code($1.cod,$3.cod);
									$$.cod = merge_code($$.cod,gen_code(OP_PARAM,$3.place,NULL,NULL,TYPE_OF(temp)));
									$$.type = 0;
								}
							}
                        ;

return_statement:       TOK_RETURN_WORD simple_expr	TOK_SEMICOLON
							{
								if (TYPE_OF(curr_symbol) != $2.type)
									if (TYPE_OF(curr_symbol) == TYPE_INT)
										static_error(WARN,"truncating from float to int while return from function `%s'",curr_symbol -> name);
										
								$$.cod = merge_code($2.cod,gen_code(OP_RET,$2.place,NULL,NULL,TYPE_OF(curr_symbol)));
								$$.place = NULL;
								$$.type = 0;
							}
                        ;
                        
const_expr:		TOK_INTEGER_VAL
				| TOK_FLOATING_VAL
				| const_expr TOK_PLUS const_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                        if ($$.type == TYPE_INT)
							$$.val.ival = (int)(VALUE($1) + VALUE($3));
						else
							$$.val.fval = VALUE($1) + VALUE($3);
                    }
                | const_expr TOK_MINUS const_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                        if ($$.type == TYPE_INT)
							$$.val.ival = (int)(VALUE($1) - VALUE($3));
						else
							$$.val.fval = VALUE($1) - VALUE($3);
                    }
                | const_expr TOK_MULT const_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                        if ($$.type == TYPE_INT)
							$$.val.ival = (int)(VALUE($1) * VALUE($3));
						else
							$$.val.fval = VALUE($1) * VALUE($3);
                    }
                | const_expr TOK_DIV const_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                        if (VALUE($3) == 0)
							{
								static_error(ERROR,"divide by zero");
								//YYERROR;
							}
						else
						{
							if ($$.type == TYPE_INT)
								$$.val.ival = (int)VALUE($1) / (int)VALUE($3);
							else
								$$.val.fval = VALUE($1) / VALUE($3);
						}
                    }
                | const_expr TOK_MOD const_expr
                    {
						if ($1.type == $3.type && $1.type == TYPE_INT)
						{
							$$.type = TYPE_INT;
							$$.val.ival = (int)VALUE($1) % (int)VALUE($3);
						}
						else
							{
								static_error(ERROR,"mod operator takes only int params");
								//YYERROR;
							}
				    }
                | TOK_LBRACKET const_expr TOK_RBRACKET
					{
						$$ = $2;
					}
				| TOK_LBRACKET error TOK_RBRACKET { syn_errflag = 1; yyerrok; }
                | TOK_MINUS const_expr %prec TOK_UNARY_MINUS
					{
						$$.type = $2.type;
						if ($$.type == TYPE_INT)
							$$.val.ival = -(int)(VALUE($2));
						else
							$$.val.fval = -VALUE($2);
                    }
                | TOK_PLUS const_expr %prec TOK_UNARY_PLUS
					{
						$$ = $2;
					}
                | const_expr TOK_POW const_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                            
                        if ($$.type == TYPE_INT)
							$$.val.ival = (int)pow(VALUE($1),VALUE($3));
						else
							$$.val.fval = (float)pow(VALUE($1),VALUE($3));
                    }
				;
				
simple_expr:    TOK_INTEGER_VAL
					{
						temp = install_const(TYPE_INT | TYPE_CONST,$1.val);
						$$.type = TYPE_INT;
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						$$.cod = gen_code(OP_ASSIGN,temp,NULL,$$.place,TYPE_INT);
					}
                | TOK_FLOATING_VAL
					{
						temp = install_const(TYPE_FLOAT | TYPE_CONST,$1.val);
						$$.type = TYPE_FLOAT;
						$$.place = install_temp(TYPE_TEMP | TYPE_FLOAT);
						$$.cod = gen_code(OP_ASSIGN,temp,NULL,$$.place,TYPE_FLOAT);
					}
                | variable_id
                    {
                        if (!IS_DECLARED($1))
							{
								static_error(ERROR,"%s: undeclarated identificator",$1 -> name);
								//YYERROR;
							}
						if (IS_PROC($1))
							{
								static_error(ERROR,"using function name (`%s') as a variable's",$1 -> name);
								//YYERROR;
							}
                        $$.type = TYPE_OF($1);
                        $$.place = install_temp(TYPE_TEMP | $$.type);
                        $$.cod = gen_code(OP_ASSIGN,$1,NULL,$$.place,TYPE_OF($1));
                    }
                | variable_id TOK_LBRACKET
					{
						if (!IS_DECLARED($1))
							{
								static_error(ERROR,"%s: undeclarated identificator",$1 -> name);
								//YYERROR;
							}
						if (!IS_PROC($1))
						{
                        	static_error(ERROR,"`%s': not a function",$1 -> name);
                        	//YYERROR;
                        }
                        if (call_function)
                        {
							fci_t frame = { call_function, param_index, param_count };
                        			
                        	fci_stack = push(fci_stack,&frame,sizeof(frame));                        
                        }
                        call_function = $1;
						param_count = count_args(call_function);
						param_index = 0;
					}
				actual_arg_set TOK_RBRACKET
					{
						if (IS_PROC($1))
							if (param_index != param_count)
							{
								static_error(ERROR,"count of parameters of function `%s' isnot %d",$1 -> name,param_index);
								//YYERROR;
							}
						$$.type = TYPE_OF($1); 
						$$.place = install_temp(TYPE_TEMP | $$.type);
						$$.cod = merge_code($4.cod,gen_code(OP_CALL,call_function,NULL,$$.place,TYPE_OF($1)));
						call_function = NULL;
						param_count = 0;
						param_index = 0;
						if (top(fci_stack))
						{
							fci_t *frame;
							
							frame = (fci_t *)top(fci_stack);
							call_function = frame -> call_function;
							param_index = frame -> param_index;
							param_count = frame -> param_count;									
						}
					}
				| variable_id TOK_LBRACKET error TOK_RBRACKET { syn_errflag = 1; yyerrok; }
                | simple_expr TOK_PLUS simple_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                        $$.place = install_temp(TYPE_TEMP | $$.type);
                        
						$$.cod = merge_code($1.cod,$3.cod);
                        $$.cod = merge_code($$.cod,gen_code(OP_PLUS,$1.place,$3.place,$$.place,$$.type));
                    }
                | simple_expr TOK_MINUS simple_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                        $$.place = install_temp(TYPE_TEMP | $$.type);
                        $$.cod = merge_code($1.cod,$3.cod);
                        $$.cod = merge_code($$.cod,gen_code(OP_MINUS,$1.place,$3.place,$$.place,$$.type));
                    }
                | simple_expr TOK_MULT simple_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                        $$.place = install_temp(TYPE_TEMP | $$.type);
                        $$.cod = merge_code($1.cod,$3.cod);
                        $$.cod = merge_code($$.cod,gen_code(OP_MULT,$1.place,$3.place,$$.place,$$.type));
                    }
                | simple_expr TOK_DIV simple_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                        $$.place = install_temp(TYPE_TEMP | $$.type);
                        $$.cod = merge_code($1.cod,$3.cod);
                        $$.cod = merge_code($$.cod,gen_code(OP_DIV,$1.place,$3.place,$$.place,$$.type));
                    }
                | simple_expr TOK_MOD simple_expr
                    {
						if ($1.type == $3.type && $1.type == TYPE_INT)
							$$.type = TYPE_INT;
						else
							{
								static_error(ERROR,"mod operator takes only int params");
								//YYERROR;
							}
						$$.place = install_temp(TYPE_TEMP | $$.type);
                        $$.cod = merge_code($1.cod,$3.cod);
                        $$.cod = merge_code($$.cod,gen_code(OP_MOD,$1.place,$3.place,$$.place,TYPE_INT));
                    }
                | TOK_LBRACKET simple_expr TOK_RBRACKET
					{
						$$ = $2;
					}
				| TOK_LBRACKET error TOK_RBRACKET { syn_errflag = 1; yyerrok; }
                | TOK_MINUS simple_expr %prec TOK_UNARY_MINUS
					{
                        $$.type = $2.type;
                        $$.place = install_temp(TYPE_TEMP | $$.type);
                        $$.cod = merge_code($2.cod,gen_code(OP_MINUS,$2.place,NULL,$$.place,$$.type));
                    }
                | TOK_PLUS simple_expr %prec TOK_UNARY_PLUS
					{
						$$.type = $2.type;
						$$.place = install_temp(TYPE_TEMP | $$.type);
                        $$.cod = merge_code($2.cod,gen_code(OP_PLUS,$2.place,NULL,$$.place,$$.type));
					}
                | simple_expr TOK_POW simple_expr
                    {
                        if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;   /* не€вное приведение типа к float */
                        $$.place = install_temp(TYPE_TEMP | $$.type);
                        $$.cod = merge_code($1.cod,$3.cod);
                        $$.cod = merge_code($$.cod,gen_code(OP_POW,$1.place,$3.place,$$.place,$$.type));
                    }
                ;

logical_expr:   simple_expr
                | simple_expr TOK_EQ simple_expr
					{
						if ($1.type == $3.type)			// <- in fact, all logical_expr are int,
                            $$.type = $1.type;			// but while code generation we need to know
                        else							// is threre were comparsions of floating ops (simple_expr)
                            $$.type = TYPE_FLOAT;		// so we set flag
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						$$.cod = merge_code($1.cod,$3.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_EQ,$1.place,$3.place,$$.place,$$.type));
					}
                | simple_expr TOK_GT simple_expr
					{
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;
						$$.cod = merge_code($1.cod,$3.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_GT,$1.place,$3.place,$$.place,$$.type));
					}
                | simple_expr TOK_LW simple_expr
					{
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;
						$$.cod = merge_code($1.cod,$3.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_LW,$1.place,$3.place,$$.place,$$.type));
					}
                | simple_expr TOK_GE simple_expr
					{
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;
						$$.cod = merge_code($1.cod,$3.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_GE,$1.place,$3.place,$$.place,$$.type));
					}
                | simple_expr TOK_LE simple_expr
					{
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;
						$$.cod = merge_code($1.cod,$3.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_LE,$1.place,$3.place,$$.place,$$.type));
					}
                | simple_expr TOK_NEQ simple_expr
					{
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						if ($1.type == $3.type)
                            $$.type = $1.type;
                        else
                            $$.type = TYPE_FLOAT;
						$$.cod = merge_code($1.cod,$3.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_NEQ,$1.place,$3.place,$$.place,$$.type));
					}
                | TOK_LBRACKET logical_expr TOK_RBRACKET
					{
						$$ = $2;
					}
				| TOK_NOT logical_expr
					{
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						$$.type = TYPE_INT;
						$$.cod = merge_code($2.cod,gen_code(OP_NOT,$2.place,NULL,$$.place,$$.type));
					}
                | logical_expr TOK_OR logical_expr
					{
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						$$.type = TYPE_INT;
						$$.cod = merge_code($1.cod,$3.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_OR,$1.place,$3.place,$$.place,$$.type));
					}
                | logical_expr TOK_AND logical_expr
					{
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						$$.type = TYPE_INT;
						$$.cod = merge_code($1.cod,$3.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_AND,$1.place,$3.place,$$.place,$$.type));
					}
                | logical_expr TOK_XOR logical_expr
					{
						$$.place = install_temp(TYPE_TEMP | TYPE_INT);
						$$.type = TYPE_INT;
						$$.cod = merge_code($1.cod,$3.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_XOR,$1.place,$3.place,$$.place,$$.type));
					}
                ;
                
expr_list:	simple_expr
				{
					$$.cod = merge_code($1.cod,gen_code(OP_PARAM,$1.place,NULL,NULL,$1.type));
					/* second OP_PARAM defines type of first OP_PARAM */
					$$.cod = merge_code($$.cod,gen_code(OP_PARAM,NULL,NULL,NULL,$1.type));
					$$.type = 0;
					expr_count++;
				}
			| expr_list TOK_COMA simple_expr
				{
					expr_count++;
					$$.cod = merge_code($1.cod,$3.cod);
					$$.cod = merge_code($$.cod,gen_code(OP_PARAM,$3.place,NULL,NULL,$3.type));
					$$.cod = merge_code($$.cod,gen_code(OP_PARAM,NULL,NULL,NULL,$3.type));
					$$.type = 0;
				}
			;                

io_op:  TOK_READ_WORD
			{
				param_count = 0;
			}
		variable_list TOK_SEMICOLON
			{
				$$.place = NULL;
				$$.cod = merge_code($3.cod,gen_code(OP_READ,NULL,NULL,NULL,param_count));	// using `type' field for storing param count of io op
				$$.type = 0;
				param_count = 0;
			}
        | TOK_PRINT_WORD
			{
				expr_count = 0;
			}
		expr_list TOK_SEMICOLON
			{
				$$.place = NULL;
				$$.cod = merge_code($3.cod,gen_code(OP_PRINT,NULL,NULL,NULL,expr_count));
				$$.type = 0;
				expr_count = 0;
			}
        | TOK_PRINT_WORD TOK_STRING TOK_SEMICOLON
			{
				$$.place = NULL;
				$$.cod = merge_code(gen_code(OP_PARAM,$2,NULL,NULL,TYPE_STRING),
									gen_code(OP_PRINT,NULL,NULL,NULL,-1));	// signal with -1 about string
				$$.type = 0;
			}
		| TOK_PRINTLN_WORD TOK_SEMICOLON
			{
				$$.place = NULL;
				$$.cod = gen_code(OP_PRINTLN,NULL,NULL,NULL,NULL);
				$$.type = 0;
			}
        ;

composite_statement:    TOK_LFBRACKET statement_list TOK_RFBRACKET
							{
								$$.place = NULL;
								$$.cod = $2.cod;
								$$.type = 0;
							}
						| TOK_LFBRACKET error TOK_RBRACKET { syn_errflag = 1; yyerrok; }
                        ;

complex_statement:      if_statement			/* default action */
                        | while_statement
                        | for_statement
                        | do_statement
                        ;

if_statement:   TOK_IF_WORD TOK_LBRACKET logical_expr TOK_RBRACKET statement
					{
						$$.begin = install_label();
						$$.after = install_label();
												
						$$.cod = merge_code($3.cod,gen_code(OP_IF,$3.place,NULL,$$.begin,0));
						$$.cod = merge_code($$.cod,$5.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_GOTO,NULL,NULL,$$.after,0));
						$$.cod = merge_code($$.cod,gen_code(OP_LABEL,$$.begin,NULL,NULL,0));
					}
				end_if
					{
						$$.place = NULL;
						$$.cod = merge_code($<instr>6.cod,$7.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_LABEL,$<instr>6.after,NULL,NULL,0));
						$$.type = 0;
					}
				;

end_if:         /* empty */	{ $$.cod = NULL; $$.place = NULL; $$.type = 0; }
                | TOK_ELSE_WORD statement
					{
						$$.place = NULL;
						$$.cod = $2.cod;
						$$.type = 0;
					}
                ;

while_statement:        TOK_WHILE_WORD TOK_LBRACKET logical_expr TOK_RBRACKET statement
							{
								$$.place = NULL;
								$$.begin = install_label();
								$$.after = install_label();
								
								$$.cod = merge_code(gen_code(OP_LABEL,$$.begin,NULL,NULL,0),$3.cod);
								$$.cod = merge_code($$.cod,gen_code(OP_IF,$3.place,NULL,$$.after,0));
								$$.cod = merge_code($$.cod,$5.cod);
								$$.cod = merge_code($$.cod,gen_code(OP_GOTO,NULL,NULL,$$.begin,0));
								$$.cod = merge_code($$.cod,gen_code(OP_LABEL,$$.after,NULL,NULL,0));
								$$.type = 0;
							}
                        ;
do_statement:   TOK_DO_WORD statement TOK_WHILE_WORD TOK_LBRACKET logical_expr TOK_RBRACKET TOK_SEMICOLON
					{
						$$.place = NULL;
						$$.begin = install_label();
						$$.after = install_label();
						
						$$.cod = merge_code(gen_code(OP_LABEL,$$.begin,NULL,NULL,0),$2.cod);
						$$.cod = merge_code($$.cod,$5.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_IF,$5.place,NULL,$$.after,0));
						$$.cod = merge_code($$.cod,gen_code(OP_GOTO,NULL,NULL,$$.begin,0));
						$$.cod = merge_code($$.cod,gen_code(OP_LABEL,$$.after,NULL,NULL,0));
						$$.type = 0;
					}
                ;

for_statement:  TOK_FOR_WORD TOK_LBRACKET for_expr TOK_SEMICOLON
                logical_expr TOK_SEMICOLON for_end_expr TOK_RBRACKET statement
					{
						$$.place = NULL;
						$$.begin = install_label();
						$$.after = install_label();
						
						$$.cod = merge_code($3.cod,gen_code(OP_LABEL,$$.begin,NULL,NULL,0));
						$$.cod = merge_code($$.cod,$5.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_IF,$5.place,NULL,$$.after,0));
						$$.cod = merge_code($$.cod,$9.cod);
						$$.cod = merge_code($$.cod,$7.cod);
						$$.cod = merge_code($$.cod,gen_code(OP_GOTO,NULL,NULL,$$.begin,0));
						$$.cod = merge_code($$.cod,gen_code(OP_LABEL,$$.after,NULL,NULL,0));
						$$.type = 0;
					}
                ;

for_expr:       /*empty*/ { $$.cod = NULL; $$.place = NULL; $$.type = 0; }
                | for_expr_list
					{
						$$.place = NULL;
						$$.cod = $1.cod;
						$$.type = 0;
					}
                ;

for_end_expr:		/* empty */ { $$.cod = NULL; $$.place = NULL; $$.type = 0; }
					| for_end_expr_list
						{
							$$.cod = $1.cod;
							$$.place = NULL;
							$$.type = 0;
						}
					;
                
for_end_expr_list:	for_end_expr_list_member
						{
							$$.cod = $1.cod;
							$$.place = NULL;
							$$.type = 0;
						}
					| for_end_expr_list TOK_COMA for_end_expr_list_member
						{
							$$.place = NULL;
							$$.cod = merge_code($1.cod,$3.cod);
							$$.type = 0;
						}
					;
                

for_expr_list:  assign_expr
					{
						$$.place = NULL;
						$$.cod = $1.cod;
						$$.type = 0;
					}
                | for_expr_list TOK_COMA assign_expr
					{
						$$.place = NULL;
						$$.cod = merge_code($1.cod,$3.cod);
						$$.type = 0;
					}
                ;
                
assign_expr:    variable_id TOK_ASSIGN simple_expr
                    {
                        if (!IS_DECLARED($1))
							{
								static_error(ERROR,"%s: undeclarated identificator",$1 -> name);
								//YYERROR;
							}
						if (IS_PROC($1))
							{
								static_error(ERROR,"using function name (`%s') as a variable's",$1 -> name);
								//YYERROR;
							}
                        if (TYPE_OF($1) != $3.type)
                            if (TYPE_OF($1) == TYPE_INT)
								static_error(WARN,"truncating from float to int in assignment to %s",$1 -> name);
						$$.place = NULL;								
						$$.cod = merge_code($3.cod,gen_code(OP_ASSIGN,$3.place,NULL,$1,TYPE_OF($1)));
						$$.type = 0;
                    }
                ;
                
for_end_expr_list_member:	assign_expr
				| variable_id TOK_PLUSPLUS
                    {
                        if (!IS_DECLARED($1))
							{
								static_error(ERROR,"%s: undeclarated identificator",$1 -> name);
								//YYERROR;
							}
						if (IS_PROC($1))
							{
								static_error(ERROR,"using function name (`%s') as a variable's",$1 -> name);
								//YYERROR;
							}
                        if (TYPE_OF($1) == TYPE_FLOAT)
                        {
                            static_error(ERROR,"%s: ++ operator expects integer values",$1 -> name);
                            //YYERROR;
                        }
                        $$.place = NULL;
                        $$.cod = gen_code(OP_PLUSPLUS,$1,NULL,NULL,TYPE_INT);
                        $$.type = 0;
                    }
				| variable_id TOK_MINUSMINUS
                    {
                        if (!IS_DECLARED($1))
							{
								static_error(ERROR,"%s: undeclarated identificator",$1 -> name);
								//YYERROR;
							}
						if (IS_PROC($1))
							{
								static_error(ERROR,"using function name (`%s') as a variable's",$1 -> name);
								//YYERROR;
							}
                        if (TYPE_OF($1) == TYPE_FLOAT)
                        {
                            static_error(ERROR,"%s: -- operator expects integer values",$1 -> name);
                            //YYERROR;
                        }
                        $$.place = NULL;
                        $$.cod = gen_code(OP_MINUSMINUS,$1,NULL,NULL,TYPE_INT);
                        $$.type = 0;
                    }
                ;                

%%

//! \endcond
