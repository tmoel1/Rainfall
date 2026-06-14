#include <stdio.h>
#include <stdlib.h>

int m = 0; // Stored at 0x804988c

void v(void) {
	char buffer[520];

	fgets(buffer, 512, stdin);
	printf(buffer); // Vulnerable to Format String Attack

	if (m == 64) {
		fwrite("Wait what?!\n", 1, 12, stdout);
		system("/bin/sh");
	}
}

int main(void) {
	v();
	return 0;
}
