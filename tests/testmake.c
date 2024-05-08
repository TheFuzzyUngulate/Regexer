#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "..\types\list\lists.h"

#define setspacing(fptr, count) do {\
    for (int i = 0; i < (count); ++i)\
        fputc('\t', fptr);\
} while (0)

/**
 * @brief A struct containing individual testcases for a single testfile.
 * 
 */
typedef struct
m_testcase {
    char* str;
    bool exp;
    struct m_testcase* next;
} m_testcase;

typedef struct
m_testsuite {
    char* name;
    m_testcase* cases;
} m_testsuite;

// return true if the file specified
// by the filename exists
bool file_exists(const char *filename)
{
    struct stat buffer;
    return stat(filename, &buffer) == 0 ? true : false;
}

/**
 * @brief Print out a testcase into a file.
 * 
 * @param mcase Case to be printed out.
 * @param fptr Pointer to file structure to print into.
 * @param INDENT Current indent, for aesthetic reasons.
 */
void m_testcase_print(m_testcase* mcase, FILE* fptr, int INDENT)
{
    m_testcase* root;

    root = mcase;

    if (root != NULL) {
        setspacing(fptr, INDENT);
        fprintf(fptr, "set TEST=\"%s\"\n", root->str);
        setspacing(fptr, INDENT);
        fprintf(fptr, "tmp.exe !TEST!\n");
        setspacing(fptr, INDENT);
        fprintf(fptr, "IF ERRORLEVEL %d (\n", root->exp ? 0 : 1);
        setspacing(fptr, INDENT+1);
        fprintf(
            fptr,
            "echo !TEST! %s, as expected\n",
            root->exp ? "passed" : "failed"
        );
        if (root->next) 
            m_testcase_print(mcase->next, fptr, INDENT+1);
        setspacing(fptr, INDENT);
        fprintf(
            fptr,
            ") ELSE (echo !TEST! unexpectedly %s)\n",
            root->exp ? "passed" : "failed"
        );
    }
}

void m_testmake_print(m_list* testmake, FILE* fptr)
{
    int i;
    m_testcase*  mcase;
    m_testsuite* suite;

    fprintf(fptr, "@echo off\nsetlocal EnableDelayedExpansion\n");
    fprintf(fptr, "\necho compiling regexer.c\ncd ..\nmake\necho compiled regexer.exe\n");

    for (i = 0; i < testmake->count; ++i)
    {
        suite = *(m_testsuite**)m_list_get(testmake, i);
        fprintf(fptr, "\necho testing %s\nregexer.exe tmp.c -f tests\\%s\ngcc tmp.c -o tmp\n", suite->name, suite->name);
        m_testcase_print(suite->cases, fptr, 0);
        fprintf(fptr, "echo deleting temp files\ndel tmp.exe\ndel tmp.c\necho .\n", suite->name);
    }

    fprintf(fptr, "\necho deleting regexer.exe\ndel regexer.exe\ncd tests\n");
    fprintf(fptr, "\nendlocal & set FOO=%%FOO%%\n@echo on\n");
}

/**
 * @brief Create testfile
 * 
 * @param name Name of file to create.
 * @param regex Regular expression to be written.
 */
void m_testfile_create(char* name, char* regex)
{
    FILE* file;
    file = fopen(name, "w");
	if (file == NULL) {
		strerror(errno);
		exit(EXIT_FAILURE);
	}
    fwrite(regex, sizeof(char), strlen(regex), file);
    fclose(file);
}

/**
 * @brief Create testmake map
 * 
 * @return A pointer to a `m_list` struct that contains all testfile info.
 */
m_list*
m_testmake_create() {
    m_list* suites;
    suites = m_list_create(m_testsuite*);
    return suites;
}

/**
 * @brief Add a testfile to the testmake struct.
 * 
 * @param testmake Testmake struct to add to.
 * @param name Name of the testfile to add.
 * @param regex Regex being added to the testfile.
 * @return `EXIT_SUCCESS` if successful, `EXIT_FAILURE` otherwise.
 */
int m_testmake_suite_init(m_list* testmake, char* name, char* regex)
{
    m_testsuite *suite;
    if (testmake && name && regex) {
        if (strlen(name) > 0) {
            if (strlen(regex) > 0) {
                m_testfile_create(name, regex);
                suite        = (m_testsuite*)malloc(sizeof(m_testsuite));
                suite->name  = name;
                suite->cases = NULL;
                m_list_append(testmake, &suite);
                return EXIT_SUCCESS;
            }
        }
    } return EXIT_FAILURE;
}

/**
 * @brief Add a single testcase to a testfile
 * 
 * @param testmake Testmake struct to add to.
 * @param name A regular expression containing the name of the testfile being added to.
 * @param str A string showing the testcase being added.
 * @return `EXIT_SUCCESS` if successful, `EXIT_FAILURE` otherwise.
 */
int m_testmake_add(m_list* testmake, char* name, char* str, bool exp)
{
    int i;
    m_testcase  *tc;
    m_testsuite *ts;

    if (testmake && name && str) 
    {
        for (i = 0; i < testmake->count; ++i)
        {
            ts = *(m_testsuite**)m_list_get(testmake, i);

            if (!strcmp(ts->name, name))
            {
                tc = ts->cases;

                if ((ts->cases) == NULL) {
                    (ts->cases)       = (m_testcase*)malloc(sizeof(m_testcase));
                    (ts->cases)->str  = str;
                    (ts->cases)->exp  = exp;
                    (ts->cases)->next = NULL;
                } else {
                    while (tc->next != NULL)
                        tc = tc->next;
                    tc->next       = (m_testcase*)malloc(sizeof(m_testcase));
                    tc->next->str  = str;
                    tc->next->exp  = exp;
                    tc->next->next = NULL;
                }

                return EXIT_SUCCESS;
            }
        }
    }
    
    return EXIT_FAILURE;
}

int main()
{
    FILE* file;
    m_list* testmake;

    testmake = m_testmake_create();

    // Testing a single atom

    m_testmake_suite_init(testmake, "atom.txt", "A");
    m_testmake_add(testmake, "atom.txt", "A", true);
    m_testmake_add(testmake, "atom.txt", "B", false);
    m_testmake_add(testmake, "atom.txt", "a", false);
    m_testmake_add(testmake, "atom.txt", "AB", true);
    m_testmake_add(testmake, "atom.txt", "AAA", true);
    m_testmake_add(testmake, "atom.txt", "BA", false);

    // Testing two characters

    m_testmake_suite_init(testmake, "twatom.txt", "be");
    m_testmake_add(testmake, "twatom.txt", "b", false);
    m_testmake_add(testmake, "twatom.txt", "e", false);
    m_testmake_add(testmake, "twatom.txt", "be", true);
    m_testmake_add(testmake, "twatom.txt", "bee", true);
    m_testmake_add(testmake, "twatom.txt", "babe", false);
    m_testmake_add(testmake, "twatom.txt", "best", true);

    // Testing seven characters

    m_testmake_suite_init(testmake, "seven.txt", "seven");
    m_testmake_add(testmake, "seven.txt", "seven", true);
    m_testmake_add(testmake, "seven.txt", "seventy", true);
    m_testmake_add(testmake, "seven.txt", "sev", false);
    m_testmake_add(testmake, "seven.txt", "four score and seven years ago", false);
    m_testmake_add(testmake, "seven.txt", "seven sons of sceva", true);

    // Testing simple kleene-star

    m_testmake_suite_init(testmake, "klsimp.txt", "a*");
    m_testmake_add(testmake, "klsimp.txt", "", true);
    m_testmake_add(testmake, "klsimp.txt", "a", true);
    m_testmake_add(testmake, "klsimp.txt", "aaaa", true);
    m_testmake_add(testmake, "klsimp.txt", "random", true);
    m_testmake_add(testmake, "klsimp.txt", "coyote", true);

    // Testing kleene start with prefix

    m_testmake_suite_init(testmake, "klwpref.txt", "a(ba)*");
    m_testmake_add(testmake, "klwpref.txt", "a", true);
    m_testmake_add(testmake, "klwpref.txt", "add", true);
    m_testmake_add(testmake, "klwpref.txt", "bark", false);
    m_testmake_add(testmake, "klwpref.txt", "abababa", true);
    m_testmake_add(testmake, "klwpref.txt", "baba", false);
    m_testmake_add(testmake, "klwpref.txt", "academic", true);

    // Testing kleene star with suffix

    m_testmake_suite_init(testmake, "klwsuf.txt", "(ba)*a");
    m_testmake_add(testmake, "klwsuf.txt", "a", true);
    m_testmake_add(testmake, "klwsuf.txt", "b", false);
    m_testmake_add(testmake, "klwsuf.txt", "ba", false);
    m_testmake_add(testmake, "klwsuf.txt", "baa", true);
    m_testmake_add(testmake, "klwsuf.txt", "add", true);
    m_testmake_add(testmake, "klwsuf.txt", "baaa", false);
    m_testmake_add(testmake, "klwsuf.txt", "baaaa", true);
    m_testmake_add(testmake, "klwsuf.txt", "baaaakery", true);
    m_testmake_add(testmake, "klwsuf.txt", "academic", true);

    // Testing kleene star with prefix and suffix

    m_testmake_suite_init(testmake, "klwprsuf.txt", "9(1)*4");
    m_testmake_add(testmake, "klwprsuf.txt", "9", false);
    m_testmake_add(testmake, "klwprsuf.txt", "1", false);
    m_testmake_add(testmake, "klwprsuf.txt", "91", false);
    m_testmake_add(testmake, "klwprsuf.txt", "94", true);
    m_testmake_add(testmake, "klwprsuf.txt", "914", true);
    m_testmake_add(testmake, "klwprsuf.txt", "9124", false);
    m_testmake_add(testmake, "klwprsuf.txt", "91411", true);
    m_testmake_add(testmake, "klwprsuf.txt", "911111114ab", true);

    // Testing simple repetition

    m_testmake_suite_init(testmake, "rpsimp.txt", "c+");
    m_testmake_add(testmake, "rpsimp.txt", "", false);
    m_testmake_add(testmake, "rpsimp.txt", "b", false);
    m_testmake_add(testmake, "rpsimp.txt", "c", true);
    m_testmake_add(testmake, "rpsimp.txt", "catholic", true);
    m_testmake_add(testmake, "rpsimp.txt", "ccc", true);

    // Testing repetition with prefix

    m_testmake_suite_init(testmake, "rpwpref.txt", "ba+");
    m_testmake_add(testmake, "rpwpref.txt", "b", false);
    m_testmake_add(testmake, "rpwpref.txt", "a", false);
    m_testmake_add(testmake, "rpwpref.txt", "ba", true);
    m_testmake_add(testmake, "rpwpref.txt", "baa", true);
    m_testmake_add(testmake, "rpwpref.txt", "baathwater", true);

    // Testing repetition with suffix

    m_testmake_suite_init(testmake, "rpwsuf.txt", "b+a");
    m_testmake_add(testmake, "rpwsuf.txt", "a", false);
    m_testmake_add(testmake, "rpwsuf.txt", "b", false);
    m_testmake_add(testmake, "rpwsuf.txt", "ba", true);
    m_testmake_add(testmake, "rpwsuf.txt", "bbq meetup", false);
    m_testmake_add(testmake, "rpwsuf.txt", "bbbbbakar", true);
    m_testmake_add(testmake, "rpwsuf.txt", "qardzhabba", false);

    // Testing repetition with prefix and suffix

    m_testmake_suite_init(testmake, "rpwprsuf.txt", "bi+g");
    m_testmake_add(testmake, "rpwprsuf.txt", "b", false);
    m_testmake_add(testmake, "rpwprsuf.txt", "bi", false);
    m_testmake_add(testmake, "rpwprsuf.txt", "i", false);
    m_testmake_add(testmake, "rpwprsuf.txt", "ig", false);
    m_testmake_add(testmake, "rpwprsuf.txt", "iiiiig", false);
    m_testmake_add(testmake, "rpwprsuf.txt", "bg", false);
    m_testmake_add(testmake, "rpwprsuf.txt", "big", true);
    m_testmake_add(testmake, "rpwprsuf.txt", "bigger people", true);
    m_testmake_add(testmake, "rpwprsuf.txt", "biiiiig", true);
    m_testmake_add(testmake, "rpwprsuf.txt", "biiIiig", false);
    m_testmake_add(testmake, "rpwprsuf.txt", "biig natiion", true);

    // Testing simple alternates

    m_testmake_suite_init(testmake, "altsimp.txt", "a|e");
    m_testmake_add(testmake, "altsimp.txt", "a", true);
    m_testmake_add(testmake, "altsimp.txt", "e", true);
    m_testmake_add(testmake, "altsimp.txt", "k", false);
    m_testmake_add(testmake, "altsimp.txt", "ae", true);
    m_testmake_add(testmake, "altsimp.txt", "addition", true);
    m_testmake_add(testmake, "altsimp.txt", "evil people", true);

    // Testing multiple alternates

    m_testmake_suite_init(testmake, "altmult.txt", "a|e|i|o|u");
    m_testmake_add(testmake, "altmult.txt", "a", true);
    m_testmake_add(testmake, "altmult.txt", "e", true);
    m_testmake_add(testmake, "altmult.txt", "i", true);
    m_testmake_add(testmake, "altmult.txt", "o", true);
    m_testmake_add(testmake, "altmult.txt", "u", true);
    m_testmake_add(testmake, "altmult.txt", "eager", true);
    m_testmake_add(testmake, "altmult.txt", "best", false);
    m_testmake_add(testmake, "altmult.txt", "hate", false);
    m_testmake_add(testmake, "altmult.txt", "aeiou", true);

    // Testing alternates with prefix

    m_testmake_suite_init(testmake, "altwpref.txt", "b(y|e)");
    m_testmake_add(testmake, "altwpref.txt", "b", false);
    m_testmake_add(testmake, "altwpref.txt", "y", false);
    m_testmake_add(testmake, "altwpref.txt", "e", false);
    m_testmake_add(testmake, "altwpref.txt", "by", true);
    m_testmake_add(testmake, "altwpref.txt", "be", true);
    m_testmake_add(testmake, "altwpref.txt", "bk", false);
    m_testmake_add(testmake, "altwpref.txt", "k", false);
    m_testmake_add(testmake, "altwpref.txt", "bey", true);
    m_testmake_add(testmake, "altwpref.txt", "bye\\-bye, dear friend\\.", true);

    // Testing alternates with suffix

    m_testmake_suite_init(testmake, "altwprsuf.txt", "(l|b)ad");
    m_testmake_add(testmake, "altwprsuf.txt", "l", false);
    m_testmake_add(testmake, "altwprsuf.txt", "b", false);
    m_testmake_add(testmake, "altwprsuf.txt", "ba", false);
    m_testmake_add(testmake, "altwprsuf.txt", "la", false);
    m_testmake_add(testmake, "altwprsuf.txt", "bad", true);
    m_testmake_add(testmake, "altwprsuf.txt", "lad", true);
    m_testmake_add(testmake, "altwprsuf.txt", "blad", false);
    m_testmake_add(testmake, "altwprsuf.txt", "bad things that lad did", true);
    m_testmake_add(testmake, "altwprsuf.txt", "lads doing bad things", true);

    // C's binary number implementation

    m_testmake_suite_init(testmake, "binimpl.txt", "0(b|B)(0|1)+");
    m_testmake_add(testmake, "binimpl.txt", "0", false);
    m_testmake_add(testmake, "binimpl.txt", "b", false);
    m_testmake_add(testmake, "binimpl.txt", "0B", false);
    m_testmake_add(testmake, "binimpl.txt", "0b0", true);
    m_testmake_add(testmake, "binimpl.txt", "0B001", true);
    m_testmake_add(testmake, "binimpl.txt", "0b2", false);
    m_testmake_add(testmake, "binimpl.txt", "0B0101", true);
    m_testmake_add(testmake, "binimpl.txt", "0B111", true);

    // A common string implementation

    m_testmake_suite_init(testmake, "strimpl.txt", "\"([ !#-&\\(-\\[\\]-}]|\\\\[tnr\"\'])*\"");
    m_testmake_add(testmake, "strimpl.txt", "\\\"", false);
    m_testmake_add(testmake, "strimpl.txt", "string", false);
    m_testmake_add(testmake, "strimpl.txt", "\"\"", true);
    m_testmake_add(testmake, "strimpl.txt", "\"string\"", true);
    m_testmake_add(testmake, "strimpl.txt", "\"\\t\"", true);
    m_testmake_add(testmake, "strimpl.txt", "\"\\k\"", false);
    m_testmake_add(testmake, "strimpl.txt", "\"boast\"", true);
    m_testmake_add(testmake, "strimpl.txt", "\"string\"", true);
    m_testmake_add(testmake, "strimpl.txt", "\"\\\"\\\"", true);

    // C's hexadecimal number implementation

    m_testmake_suite_init(testmake, "heximpl.txt", "0(x|X)[0-9A-Fa-f]+");
    m_testmake_add(testmake, "heximpl.txt", "0", false);
    m_testmake_add(testmake, "heximpl.txt", "x", false);
    m_testmake_add(testmake, "heximpl.txt", "0x", false);
    m_testmake_add(testmake, "heximpl.txt", "0x0", true);
    m_testmake_add(testmake, "heximpl.txt", "0XA", true);
    m_testmake_add(testmake, "heximpl.txt", "0xFg", false);
    m_testmake_add(testmake, "heximpl.txt", "0XACC", true);
    m_testmake_add(testmake, "heximpl.txt", "0X100", true);

    // feel free to make more tests

    file = fopen("tests.bat", "w");
	if (file == NULL) {
		strerror(errno);
		exit(EXIT_FAILURE);
	}

    m_testmake_print(testmake, file);
    fclose(file);
}