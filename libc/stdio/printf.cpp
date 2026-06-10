#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <kernel/tty.h>

/**
 * @brief Helper to convert an unsigned integer to a string and print it.
 * 
 * @param n The number to print.
 * @param base The base (e.g., 10 for decimal, 16 for hex).
 * @return int The number of characters written.
 */
static int print_unsigned(unsigned int n, int base) {
	const char digits[] = "0123456789abcdef";
	char buffer[32];
	int i = 0;

	// Handle the zero case explicitly
	if (n == 0) {
		terminal_write("0", 1);
		return 1;
	}

	// Decompose the number into the buffer in reverse order
	while (n > 0) {
		buffer[i++] = digits[n % base];
		n /= base;
	}

	int written = i;
	// Print the buffer backwards to restore correct order
	while (i > 0) {
		terminal_write(&buffer[--i], 1);
	}
	return written;
}

int printf(const char* __restrict format, ...) {
	// va_list is used to hold the information needed by va_arg and va_end
	va_list parameters;
	// va_start initializes the va_list for use with va_arg and va_end
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		// Handle literal characters (not format specifiers)
		if (*format != '%') {
			terminal_write(format, 1);
			format++;
			written++;
			continue;
		}

		format++; // Skip the '%' character

		// Handle "%%" (escaped percent sign)
		if (*format == '%') {
			terminal_write("%", 1);
			format++;
			written++;
		} 
		// Handle strings
		else if (*format == 's') {
			const char* str = va_arg(parameters, const char*);
			if (!str) str = "(null)";
			size_t len = strlen(str);
			terminal_write(str, len);
			format++;
			written += len;
		} 
		// Handle characters (chars are promoted to int when passed through ...)
		else if (*format == 'c') {
			char c = (char) va_arg(parameters, int);
			terminal_write(&c, 1);
			format++;
			written++;
		} 
		// Handle signed integers
		else if (*format == 'd' || *format == 'i') {
			int n = va_arg(parameters, int);
			unsigned int u;

			if (n < 0) {
				terminal_write("-", 1);
				written++;
				// Cast to long long before negation to safely handle INT_MIN
				u = (unsigned int)(-(long long)n);
			} else {
				u = (unsigned int)n;
			}

			written += print_unsigned(u, 10);
			format++;
		} 
		// Handle unsigned decimal integers
		else if (*format == 'u') {
			unsigned int n = va_arg(parameters, unsigned int);
			written += print_unsigned(n, 10);
			format++;
		} 
		// Handle hexadecimal integers
		else if (*format == 'x') {
			unsigned int n = va_arg(parameters, unsigned int);
			written += print_unsigned(n, 16);
			format++;
		} 
		// Fallback for unknown specifiers
		// Treat the '%' as a literal and continue to the next character.
		else {
			terminal_write("%", 1);
			written++;
			// We do not increment format here, so the next loop 
			// iteration processes the character after the '%'
		}
	}

	// va_end cleans up the va_list
	va_end(parameters);
	return written;
}