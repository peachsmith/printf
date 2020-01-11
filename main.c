#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "my_printf.h"
#include "extract_float.h"


int main(int argc, char** argv)
{
	printf("printf test\n");

	char c = 'a';

	my_printf("%%\n");
	printf("%%\n");

	printf("--------------------------\n");

	// character
	my_printf("char: %c, %c, %c\n", 'A', 'B', 'C');
	printf("char: %c, %c, %c\n", 'A', 'B', 'C');

	printf("--------------------------\n");

	// string
	my_printf("string: %s, %s\n", "Hello", "World!");
	printf("string: %s, %s\n", "Hello", "World!");

	printf("--------------------------\n");

	// signed dec d
	my_printf("signed dec: %i, %i, %i\n", 4, -10, 254);
	printf("signed dec: %i, %i, %i\n", 4, -10, 254);

	printf("--------------------------\n");

	// signed dec i
	my_printf("signed dec: %d, %d, %d\n", 4, -10, 254);
	printf("signed dec: %d, %d, %d\n", 4, -10, 254);

	printf("--------------------------\n");

	// unsigned dec
	my_printf("unsigned dec: %u, %u, %u\n", 4, -10, 254);
	printf("unsigned dec: %u, %u, %u\n", 4, -10, 254);

	printf("--------------------------\n");

	// unsigned hex lower case
	my_printf("hex: %x, %x, %x\n", 4, -10, 254);
	printf("hex: %x, %x, %x\n", 4, -10, 254);

	printf("--------------------------\n");

	// unsigned hex upper case
	my_printf("hex: %X, %X, %X\n", 4, -10, 254);
	printf("hex: %X, %X, %X\n", 4, -10, 254);

	printf("--------------------------\n");

	// signed octal
	my_printf("oct: %o, %o, %o\n", 4, -10, 254);
	printf("oct: %o, %o, %o\n", 4, -10, 254);

	printf("--------------------------\n");

	// pointer
	my_printf("pointer: %p\n", &c);
	printf("pointer: %p\n", &c);

	float f1 = 3.14f;
	float f2 = 33.147f;
	float f3 = 0.33147f;
	float f4 = 45.0f;
	float f5 = 0.0f;
	float f6 = -1.25f;

	// float
	printf("--------------------------\n");

	my_printf("float: %f\n", f1);

	my_printf("float: %f\n", f2);

	my_printf("float: %f\n", f3);

	my_printf("float: %f\n", f4);

	my_printf("float: %f\n", f5);

	my_printf("float: %f\n", f6);

	printf("--------------------------\n");

	// E and e
	my_printf("J E: %E\n", f1);
	printf("M E: %E\n", f1);

	my_printf("J .3E: %.3E\n", f2);
	printf("M .3E: %.3E\n", f2);

	my_printf("J .5E: %.5E\n", f3);
	printf("M .5E: %.5E\n", f3);

	my_printf("J .5E: %.5E\n", 0.00625);
	printf("M .5E: %.5E\n", 0.00625);

	my_printf("J .2E: %.0E\n", 9.5f);
	printf("M .2E: %.0E\n", 9.5f);

	return 0;
}
