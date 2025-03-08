#include <stdio.h>

#include "h.h"

#define MAX_ARRAY_VALUE_SIZE 256

void h_value_to_string_buf(const struct h_value* value, char* buf, size_t buf_size)
{
	char array_buf[MAX_ARRAY_VALUE_SIZE];

	switch (value->type) {
	case H_NUMBER:
		if (cimag(value->value.number) != 0) {
			snprintf(buf, buf_size, "%g + i%g", creal(value->value.number),
					cimag(value->value.number));
			break;
		}

		snprintf(buf, buf_size, "%g", creal(value->value.number));

		break;

	case H_FUNCTION:
		snprintf(buf, buf_size, "<function at %p>", &value->value.function);
		break;

	case H_CHAR:
		snprintf(buf, buf_size, "%c", value->value.charester);
		break;

	case H_ARRAY:
		if (h_is_array_string(&value->value.array)) {
			buf += snprintf(buf, buf_size, "\"");

			for (int i = 0; i < value->value.array.count; i++) {
				buf += snprintf(buf, buf_size, "%c",
						value->value.array.value[i].value.charester);
			}

			buf += snprintf(buf, buf_size, "\"");

			break;
		}

		buf += snprintf(buf, buf_size, "[");

		for (int i = value->value.array.count; i != 0; i--) {
			h_value_to_string_buf(&value->value.array.value[i], array_buf, sizeof(array_buf));
			buf += snprintf(buf, buf_size, "%s", array_buf);
		}

		buf += snprintf(buf, buf_size, "]");
	}
}

void h_value_stack_to_string_buf(const struct h_value_stack* stack, char* buf, size_t buf_size)
{
	char value_buf[MAX_ARRAY_VALUE_SIZE];

	for (int i = 0; i < stack->count; i++) {
		h_value_to_string_buf(&stack->value[i], value_buf, sizeof(value_buf));
		buf += snprintf(buf, buf_size, "%s\n", value_buf);
	}
}

bool h_is_array_string(const struct h_value_stack* stack)
{
	for (int i = 0; i < stack->count; i++) {
		if (stack->value[i].type != H_CHAR)
			return false;
	}

	return true;
}
