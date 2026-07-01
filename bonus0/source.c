#include <unistd.h>
#include <string.h>
#include <stdio.h>

void p(char *dest, char *msg) {
	char buffer[4096];
	
	puts(msg);
	read(0, buffer, 4096);
	*strchr(buffer, '\n') = '\0';
	strncpy(dest, buffer, 20);
}

void pp(char *dest) {
	char buffer1[20];
	char buffer2[20];

	p(buffer1, " - ");
	p(buffer2, " - ");

	strcpy(dest, buffer1);
	strcat(dest, " ");
	strcat(dest, buffer2);
}

int main(void) {
	char dest[42];

	pp(dest);
	puts(dest);

	return 0;
}
