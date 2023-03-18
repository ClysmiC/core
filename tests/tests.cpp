// TODO
// - Add meaningful error codes to the OpenDdl lib (basically each goto LError should have its own code)
// - Add tests with malformed input and test that we get the error code we'd expect
// - Add test 

#include <cstdio>
#include <cstdlib>

#include "../common.h"


//
// Convenience utilities for tests
//

#define DoTest(EXPRESSION)                                              \
    do {                                                                \
        if (!(EXPRESSION)) {                                            \
            fprintf(stderr, "FAILED: %s(%d): %s\n", __FILE__, __LINE__, #EXPRESSION); \
            return false;                                               \
        }                                                               \
    } while (0)

#define AllTestsPass() printf("PASSED: %s\n", __FILE__); return true;

//
// Test runner
//

int g_cntNew;
int g_cntDelete;

void* operator new[](std::size_t cBytes)
{
    g_cntNew++;
    return malloc(cBytes);
}

void operator delete[](void* ptr)
{
    g_cntDelete++;
    return free(ptr);
}

#define DoTestAuditLeaks() do { DoTest(g_cntNew == g_cntDelete); } while(0)
    
#include "mem.cpp"
#include "array.cpp"

int main()
{
    printf("\n");
    fflush(stdout);

    int cntTest = 0;
    int cntPass = 0;

#define RunTest(TEST) cntPass += TEST(); cntTest++;

    RunTest(TestMemory);
    RunTest(TestDynArray);

#undef RunTest

    fflush(stderr);

    printf("\n");
    
    if (cntPass == cntTest)
    {
        printf("All tests PASSED\n");
    }
    else
    {
        printf("!! Some tests FAILED !!\n");
    }
    
    int cntFail = cntTest - cntPass;
    printf("(%d passed, %d failed, %d total)\n", cntPass, cntFail, cntTest);
        
    
    return 0;
}
