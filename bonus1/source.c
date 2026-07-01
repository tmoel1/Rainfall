#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv) {
	char buffer[40]; // Allocated on stack, esp+0x14
	int size;

	size = atoi(argv[1]);

	if (size > 9) {
		return 1;
	}

	memcpy(buffer, argv[2], size * 4);

	if (size == 0x574f4c46) {
		execl("/bin/sh", "sh", NULL);
	}

	return 0;
}
