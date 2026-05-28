#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <kernel/tty.h>

int printf(const char* __restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		if (format[0] != '%') {
			size_t len = 1;
			while (format[len] && format[len] != '%')
				len++;
			terminal_write(format, len);
			format += len;
			written += len;
			continue;
		}

		format++; // skip '%'

		if (*format == '%') {
			terminal_write("%", 1);
			format++;
			written++;
		} else if (*format == 's') {
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			terminal_write(str, len);
			format++;
			written += len;
		} else {
			terminal_write("%", 1);
			written++;
		}
	}

	va_end(parameters);
	return written;
}