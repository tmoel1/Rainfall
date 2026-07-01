#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int language = 0;

void greetuser(char *arg) {
	char greeting[72];
	
	if (language == 1) {
		// "Hyvää päivää "
		strcpy(greeting, "Hyvää päivää "); // 18 characters, 19 bytes
	} else if (language == 2) {
		// "Goedemiddag "
		strcpy(greeting, "Goedemiddag ");
	} else if (language == 0) {
		// "Hello "
		strcpy(greeting, "Hello ");
	}
	
	strcat(greeting, arg);
	puts(greeting);
}

int main(int argc, char **argv) {
	char buf[76];
	char *lang;

	if (argc != 3) {
		return 1;
	}

	memset(buf, 0, sizeof(buf)); // Zeroes out the buffer

	// Copies argv[1] up to 40 bytes. If exactly 40 bytes, no null terminator!
	strncpy(buf, argv[1], 40);

	// Copies argv[2] up to 32 bytes adjacent to argv[1] (at offset +40)
	strncpy(buf + 40, argv[2], 32);

	lang = getenv("LANG");
	if (lang != NULL) {
		if (memcmp(lang, "fi", 2) == 0) {
			language = 1;
		} else if (memcmp(lang, "nl", 2) == 0) {
			language = 2;
		}
	}

	greetuser(buf);

	return 0;
}
