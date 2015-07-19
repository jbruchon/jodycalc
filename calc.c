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
#define TOK_EQUAL	0x06
#define TOK_COMPARE	0x07

#define TOK_FORCE_EXPR	0x7d
#define TOK_INVALID	0x7e
#define TOK_EOL		0x7f

#define IS_SPACE(a) (a == ' ' || a == '\t')
#define IS_DIGIT(a) (a >= '0' && a <= '9')
#define IS_ALPHA(a) (a >= 'a' && a <= 'z')
#define IS_EQUAL(a) (a == '=')
#define IS_OP(a) (a == '+' || a == '/' || a == '*' || \
		a == '-' || a == '^' || a == '%')
#define MAYBE_COMP(a) (a == '=' || a == '>' || a == '<' || a == '!')

struct var {
	char name[MAX_VAR_NAME];
	int value;
	struct var *next;
};

struct var *var_head;


void do_help(void)
{
	fprintf(stderr, "\nType math stuff and get answers!\n");
	fprintf(stderr, "Supports: +, -, /, *, ^, %%, (), integers only\n");
	fprintf(stderr, "For example, type in:\n");
	fprintf(stderr, "123 + 321\n");
	fprintf(stderr, "You'll get the answer:\n");
	fprintf(stderr, "444\n\n");
	fprintf(stderr, "Type 'quit' to exit the program.\n\n");
	return;
}


/* Locate a variable in the variable list */
struct var *find_var(char *name, struct var **tail)
{
	struct var *p = var_head;

	while (p != NULL) {
		if (!strncmp(p->name, name, MAX_VAR_NAME))
			return p;
		if (tail != NULL) *tail = p;
		p = p->next;
	}
	return NULL;
}


/* Retrieve a variable's value (returns 1 if successful) */
int get_var(char *name, int *var)
{
	struct var *p;

	p = find_var(name, NULL);
	if (p) {
		*var = p->value;
		return 1;
	} else return 0;
}


/* Set (or add) a variable */
void set_var(char *name, int value)
{
	struct var *p, *tail;

	if (var_head == NULL) {
		/* Allocate first variable if needed */
		var_head = (struct var *)malloc(sizeof(struct var));
		if (!var_head) goto oom;
		p = var_head;
	} else {
		p = find_var(name, &tail);
		if (p) {
			/* Variable already exists */
			p->value = value;
			return;
		} else {
			tail->next = (struct var *)malloc(sizeof(struct var));
			if (!tail->next) goto oom;
			p = tail->next;
		}
	}
	strncpy(p->name, name, MAX_VAR_NAME);
	p->value = value;
	p->next = NULL;
	return;
oom:
	fprintf(stderr, "Out of memory\n");
	exit(EXIT_FAILURE);
}

int readtok(char *line, int lpos, char *tok, int *type)
{
	int len;
	int tpos = 0;

	if (type != NULL) *type = TOK_INVALID;
	tok[0] = 0;

	len = strlen(line);

	/* Handle obvious no-go cases */
	if (len == 0 || lpos > len) return -1;
	if (line[lpos] == '\0') return -1;

	/* Skip spaces */
	while (IS_SPACE(line[lpos])) {
		if (lpos == MAX_LINE || line[lpos] == '\0') {
			*type = TOK_EOL;
			return -1;
		}
		lpos++;
	}

	/* Look-ahead functionality */
	if (type == NULL) {
		if (lpos < len) return TOK_NULL;
		else return TOK_EOL;
	}

	/* Comparison operators */
	if (*type == TOK_INVALID) {
		if (MAYBE_COMP(line[lpos])) {
			switch (line[lpos]) {
				case '!':
					if (line[lpos+1] != '=') break;
				case '=':
					if (line[lpos+1] == '=' ||
					line[lpos+1] == '>' ||
					line[lpos+1] == '<') {
						*type = TOK_COMPARE;
						tok[0] = line[lpos];
						lpos++;
						tok[1] = line[lpos];
						lpos++;
						tok[2] = '\0';
						return lpos;
					}
					break;
				case '>':
				case '<':
					*type = TOK_COMPARE;
					tok[0] = line[lpos];
					lpos++;
					if (line[lpos+1] == '=') {
						tok[1] = line[lpos];
						lpos++;
						tok[2] = '\0';
					} else tok[1] = '\0';
					return lpos;
			}
		}
	}

	/* Equals sign - only valid for variables */
	if (*type == TOK_INVALID) {
		if (IS_EQUAL(line[lpos])) {
			*type = TOK_EQUAL;
			tok[tpos++] = line[lpos++];
		}
	}

	/* Good old straight up numbers */
	if (*type == TOK_INVALID) {
		while (IS_DIGIT(line[lpos])) {
			*type = TOK_NUM;
			if (lpos == MAX_LINE) break;
			tok[tpos++] = line[lpos++];
		}
	}

	/* Variables */
	if (*type == TOK_INVALID) {
		while (IS_ALPHA(line[lpos])) {
			*type = TOK_VAR;
			if (lpos == MAX_LINE) break;
			tok[tpos++] = line[lpos++];
		}
	}

	/* Operators */
	if (*type == TOK_INVALID) {
		if (IS_OP(line[lpos])) {
			*type = TOK_OP;
			tok[tpos++] = line[lpos++];
		}
	}

	/* Parentheses */
	if (*type == TOK_INVALID) {
		if (line[lpos] == '(') {
			*type = TOK_PAREN;
			tok[tpos++] = line[lpos++];
		} else if (line[lpos] == ')') {
			*type = TOK_ENDPAREN;
			tok[tpos++] = line[lpos++];
		}
	}

	if (lpos == MAX_LINE || *type == TOK_INVALID || *type == TOK_EOL)
		lpos = -1;
	tok[tpos] = 0;

	return lpos;
}


/* Perform an operation */
int do_operation(int lvalue, int rvalue, int op)
{
	switch (op) {
		case '/':
			  if (rvalue == 0) {
				  fprintf(stderr, "error: divide by zero\n");
				  return 0;
			  }
			  return (lvalue / rvalue);
		case '+': return (lvalue + rvalue);
		case '-': return (lvalue - rvalue);
		case '*': return (lvalue * rvalue);
		case '%': return (lvalue % rvalue);
		case '^': return pow(lvalue, rvalue);
		default:
			  fprintf(stderr, "error: bad operation\n");
			  return 0;
	}
	return 0;
}


/* Evaluate an expression */
int expression(char *line, int len)
{
	int lpos = 0;
	int neg = 0, lset = 0, lvar = 0;
	int tok_type;
	int result = 0;
	char tok[MAX_LINE], subexpr[MAX_LINE];
	char lvname[MAX_VAR_NAME];
	char *partial;
	int lvalue = 0, rvalue = 0, op = 0;

	if (len == 0) return 0;

	while (lpos < MAX_LINE && line[lpos] != 0) {
//		fprintf(stderr, "Loop; lpos %d, len %d\n", lpos, len);
		if (lpos == len) return result;
		lpos = readtok(line, lpos, tok, &tok_type);
		if (tok[0] == '\0') {
			if (lvar == -1) fprintf(stderr, "error: no such variable: %s\n", lvname);
			break;
		}
//		fprintf(stderr, "Read token: '%s' (type %d)\n", tok, tok_type);

		switch (tok_type) {
			case TOK_COMPARE:
				if (!lset) {
					fprintf(stderr, "error: left side of comparison empty\n");
					return 0;
				}
				partial = line + lpos;
				result = expression(partial, len - lpos);
				if (!strncmp(tok, "==", 3)) {
					if (lvalue == result) result = 1;
					else result = 0;
				} else if (!strncmp(tok, "!=", 3)) {
					if (lvalue != result) result = 1;
					else result = 0;
				} else if (!strncmp(tok, ">=", 3) || !strncmp(tok, "=>", 3)) {
					if (lvalue >= result) result = 1;
					else result = 0;
				} else if (!strncmp(tok, "<=", 3) || !strncmp(tok, "=<", 3)) {
					if (lvalue <= result) result = 1;
					else result = 0;
				} else if (!strncmp(tok, ">", 2)) {
					if (lvalue > result) result = 1;
					else result = 0;
				} else if (!strncmp(tok, "<", 2)) {
					if (lvalue < result) result = 1;
					else result = 0;
				} else {
					fprintf(stderr, "error: bad comparison operator '%s'\n", tok);
					return 0;
				}
				return result;
			case TOK_EQUAL:
				/* Expect to set a variable and nothing more */
				if (lvar == 0) {
					fprintf(stderr, "error: lvalue not a variable\n");
					return 0;
				}
				/* The right side of the '=' is always an expression */
				partial = line + lpos;
				result = expression(partial, len - lpos);
				set_var(lvname, result);
				return result;

			case TOK_OP:
				/* Get operation type */
				if (lvar == -1) {
					fprintf(stderr, "error: no such variable: %s\n", lvname);
					return 0;
				}
				if (!lset || op) {
					/* Handle + or - in front of numbers */
					if (tok[0] == '+') {
						if (neg != 0) fprintf(stderr, "warning: too many sign specifiers\n");
						neg = -1;
						continue;
					}
					if (tok[0] == '-') {
						if (neg != 0) fprintf(stderr, "warning: too many sign specifiers\n");
						neg = 1;
						continue;
					}
					if (!lset) fprintf(stderr, "error: no lvalue specified\n");
					else if (op) fprintf(stderr, "error: two operations specified\n");
					else fprintf(stderr, "error: lset+op: this should not happen...\n");
					return 0;
				}
				op = tok[0];
				break;

			case TOK_PAREN:
				/* Read the subexpression and execute it */
				/* Note: NUM, PAREN, VAR share final evaluation code */
				subexpr[0] = '\0';
				while (1) {
					lpos = readtok(line, lpos, tok, &tok_type);
					if (tok_type == TOK_EOL) {
						fprintf(stderr, "Error: end-of-line; expected ')'\n");
						return 0;
					}
					if (tok_type == TOK_INVALID) {
						fprintf(stderr, "Error: unknown character; expected ')'\n");
						return 0;
					}
					if (strncmp(tok, ")", 2)) {
						/* Empty parentheses = 0 */
						strncat(subexpr, tok, MAX_LINE);
					} else break;
				}
				result = expression(subexpr, strlen(subexpr));

			case TOK_NUM:
				if (tok_type == TOK_NUM) {
					if (strlen(tok) > 10) fprintf(stderr, "warning: overflow\n");
					result = atoi(tok);
				}

			case TOK_VAR:
				if (tok_type == TOK_VAR) {
					/* Variable may be set later */
					if (!lvar && !op && !lset) {
						strncpy(lvname, tok, MAX_VAR_NAME);
						/* In case it won't be set, pull an lvalue */
						if (get_var(tok, &result)) lvar = 1;
						else lvar = -1;
					}
					/* Variable can't possibly be set after an operation */
					if (op && !get_var(tok, &result)) {
						fprintf(stderr, "error: no such variable: %s\n", tok);
						return 0;
					}
				}

				/* Final evaluation code */
				if (!lset) {
					lvalue = result;
					if (neg > 0) lvalue = -lvalue;
					neg = 0;
					lset = 1;
					continue;
				} else if (op) {
					rvalue = result;
					if (neg > 0) rvalue = -rvalue;
					neg = 0;
					result = do_operation(lvalue, rvalue, op);
					op = 0;
					lvalue = result;
					continue;
				} else {
					/* Operation required */
					fprintf(stderr, "error: operation required\n");
					return 0;
				}
				break;

			case TOK_ENDPAREN:
				fprintf(stderr, "error: ')' without matching '('\n");
				return 0;

			case TOK_INVALID:
				fprintf(stderr, "error: unknown character\n");
				return 0;

			case TOK_NULL:
				continue;

			case TOK_EOL:
				if (neg != 0) fprintf(stderr, "error: no values given\n");
				if (lvar == -1) fprintf(stderr, "no such variable: %s\n", lvname);

			default:
				return result;
		}
		if (lpos < 0) return result;
	}
	return result;
}


int main(int argc, char **argv)
{
	int len = 0;
	char line[MAX_LINE];

	line[MAX_LINE] = '\0';

	fprintf(stderr, "Jody's little calculator\n\n");

	while (1) {
		if (!fgets(line, MAX_LINE, stdin)) {
			fprintf(stderr, "Error reading stdin\n");
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

