#include <string.h>
#include <stdio.h>

char s[20] = "ROOTSERVICE";
char *p;
int main(void)
{
    p = strcat(s, "!");
    printf("\n\nRoots service start...\n\n");
    return 0;
}