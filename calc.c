#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE 80	/* Maximum input line length */
#define MAX_VAR_NAME 16	/* Maximum variable name length */

#define TOK_NULL	0x00
#define TOK_NUM		0x01
#define TOK_VAR		0x02
#define TOK_OP		0x03
#define TOK_PAREN	0x04
#define TOK_ENDPAREN	0x05

#define TOK_EOL		0x7f

#define IS_SPACE(a) (a == ' ')
#define IS_DIGIT(a) (a >= '0' && a <= '9')
#define IS_ALPHA(a) (a >= 'a' && a <= 'z')
#define IS_OP(a) (a == '+' || a == '/' || a == '*' || \
		a == '-' || a == '^')


struct var {
	char name[MAX_VAR_NAME];
	int value;
	struct var *next;
};

struct var *var_head;


void do_help(void)
{
	printf("Type math stuff and get answers!\n");
	printf("Supports: +, -, /, *, ^, (), integers only\n");
	printf("For example, type in:\n");
	printf("123 + 321\n");
	printf("You'll get the answer:\n");
	printf("444\n\n");
	return;
}


/* Locate a variable in the variable list */
struct var *find_var(char *name)
{
	struct var *p = var_head;

	while (p != NULL) {
		if (!strncmp(p->name, name, MAX_VAR_NAME))
			return p;
		p = p->next;
	}
	return NULL;
}


/* Retrieve a variable's value */
int get_var(char *name, int *var)
{
	return -1;
}


/* Set (or add) a variable */
int set_var(char *name, int value)
{
	return -1;
}

int readtok(char *line, int lpos, char *tok, int *type)
{
	int len;
	int tpos = 0;

	*type = TOK_NULL;
	tok[0] = 0;

	len = strlen(line);

	/* Handle obvious no-go cases */
	if (len == 0 || lpos > len) return TOK_EOL;
	if (line[lpos] == 0) return TOK_EOL;

	/* Skip spaces */
	while (IS_SPACE(line[lpos])) {
		if (lpos == MAX_LINE) return TOK_EOL;
		lpos++;
	}

	/* Good old straight up numbers */
	if (*type == TOK_NULL) {
		while (IS_DIGIT(line[lpos])) {
			*type = TOK_NUM;
			if (lpos == MAX_LINE) break;
			tok[tpos++] = line[lpos++];
		}
	}

	/* Variables */
	if (*type == TOK_NULL) {
		while (IS_ALPHA(line[lpos])) {
			*type = TOK_VAR;
			if (lpos == MAX_LINE) break;
			tok[tpos++] = line[lpos++];
		}
	}

	/* Operators */
	if (*type == TOK_NULL) {
		if (IS_OP(line[lpos])) {
			*type = TOK_OP;
			tok[tpos++] = line[lpos++];
		}
	}

	/* Parentheses */
	if (*type == TOK_NULL) {
		if (line[lpos] == '(') {
			*type = TOK_PAREN;
			tok[tpos++] = line[lpos++];
		} else if (line[lpos] == ')') {
			*type = TOK_ENDPAREN;
			tok[tpos++] = line[lpos++];
		}
	}

	if (lpos == MAX_LINE || *type == TOK_NULL) lpos = -1;
	tok[tpos] = 0;

	return lpos;
}


/* Perform an operation */
int do_operation(int lvalue, int rvalue, int op)
{
	switch (op) {
		case '/': 
			  if (rvalue == 0) {
				  printf("error: divide by zero\n");
				  return 0;
			  }
			  return (lvalue / rvalue);
		case '+': return (lvalue + rvalue);
		case '-': return (lvalue - rvalue);
		case '*': return (lvalue * rvalue);
		case '^': return pow(lvalue, rvalue);
		default:
			  printf("error: bad operation\n");
			  return 0;
	}
	return 0;
}


/* Evaluate an expression */
int expression(char *line, int len)
{
	int lpos = 0, neg = 0, lset = 0;
	int tok_type;
	int result = 0;
	char tok[MAX_LINE], subexpr[MAX_LINE];
	int lvalue = 0, rvalue = 0, op = 0;

	if (len == 0) return 0;

	while (lpos < MAX_LINE && line[lpos] != 0) {
//		printf("Loop; lpos %d, len %d\n", lpos, len);
		if (lpos == len) return 0;
		lpos = readtok(line, lpos, tok, &tok_type);
//		printf("Read token: %s (type %d)\n", tok, tok_type);
		switch (tok_type) {
			case TOK_VAR:
//				num = resolve_var(tok);
				return 0;
			case TOK_NUM:
/*				printf("TOK_NUM: lset %d, lvalue %d, rvalue %d, op %c, tok %s\n",
						lset, lvalue, rvalue, op, tok); */
				/* Handle numbers */
				if (strlen(tok) > 11) printf("error: integer overflow\n");
				if (!lset) {
					lvalue = atoi(tok);
					if (neg > 0) lvalue = -lvalue;
					neg = 0;
					result = lvalue;
					lset = 1;
/*					printf("!lset: lvalue %d, result %d, lset %d\n",
							lvalue, result, lset); */
					continue;
				} else if (op) {
					/* Do the operation */
					rvalue = atoi(tok);
					if (neg > 0) rvalue = -rvalue;
					neg = 0;
					result = do_operation(lvalue, rvalue, op);
					/* Keep going if more stuff exists */
					if (line[lpos] != '\0') {
						lvalue = result;
						op = 0;
						continue;
					} else return result;
				} else {
					/* Operation required */
					printf("error: operation required\n");
					return 0;
				}
				break;
			case TOK_OP:
				/* Get operation type */
				if (!lset || op) {
					/* Handle + or - in front of numbers */
					if (tok[0] == '+') {
						if (neg != 0) printf("warning: too many sign specifiers\n");
						neg = -1;
						continue;
					}
					if (tok[0] == '-') {
						if (neg != 0) printf("warning: too many sign specifiers\n");
						neg = 1;
						continue;
					}
					if (!lset) printf("error: no lvalue specified\n");
					if (op) printf("error: two operations specified\n");
					return 0;
				}
				op = tok[0];
				break;
			case TOK_PAREN:
				/* Read the subexpression and execute it */
				subexpr[0] = '\0';
				while (1) {
					lpos = readtok(line, lpos, tok, &tok_type);
					if (tok_type == TOK_EOL) {
						printf("Error: end-of-line; expected ')'\n");
						return 0;
					}
					if (tok_type == TOK_NULL) {
						printf("Error: unknown character; expected ')'\n");
						return 0;
					}
					if (strncmp(tok, ")", 2)) {
						/* Empty parentheses = 0 */
						strncat(subexpr, tok, MAX_LINE);
					} else break;
				}
				result = expression(subexpr, strlen(subexpr));
				if (!lset) {
					lvalue = result;
					result = 0;
					if (neg > 0) lvalue = -lvalue;
					neg = 0;
					lset = 1;
					continue;
				} else if (op) {
					rvalue = result;
					if (neg > 0) rvalue = -rvalue;
					neg = 0;
					result = do_operation(lvalue, rvalue, op);
					if (line[lpos] != '\0') {
						lvalue = result;
						op = 0;
						continue;
					} else return result;
				} else {
					/* Operation required */
					printf("error: operation required\n");
					return 0;
				}
				break;
			case TOK_ENDPAREN:
				printf("error: ')' without matching '('\n");
				return 0;
			case TOK_NULL:
				printf("error: unknown character\n");
				return 0;
			case TOK_EOL:
				if (neg != 0) printf("error: no values given\n");
			default:
				return result;
		}
		if (lpos < 0) return result;
	}
//	printf("meh, fell through\n");
	return result;
}


int main(int argc, char **argv)
{
	int len = 0;
	char line[MAX_LINE];

	line[MAX_LINE] = '\0';

	printf("Jody's little calculator\n\n");

	while (1) {
		printf("> ");
		if (!fgets(line, MAX_LINE, stdin)) {
			printf("Error reading stdin\n");
			exit(EXIT_FAILURE);
		}
		len = strlen(line);
		/* Process line */
		if (line[len-1] == '\n') {
			len--;
			line[len] = '\0';
		}
		if (len == 0) continue;
		if (!strncmp(line, "quit", MAX_LINE)) break;
		else if (!strncmp(line, "help", MAX_LINE)) do_help();
		else printf("%d\n", expression(line, len));
	}
	exit(EXIT_SUCCESS);
}

