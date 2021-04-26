#include <stdio.h>
#include <string.h>

int main()
{
	char* str = "My name is yoav";
	char* needle = "yoav";
	
	printf("%p\n", str);
	printf("%p\n", strstr(str, needle));

	return 0;
}
