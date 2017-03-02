#include "unity.h"


void setUp(void)

{}



void tearDown(void)

{}



void testMain(void)

{

 char output[5] = "";

 UnityAssertEqualNumber((UNITY_INT)((3)), (UNITY_INT)((sprintf(output, "hey"))), (

((void *)0)

), (UNITY_UINT)(12), UNITY_DISPLAY_STYLE_INT);

 UnityAssertEqualString((const char*)(("hey")), (const char*)((output)), (

((void *)0)

), (UNITY_UINT)(13));

 printf("Hello, ceedling fox!\n");

}
