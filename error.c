/*
	Permission to use, copy, modify, and/or distribute this software for
	any purpose with or without fee is hereby granted.

	THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL
	WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
	OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
	FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
	DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
	AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
	OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdio.h>
#include <stdbool.h>

#include "h.h"

#define RESET "\033[0m"
#define STANDART "\033[39m"
#define BLACK "\033[30m"
#define DARK_RED "\033[31m"
#define DARK_GREEN "\033[32m"
#define DARK_YELLOW "\033[33m"
#define DARK_BLUE "\033[34m"
#define DARK_PURPURE "\033[35m"
#define DARK_LIGHT_BLUE "\033[36m"
#define LIGHT_GRAY "\033[37m"
#define DARK_GRAY "\033[90m"
#define RED "\033[91m"
#define GREEN "\033[92m"
#define ORANGE "\033[93m"
#define BLUE "\033[94m"
#define PURPURE "\033[95m"
#define LIGHT_BLUE "\033[96m"
#define WHITE "\033[97m"

static const char* get_error_message(enum h_error_type type);
static const char* get_type_name(enum h_value_type type);

static void write_error_messange(const struct h_error* error, char** buf, size_t buf_len);

#define MAX_ERROR_BUF 512

void h_create_error_message(const struct h_error* error, char* buf, size_t buf_len)
{
	write_error_messange(error, &buf, buf_len);
}

static const char* get_error_message(enum h_error_type type)
{
	switch (type) {
	case H_OK: return "Noting wrong";
	case H_ERROR_TYPE_ERROR: return "Invalid type of argument, expected " GREEN "%s" WHITE
				 " but got " RED "%s";
	case H_ERROR_NOT_CLOSED_SUBINSTR: return "Not closed subinstr";
	case H_ERROR_EMPTY_STACK: return "Empty stack";
	case H_ERROR_UNDEFINED_TOKEN: return "Undefined token";
	case H_ERROR_DIVISON_BY_ZERO: return "Division by zero";
	case H_ERROR_UNDEFINED_VM_INSTRUCTION: return "Undefined vm instruction type";
	case H_ERROR_SUMBOIL_NOT_FOUND_IN_VARIABLE_CREATING:
		return "Sumboil name not specified in variable creating";
	case H_ERROR_SUMBOIL_NOT_FOUND: return "Sumboil not found";
	case H_ERROR_APPLYING_REDUCE_TO_ONE_VALUE_ARRAY:
		return "Can't apply reduce to array with value count less 2";
	case H_ERROR_BYTECODE_READ_ERROR:
		return "Can't read bytecode, file format corrupted";
	}
}

static const char* get_type_name(enum h_value_type type)
{
	switch (type) {
	case H_NUMBER: return "number";
	case H_FUNCTION: return "function";
	case H_ARRAY: return "array";
	case H_CHAR: return "char";
	}
}

static void write_error_messange(const struct h_error* error, char** buf, size_t buf_len)
{
	*buf += snprintf(*buf, buf_len, RED "E%i" WHITE ": ", error->type);

	switch (error->type) {
	case H_ERROR_TYPE_ERROR:
		*buf += snprintf(*buf, buf_len, get_error_message(error->type),
				get_type_name(error->value.type_error.excepted),
				get_type_name(error->value.type_error.got));
		break;

	default:
		*buf += snprintf(*buf, buf_len, "%s", get_error_message(error->type));
		break;
	}

	*buf += snprintf(*buf, buf_len, "\n");
}
