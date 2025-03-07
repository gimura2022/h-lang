#include <stdio.h>

#include "h.h"

int main(int argc, char* argv[])
{
	struct h_instr_stack instrs = {0};
	struct h_runtime runtime    = {0};
	struct h_error error        = {0};
	char buf[512];
	char text[]                 = "^ 3 i";

	error = h_parse_code(&instrs, text);
	if (error.type != H_OK) {
		h_create_error_message(&error, buf, sizeof(buf));
		printf("%s", buf);

		return 1;
	}

	error = h_execute_instr_stack(&instrs, &runtime);
	if (error.type != H_OK) {
		h_create_error_message(&error, buf, sizeof(buf));
		printf("%s", buf);

		return 1;
	}

	h_value_to_string_buf(&runtime.value_stack.value[0], buf, sizeof(buf));
	printf("%s\n", buf);

	h_instr_stack_free(&instrs);
	h_sumboil_stack_free(&runtime.sumboil_stack);
	h_value_stack_free(&runtime.value_stack);

	return 0;
}
