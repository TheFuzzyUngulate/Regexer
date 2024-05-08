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
 * @param suite Name of suite in which case is contained
 * @param fptr Pointer to file structure to print into.
 * @param num The index of the current testcase being printed
 * @param INDENT Current indent, for aesthetic reasons.
 */
void m_testcase_print(m_testcase* mcase, char* suite, FILE* fptr, int num, int INDENT)
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
        fprintf(fptr, "echo %s %s testcase#%d, as expected\n",
            suite,
            root->exp ? "passes" : "fails",
            num
        );
        if (root->next) 
            m_testcase_print(mcase->next, suite, fptr, num+1, INDENT+1);
        setspacing(fptr, INDENT);
        fprintf(
            fptr,
            ") ELSE (echo %s unexpectedly %s on testcase#%d)\n",
            suite,
            root->exp ? "fails" : "passes",
            num
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
        m_testcase_print(suite->cases, suite->name, fptr, 1, 0);
        fprintf(fptr, "echo deleting temp files\ndel tmp.exe\ndel tmp.c\n", suite->name);
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
        if (strlen(name) > 1) {
            if (strlen(regex) > 1) {
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

    m_testmake_suite_init(testmake, "t0.txt", "a(a|b)*");
    m_testmake_add(testmake, "t0.txt", "a", true);
    m_testmake_add(testmake, "t0.txt", "b", false);
    m_testmake_add(testmake, "t0.txt", "ab", true);
    m_testmake_add(testmake, "t0.txt", "abab", true);
    m_testmake_add(testmake, "t0.txt", "aaaaab", true);

    m_testmake_suite_init(testmake, "t1.txt", "0(b|B)(0|1)+");
    m_testmake_add(testmake, "t1.txt", "0", false);
    m_testmake_add(testmake, "t1.txt", "b", false);
    m_testmake_add(testmake, "t1.txt", "0B", false);
    m_testmake_add(testmake, "t1.txt", "0b0", true);
    m_testmake_add(testmake, "t1.txt", "0B001", true);
    m_testmake_add(testmake, "t1.txt", "0b2", false);
    m_testmake_add(testmake, "t1.txt", "0B0101", true);
    m_testmake_add(testmake, "t1.txt", "0B111", true);

    // "([ !#-&\(-\[\]-}]|\\[tnr"'])*"

    m_testmake_suite_init(testmake, "t2.txt", "\"([ !#-&\\(-\\[\\]-}]|\\\\[tnr\"\'])*\"");
    m_testmake_add(testmake, "t2.txt", "\\\"", false);
    m_testmake_add(testmake, "t2.txt", "string", false);
    m_testmake_add(testmake, "t2.txt", "\\\"\\\"", true);
    m_testmake_add(testmake, "t2.txt", "\\\"string\\\"", true);
    m_testmake_add(testmake, "t2.txt", "\\\"\\\"\\\"", true);
    m_testmake_add(testmake, "t2.txt", "\\\"friday afternoon\\nyou already know\\n\\\"", true);

    m_testmake_suite_init(testmake, "t3.txt", "0(x|X)[0-9A-Fa-f]+");
    m_testmake_add(testmake, "t3.txt", "0", false);
    m_testmake_add(testmake, "t3.txt", "x", false);
    m_testmake_add(testmake, "t3.txt", "0x", false);
    m_testmake_add(testmake, "t3.txt", "0x0", true);
    m_testmake_add(testmake, "t3.txt", "0XA", true);
    m_testmake_add(testmake, "t3.txt", "0xFg", false);
    m_testmake_add(testmake, "t3.txt", "0XACC", true);
    m_testmake_add(testmake, "t3.txt", "0X100", true);

    m_testmake_suite_init(testmake, "t4.txt", "cat|dog");
    m_testmake_add(testmake, "t4.txt", "cat", true);
    m_testmake_add(testmake, "t4.txt", "dog", true);
    m_testmake_add(testmake, "t4.txt", "at", false);
    m_testmake_add(testmake, "t4.txt", "og", false);

    // feel free to make more tests

    file = fopen("tests.bat", "w");
	if (file == NULL) {
		strerror(errno);
		exit(EXIT_FAILURE);
	}

    m_testmake_print(testmake, file);
    fclose(file);
}