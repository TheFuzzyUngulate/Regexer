#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>

#include "types/stack/stack.h"
#include "types/list/lists.h"

struct re_exp;
struct re_scan_t;
struct re_comp;
struct re_state;
struct re_parse_t;

#define SPACING_COUNT 3

void re_exp_print(struct re_exp*, int);
void re_comp_print(struct re_comp*, int);

int issubstr(char* line, char* sub) {
	size_t slen = strlen(sub);
	size_t llen = strlen(line);
	
	if (llen >= slen) {
		char buf[slen+1];	
		size_t max  = llen - slen;

		for (int i = 0; i < max; ++i) {
			memcpy(buf, line + i, slen);
			buf[slen] = '\0';
			if (!strcmp(buf, sub)) return i;
		}
	}

	return -1;
}

/* https://stackoverflow.com/questions/735126/are-there-alternate-implementations-of-gnu-getline-interface/47229318#47229318 */
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    size_t pos;
    int c;

    if (lineptr == NULL || stream == NULL || n == NULL) {
        errno = EINVAL;
        return -1;
    }

    c = getc(stream);
    if (c == EOF) {
        return -1;
    }

    if (*lineptr == NULL) {
        *lineptr = malloc(128);
        if (*lineptr == NULL) {
            return -1;
        }
        *n = 128;
    }

    pos = 0;
    while(c != EOF) {
        if (pos + 1 >= *n) {
            size_t new_size = *n + (*n >> 2);
            if (new_size < 128) {
                new_size = 128;
            }
            char *new_ptr = realloc(*lineptr, new_size);
            if (new_ptr == NULL) {
                return -1;
            }
            *n = new_size;
            *lineptr = new_ptr;
        }

        ((unsigned char *)(*lineptr))[pos ++] = c;
        if (c == '\n') {
            break;
        }
        c = getc(stream);
    }

    (*lineptr)[pos] = '\0';
    return pos;
}

#define getspacing(compt) do {\
	for (int i = 0; i < (compt)*SPACING_COUNT; ++i)\
		putchar(' ');\
} while (0);

typedef enum re_tk {
	P_START_LIT,
	P_TOK_END,
	P_LIT_re,
	P_LIT_exp,
	P_LIT_re_BAR,
	P_TOK_BAR,
	P_LIT_msub,
	P_LIT_exp_BAR,
	P_LIT_sub,
	P_LIT_msub_BAR,
	P_TOK_QUESTION,
	P_TOK_PLUS,
	P_TOK_TIMES,
	P_LIT_elm,
	P_TOK_LBRACK,
	P_LIT_slc,
	P_TOK_RBRACK,
	P_TOK_LPAREN,
	P_TOK_RPAREN,
	P_LIT_fch,
	P_TOK_CAP,
	P_TOK_MINUS,
	P_LIT_esc,
	P_TOK_DOT,
	P_TOK_SLASH,
	P_TOK_NEWLINE_CHAR,
	P_TOK_CRETURN_CHAR,
	P_TOK_TABULATE_CHAR,
	P_LIT_sli,
	P_LIT_slc_BAR,
	P_TOK_CHAR
} re_tk;

static inline char*
re_tk_string(re_tk tok) {
	switch (tok) {
		case P_START_LIT: return "S*";
		case P_TOK_END: return "$";
		case P_LIT_re: return "re";
		case P_LIT_exp: return "exp";
		case P_LIT_re_BAR: return "re'";
		case P_TOK_BAR: return "#BAR";
		case P_LIT_msub: return "msub";
		case P_LIT_exp_BAR: return "exp'";
		case P_LIT_sub: return "sub";
		case P_LIT_msub_BAR: return "msub'";
		case P_TOK_QUESTION: return "#QUESTION";
		case P_TOK_PLUS: return "#PLUS";
		case P_TOK_TIMES: return "#TIMES";
		case P_LIT_elm: return "elm";
		case P_TOK_LBRACK: return "#LBRACK";
		case P_LIT_slc: return "slc";
		case P_TOK_RBRACK: return "#RBRACK";
		case P_TOK_LPAREN: return "#LPAREN";
		case P_TOK_RPAREN: return "#RPAREN";
		case P_LIT_fch: return "fch";
		case P_TOK_CAP: return "#CAP";
		case P_TOK_MINUS: return "#MINUS";
		case P_LIT_esc: return "esc";
		case P_TOK_DOT: return "#DOT";
		case P_TOK_SLASH: return "#SLASH";
		case P_TOK_NEWLINE_CHAR: return "#NEWLINE_CHAR";
		case P_TOK_CRETURN_CHAR: return "#CRETURN_CHAR";
		case P_TOK_TABULATE_CHAR: return "#TABULATE_CHAR";
		case P_LIT_sli: return "sli";
		case P_LIT_slc_BAR: return "slc'";
		case P_TOK_CHAR: return "#CHAR";
		default: return "?";
	}
}

typedef struct re_exp {
    enum { char_exp, empty_exp,
		   dot_exp, rep_exp, bar_exp, 
		   plain_exp, opt_exp, range_exp, 
		   select_exp, kleene_exp } 		   tag;
    union { char                               charExp;
			char                               emptyExp;
			char						       dotExp;
            struct re_comp*                    repExp;
            struct re_comp*                    plainExp;
            struct { struct re_comp* left;
                     struct re_comp* right; }  barExp;
            struct re_comp*                    optExp;
			struct { char min; char max; }     rangeExp;
            struct { int pos;
			         struct re_comp* select; } selectExp;
            struct re_comp*                    kleeneExp; } op;
} re_exp;

typedef struct re_comp {
    re_exp*         elem;
    struct re_comp* next;
} re_comp;

void re_exp_print(re_exp* re, int ind)
{
	if (re)
	{
		switch(re->tag)
		{
			case kleene_exp:
				getspacing(ind);
				printf("rep-exp:\n");
				re_comp_print(re->op.kleeneExp, ind+1);
				break;

			case select_exp:
				getspacing(ind);
				printf("select-exp: %s\n", re->op.selectExp.pos ? "" : " not");
				re_comp_print(re->op.selectExp.select, ind+1);
				break;

			case range_exp:
				getspacing(ind);
				printf(
					"range: %c-%c\n", 
					re->op.rangeExp.min,
					re->op.rangeExp.max
				);
				break;

			case opt_exp:
				getspacing(ind);
				printf("opt-exp:\n");
				re_comp_print(re->op.optExp, ind+1);
				break;

			case bar_exp:
				getspacing(ind);
				printf("bar-exp:\n");

				getspacing(ind+1);
				printf("alt #1:\n");
				re_comp_print(re->op.barExp.left, ind+2);
				
				getspacing(ind+1);
				printf("alt #2:\n");
				re_comp_print(re->op.barExp.right, ind+2);
				break;

			case plain_exp:
				getspacing(ind);
				printf("plain-exp:\n");
				re_comp_print(re->op.plainExp, ind+1);
				break;

			case rep_exp:
				getspacing(ind);
				printf("rep-exp:\n");
				re_comp_print(re->op.repExp, ind+1);
				break;

			case empty_exp:
				getspacing(ind);
				printf("empty\n");
				break;

			case char_exp:
				getspacing(ind);
				printf("char: %c\n", re->op.charExp);
				break;
		}
	}
}

void re_comp_print(re_comp* comp, int indent)
{
	re_comp* top;

	if (comp) {
		top = comp;
		while (top) {
			re_exp_print(top->elem, indent);
			top = top->next;
		}
	}
}

re_exp* re_exp_new(re_exp re) {
	re_exp* ptr = (re_exp*)malloc(sizeof(re_exp));
	if (ptr) *ptr = re;
	return ptr;
}

re_comp* re_comp_new(re_comp re) {
	re_comp* ptr = (re_comp*)malloc(sizeof(re_comp));
	if (ptr) *ptr = re;
	return ptr;
}

typedef struct 
re_sc
{
    char*   src;
	char*   cur;
    int     line;
	m_stack unget;
	m_stack unlex;
    int     column;
    int     lastchar;
}
re_scan_t;

/*
#define re_scan_init(ptr, str) do {\
	(ptr)->line     = 0;\
	(ptr)->lastchar = 0;\
	(ptr)->src      = (str);\
	(ptr)->cur      = (str);\
	(ptr)->column   = 0;\
	(ptr)->unget    = m_stack_init(char);\
	(ptr)->unlex    = m_stack_init(re_tk);\
} while (0); */

re_scan_t
re_scan_init(char* str)
{
	re_scan_t sc;

	sc.line     = 0;
	sc.lastchar = 0;
	sc.src      = str;
	sc.cur      = str;
	sc.column   = 0;
	sc.unget    = m_stack_init(char);
	sc.unlex    = m_stack_init(re_tk);

	return sc;
}

char
re_getch(re_scan_t* sc)
{
    char ch;

	if (sc->unget.count > 0) {
		ch = *(char*)m_stack_pop(&(sc->unget));
		sc->lastchar = ch;
		return ch;
	}

	ch = *(sc->cur);

	if (ch == '\0') {
		sc->lastchar = -1;
		return -1;
	}

	sc->cur++;
	sc->lastchar = ch;
	return ch;
}

#define re_unget(sc, ch) {\
	m_stack_push(&((sc)->unget), (char[]){ch});\
}

#define re_unlex(sc, tok) {\
	m_stack_push(&((sc)->unlex), (re_tk[]){tok});\
}

re_tk
re_lex(re_scan_t* sc)
{
    int ch;

	if (sc->unlex.count > 0) {
		ch = *(re_tk*)m_stack_pop(&(sc->unlex));
		return ch;
	}

    while (1)
    {
		ch = re_getch(sc);
		switch (ch)
		{
			case '[': return P_TOK_LBRACK;
			case ']': return P_TOK_RBRACK;
			case '+': return P_TOK_PLUS;
			case '-': return P_TOK_MINUS;
			case '*': return P_TOK_TIMES;
			case '?': return P_TOK_QUESTION;
			case '|': return P_TOK_BAR;
			case '\\': return P_TOK_SLASH;
			case '^': return P_TOK_CAP;
			case '(': return P_TOK_LPAREN;
			case ')': return P_TOK_RPAREN;
			case 'n': return P_TOK_NEWLINE_CHAR;
			case 'r': return P_TOK_CRETURN_CHAR;
			case 't': return P_TOK_TABULATE_CHAR;
			case '.': return P_TOK_DOT;
            case -1:  return P_TOK_END;
            default:  return P_TOK_CHAR;
		}
    }
}

/**
 * so, the tokens allowed are
 * ']' '[' '?' '+' '*' '|' '\' '^' '-' '(' ')' CHAR
 * here, CHAR is just every other char one sees
 * also, escape characters are not handled in scanner level
 * 
 * <re>    ::= <exp> <re>
 *         | empty
 *         ;
 * 
 * <exp>   ::= <elem>
 *         | <elem> '?'
 *         | <elem> '+'
 *         | <elem> '*'
 *         | '[' <slct> ']'
 *         | '(' <re> ')'
 *         ;
 * 
 * <elem>  ::= CHAR
 *         | '^'
 *         | '-'
 *         | '\' <canc>
 *         ;
 * 
 * <canc>  ::= '?' 
 *         | '+' 
 *         | '\' 
 *         | '*' 
 *         | '[' 
 *         | ']' 
 *         | '(' 
 *         | ')' 
 *         | '|'
 *         ;
 * 
 * <slct>  ::= '^' <slin>
 *         | <slin>
 *         ;
 * 
 * <slin>  ::= CHAR <slinb>
 *         | CHAR '-' CHAR <slinb>
 *         ;
 * 
 * <slinb> ::= CHAR <slinb>
 *         | CHAR '-' CHAR <slinb>
 *         | empty
 *         ;
 */

#define P_RULE_COUNT 47
#define P_STATE_COUNT 60
#define P_ELEMENT_COUNT 31

typedef struct 
re_pobj
{
	int action;
    union { 
        int shift;
        int sgoto;
        char accept;
        char error;
        struct { 
            int rule;
            int count;
            re_tk lhs_tok;
        } reduce;
    } op;
}
re_pobj;

#define GOTO   1
#define ERROR  0
#define SHIFT  2
#define REDUCE 3
#define ACCEPT 4

char* 
re_pobj_print(re_pobj re)
{
	int len   = 0;
	char* buf = NULL;

	switch (re.action)
	{
		case GOTO:
			len    = snprintf(NULL, 0, "g%d", re.op.sgoto);
			buf    = (char*)malloc(len);
			buf[0] = 'g';
			itoa(re.op.sgoto, buf+1, 10);
			break;

		case SHIFT:
			len    = snprintf(NULL, 0, "s%d", re.op.shift);
			buf    = (char*)malloc(len);
			buf[0] = 's';
			itoa(re.op.shift, buf+1, 10);
			break;

		case REDUCE:
			len    = snprintf(NULL, 0, "r%d", re.op.reduce.rule);
			buf    = (char*)malloc(len);
			buf[0] = 'r';
			itoa(re.op.reduce.rule, buf+1, 10);
			break;

		case ERROR:
			buf = "error";
			break;

		case ACCEPT:
			buf = "accept";
			break;
	}

	return buf;
}

#define re_table_next(table, state, token) (*((table) + ((state) * P_ELEMENT_COUNT) + (token)))

static inline void
re_table_set(re_pobj* table, int state, re_tk token, re_pobj data)
{
    re_pobj old = re_table_next(table, state, token);
    if (old.action != ERROR) {
        if (old.action == SHIFT && data.action == REDUCE) {
            fprintf(stderr, "warning: shift-reduce conflict at (state#%d, %s)\n", state, re_tk_string(token));
            fprintf(stderr, "opting to shift...\n");
            return;
        }
        if (old.action == REDUCE && data.action == SHIFT) {
            fprintf(stderr, "warning: shift-reduce conflict at (state#%d, %s)\n", state, re_tk_string(token));
            fprintf(stderr, "opting to shift...\n");
        }
        if (old.action == REDUCE && data.action == REDUCE) {
            fprintf(stderr, "error: reduce-reduce conflict at (state#%d, %s)\n", state, re_tk_string(token));
            exit(EXIT_FAILURE);
        }
    }

    re_table_next(table, state, token) = data;
}

static inline re_pobj*
re_table_prepare()
{
    re_pobj* table = (re_pobj*)calloc(P_STATE_COUNT * P_ELEMENT_COUNT, sizeof(re_pobj));

	re_table_set(table, 0, P_TOK_CAP, (re_pobj){ .action = SHIFT, .op.shift = 1});
	re_table_set(table, 0, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 0, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 0, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 4});
	re_table_set(table, 0, P_TOK_LBRACK, (re_pobj){ .action = SHIFT, .op.shift = 5});
	re_table_set(table, 0, P_TOK_LPAREN, (re_pobj){ .action = SHIFT, .op.shift = 6});
	re_table_set(table, 0, P_TOK_MINUS, (re_pobj){ .action = SHIFT, .op.shift = 7});
	re_table_set(table, 0, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 0, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 0, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 0, P_LIT_elm, (re_pobj){ .action = GOTO, .op.sgoto = 11});
	re_table_set(table, 0, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 12});
	re_table_set(table, 0, P_LIT_exp, (re_pobj){ .action = GOTO, .op.sgoto = 13});
	re_table_set(table, 0, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 14});
	re_table_set(table, 0, P_LIT_msub, (re_pobj){ .action = GOTO, .op.sgoto = 15});
	re_table_set(table, 0, P_LIT_re, (re_pobj){ .action = GOTO, .op.sgoto = 16});
	re_table_set(table, 0, P_LIT_sub, (re_pobj){ .action = GOTO, .op.sgoto = 17});
	re_table_set(table, 5, P_TOK_CAP, (re_pobj){ .action = SHIFT, .op.shift = 18});
	re_table_set(table, 5, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 5, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 5, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 19});
	re_table_set(table, 5, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 5, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 5, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 5, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 20});
	re_table_set(table, 5, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 21});
	re_table_set(table, 5, P_LIT_slc, (re_pobj){ .action = GOTO, .op.sgoto = 22});
	re_table_set(table, 5, P_LIT_sli, (re_pobj){ .action = GOTO, .op.sgoto = 23});
	re_table_set(table, 6, P_TOK_CAP, (re_pobj){ .action = SHIFT, .op.shift = 1});
	re_table_set(table, 6, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 6, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 6, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 4});
	re_table_set(table, 6, P_TOK_LBRACK, (re_pobj){ .action = SHIFT, .op.shift = 5});
	re_table_set(table, 6, P_TOK_LPAREN, (re_pobj){ .action = SHIFT, .op.shift = 6});
	re_table_set(table, 6, P_TOK_MINUS, (re_pobj){ .action = SHIFT, .op.shift = 7});
	re_table_set(table, 6, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 6, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 6, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 6, P_LIT_elm, (re_pobj){ .action = GOTO, .op.sgoto = 11});
	re_table_set(table, 6, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 12});
	re_table_set(table, 6, P_LIT_exp, (re_pobj){ .action = GOTO, .op.sgoto = 13});
	re_table_set(table, 6, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 14});
	re_table_set(table, 6, P_LIT_msub, (re_pobj){ .action = GOTO, .op.sgoto = 15});
	re_table_set(table, 6, P_LIT_re, (re_pobj){ .action = GOTO, .op.sgoto = 24});
	re_table_set(table, 6, P_LIT_sub, (re_pobj){ .action = GOTO, .op.sgoto = 17});
	re_table_set(table, 9, P_TOK_BAR, (re_pobj){ .action = SHIFT, .op.shift = 25});
	re_table_set(table, 9, P_TOK_CAP, (re_pobj){ .action = SHIFT, .op.shift = 26});
	re_table_set(table, 9, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 27});
	re_table_set(table, 9, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 28});
	re_table_set(table, 9, P_TOK_LBRACK, (re_pobj){ .action = SHIFT, .op.shift = 29});
	re_table_set(table, 9, P_TOK_LPAREN, (re_pobj){ .action = SHIFT, .op.shift = 30});
	re_table_set(table, 9, P_TOK_MINUS, (re_pobj){ .action = SHIFT, .op.shift = 31});
	re_table_set(table, 9, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 32});
	re_table_set(table, 9, P_TOK_PLUS, (re_pobj){ .action = SHIFT, .op.shift = 33});
	re_table_set(table, 9, P_TOK_QUESTION, (re_pobj){ .action = SHIFT, .op.shift = 34});
	re_table_set(table, 9, P_TOK_RBRACK, (re_pobj){ .action = SHIFT, .op.shift = 35});
	re_table_set(table, 9, P_TOK_RPAREN, (re_pobj){ .action = SHIFT, .op.shift = 36});
	re_table_set(table, 9, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 37});
	re_table_set(table, 9, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 38});
	re_table_set(table, 9, P_TOK_TIMES, (re_pobj){ .action = SHIFT, .op.shift = 39});
	re_table_set(table, 13, P_TOK_BAR, (re_pobj){ .action = SHIFT, .op.shift = 40});
	re_table_set(table, 13, P_LIT_re_BAR, (re_pobj){ .action = GOTO, .op.sgoto = 41});
	re_table_set(table, 15, P_TOK_CAP, (re_pobj){ .action = SHIFT, .op.shift = 1});
	re_table_set(table, 15, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 15, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 15, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 4});
	re_table_set(table, 15, P_TOK_LBRACK, (re_pobj){ .action = SHIFT, .op.shift = 5});
	re_table_set(table, 15, P_TOK_LPAREN, (re_pobj){ .action = SHIFT, .op.shift = 6});
	re_table_set(table, 15, P_TOK_MINUS, (re_pobj){ .action = SHIFT, .op.shift = 7});
	re_table_set(table, 15, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 15, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 15, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 15, P_LIT_elm, (re_pobj){ .action = GOTO, .op.sgoto = 11});
	re_table_set(table, 15, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 12});
	re_table_set(table, 15, P_LIT_exp_BAR, (re_pobj){ .action = GOTO, .op.sgoto = 42});
	re_table_set(table, 15, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 14});
	re_table_set(table, 15, P_LIT_msub, (re_pobj){ .action = GOTO, .op.sgoto = 43});
	re_table_set(table, 15, P_LIT_sub, (re_pobj){ .action = GOTO, .op.sgoto = 17});
	re_table_set(table, 17, P_TOK_PLUS, (re_pobj){ .action = SHIFT, .op.shift = 44});
	re_table_set(table, 17, P_TOK_QUESTION, (re_pobj){ .action = SHIFT, .op.shift = 45});
	re_table_set(table, 17, P_TOK_TIMES, (re_pobj){ .action = SHIFT, .op.shift = 46});
	re_table_set(table, 17, P_LIT_msub_BAR, (re_pobj){ .action = GOTO, .op.sgoto = 47});
	re_table_set(table, 18, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 18, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 18, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 19});
	re_table_set(table, 18, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 18, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 18, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 18, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 20});
	re_table_set(table, 18, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 21});
	re_table_set(table, 18, P_LIT_sli, (re_pobj){ .action = GOTO, .op.sgoto = 48});
	re_table_set(table, 21, P_TOK_MINUS, (re_pobj){ .action = SHIFT, .op.shift = 49});
	re_table_set(table, 22, P_TOK_RBRACK, (re_pobj){ .action = SHIFT, .op.shift = 50});
	re_table_set(table, 23, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 23, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 23, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 19});
	re_table_set(table, 23, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 23, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 23, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 23, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 20});
	re_table_set(table, 23, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 21});
	re_table_set(table, 23, P_LIT_slc_BAR, (re_pobj){ .action = GOTO, .op.sgoto = 51});
	re_table_set(table, 23, P_LIT_sli, (re_pobj){ .action = GOTO, .op.sgoto = 52});
	re_table_set(table, 24, P_TOK_RPAREN, (re_pobj){ .action = SHIFT, .op.shift = 53});
	re_table_set(table, 40, P_TOK_CAP, (re_pobj){ .action = SHIFT, .op.shift = 1});
	re_table_set(table, 40, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 40, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 40, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 4});
	re_table_set(table, 40, P_TOK_LBRACK, (re_pobj){ .action = SHIFT, .op.shift = 5});
	re_table_set(table, 40, P_TOK_LPAREN, (re_pobj){ .action = SHIFT, .op.shift = 6});
	re_table_set(table, 40, P_TOK_MINUS, (re_pobj){ .action = SHIFT, .op.shift = 7});
	re_table_set(table, 40, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 40, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 40, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 40, P_LIT_elm, (re_pobj){ .action = GOTO, .op.sgoto = 11});
	re_table_set(table, 40, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 12});
	re_table_set(table, 40, P_LIT_exp, (re_pobj){ .action = GOTO, .op.sgoto = 54});
	re_table_set(table, 40, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 14});
	re_table_set(table, 40, P_LIT_msub, (re_pobj){ .action = GOTO, .op.sgoto = 15});
	re_table_set(table, 40, P_LIT_sub, (re_pobj){ .action = GOTO, .op.sgoto = 17});
	re_table_set(table, 43, P_TOK_CAP, (re_pobj){ .action = SHIFT, .op.shift = 1});
	re_table_set(table, 43, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 43, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 43, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 4});
	re_table_set(table, 43, P_TOK_LBRACK, (re_pobj){ .action = SHIFT, .op.shift = 5});
	re_table_set(table, 43, P_TOK_LPAREN, (re_pobj){ .action = SHIFT, .op.shift = 6});
	re_table_set(table, 43, P_TOK_MINUS, (re_pobj){ .action = SHIFT, .op.shift = 7});
	re_table_set(table, 43, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 43, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 43, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 43, P_LIT_elm, (re_pobj){ .action = GOTO, .op.sgoto = 11});
	re_table_set(table, 43, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 12});
	re_table_set(table, 43, P_LIT_exp_BAR, (re_pobj){ .action = GOTO, .op.sgoto = 55});
	re_table_set(table, 43, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 14});
	re_table_set(table, 43, P_LIT_msub, (re_pobj){ .action = GOTO, .op.sgoto = 43});
	re_table_set(table, 43, P_LIT_sub, (re_pobj){ .action = GOTO, .op.sgoto = 17});
	re_table_set(table, 48, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 48, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 48, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 19});
	re_table_set(table, 48, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 48, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 48, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 48, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 20});
	re_table_set(table, 48, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 21});
	re_table_set(table, 48, P_LIT_slc_BAR, (re_pobj){ .action = GOTO, .op.sgoto = 56});
	re_table_set(table, 48, P_LIT_sli, (re_pobj){ .action = GOTO, .op.sgoto = 52});
	re_table_set(table, 49, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 49, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 49, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 49, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 49, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 57});
	re_table_set(table, 52, P_TOK_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 2});
	re_table_set(table, 52, P_TOK_CRETURN_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 3});
	re_table_set(table, 52, P_TOK_DOT, (re_pobj){ .action = SHIFT, .op.shift = 19});
	re_table_set(table, 52, P_TOK_NEWLINE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 8});
	re_table_set(table, 52, P_TOK_SLASH, (re_pobj){ .action = SHIFT, .op.shift = 9});
	re_table_set(table, 52, P_TOK_TABULATE_CHAR, (re_pobj){ .action = SHIFT, .op.shift = 10});
	re_table_set(table, 52, P_LIT_esc, (re_pobj){ .action = GOTO, .op.sgoto = 20});
	re_table_set(table, 52, P_LIT_fch, (re_pobj){ .action = GOTO, .op.sgoto = 21});
	re_table_set(table, 52, P_LIT_slc_BAR, (re_pobj){ .action = GOTO, .op.sgoto = 58});
	re_table_set(table, 52, P_LIT_sli, (re_pobj){ .action = GOTO, .op.sgoto = 52});
	re_table_set(table, 54, P_TOK_BAR, (re_pobj){ .action = SHIFT, .op.shift = 40});
	re_table_set(table, 54, P_LIT_re_BAR, (re_pobj){ .action = GOTO, .op.sgoto = 59});
	re_table_set(table, 0, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {3, 0, P_LIT_re}});
	re_table_set(table, 0, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {3, 0, P_LIT_re}});
	re_table_set(table, 1, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 1, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {16, 1, P_LIT_elm}});
	re_table_set(table, 2, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 2, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {43, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 3, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {46, 1, P_LIT_fch}});
	re_table_set(table, 4, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 4, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {19, 1, P_LIT_elm}});
	re_table_set(table, 6, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {3, 0, P_LIT_re}});
	re_table_set(table, 6, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {3, 0, P_LIT_re}});
	re_table_set(table, 7, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 7, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {17, 1, P_LIT_elm}});
	re_table_set(table, 8, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 8, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {45, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 10, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {44, 1, P_LIT_fch}});
	re_table_set(table, 11, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 11, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {12, 1, P_LIT_sub}});
	re_table_set(table, 12, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 12, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {18, 1, P_LIT_elm}});
	re_table_set(table, 13, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {2, 0, P_LIT_re_BAR}});
	re_table_set(table, 13, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {2, 0, P_LIT_re_BAR}});
	re_table_set(table, 14, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 14, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {15, 1, P_LIT_elm}});
	re_table_set(table, 15, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {6, 0, P_LIT_exp_BAR}});
	re_table_set(table, 15, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {6, 0, P_LIT_exp_BAR}});
	re_table_set(table, 15, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {6, 0, P_LIT_exp_BAR}});
	re_table_set(table, 16, P_TOK_END, (re_pobj){.action = ACCEPT, .op.accept = 0});
	re_table_set(table, 17, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 17, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {11, 0, P_LIT_msub_BAR}});
	re_table_set(table, 19, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {42, 1, P_LIT_sli}});
	re_table_set(table, 19, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {42, 1, P_LIT_sli}});
	re_table_set(table, 19, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {42, 1, P_LIT_sli}});
	re_table_set(table, 19, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {42, 1, P_LIT_sli}});
	re_table_set(table, 19, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {42, 1, P_LIT_sli}});
	re_table_set(table, 19, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {42, 1, P_LIT_sli}});
	re_table_set(table, 19, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {42, 1, P_LIT_sli}});
	re_table_set(table, 20, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {40, 1, P_LIT_sli}});
	re_table_set(table, 20, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {40, 1, P_LIT_sli}});
	re_table_set(table, 20, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {40, 1, P_LIT_sli}});
	re_table_set(table, 20, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {40, 1, P_LIT_sli}});
	re_table_set(table, 20, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {40, 1, P_LIT_sli}});
	re_table_set(table, 20, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {40, 1, P_LIT_sli}});
	re_table_set(table, 20, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {40, 1, P_LIT_sli}});
	re_table_set(table, 21, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {39, 1, P_LIT_sli}});
	re_table_set(table, 21, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {39, 1, P_LIT_sli}});
	re_table_set(table, 21, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {39, 1, P_LIT_sli}});
	re_table_set(table, 21, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {39, 1, P_LIT_sli}});
	re_table_set(table, 21, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {39, 1, P_LIT_sli}});
	re_table_set(table, 21, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {39, 1, P_LIT_sli}});
	re_table_set(table, 21, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {39, 1, P_LIT_sli}});
	re_table_set(table, 23, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {37, 0, P_LIT_slc_BAR}});
	re_table_set(table, 25, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 25, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {26, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 26, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {28, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 27, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {33, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 28, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {31, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 29, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {21, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 30, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {29, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 31, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {24, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 32, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {32, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 33, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {23, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 34, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {20, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 35, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {22, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 36, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {30, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 37, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {27, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 38, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {34, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 39, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {25, 2, P_LIT_esc}});
	re_table_set(table, 41, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {0, 2, P_LIT_re}});
	re_table_set(table, 41, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {0, 2, P_LIT_re}});
	re_table_set(table, 42, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {4, 2, P_LIT_exp}});
	re_table_set(table, 42, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {4, 2, P_LIT_exp}});
	re_table_set(table, 42, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {4, 2, P_LIT_exp}});
	re_table_set(table, 43, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {6, 0, P_LIT_exp_BAR}});
	re_table_set(table, 43, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {6, 0, P_LIT_exp_BAR}});
	re_table_set(table, 43, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {6, 0, P_LIT_exp_BAR}});
	re_table_set(table, 44, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 44, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {9, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 45, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {8, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 46, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {10, 1, P_LIT_msub_BAR}});
	re_table_set(table, 47, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 47, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {7, 2, P_LIT_msub}});
	re_table_set(table, 48, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {37, 0, P_LIT_slc_BAR}});
	re_table_set(table, 50, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 50, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {13, 3, P_LIT_sub}});
	re_table_set(table, 51, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {38, 2, P_LIT_slc}});
	re_table_set(table, 52, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {37, 0, P_LIT_slc_BAR}});
	re_table_set(table, 53, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_CAP, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_LBRACK, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_LPAREN, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_MINUS, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_PLUS, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_QUESTION, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 53, P_TOK_TIMES, (re_pobj){.action = REDUCE, .op.reduce = {14, 3, P_LIT_sub}});
	re_table_set(table, 54, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {2, 0, P_LIT_re_BAR}});
	re_table_set(table, 54, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {2, 0, P_LIT_re_BAR}});
	re_table_set(table, 55, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {5, 2, P_LIT_exp_BAR}});
	re_table_set(table, 55, P_TOK_BAR, (re_pobj){.action = REDUCE, .op.reduce = {5, 2, P_LIT_exp_BAR}});
	re_table_set(table, 55, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {5, 2, P_LIT_exp_BAR}});
	re_table_set(table, 56, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {35, 3, P_LIT_slc}});
	re_table_set(table, 57, P_TOK_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {41, 3, P_LIT_sli}});
	re_table_set(table, 57, P_TOK_CRETURN_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {41, 3, P_LIT_sli}});
	re_table_set(table, 57, P_TOK_DOT, (re_pobj){.action = REDUCE, .op.reduce = {41, 3, P_LIT_sli}});
	re_table_set(table, 57, P_TOK_NEWLINE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {41, 3, P_LIT_sli}});
	re_table_set(table, 57, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {41, 3, P_LIT_sli}});
	re_table_set(table, 57, P_TOK_SLASH, (re_pobj){.action = REDUCE, .op.reduce = {41, 3, P_LIT_sli}});
	re_table_set(table, 57, P_TOK_TABULATE_CHAR, (re_pobj){.action = REDUCE, .op.reduce = {41, 3, P_LIT_sli}});
	re_table_set(table, 58, P_TOK_RBRACK, (re_pobj){.action = REDUCE, .op.reduce = {36, 2, P_LIT_slc_BAR}});
	re_table_set(table, 59, P_TOK_END, (re_pobj){.action = REDUCE, .op.reduce = {1, 3, P_LIT_re_BAR}});
	re_table_set(table, 59, P_TOK_RPAREN, (re_pobj){.action = REDUCE, .op.reduce = {1, 3, P_LIT_re_BAR}});

	return table;
}

typedef struct
re_parse_t
{
	int        cid;
	re_pobj    next;
    re_pobj*   table;
    re_scan_t* scanner;
    m_stack    ststack;
	m_stack    tkstack;
	m_stack    restack;
}
re_parse_t;

/*
#define re_parse_init(ptr, sc) do {\
	(ptr)->tos      = 0;\
	(ptr)->state    = 0;\
	(ptr)->scanner  = (sc);\
	(ptr)->cid      = 0;\
	(ptr)->ststack  = m_stack_init(int);\
	(ptr)->tkstack  = m_stack_init(re_tk);\
	(ptr)->restack  = m_stack_init(re_exp*);\
	re_table_prepare((ptr)->table);\
} while (0);*/

re_parse_t
re_parse_init(re_scan_t* sc)
{
	re_parse_t ps;

	ps.scanner = sc;
	ps.cid     = 0;
	ps.ststack = m_stack_init(int);
	ps.tkstack = m_stack_init(re_tk);
	ps.restack = m_stack_init(re_exp*);
	ps.table   = re_table_prepare();
	
	return ps;
}

re_exp*
re_compute(re_parse_t* pr)
{
	int i   = 0;
	int tos = 0;
	re_tk a = 0;

	re_exp* retmp1;
	re_exp* retmp2;
	re_exp* retmp3;
	re_exp* retmp4;

	/* add to state stack a zero value */
	m_stack_push(&(pr->ststack), &i);

	/* get TOS before algorithm begins */
	a = re_lex(pr->scanner);

    while (1)
    {
		/* state stack should not be empty here */
		if (pr->ststack.count < 1) {
			fprintf(stderr, "empty state stack!");
			exit(EXIT_FAILURE);
		}

		/* set all the values for this round */
		tos      = *(int*)m_stack_tos(pr->ststack);
        pr->next = re_table_next(pr->table, tos, a);
		
		/* print out information, i guess */
		/* printf(
			"action(%s, state#%d) := %s",
			re_tk_string(a), tos,
			re_pobj_print(pr->next)
		);
		if (a == P_TOK_CHAR) {
			printf(
				", where %s = \"%c\".\n",
				re_tk_string(a), 
				pr->scanner->lastchar
			);
		} else printf(".\n"); */
		
		/* reset */
		retmp1 = NULL;
		retmp2 = NULL;
		retmp3 = NULL;
		retmp4 = NULL;

		switch (pr->next.action)
		{
            case SHIFT:
				/* push next state to state stack */
                m_stack_push(&(pr->ststack), &(pr->next.op.shift));
				
				/* create new regex S->END by char */
				retmp1 = re_exp_new((re_exp) {
					.tag = char_exp,
					.op.charExp = pr->scanner->lastchar
				});

				/* push this new regex to regex stack */
                m_stack_push(&(pr->restack), &retmp1);

				/* get new token */
				a = re_lex(pr->scanner);

				/* do my printing thing
				getspacing(1);
				printf("Pushed state#%i to state stack.\n", pr->next.op.shift);
				getspacing(1);
				printf("Pushed new ast to stack:\n");
				re_exp_print(retmp1, 2);
				printf("\n");*/
                break;

            case REDUCE:
				/* handle different reductions */
				switch (pr->next.op.reduce.rule) {
					case 2:
					case 3:
					case 6:
					case 11:
					case 37:
						/* empty statement */
						retmp1 = re_exp_new((re_exp) {
							.tag = empty_exp,
							.op.emptyExp = 0
						});
						break;

					case 46:
					case 45:
					case 44:
					case 43:
						/* fch <- CRETURN_CHAR */
						/* fch <- NEWLINE_CHAR */
						/* fch <- TABULATE_CHAR */
						/* fch <- CHAR_CHAR */
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // ...
						retmp1 = retmp2;
						break;
						
					case 42:
						/* sli <- DOT */
						m_stack_pop(&(pr->restack));
						retmp1 = re_exp_new((re_exp) {
							.tag = dot_exp,
							.op.dotExp = 0
						});
						break;

					case 41:
						/* sli <- fch '-' fch */
						retmp3 = *(re_exp**)m_stack_pop(&(pr->restack)); // fch
						m_stack_pop(&(pr->restack));
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // fch
						retmp1 = re_exp_new((re_exp) {
							.tag             = range_exp,
							.op.rangeExp.min = retmp2->op.charExp,
							.op.rangeExp.max = retmp3->op.charExp
						});
						break;

					case 40:
					case 39:
						/* sli <- esc  */
						/* sli <- fch */
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // esc, fch
						retmp1 = retmp2;
						break;

					case 38:
					case 36:
						/* slc' <- sli slc' */
						/* slc  <- sli slc' */
						retmp3 = *(re_exp**)m_stack_pop(&(pr->restack)); // slc'
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // sli

						if (retmp3->tag == empty_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag                 = select_exp,
								.op.selectExp.pos    = 1,
								.op.selectExp.select = re_comp_new((re_comp) {
									.elem = retmp2,
									.next = NULL
								})
							});
						}
						else
						if (retmp3->tag == select_exp) {
							retmp3->op.selectExp.select = re_comp_new((re_comp) {
								.elem = retmp2,
								.next = retmp3->op.selectExp.select
							});
							retmp1 = retmp3;
						}
						else {
							fprintf(stderr, "incorrect type\n");
							exit(EXIT_FAILURE);
						}
						break;

					case 35:
						/* slc <- CAP sli slc' */
						retmp3 = *(re_exp**)m_stack_pop(&(pr->restack)); // slc'
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // sli
						m_stack_pop(&(pr->restack));

						if (retmp3->tag == empty_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag                 = select_exp,
								.op.selectExp.pos    = 0,
								.op.selectExp.select = re_comp_new((re_comp) {
									.elem = retmp2,
									.next = NULL
								})
							});
						}
						else
						if (retmp3->tag == select_exp) {
							retmp3->op.selectExp.pos    = 0;
							retmp3->op.selectExp.select = re_comp_new((re_comp) {
								.elem = retmp2,
								.next = retmp3->op.selectExp.select
							});
							retmp1 = retmp3;
						}
						else {
							fprintf(stderr, "incorrect type\n");
							exit(EXIT_FAILURE);
						}
						break;

					case 34:
						m_stack_pop(&(pr->restack));
						retmp1 = re_exp_new((re_exp) {
							.tag = char_exp,
							.op.charExp = '\t'
						});
						break;

					case 33:
						m_stack_pop(&(pr->restack));
						retmp1 = re_exp_new((re_exp) {
							.tag = char_exp,
							.op.charExp = '\r'
						});
						break;
						
					case 32:
						m_stack_pop(&(pr->restack));
						retmp1 = re_exp_new((re_exp) {
							.tag = char_exp,
							.op.charExp = '\n'
						});
						break;
					
					case 31:
					case 30:
					case 29:
					case 28:
					case 27:
					case 26:
					case 25:
					case 24:
					case 23:
					case 22:
					case 21:
					case 20:
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack));
						m_stack_pop(&(pr->restack));
						retmp1 = retmp2;
						break;

					case 19:
						m_stack_pop(&(pr->restack));
						retmp1 = re_exp_new((re_exp) {
							.tag = dot_exp,
							.op.dotExp = 0
						});
						break;

					case 18:
					case 17:
					case 16:
					case 15:
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack));
						retmp1 = retmp2;
						break;

					case 14:
					case 13:
						/* sub <- LBRACK slc RBRACK */
						/* sub <- LPAREN re RPAREN  */
						m_stack_pop(&(pr->restack));
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // slc, re
						m_stack_pop(&(pr->restack));
						retmp1 = retmp2;
						break;

					case 12:
						/* sub <- elm */
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack));
						retmp1 = retmp2;
						break;

					case 10:
					case 9:
					case 8:
						/* msub' <- PLUS */
						/* msub' <- TIMES */
						/* msub' <- QUESTION */
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack));
						retmp1 = retmp2;
						break;

					case 7:
						/* msub <- sub msub' */
						retmp3 = *(re_exp**)m_stack_pop(&(pr->restack)); // msub'
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // sub

						if (retmp3->tag == empty_exp) {
							retmp1 = retmp2;
						}
						else {
							if (retmp3->tag == char_exp) {
								switch (retmp3->op.charExp) {
									case '*':
										retmp1 = re_exp_new((re_exp) {
											.tag          = kleene_exp,
											.op.kleeneExp = re_comp_new((re_comp) {
												.elem = retmp2,
												.next = NULL
											})
										});
										break;

									case '+':
										retmp1 = re_exp_new((re_exp) {
											.tag       = rep_exp,
											.op.repExp = re_comp_new((re_comp) {
												.elem = retmp2,
												.next = NULL
											})
										});
										break;
										
									case '?':
										retmp1 = re_exp_new((re_exp) {
											.tag       = opt_exp,
											.op.optExp = re_comp_new((re_comp) {
												.elem = retmp2,
												.next = NULL
											})
										});
										break;
									
									default:
										fprintf(stderr, "incorrect type\n");
										exit(EXIT_FAILURE);
								}
							}
							else {
								fprintf(stderr, "incorrect type\n");
								exit(EXIT_FAILURE);
							}
						}
						break;

					case 5:
					case 4:
						/* exp' <- msub exp' */
						/* exp  <- msub exp' */
						retmp3 = *(re_exp**)m_stack_pop(&(pr->restack)); // exp'
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // sub
						
						assert(retmp3->tag == plain_exp || retmp3->tag == empty_exp);
						
						if (retmp3->tag == plain_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag         = plain_exp,
								.op.plainExp = re_comp_new((re_comp) {
									.elem = retmp2,
									.next = retmp3->op.plainExp
								})
							});
						}
						else
						if (retmp3->tag == empty_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag         = plain_exp,
								.op.plainExp = re_comp_new((re_comp) {
									.elem = retmp2,
									.next = NULL
								})
							});
						}
						else {
							fprintf(stderr, "incorrect type\n");
							exit(EXIT_FAILURE);
						}
						break;

					case 1:
						/* re' <- BAR exp re' */
						retmp3 = *(re_exp**)m_stack_pop(&(pr->restack)); // re'
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // exp
						m_stack_pop(&(pr->restack));
						
						assert(retmp2->tag == plain_exp);
						assert(retmp3->tag == bar_exp || retmp3->tag == empty_exp);

						if (retmp3->tag == bar_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag             = bar_exp,
								.op.barExp.left  = NULL,
								.op.barExp.right = re_comp_new((re_comp) {
									.elem = re_exp_new((re_exp) {
										.tag             = bar_exp,
										.op.barExp.left  = retmp2->op.plainExp,
										.op.barExp.right = retmp3->op.barExp.right
									}),
									.next = NULL
								})
							});
						}
						else
						if (retmp3->tag == empty_exp) {
							retmp1 = re_exp_new((re_exp) {
								.tag             = bar_exp,
								.op.barExp.left  = NULL,
								.op.barExp.right = retmp2->op.plainExp
							});
						}

						break;

					case 0:
						/* re <- exp re' */
						retmp3 = *(re_exp**)m_stack_pop(&(pr->restack)); // re'
						retmp2 = *(re_exp**)m_stack_pop(&(pr->restack)); // exp

						assert(retmp2->tag == plain_exp);
						assert(retmp3->tag == bar_exp || retmp3->tag == empty_exp);

						if (retmp3->tag == bar_exp) {
							retmp3->op.barExp.left = retmp2->op.plainExp;
							retmp1 = retmp3;
						}
						else
						if (retmp3->tag == empty_exp) {
							retmp1 = retmp2;
						}

						break;

					default:
						fprintf(stderr, "state out of range.\n");
						exit(EXIT_FAILURE);
				}

				/*if (retmp2) {
					getspacing(1);
					printf("AST(s) popped:\n");
					re_exp_print(retmp2, 2);
					re_exp_print(retmp3, 2);
					re_exp_print(retmp4, 2);
				}*/

				//getspacing(1);
				//printf("AST(s) pushed:\n");
				//re_exp_print(retmp1, 2);
				m_stack_push(&(pr->restack), &retmp1);

				for (i = 0; i < pr->next.op.reduce.count; ++i)
					m_stack_pop(&(pr->ststack));
				
				//getspacing(1);
				//printf("Popped %i state%s from stack.\n", pr->next.op.reduce.count, pr->next.op.reduce.count == 1 ? "" : "s");

				tos      = *(int*)m_stack_tos(pr->ststack);
				pr->next = re_table_next(pr->table, tos, pr->next.op.reduce.lhs_tok);
				if (pr->next.action == GOTO) {
					m_stack_push(&(pr->ststack), &(pr->next.op.sgoto));
				} else {
					fprintf(stderr, "invalid token \"%s\" in state#%d.\n", re_tk_string(pr->next.op.reduce.lhs_tok), tos);
					exit(EXIT_FAILURE);
				}

				//printf("\n");
				break;

            case GOTO:
				//printf("using a goto...\n\n");
				a = re_lex(pr->scanner);
                m_stack_push(&(pr->ststack), &(pr->next.op.sgoto));
                break;

            case ACCEPT:
				//printf("we out...\n\n");
                retmp1 = *(re_exp**)m_stack_pop(&(pr->restack));
                return retmp1;

            case ERROR:
                fprintf(stderr, "invalid token \"%s\" in state#%d.\n", re_tk_string(a), tos);
                exit(EXIT_FAILURE);
		}
    }
}

/* utility concat function */
char* re_strcat(char* dst, char* src) {
	char* buf = NULL;
	int dlen = strlen(dst);
	int slen = strlen(src);

	if (slen == 0) return dst;

	buf = realloc(dst, (dlen + slen + 1) * sizeof(char));
	
	if (buf) {
		dst = buf;
		memcpy(dst+dlen, src, slen);
		printf("Result: %s\n", dst);
		return dst;
	} else return NULL;
}

#define PAD_COUNT 4
#define re_write(f, str, space) do {\
	for (int i = 0; i < ((space) * PAD_COUNT); ++i)\
		fwrite(" ", sizeof(char), 1, (fptr));\
	fwrite((str), sizeof(char), strlen((str)), (f));\
} while (0);

#define ch_to_str(ch) ((ch) == '\n' ? "\\n" : ((ch) == '\t' ? "\\t" : ((ch) == '\r' ? "\\r" : ((ch) == '\"' ? "\\\"" : ((ch) == '\'' ? "\\\'" : (char[]){(ch), 0})))))

/* get string form of regular expression */
void re_conv(re_exp* re, FILE* fptr, int space)
{
	int k         = 0;
	int curspace  = 0;
	int depth     = 0;
	re_exp* curr  = NULL;
	bool pol      = false;
	re_comp* iter = NULL;

	switch (re->tag)
	{
		case char_exp:
			re_write(fptr, "save_bool(ch == \'", space);
			re_write(fptr, ch_to_str(re->op.charExp), 0);
			re_write(fptr, "\');\n", 0);
			re_write(fptr, "ch = scan();\n", space);
			break;

		case dot_exp:
			re_write(fptr, "save_bool(true);\n", space);
			re_write(fptr, "ch = scan();\n", space);
			break;

		case range_exp:
			re_write(fptr, "save_bool(ch >= \'", space);
			re_write(fptr, ch_to_str(re->op.rangeExp.min), 0);
			re_write(fptr, "\' && ch <= \'", 0);
			re_write(fptr, ch_to_str(re->op.rangeExp.max), 0);
			re_write(fptr, "\');\n", 0);
			re_write(fptr, "ch = scan();\n", space);
			break;

		case empty_exp:
			re_write(fptr, "save_bool(true);\n", space);
			break;

		case kleene_exp:
		case rep_exp:
			re_write(fptr, "save_pos();\n", space);
			re_write(fptr, "new_counter();\n", space);
			re_write(fptr, "while (true) {\n", space);
			
			re_conv(re_exp_new((re_exp) {
				.tag = plain_exp,
				.op.plainExp = re->tag == kleene_exp ? re->op.kleeneExp : re->op.repExp
			}), fptr, space + 1);

			re_write(fptr, "if (!load_bool()) {\n", space + 1);
			re_write(fptr, "ch = prev_pos();\n", space + 2);
			re_write(fptr, "break;\n", space + 1);
			re_write(fptr, "} else {\n", space + 2);
			re_write(fptr, "save_pos();\n", space + 2);
			re_write(fptr, "inc_counter();\n", space + 2);
			re_write(fptr, "}\n", space + 1);

			re_write(fptr, "} save_bool(count() > ", space);
			re_write(fptr, re->tag == kleene_exp ? "-1" : "0", 0);
			re_write(fptr, ");\n", 0);

			break;

		case opt_exp:
			re_write(fptr, "save_pos();\n", space);
			
			re_conv(re_exp_new((re_exp) {
				.tag = plain_exp,
				.op.plainExp = re->tag == kleene_exp ? re->op.kleeneExp : re->op.repExp
			}), fptr, space);

			re_write(fptr, "if (!load_bool())\n", space);
			re_write(fptr, "ch = prev_pos();\n", space + 1);
			re_write(fptr, "save_bool(true);\n", space);
			
			break;

		case select_exp:
			pol  = re->op.selectExp.pos;
			iter = re->op.selectExp.select;

			if (iter) {
				if (iter->next) {
					re_write(fptr, "do {\n", space);		
					while (iter) {
						re_conv(iter->elem, fptr, space + 1);
						re_write(fptr, "if (", space + 1);
						re_write(fptr, pol ? "" : "!", 0);
						re_write(fptr, "load_bool()) {\n", 0);
						re_write(fptr, "save_bool(true);\n", space + 2);
						re_write(fptr, "break;\n", space + 2);
						re_write(fptr, "}\n", space + 1);
						iter = iter->next;
					}
					re_write(fptr, "save_bool(false);\n", space + 1);
					re_write(fptr, "} while (0);\n", space);
				} else re_conv(iter->elem, fptr, space);
			}

			break;

		case bar_exp:
			if (re->op.barExp.left && re->op.barExp.right)
			{
				iter = re->op.barExp.left;
				re_write(fptr, "save_pos();\n", space);
				re_conv(re_exp_new((re_exp) {
					.tag = plain_exp,
					.op.plainExp = iter
				}), fptr, space);
				re_write(fptr, "if (!load_bool()) {\n", space);
				iter = re->op.barExp.right;
				re_write(fptr, "ch = prev_pos();\n", space + 1);
				re_conv(re_exp_new((re_exp) {
					.tag = plain_exp,
					.op.plainExp = iter
				}), fptr, space + 2);
				re_write(fptr, "} else save_bool(true);\n", space);
			}
			else {
				iter = re->op.barExp.left ? re->op.barExp.left : re->op.barExp.right;
				re_conv(re_exp_new((re_exp) {
					.tag = plain_exp,
					.op.plainExp = iter
				}), fptr, space);
			}
			break;

		case plain_exp:

			iter = re->op.plainExp;

			if (iter) {
				if (iter->next) {
					re_write(fptr, "do {\n", space);		
					while (iter) {
						re_conv(iter->elem, fptr, space + 1);
						re_write(fptr, "if (!load_bool()) {\n", space + 1);
						re_write(fptr, "save_bool(false);\n", space + 2);
						re_write(fptr, "break;\n", space + 2);
						re_write(fptr, "}\n", space + 1);
						iter = iter->next;
					}
					re_write(fptr, "save_bool(true);\n", space + 1);
					re_write(fptr, "} while (0);\n", space);
				} else re_conv(iter->elem, fptr, space);
			}

			break;
	}
}

#undef re_write
#undef ch_to_str

int main(int argc, char** argv)
{
	int pos;
	int stat;
	re_tk tk;
	char* str;
	char* fnom;
	FILE* inf;
	FILE* outf;
	char* line;
	size_t len;
	re_exp* rexpr;
	ssize_t nread;
	re_scan_t scptr;
	re_parse_t psptr;

	switch (argc) {
		case 1:
			fprintf(stderr, "regular expression not provided\n");
			exit(EXIT_FAILURE);
			break;
		case 2:
			str  = argv[1];
			fnom = "output.c";
			break;
		case 3:
			str  = argv[1];
			fnom = argv[2];
			break;
		default:
			fprintf(stderr, "too many arguments!\n");
			exit(EXIT_FAILURE);
			break;
	}

	/* open input file ptr */
	inf = fopen("./res/base.txt", "r");
	if (inf == NULL) {
		strerror(errno);
		exit(EXIT_FAILURE);
	}

	/* open output file ptr */
	outf = fopen(fnom, "w");
	if (outf == NULL) {
		strerror(errno);
		exit(EXIT_FAILURE);
	}
	
	/* prepare variables */
	stat  = 0;
	line  = NULL;
	scptr = re_scan_init(str);
	psptr = re_parse_init(&scptr);
	rexpr = re_compute(&psptr);

	/* line by line, read input and compare */
	while ((nread = getline(&line, &len, inf)) != -1) {
		/* check if the subtree doesn't exist */
		if ((pos = issubstr(line, "/* input */")) == -1) {
			/* write line to output file */
			fwrite(line, sizeof(char), strlen(line), outf);
		} 
		else {
			/* depends where you are, I guess... */
			switch (stat) {
				case 0:
					/* write info, depending on place */
					re_conv(rexpr, outf, pos / PAD_COUNT);
					break;
			}
		}
	}

	fclose(inf);
	fclose(outf);
}