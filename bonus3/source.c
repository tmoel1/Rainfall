#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	char buf1[66];
	char buf2[65]; // +0x42 (66 bytes in)
	FILE *fp;

	fp = fopen("/home/user/end/.pass", "r");

	memset(buf1, 0, sizeof(buf1) + sizeof(buf2)); // roughly 0x21 * 4 = 132 bytes

	if (fp == NULL || argc != 2) {
		return -1;
	}

	// Read 66 bytes from the password file
	fread(buf1, 1, 66, fp);
	buf1[65] = '\0'; // ensure null termination

	// Convert argv[1] to an integer
	int index = atoi(argv[1]);
	
	// Nullify a byte in the buffer based on argv[1]
	buf1[index] = '\0';

	// Read another 65 bytes
	fread(buf2, 1, 65, fp);
	fclose(fp);

	// If buf1 is equal to argv[1], we get a shell!
	if (strcmp(buf1, argv[1]) == 0) {
		execl("/bin/sh", "sh", NULL);
	} else {
		puts(buf2);
	}

	return 0;
}
