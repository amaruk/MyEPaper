#include "unity.h"

void setUp(void)
{}

void tearDown(void)
{}

void testMain(void)
{
	char output[5] = "";
	TEST_ASSERT_EQUAL(3, sprintf(output, "hey"));
	TEST_ASSERT_EQUAL_STRING("hey", output);
	printf("Hello, ceedling fox!\n");
}

