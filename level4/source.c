#include <stdio.h>
#include <stdlib.h>

int m = 0; // Stored at 0x8049810

void p(char *buffer) {
	printf(buffer); // Vulnerable to Format String Attack
}

void n(void) {
	char buffer[520];

	fgets(buffer, 512, stdin);
	p(buffer);

	if (m == 0x1025544) {
		system("/bin/cat /home/user/level5/.pass");
	}
}

int main(void) {
	n();
	return 0;
}
