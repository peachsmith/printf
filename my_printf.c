#include "my_printf.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

// format flag bit flags
#define FMT_LEFT   0x01 /* -                               */
#define FMT_SIGN   0x02 /* +                               */
#define FMT_SPACE  0x04 /* [space]                         */
#define FMT_POINT  0x08 /* #                               */
#define FMT_ZERO   0x10 /* 0                               */
#define FMT_WIDTH  0x20 /* width is passed as argument     */
#define FMT_PREC   0x40 /* precision is passed as argument */
#define FMT_ZPREC  0x40 /* precision is zero               */

// format specifiers
#define SPEC_c 'c'
#define SPEC_s 's'
#define SPEC_d 'd'
#define SPEC_i 'i'
#define SPEC_u 'u'
#define SPEC_f 'f'
#define SPEC_e 'e'
#define SPEC_E 'E'
#define SPEC_g 'g'
#define SPEC_G 'G'
#define SPEC_o 'o'
#define SPEC_x 'x'
#define SPEC_X 'X'
#define SPEC_p 'p'
#define SPEC_n 'n'
#define SPEC_per '%'

// format length bit flags
#define LEN_h 0x01 /* h */
#define LEN_l 0x02 /* l */
#define LEN_L 0x04 /* L */

// format tag parser states
#define STATE_FLAGS  1
#define STATE_WIDTH  2
#define STATE_PREC   3
#define STATE_LENGTH 4
#define STATE_SPEC   5
#define STATE_DONE   6

/**
 * Determines if a character within a format tag is a valid format flag.
 * If the character is a format flag, then a flag variable is set to have
 * the value of one of the format bit flags.
 * If the character is not a valid flag value, the variable is set to 0.
 *
 * Params:
 *   c - a character within a format tag
 *   f - a variable to contain the bit flag
 */
#define is_flag(c, f) (     \
    f = c == '-' ? FMT_LEFT \
    : c == '+' ? FMT_SIGN   \
    : c == ' ' ? FMT_SPACE  \
    : c == '#' ? FMT_POINT  \
    : c == '0' ? FMT_ZERO   \
    : 0                     \
)

/**
 * Determines if a character within a format tag is a valid length value.
 * If the character is a valid length value, then a flag variable is set to
 * have the value of one of the length bit flags.
 * If the character is not a valid length value, the variable is set to 0.
 *
 * Params:
 *   c - a character within a format tag
 *   l - a variable to contain the bit flag
 */
#define is_len(c, l) (   \
    l = c == 'h' ? LEN_h \
    : c == 'l' ? LEN_l   \
    : c == 'L' ? LEN_L   \
    : 0                  \
)

/**
 * Determines if a character within a format tag is a valid specifier.
 * If the character is a valid specifier, then a variable is set with the
 * value of that specifier.
 * If the character is not a valid specifier, the variable is set to 0.
 *
 * Params:
 *   c - a character within a format tag
 *   s - a variable to contain the specifier character
 */
#define is_spec(c, s) (   \
    s = c == 'c' ? SPEC_c \
    : c == 's' ? SPEC_s   \
    : c == 'd' ? SPEC_d   \
    : c == 'i' ? SPEC_i   \
    : c == 'u' ? SPEC_u   \
    : c == 'f' ? SPEC_f   \
    : c == 'e' ? SPEC_e   \
    : c == 'E' ? SPEC_E   \
    : c == 'g' ? SPEC_g   \
    : c == 'G' ? SPEC_G   \
    : c == 'o' ? SPEC_o   \
    : c == 'x' ? SPEC_x   \
    : c == 'X' ? SPEC_X   \
    : c == 'p' ? SPEC_p   \
    : c == 'n' ? SPEC_n   \
    : c == '%' ? SPEC_per \
    : 0                   \
)

#define FLOAT_SGN_BIT(n) ((n & 0x80000000) >> 31)
#define FLOAT_EXP_BIT(n) (((n & 0x7F800000) >> 23) - 0x7F)
#define FLOAT_MNT_BIT(n) ((n & 0x007FFFFF) | 0x00800000)

// maximum exponent absolute value
#define FLOAT_EXP_N 151

// maximum digits in binary number component
#define FLOAT_BIN_DIG 152


#define DOUBLE_SGN_BIT(n) ((n & 0x8000000000000000) >> 63)
#define DOUBLE_EXP_BIT(n) (((n & 0x7FF0000000000000) >> 52) - 0x3FF)
#define DOUBLE_MNT_BIT(n) ((n & 0xFFFFFFFFFFFFF) | 0x10000000000000)

// maximum exponent absolute value
#define DOUBLE_EXP_N 1200

// maximum digits in binary number component
#define DOUBLE_BIN_DIG 1200


/**
 * A format tag within a format string.
 */
typedef struct ftag {
	unsigned char flags;
	size_t width;
	size_t prec;
	unsigned char len;
	char spec;
}ftag;

/**
 * An IEEE 754 single-precision floating point number.
 */
typedef struct ieee_754_float {
	uint32_t raw;  // raw, 32-bit binary data
	uint8_t sign;  // sign (0 for positive, 1 for negative)
	int8_t exp;    // exponent
	uint32_t mant; // mantissa
}ieee_754_float;

/**
 * An IEEE 754 double-precision floating point number.
 */
typedef struct ieee_754_double {
	uint64_t raw;  // raw, 32-bit binary data
	uint8_t sign;  // sign (0 for positive, 1 for negative)
	int16_t exp;   // exponent
	uint64_t mant; // mantissa
}ieee_754_double;




/**
 * Converts a float to a uint32_t by moving the raw binary data into an
 * integer register.
 * This particular implementation is intended for 64-bit Microsoft Windows
 * systems running on an x86 architecture.
 *
 * Params:
 *   float - a 32-bit IEEE 754 single-precision floating point number
 *
 * Returns:
 *   uint32_t - the raw binary data of the floating point number
 */
#ifdef _WIN64
extern uint32_t extract_float_win64(float f);
#endif

/**
 * Converts a double to a uint64_t by moving the raw binary data into an
 * integer register.
 * This particular implementation is intended for 64-bit Microsoft Windows
 * systems running on an x86 architecture.
 *
 * Params:
 *   double - a 64-bit IEEE 754 double-precision floating point number
 *
 * Returns:
 *   uint32_t - the raw binary data of the floating point number
 */
#ifdef _WIN64
extern uint64_t extract_double_win64(double d);
#endif

/**
 * Extracts the raw binary components of a 32-bit IEEE 754 single-precision
 * floating point number.
 *
 * Params:
 *   float - a 32-bit IEEE 754 single-precision floating point number
 *
 * Returns:
 *   ieee_754_float - the individual binary components of the number
 */
static ieee_754_float extract_float(float f);

/**
 * Extracts the raw binary components of a 64-bit IEEE 754 double-precision
 * floating point number.
 *
 * Params:
 *   double - a 64-bit IEEE 754 double-precision floating point number
 *
 * Returns:
 *   ieee_754_double - the individual binary components of the number
 */
static ieee_754_double extract_double(double d);

/**
 * Reverses the order of a character array.
 * Given a length of l, only the first l characters will be reversed.
 * Any characters after that in the buffer will remain unmoved.
 *
 * Params:
 *   char* - a pointer to a character array
 *   size_t - the number of characters to reverse
 */
static void reverse(char* str, size_t len);

/**
 * Converts an integer to a character array.
 * A buffer is populate with a NUL-terminated string of characters.
 * The returned value is the number of characters in the output buffer
 * not including the NUL character, or 0 on failure.
 *
 * This function is inteded to be used with printf.
 * As such, if the base is 16, then it casts the integer as an unsigned
 * integer, since the %x and %X specifiers are meant for unsigned
 * hexadecimal numbers.
 *
 * Params:
 *   int - an integer
 *   char* - a character buffer
 *   int - the base (i.e. 8, 10, or 16)
 *   int - capitalization flag (0 if lower case, or 1 for upper case)
 *   int - sign flag (0 for unsigned, 1 for signed)
 *
 * Returns:
 *   size_t - the number of characters in the output buffer
 */
static size_t int_to_str(int n, char* buffer, int radix, int cap, int sign);

/**
 * Converts an unsigned pointer to a character array.
 * A buffer is populate with a NUL-terminated string of characters.
 * The returned value is the number of characters in the output buffer
 * not including the NUL character, or 0 on failure.
 *
 * This function is inteded to be used with printf.
 *
 * Params:
 *   uintptr_t - an unsigned pointer
 *   char* - a character buffer
 *   int - capitalization flag (0 if lower case, or 1 for upper case)
 *
 * Returns:
 *   size_t - the number of characters in the output buffer
 */
static size_t uintptr_to_str(uintptr_t n, char* buffer, int cap);

/**
 * Converts a string of characters into an unsigned integral integer.
 * This function scans a character array looking for decimal digits and parses
 * them until it encounters a character outside of the range of
 * '0' through '9'.
 * The first argument is intended to be a string of unsigned decimal integer
 * digits that may occur within a larger string.
 * The second argument is a reference to a memory location that will receive
 * the number that resulted from the conversion.
 *
 *
 * Params:
 *   const char* - a pointer to a sequence of characters
 *   size_t* - a pointer to a size_t to store the resulting number
 *
 * Returns:
 *   size_t - the number of digits in the resulting number
 */
static size_t parse_udec(const char* str, size_t* res);

/**
 * Parses a format tag within a format string for printf.
 * This function is intended to be called while traversing a string.
 * The first argument will be a pointer to the current position within
 * that string. The second argument is a reference to a pointer to the
 * new position in the string.
 *
 * Params:
 *   const char* start - a pointer to the starting index in the string
 *   char** - a reference to a pointer that contains the new position
 *     in the original string.
 *
 * Returns:
 *   ftag - a format tag structure
 */
static ftag parse_format(const char* start, char** end);




static ieee_754_float extract_float(float f)
{
	ieee_754_float comp;
	uint32_t raw;

#ifdef _WIN64
	raw = extract_float_win64(f);
#endif

	comp.raw = raw;
	comp.sign = FLOAT_SGN_BIT(raw);
	comp.exp = FLOAT_EXP_BIT(raw);
	comp.mant = FLOAT_MNT_BIT(raw);

	// Remove the implicit 1 bit for denormalized numbers.
	if (((raw & 0x7F800000) >> 23) == 0)
	{
		comp.mant ^= 0x00800000;
	}

	return comp;
}

static ieee_754_double extract_double(double d)
{
	ieee_754_double comp;
	uint64_t raw;

#ifdef _WIN64
	raw = extract_double_win64(d);
#endif

	comp.raw = raw;
	comp.sign = DOUBLE_SGN_BIT(raw);
	comp.exp = DOUBLE_EXP_BIT(raw);
	comp.mant = DOUBLE_MNT_BIT(raw);

	// Remove the implicit 1 bit for denormalized numbers.
	if (((raw & 0x7FF0000000000000) >> 52) == 0)
	{
		comp.mant ^= 0x10000000000000;
	}

	return comp;
}

static void reverse(char* str, size_t len)
{
	size_t start;
	size_t end;
	char tmp;

	start = 0;
	end = len - 1;

	while (start < end)
	{
		tmp = str[start];
		str[start] = str[end];
		str[end] = tmp;
		start++;
		end--;
	}
}

static size_t int_to_str(int n, char* buffer, int radix, int cap, int sign)
{
	size_t i;       // index
	int neg;        // negative flag
	int r;          // remainder
	unsigned int h; // hex integer

	// Ensure that the buffer is not a NULL pointer
	if (buffer == NULL)
		return 0;

	// The only acceptable bases are 8, 10, and 16
	if (radix != 8 && radix != 10 && radix != 16)
		return 0;

	i = 0;
	neg = 0;

	if (n == 0)
	{
		buffer[i++] = '0';
		buffer[i] = '\0';
		return 1;
	}

	h = (unsigned int)n;

	if (n < 0 && radix != 16 && sign)
	{
		n = -n;
		neg = 1;
	}

	if (radix == 16 || !sign)
	{
		while (h)
		{
			r = h % radix;
			buffer[i++] = r > 9 ? r - 10 + (cap ? 'A' : 'a') : r + '0';
			h /= radix;
		}
	}
	else
	{
		while (n)
		{
			r = n % radix;
			buffer[i++] = r > 9 ? r - 10 + (cap ? 'A' : 'a') : r + '0';
			n /= radix;
		}
	}

	if (neg)
		buffer[i++] = '-';

	buffer[i] = '\0';

	reverse(buffer, i);

	return i;
}

static size_t size_to_str(size_t n, char* buffer, int radix, int cap)
{
	size_t i;       // index
	int neg;        // negative flag
	int r;          // remainder

	// Ensure that the buffer is not a NULL pointer
	if (buffer == NULL)
		return 0;

	// The only acceptable bases are 8, 10, and 16
	if (radix != 8 && radix != 10 && radix != 16)
		return 0;

	i = 0;
	neg = 0;

	if (n == 0)
	{
		buffer[i++] = '0';
		buffer[i] = '\0';
		return 1;
	}

	if (radix == 16)
	{
		while (n)
		{
			r = n % radix;
			buffer[i++] = r > 9 ? r - 10 + (cap ? 'A' : 'a') : r + '0';
			n /= radix;
		}
	}
	else
	{
		while (n)
		{
			r = n % radix;
			buffer[i++] = r > 9 ? r - 10 + (cap ? 'A' : 'a') : r + '0';
			n /= radix;
		}
	}

	buffer[i] = '\0';

	reverse(buffer, i);

	return i;
}

static size_t uintptr_to_str(uintptr_t n, char* buffer, int cap)
{
	size_t i;       // index
	int neg;        // negative flag
	int r;          // remainder

	// Ensure that the buffer is not a NULL pointer
	if (buffer == NULL)
		return 0;

	i = 0;
	neg = 0;

	if (n == 0)
	{
		buffer[i++] = '0';
		buffer[i] = '\0';
		return 1;
	}

	while (n)
	{
		r = n % 16;
		buffer[i++] = r > 9 ? r - 10 + (cap ? 'A' : 'a') : r + '0';
		n /= 16;
	}

	if (neg)
		buffer[i++] = '-';

	buffer[i] = '\0';

	reverse(buffer, i);

	return i;
}

static size_t parse_udec(const char* str, size_t* res)
{
	size_t count, n, tmp, err;

	count = n = err = 0;

	while (*str >= '0' && *str <= '9' && !err)
	{
		tmp = (size_t)(*str) - '0';

		if (n == 0 && count == 1)
		{
			err = 1;
		}
		else
		{
			n = n * 10 + tmp;

			str += sizeof(char);
			count++;
		}
	}

	*res = n;

	return count;
}




static ftag parse_format(const char* start, char** end)
{
	size_t n;           // conversion result
	size_t d;           // index change
	int state;          // current parser state
	unsigned char flag; // flag
	unsigned char len;  // length
	char spec;          // specifier
	ftag tag;           // format tag
	size_t e;           // element size

	tag.flags = 0;
	tag.width = 0;
	tag.prec = 0;
	tag.len = 0;
	tag.spec = 0;

	state = STATE_FLAGS;
	e = sizeof(char);

	while (*start != '\0' && state != STATE_DONE)
	{
		switch (state)
		{
		case STATE_FLAGS:
		{
			if (is_flag(*start, flag))
			{
				tag.flags |= flag;
				start++;
			}
			else
				state++;
		}
		break;

		case STATE_WIDTH:
		{
			if (*start == '*')
			{
				tag.flags |= FMT_WIDTH;
				start++;
			}
			else
			{
				d = parse_udec(start, &n);
				tag.width = n;
				start += e * d;
			}

			state++;
		}
		break;

		case STATE_PREC:
		{
			if (*start == '.')
			{
				start += e;
				if (*start == '*')
				{
					tag.flags |= FMT_PREC;
					start += e;
				}
				else if (*start >= '0' && *start <= '9')
				{
					d = parse_udec(start, &n);
					tag.prec = n;
					tag.flags |= FMT_ZPREC;
					start += e * d;
				}
				else
				{
					tag.prec = 0;
					tag.flags |= FMT_ZPREC;
				}
			}

			state++;
		}
		break;

		case STATE_LENGTH:
		{
			if (is_len(*start, len))
			{
				tag.len |= len;
				start += e;
			}
			else
				state++;
		}
		break;

		case STATE_SPEC:
			tag.spec = is_spec(*start, spec) ? spec : 0;
			state++;
			break;

		default:
			break;
		}
	}

	// Update the end position
	*end = (char*)start;

	return tag;
}

FILE* my_get_stdout()
{
	return stdout;
}


/**
 * Converts a binary fraction value to decimal.
 * The first argument is a pointer to a character array containing
 * all of the digits to the right of the radix point in a binary number.
 * So for the given binary number 110.010, the input string would be "010",
 * and the expected output would be "25".
 * The second argument is a pointer to the output buffer.
 * The output buffer is assumed to be large enough to hold at least 152
 * 8-bit numbers.
 *
 * Params:
 *   const char* - a pointer to a character array of '1' and '0'
 *   uint8_t* - a pointer to the output buffer
 */
static size_t fbin_to_fdec_frac(const char* raw, uint8_t* frac)
{
	size_t i, j, k, c;              // indices
	uint8_t conv[2][FLOAT_BIN_DIG]; // binary to decimal conversion array
	uint8_t dec[FLOAT_BIN_DIG];     // decimal number
	uint8_t next;                   // next digit in conversion array
	uint8_t half;                   // half of the next conversion digit
	size_t n;                       // decimal digit counter

	// Prepopulate the fraction evaluator arrays with 0
	for (i = 0; i < FLOAT_BIN_DIG; i++)
	{
		dec[i] = 0;
		conv[0][i] = 0;
		conv[1][i] = 0;
	}

	// Give the conversion array a starting value
	conv[0][0] = 5;

	i = j = k = 0;

	while (*raw != '\0' && i < FLOAT_EXP_N)
	{
		// If the current binary digit is 1, add the
		// conversion result to the current decimal value.
		if (*raw == '1')
		{
			for (j = 0; j < FLOAT_BIN_DIG; j++)
			{
				dec[j] += conv[k][j];

				// Handle carries
				if (dec[j] >= 10)
				{
					for (c = j; c > 0 && dec[c] >= 10; c--)
					{
						dec[c - 1]++;
						dec[c] = dec[c] - 10;
					}
				}
			}
		}

		// Calculate the decimal equivalent of the next binary digit
		for (j = 0; j < FLOAT_BIN_DIG && i < FLOAT_EXP_N - 1; j++)
		{
			next = conv[k][j];
			half = next / 2;

			conv[k][j] = 0;

			if (half > 0 && !(next & 1))
				conv[k ? 0 : 1][j] += half;

			else if (j < FLOAT_BIN_DIG - 1)
			{
				conv[k ? 0 : 1][j] += half;
				conv[k ? 0 : 1][j + 1] += next > 0 ? 5 : 0;
			}
		}

		k = k ? 0 : 1;
		i++;
		raw++;
	}

	// Determine the number of digits in the final decimal number
	for (n = FLOAT_BIN_DIG - 1; dec[n] == 0 && n > 0; n--);

	n++;

	for (i = 0; i < n; i++)
		frac[i] = dec[i];

	return n;
}


/**
 * Converts a binary whole number to decimal.
 * The first argument is a pointer to a character array containing
 * all of the digits to the left of the radix point in a binary number.
 * So for the given binary number 110.010, the input string would be "110",
 * and the expected output would be "6".
 * The second argument is a pointer to the output buffer.
 * The output buffer is assumed to be large enough to hold at least 152
 * 8-bit numbers.
 *
 * Params:
 *   const char* - a pointer to a character array of '1' and '0'
 *   uint8_t* - a pointer to the output buffer
 *
 * Returns:
 *   size_t - the number of values written to the output buffer
 */
static size_t fbin_to_fdec_whole(const char* raw, uint8_t* whole)
{
	size_t i, j, k, c;              // indices
	uint8_t conv[2][FLOAT_BIN_DIG]; // binary to decimal conversion array
	uint8_t dec[FLOAT_BIN_DIG];     // decimal number
	uint8_t next;                   // next digit in conversion array
	uint8_t doub;                   // double of the next conversion digit
	size_t len;                     // length of the raw string
	size_t l;                       // length counter
	size_t n;                       // decimal digit counter

	len = strlen(raw);
	l = len;

	// Prepopulate the arrays with 0
	for (i = 0; i < FLOAT_BIN_DIG; i++)
	{
		dec[i] = 0;
		conv[0][i] = 0;
		conv[1][i] = 0;
	}

	// Give the conversion array a starting value
	conv[0][FLOAT_BIN_DIG - 1] = 1;

	i = j = k = n = 0;

	while (l > 0 && i < FLOAT_EXP_N)
	{
		// If the current binary digit is 1, add the
		// conversion result to the current decimal value.
		if (raw[l - 1] == '1')
		{
			for (j = FLOAT_BIN_DIG - 1; j > 0; j--)
			{
				dec[j] += conv[k][j];

				// Handle carries
				if (dec[j] >= 10)
				{
					for (c = j; c > 0 && dec[c] >= 10; c--)
					{
						dec[c - 1]++;
						dec[c] = dec[c] - 10;
					}
				}
			}
		}

		// Calculate the decimal equivalent of the next binary digit
		for (j = FLOAT_BIN_DIG - 1; j > 0 && i < FLOAT_EXP_N - 1; j--)
		{
			next = conv[k][j];
			doub = next * 2;

			conv[k][j] = 0;

			conv[k ? 0 : 1][j] += doub;

			// Handle carries
			if (conv[k ? 0 : 1][j] >= 10)
			{
				for (c = j; c > 0 && conv[k ? 0 : 1][c] >= 10; c--)
				{
					conv[k ? 0 : 1][c - 1]++;
					conv[k ? 0 : 1][c] = conv[k ? 0 : 1][c] - 10;
				}
			}
		}

		k = k ? 0 : 1;
		i++;
		l--;
	}

	// Determine the number of digits in the final decimal number
	for (n = 0; dec[n] == 0 && n < FLOAT_BIN_DIG; n++);

	for (i = n; i < FLOAT_BIN_DIG; i++)
		whole[i - n] = dec[i];

	return FLOAT_BIN_DIG - n;
}

static void float_to_str(float f,
	uint8_t* whole,
	size_t* w_res,
	uint8_t* frac,
	size_t* f_res)
{
	ieee_754_float ieeef;
	char mstr[25];
	int16_t i, j;
	uint8_t li;
	uint8_t ri;
	uint8_t rad;
	char left[FLOAT_BIN_DIG];
	char right[FLOAT_BIN_DIG];


	// Extract the binary components of the float.
	ieeef = extract_float(f);

	// Prepopulate the character arrays with '\0'.
	for (i = 0; i < FLOAT_BIN_DIG; i++)
	{
		left[i] = '\0';
		right[i] = '\0';
		frac[i] = 0;
		whole[i] = 0;
	}

	// Convert the mantissa to a NUL-terminated string.
	for (i = 23, j = 0; i >= 0; i--)
	{
		mstr[j++] = ((ieeef.mant & (1 << i)) >> i) ? '1' : '0';
	}
	mstr[24] = '\0';

	rad = li = ri = 0;

	// If the exponent is negative, add leading zeros '0'
	// to the right side array.
	if (ieeef.exp + 1 < 0)
	{
		rad = 1;
		for (i = 0; i < -(ieeef.exp + 1); i++)
			right[ri++] = '0';
	}

	for (i = 0; i < 160; i++)
	{
		if (i == ieeef.exp + 1)
			rad = 1;

		if (i < 24)
		{
			// Get the characters from the mantissa string.
			if (rad)
				right[ri++] = mstr[i];

			else
				left[li++] = mstr[i];
		}
		else if (i < ieeef.exp + 1)
		{
			// Since we've used all the characters from the
			// mantissa string, we start outputting zeros '0'.
			if (rad)
				right[ri++] = '0';

			else
				left[li++] = '0';
		}
	}

	*w_res = fbin_to_fdec_whole(left, whole);
	*f_res = fbin_to_fdec_frac(right, frac);
}


/**
 * Converts a binary fraction value to decimal.
 * The first argument is a pointer to a character array containing
 * all of the digits to the right of the radix point in a binary number.
 * So for the given binary number 110.010, the input string would be "010",
 * and the expected output would be "25".
 * The second argument is a pointer to the output buffer.
 * The output buffer is assumed to be large enough to hold at least 152
 * 8-bit numbers.
 *
 * Params:
 *   const char* - a pointer to a character array of '1' and '0'
 *   uint8_t* - a pointer to the output buffer
 */
static size_t dbin_to_ddec_frac(const char* raw, uint8_t* frac)
{
	size_t i, j, k, c;              // indices
	uint8_t conv[2][DOUBLE_BIN_DIG]; // binary to decimal conversion array
	uint8_t dec[DOUBLE_BIN_DIG];     // decimal number
	uint8_t next;                   // next digit in conversion array
	uint8_t half;                   // half of the next conversion digit
	size_t n;                       // decimal digit counter

	// Prepopulate the fraction evaluator arrays with 0
	for (i = 0; i < DOUBLE_BIN_DIG; i++)
	{
		dec[i] = 0;
		conv[0][i] = 0;
		conv[1][i] = 0;
	}

	// Give the conversion array a starting value
	conv[0][0] = 5;

	i = j = k = 0;

	while (*raw != '\0' && i < DOUBLE_EXP_N)
	{
		// If the current binary digit is 1, add the
		// conversion result to the current decimal value.
		if (*raw == '1')
		{
			for (j = 0; j < DOUBLE_BIN_DIG; j++)
			{
				dec[j] += conv[k][j];

				// Handle carries
				if (dec[j] >= 10)
				{
					for (c = j; c > 0 && dec[c] >= 10; c--)
					{
						dec[c - 1]++;
						dec[c] = dec[c] - 10;
					}
				}
			}
		}

		// Calculate the decimal equivalent of the next binary digit
		for (j = 0; j < DOUBLE_BIN_DIG && i < DOUBLE_EXP_N - 1; j++)
		{
			next = conv[k][j];
			half = next / 2;

			conv[k][j] = 0;

			if (half > 0 && !(next & 1))
				conv[k ? 0 : 1][j] += half;

			else if (j < DOUBLE_BIN_DIG - 1)
			{
				conv[k ? 0 : 1][j] += half;
				conv[k ? 0 : 1][j + 1] += next > 0 ? 5 : 0;
			}
		}

		k = k ? 0 : 1;
		i++;
		raw++;
	}

	// Determine the number of digits in the final decimal number
	for (n = DOUBLE_BIN_DIG - 1; dec[n] == 0 && n > 0; n--);

	n++;

	for (i = 0; i < n; i++)
		frac[i] = dec[i];

	return n;
}


/**
 * Converts a binary whole number to decimal.
 * The first argument is a pointer to a character array containing
 * all of the digits to the left of the radix point in a binary number.
 * So for the given binary number 110.010, the input string would be "110",
 * and the expected output would be "6".
 * The second argument is a pointer to the output buffer.
 * The output buffer is assumed to be large enough to hold at least 152
 * 8-bit numbers.
 *
 * Params:
 *   const char* - a pointer to a character array of '1' and '0'
 *   uint8_t* - a pointer to the output buffer
 *
 * Returns:
 *   size_t - the number of values written to the output buffer
 */
static size_t dbin_to_ddec_whole(const char* raw, uint8_t* whole)
{
	size_t i, j, k, c;              // indices
	uint8_t conv[2][DOUBLE_BIN_DIG]; // binary to decimal conversion array
	uint8_t dec[DOUBLE_BIN_DIG];     // decimal number
	uint8_t next;                   // next digit in conversion array
	uint8_t doub;                   // double of the next conversion digit
	size_t len;                     // length of the raw string
	size_t l;                       // length counter
	size_t n;                       // decimal digit counter

	len = strlen(raw);
	l = len;

	// Prepopulate the arrays with 0
	for (i = 0; i < DOUBLE_BIN_DIG; i++)
	{
		dec[i] = 0;
		conv[0][i] = 0;
		conv[1][i] = 0;
	}

	// Give the conversion array a starting value
	conv[0][DOUBLE_BIN_DIG - 1] = 1;

	i = j = k = n = 0;

	while (l > 0 && i < DOUBLE_EXP_N)
	{
		// If the current binary digit is 1, add the
		// conversion result to the current decimal value.
		if (raw[l - 1] == '1')
		{
			for (j = DOUBLE_BIN_DIG - 1; j > 0; j--)
			{
				dec[j] += conv[k][j];

				// Handle carries
				if (dec[j] >= 10)
				{
					for (c = j; c > 0 && dec[c] >= 10; c--)
					{
						dec[c - 1]++;
						dec[c] = dec[c] - 10;
					}
				}
			}
		}

		// Calculate the decimal equivalent of the next binary digit
		for (j = DOUBLE_BIN_DIG - 1; j > 0 && i < DOUBLE_EXP_N - 1; j--)
		{
			next = conv[k][j];
			doub = next * 2;

			conv[k][j] = 0;

			conv[k ? 0 : 1][j] += doub;

			// Handle carries
			if (conv[k ? 0 : 1][j] >= 10)
			{
				for (c = j; c > 0 && conv[k ? 0 : 1][c] >= 10; c--)
				{
					conv[k ? 0 : 1][c - 1]++;
					conv[k ? 0 : 1][c] = conv[k ? 0 : 1][c] - 10;
				}
			}
		}

		k = k ? 0 : 1;
		i++;
		l--;
	}

	// Determine the number of digits in the final decimal number
	for (n = 0; dec[n] == 0 && n < DOUBLE_BIN_DIG; n++);

	for (i = n; i < DOUBLE_BIN_DIG; i++)
		whole[i - n] = dec[i];

	return DOUBLE_BIN_DIG - n;
}

static void double_to_str(double d,
	uint8_t* whole,
	size_t* w_res,
	uint8_t* frac,
	size_t* f_res)
{
	ieee_754_double ieeed;
	char mstr[54];
	int16_t i, j;
	uint8_t li;
	uint8_t ri;
	uint8_t rad;
	char left[DOUBLE_BIN_DIG];
	char right[DOUBLE_BIN_DIG];


	// Extract the binary components of the float.
	ieeed = extract_double(d);

	// Prepopulate the character arrays with '\0'.
	for (i = 0; i < DOUBLE_BIN_DIG; i++)
	{
		left[i] = '\0';
		right[i] = '\0';
		frac[i] = 0;
		whole[i] = 0;
	}

	// Convert the mantissa to a NUL-terminated string.
	for (i = 52, j = 0; i >= 0; i--)
	{
		mstr[j++] = ((ieeed.mant & ((uint64_t)1 << i)) >> i) ? '1' : '0';
	}
	mstr[53] = '\0';

	rad = li = ri = 0;

	// If the exponent is negative, add leading zeros '0'
	// to the right side array.
	if (ieeed.exp + 1 < 0)
	{
		rad = 1;
		for (i = 0; i < -(ieeed.exp + 1); i++)
			right[ri++] = '0';
	}

	for (i = 0; i < 160; i++)
	{
		if (i == ieeed.exp + 1)
			rad = 1;

		if (i < 24)
		{
			// Get the characters from the mantissa string.
			if (rad)
				right[ri++] = mstr[i];

			else
				left[li++] = mstr[i];
		}
		else if (i < ieeed.exp + 1)
		{
			// Since we've used all the characters from the
			// mantissa string, we start outputting zeros '0'.
			if (rad)
				right[ri++] = '0';

			else
				left[li++] = '0';
		}
	}

	*w_res = dbin_to_ddec_whole(left, whole);
	*f_res = dbin_to_ddec_frac(right, frac);
}




//--------------------------------------------------------------------------//
//                               Public API                                 //
//--------------------------------------------------------------------------//

int my_putc(int c, FILE* stream)
{
	return fputc(c, stream);
}

int my_fputc(int c, FILE* stream)
{
	return fputc(c, stream);
}

int my_putchar(int c)
{
	return my_fputc(c, my_get_stdout());
}

int my_printf(const char* fmt, ...)
{
	size_t i, j;   // index
	char* end;     // updated character pointer
	int p;         // flag indicating detection of '%'
	va_list argp;  // argument pointer
	char buf[100]; // string conversion buffer
	size_t len;    // string length
	int err;

	va_start(argp, fmt);

	i = j = 0;
	p = 0;
	err = 0;
	while (*fmt != '\0' && !err)
	{
		if (*fmt == '%')
		{
			fmt++;
			ftag t = parse_format(fmt, &end);
			fmt = end;

			if (t.spec == SPEC_c)
			{
				char c = va_arg(argp, int);
				my_putchar(c);
			}
			else if (t.spec == SPEC_s)
			{
				char* s = va_arg(argp, char*);
				size_t len = strlen(s);
				for (i = 0; i < len; i++)
				{
					my_putchar(s[i]);
				}
			}
			else if (t.spec == SPEC_d || t.spec == SPEC_i)
			{
				int n = va_arg(argp, int);
				len = int_to_str(n, buf, 10, 0, 1);

				for (i = 0; i < len; i++)
				{
					my_putchar(buf[i]);
				}
			}
			else if (t.spec == SPEC_u)
			{
				unsigned int n = va_arg(argp, unsigned int);
				len = int_to_str(n, buf, 10, 0, 0);

				for (i = 0; i < len; i++)
				{
					my_putchar(buf[i]);
				}
			}
			else if (t.spec == SPEC_X || t.spec == SPEC_x)
			{
				int n = va_arg(argp, int);
				int cap = t.spec == SPEC_X ? 1 : 0;
				len = int_to_str(n, buf, 16, cap, 0);

				for (i = 0; i < len; i++)
				{
					my_putchar(buf[i]);
				}
			}
			else if (t.spec == SPEC_o)
			{
				int n = va_arg(argp, int);
				len = int_to_str(n, buf, 8, 0, 1);

				for (i = 0; i < len; i++)
				{
					my_putchar(buf[i]);
				}
			}
			else if (t.spec == SPEC_p)
			{
				uintptr_t n = va_arg(argp, uintptr_t);
				len = uintptr_to_str(n, buf, 1);
				size_t plen = sizeof(uintptr_t) * 2;

				if (len < plen)
				{
					for (j = 0; i < plen - len; i++)
					{
						my_putchar('0');
					}
				}
				for (i = 0; i < len; i++)
				{
					my_putchar(buf[i]);
				}
			}
			else if (t.spec == SPEC_f)
			{
				double d;
				uint8_t frac[DOUBLE_BIN_DIG];
				uint8_t whole[DOUBLE_BIN_DIG];
				size_t w_res, f_res;

				d = va_arg(argp, double);

				double_to_str(d, whole, &w_res, frac, &f_res);

				if (w_res == 0)
					my_putchar('0');

				for (i = 0; i < w_res; i++)
					my_putchar(whole[i] + '0');

				my_putchar('.');

				for (i = 0; i < f_res && i < 7; i++)
				{
					if (i == 6 && f_res > 7)
					{
						if (frac[i + 1] > 4)
							frac[i]++;
					}

					my_putchar(frac[i] + '0');
				}
			}
			else if (t.spec == SPEC_E || t.spec == SPEC_e)
			{
				double d;
				uint8_t right[DOUBLE_BIN_DIG];
				uint8_t left[DOUBLE_BIN_DIG];
				size_t l_size, r_size;
				size_t exp = 0;
				size_t count = 0;
				size_t ri = 0;
				int neg = 0;
				int extra = 0; // leading 1 due to rounding
				size_t offset = 0;

				// Default the precision to 6
				if (t.prec == 0 && !(t.flags & FMT_ZPREC))
					t.prec = 6;

				d = va_arg(argp, double);

				double_to_str(d, left, &l_size, right, &r_size);


				// Determine how to round the results
				if (t.prec == 0)
				{
					// If precision is zero, we either round
					// the left side, or the right side.
					if (l_size > 1)
					{
						if (left[1] > 4)
							left[0]++;

						// Handle carries
						if (left[0] >= 10)
						{
							left[0] = 0;
							extra = 1;
						}
					}
					else if (r_size > 1)
					{
						if (right[1] > 4)
							right[0]++;

						// Handle carries
						if (right[0] >= 10)
						{
							right[0] = 0;
							extra = 1;
						}
					}
					else if (r_size > 0)
					{
						if (right[0] > 4)
						{
							if (l_size == 0)
							{
								left[0] = 1;
								extra = 1;
							}
							else
							{
								left[0]++;
								if (left[0] >= 10)
								{
									left[0] = 1;
									left[1] = 0;
									extra = 1;
								}
							}
						}
					}
				}
				else
				{
					// If precision is greter than 0, we have to do some thinking.

					if (l_size >= 1)
					{
						// Determine if the rounding digit
						// is on the left side or right side.
						for (i = 0; i < l_size && i < t.prec; i++);

						if (i < l_size && i > 0)
						{
							// The precision is within the left side
							if (left[i] > 4)
							{
								left[i - 1]++;

								// Handle carries
								if (left[i - 1] >= 10)
								{
									for (j = i - 1; j > 0 && left[j] >= 10; j--)
									{
										left[j - 1]++;
										left[j] = 0;
									}

									if (j == 0 && left[j] >= 10)
									{
										left[j] = 0;
										extra = 1;
									}
								}
							}

						}
						else
						{
							// The precision is within the right side
							for (j = 0; j < r_size && j + i < t.prec; j++);

							if (j < r_size && right[j] > 4)
							{
								if (j > 0)
								{
									// The precision is after the first element
									right[j - 1]++;

									j = j - 1;

									// Handle carries
									if (right[j] >= 10)
									{
										for (; j > 0 && right[j] >= 10; j--)
										{
											right[j - 1]++;
											right[j] = 0;
										}

										if (j == 0 && right[j] >= 10)
										{
											right[j] = 0;

											for (j = l_size - 1; j > 0 && left[j] >= 10; j--)
											{
												left[j - 1]++;
												left[j] = 0;
											}

											if (j == 0 && left[j] >= 10)
											{
												left[j] = 0;
												extra = 1;
											}
										}
									}

								}
								else
								{
									left[l_size - 1]++;

									j = l_size - 1;

									// Handle carries
									if (left[j] >= 10)
									{
										for (; j > 0 && left[j] >= 10; j--)
										{
											left[j - 1]++;
											left[j] = 0;
										}

										if (j == 0 && left[j] >= 10)
										{
											left[j] = 0;
											extra = 1;
										}
									}
								}

								j = j > 0 ? j - 1 : r_size - 1;
							}
						}
					}
					else if (r_size >= 1)
					{
						// Find the index of the first non zero element
						for (i = 0; right[i] == 0 && i < r_size && i < t.prec; i++);
						offset = i;
						for (; i < r_size && i < t.prec + offset; i++);

						if (i < r_size && i > 0)
						{
							if (right[i] > 4)
							{
								right[i] = 0;
								right[i - 1]++;
							}


							j = i - 1;

							// Handle carries
							if (right[j] >= 10)
							{
								for (; j > 0 && right[j] >= 10; j--)
								{
									right[j - 1]++;
									right[j] = 0;
								}

								if (j == 0 && right[j] >= 10)
								{
									right[j] = 0;
									left[0]++;
									extra = 1;
								}
							}
						}
					}
				}


				// Print the first character
				if (extra)
					my_putchar('1');

				else if (l_size > 0)
					my_putchar(left[0] + '0');

				else if (r_size > 0)
				{
					neg = 1;
					exp++;
					for (i = 0; i < r_size && right[i] == 0; i++)
						exp++;

					my_putchar(right[i] + '0');
				}
				else
					my_putchar('0');

				// Print the radix point
				if (t.prec)
					my_putchar('.');

				// Print the rest of the left side
				if (l_size > 1)
				{
					for (i = 1; i < l_size && count < t.prec; i++)
					{
						my_putchar(left[i] + '0');
						exp++;
						count++;
					}
				}

				// Print the right side
				if (r_size > 0 && l_size > 0)
				{
					for (i = 0; i < r_size && count < t.prec; i++)
					{
						if (count == t.prec - 1 && i < r_size - 1)
						{
							if (right[i + 1] > 4)
								right[i]++;
						}
						my_putchar(right[i] + '0');
						exp++;
						count++;
					}
				}
				else if (r_size > 1)
				{
					for (i = exp; i < r_size && count < t.prec; i++)
					{
						/*if (count == t.prec - 1 && i < r_size - 1)
						{
							if (right[i + 1] > 4)
								right[i]++;
						}*/
						my_putchar(right[i] + '0');
						count++;
					}
				}
				else
					my_putchar('0');

				if (count < t.prec)
				{
					for (; count < t.prec; count++)
					{
						my_putchar('0');
					}
				}


				// Print the 'E' or 'e' character
				if (t.spec == SPEC_E)
					my_putchar('E');
				else
					my_putchar('e');


				if (l_size > 1)
					exp = l_size - 1;
				else if (l_size == 1)
					exp = 0;

				if (l_size >= 1 && extra)
					exp++;
				else if (neg && extra)
				{
					if (exp > 0)
						exp--;
					else
					{
						exp++;
						neg = 0;
					}
				}

				len = size_to_str(exp, buf, 10, 0);

				my_putchar(neg ? '-' : '+');

				if (len < 2)
					my_putchar('0');

				for (i = 0; i < len; i++)
					my_putchar(buf[i]);
			}
			else if (t.spec == SPEC_G || t.spec == SPEC_g)
			{
				// not implemented
			}
			else if (t.spec == SPEC_n)
			{
				// Do nothing
			}
			else
			{
				// invalid specifier
				err = 1;
			}
		}
		else
		{
			my_putchar(*fmt);
		}

		fmt++;
	}

	va_end(argp);

	return 1;
}
