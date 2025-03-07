#include <string.h>

#include "h.h"

#define MAX_ARRAY_VALUE_SIZE 256

// TODO: make this more safe
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

	case H_ARRAY:
		strcat(buf, "[");

		for (int i = 0; i < value->value.array.count; i++) {
			h_value_to_string_buf(&value->value.array.value[i], array_buf, sizeof(array_buf));

			strcat(buf, array_buf);

			if (i != value->value.array.count - 1)
				strcat(buf, " ");

			memset(array_buf, 0, sizeof(array_buf));
		}

		strcat(buf, "]");

		break;
	
	case H_CHAR:
		strcat(buf, &value->value.charester);
		break;
	}
}
