/*
 * A set of simple self-tests. To use strrep() in your own project,
 * cut or comment out the lines that follow.
 *
 * Otherwise, to compile this file into a test program, do this:
 *
 * cc -o strrep strrep.c
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "vendor/strrep.h"

int main(int argc, char **argv) {
    fprintf(stderr, "Test 1... ");
    const char *s1 = 0;
    const char *s2 = 0;
    const char *s3 = 0;
    assert(strrep(s1, s2, s3) == 0);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 2... ");
    s1 = "";
    s2 = 0;
    s3 = 0;
    assert(strrep(s1, s2, s3) == 0);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 3... ");
    s1 = 0;
    s2 = "";
    s3 = 0;
    assert(strrep(s1, s2, s3) == 0);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 4... ");
    s1 = "";
    s2 = "";
    s3 = 0;
    assert(strrep(s1, s2, s3) == 0);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 5... ");
    s1 = 0;
    s2 = 0;
    s3 = "";
    assert(strrep(s1, s2, s3) == 0);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 6... ");
    s1 = "";
    s2 = 0;
    s3 = "";
    assert(strrep(s1, s2, s3) == 0);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 7... ");
    s1 = 0;
    s2 = "";
    s3 = "";
    assert(strrep(s1, s2, s3) == 0);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 8... ");
    s1 = "";
    s2 = "";
    s3 = "";
    assert(strrep(s1, s2, s3) == s1);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 9... ");
    s1 = "abc";
    s2 = "";
    s3 = "xyz";
    assert(strrep(s1, s2, s3) == s1);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 10... ");
    s1 = "";
    s2 = "abc";
    s3 = "xyz";
    assert(strrep(s1, s2, s3) == s1);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 11... ");
    s1 = "abc";
    s2 = "def";
    s3 = "xyz";
    assert(strrep(s1, s2, s3) == s1);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 12... ");
    s1 = "ab";
    s2 = "abc";
    s3 = "xyz";
    assert(strrep(s1, s2, s3) == s1);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 13... ");
    s1 = "abc";
    s2 = "abc";
    s3 = "xyz";
    char *s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "xyz") == 0);
    assert(s4 != s1);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 14... ");
    s1 = "abc";
    s2 = "a";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "xyzbc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 15... ");
    s1 = "abc";
    s2 = "b";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "axyzc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 15... ");
    s1 = "abc";
    s2 = "c";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "abxyz") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 16... ");
    s1 = "aba";
    s2 = "ab";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "xyza") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 17... ");
    s1 = "bbc";
    s2 = "bc";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "bxyz") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 18... ");
    s1 = "a";
    s2 = "a";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "xyz") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 19... ");
    s1 = "ab";
    s2 = "a";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "xyzb") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 20... ");
    s1 = "ab";
    s2 = "b";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "axyz") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 21... ");
    s1 = "abbc";
    s2 = "ab";
    s3 = "x";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "xbc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 22... ");
    s1 = "abcc";
    s2 = "bc";
    s3 = "x";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "axc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 23... ");
    s1 = "dccd";
    s2 = "cd";
    s3 = "x";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "dcx") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 24... ");
    s1 = "abab";
    s2 = "ab";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "xyzxyz") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 25... ");
    s1 = "abcab";
    s2 = "ab";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "xyzcxyz") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 26... ");
    s1 = "abcabc";
    s2 = "ab";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "xyzcxyzc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 27... ");
    s1 = "cabcab";
    s2 = "ab";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "cxyzcxyz") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 28... ");
    s1 = "cabcabc";
    s2 = "ab";
    s3 = "xyz";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "cxyzcxyzc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 29... ");
    s1 = "abc";
    s2 = "ab";
    s3 = "ab";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "abc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 30... ");
    s1 = "abc";
    s2 = "bc";
    s3 = "bc";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "abc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 31... ");
    s1 = "abcc";
    s2 = "abc";
    s3 = "ab";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "abc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 32... ");
    s1 = "abccc";
    s2 = "bc";
    s3 = "b";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "abcc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 33... ");
    s1 = "abccc";
    s2 = "cc";
    s3 = "c";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "abcc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 34... ");
    s1 = "abcd";
    s2 = "a";
    s3 = "";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "bcd") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 35... ");
    s1 = "abcd";
    s2 = "bc";
    s3 = "";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "ad") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 36... ");
    s1 = "abcd";
    s2 = "d";
    s3 = "";
    s4 = strrep(s1, s2, s3);
    assert(strcmp(s4, "abc") == 0);
    free(s4);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "Test 37... ");
    s1 = "";
    s2 = "";
    s3 = "abc";
    assert(strrep(s1, s2, s3) == s1);
    fprintf(stderr, "pass.\n");

    fprintf(stderr, "All tests pass.\n");

    return 0;
}