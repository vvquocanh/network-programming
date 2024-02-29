#include <stdint.h>
#include <stdio.h>

typedef struct Type_A{
	double c1;
	uint32_t c2;
	uint16_t c3;
	char c4[2];
} typeA;

typedef struct Type_B{
 	char c4[2] ;
 	double c1 ;
 	uint16_t c3 ;
 	uint32_t c2 ;
} typeB;

int main() {
	// Different size due to the potential addition of padding bytes for alignment purposes
	printf("Type A struct size: %ld\n", sizeof(typeA));
	printf("Type B struct size: %ld\n", sizeof(typeB));
	return 0;
}
