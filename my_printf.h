#ifndef MY_PRINTF_H
#define MY_PRINTF_H

#include <stdio.h>

/**
 * Writes a character to an output stream.
 * On success, the character written is returned.
 * On failure, EOF is returned and the error indicator is set on stdout.
 *
 * This function may be implemented as a macro, so the expression that
 * evaluates to the stream should not have any side effects.
 *
 * Params:
 *   int - a character to write
 *   FILE* - an output stream
 *
 * Returns:
 *   int - the character written or EOF on failure
 */
int my_putc(int c, FILE* stream);

/**
 * Writes a character to an output stream.
 * On success, the character written is returned.
 * On failure, EOF is returned and the error indicator is set on stdout.
 *
 * Params:
 *   int - a character to write
 *   FILE* - an output stream
 *
 * Returns:
 *   int - the character written or EOF on failure
 */
int my_fputc(int c, FILE* stream);

/**
 * Writes a character to standard output.
 * On success, the character written is returned.
 * On failure, EOF is returned and the error indicator is set on stdout.
 *
 * Params:
 *   int - a character
 *
 * Returns:
 *   int - the character written or EOF on failure
 */
int my_putchar(int c);

/**
 * Writes a formatted string of characters to stdout.
 * The first argument is a format string, and the remaining arguments
 * are values to be converted to strings and inserted into the output.
 * The placement and data type of each value is determined by a format tag.
 * For example, given the string literal "I have %d tomatoes.",
 * a call to printf("I have %d tomatoes.", 4) would write the string
 * "I have 4 tomatoes" to stdout.
 *
 * Format tags have the following structure:
 * %<flags><width><precision><length><specifier>
 *
 * <flags> can be one of the following values:
 *   - left-justify
 *   + prepends a sign onto the result
 *   [space] if no sign will be written, prepends a blank space
 *   # forces preceding 0, 0x or 0X for integers, and decimal point for floats
 *   0 left pads the result with zeros instead of spaces
 *
 * <width> can be one of the following values:
 *   [number] minimum number of characters to be printed
 *   * the width is not in the format tag, rather it is an integer passed
 *     as a preceding argument to the function.
 *
 * <precision> can be one of the following values:
 *   .[number] for integers, this is the minimum number of digits to be
 *     written. For e, E, and f, it's the number of digits to be written
 *     after then decimal point. For g and G, it's maximum number of
 *     significant digits to be printed. For s, it's the maximum number of
 *     characters to be printed.
 *   .* the precision is not in the format tag, rather it is an integer passed
 *     as a preceding argument to the function.
 *
 * <length> can be one of the following values:
 *   h the argument is interpreted as short int or unsigned short int
 *   l the argument is interpreted as a long int or unsigned long int
 *     or as a wide character or wide character string
 *   L the argument is interpreted as a long double
 *
 * Potential format specifiers:
 *   c character
 *   d signed decimal integer
 *   i signed decimal integer
 *   e scientific notation using 'e' character (not implemented)
 *   E scientific notation using 'E' character (not implemented)
 *   f decimal floating point (not implemented)
 *   g uses shorter of e or f (not implemented)
 *   G uses shorter of E or f (not implemented)
 *   o signed octal
 *   s string of characters
 *   u unsigned decimal integer
 *   x unsigned hexadecimal integer (lower case letter)
 *   X unsigned hexadecimal integer (capital letters)
 *   p pointer address
 *   n nothing printed
 *   % the '%' character
 *
 * Params:
 *   const char* - a pointer to a string
 *   va_list - a list of arguments to be converted to strings
 *
 * Returns:
 *   int - the number of characters written, or -1 on failure
 */
int my_printf(const char* fmt, ...);

#endif
