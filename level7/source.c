#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

char c[68]; // Stored at 0x8049960

void m(void) {
	printf("%s - %d\n", c, time(NULL));
}

int main(int argc, char **argv) {
	int *struct1;
	int *struct2;

	struct1 = malloc(8);
	struct1[0] = 1;
	struct1[1] = (int)malloc(8);

	struct2 = malloc(8);
	struct2[0] = 2;
	struct2[1] = (int)malloc(8);

	strcpy((char *)struct1[1], argv[1]);
	strcpy((char *)struct2[1], argv[2]);

	FILE *fd = fopen("/home/user/level8/.pass", "r");
	if (fd != NULL) {
		fgets(c, 68, fd);
	}
	
	puts("~~");

	return 0;
}
