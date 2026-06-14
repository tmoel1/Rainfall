#include <stdlib.h>
#include <unistd.h>
#include <string.h>

class N {
public:
	int value;
	char annotation[100];
	
	N(int v) {
		value = v;
	}
	
	virtual int operator+(N& right) {
		return value + right.value;
	}

	virtual int operator-(N& right) {
		return value - right.value;
	}

	void setAnnotation(char *str) {
		memcpy(annotation, str, strlen(str)); // Buffer overflow vulnerability!
	}
};

int main(int argc, char **argv) {
	if (argc <= 1) {
		_exit(1);
	}
	
	N *obj1 = new N(5);
	N *obj2 = new N(6);

	obj1->setAnnotation(argv[1]);
	
	return (*obj2 + *obj1); // It incorrectly passes obj1 as left operand, so it actually calls obj2->operator+(obj1)
}
