#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "h.h"

#define continue_or_return_if_error(x) ({ struct h_error __x = (x); if (__x.type != H_OK) return __x; })
#define return_ok() return (struct h_error) { .type = H_OK }

static struct h_error read_instrs_text(const char* path, struct h_instr_stack* instrs, bool* is_found)
{
	FILE* file = fopen(path, "r");

	if (file == NULL) {
		*is_found = false;
		return_ok();
	}

	fseek(file, 0L, SEEK_END);
	size_t size = ftell(file);

	rewind(file);

	char* text = malloc(size);
	fread(text, sizeof(char), size, file);

	fclose(file);

	continue_or_return_if_error(h_parse_code(instrs, text));

	free(text);

	*is_found = true;

	return_ok();
}

static struct h_error read_instrs(const char* path, struct h_instr_stack* instrs, bool* is_found)
{
	FILE* file = fopen(path, "rb");

	if (file == NULL) {
		*is_found = false;
		return_ok();
	}

	if (h_read_bytecode(file, instrs).type == H_OK) {
		fclose(file);
		*is_found = true;

		return_ok();
	}

	continue_or_return_if_error(read_instrs_text(path, instrs, is_found));
	
	return_ok();
}

#define MAX_ERROR_LENGTH 512

static void print_error(const struct h_error* error)
{
	char buf[MAX_ERROR_LENGTH];

	h_create_error_message(error, buf, MAX_ERROR_LENGTH);

	fprintf(stderr, "%s\n", buf);
}

#define MAX_STACK_VALUE_LENGHT 2048

int main(int argc, char* argv[])
{
	const char* out       = NULL;
	const char* input     = NULL;
	const char* prog_args = NULL;
	bool compile_mode     = false;

	char c;
	while ((c = getopt(argc, argv, "co:i:a:")) != -1) {
		switch (c) {
		case 'c':
			compile_mode = true;
			break;

		case 'o':
			out = optarg;
			break;

		case 'i':
			input = optarg;
			break;

		case 'a':
			prog_args = optarg;
			break;

		case '?':
			break;
		}
	}

	if (input == NULL) {
		fprintf(stderr, "h: input file not found!\n");
		return 1;
	}

	if (compile_mode) {
		if (out == NULL) {
			fprintf(stderr, "h: output file not found!\n");
			return 1;
		}

		struct h_instr_stack instrs = {0};
		struct h_error error        = {0};
		bool is_found               = false;

		if ((error = read_instrs_text(input, &instrs, &is_found)).type != H_OK) {
			h_instr_stack_free(&instrs);

			print_error(&error);
			return 1;
		}

		if (!is_found) {
			fprintf(stderr, "h: file not found!\n");
			return 1;
		}

		FILE* file = fopen(out, "wb");

		h_write_bytecode(file, &instrs);

		fclose(file);

		return 0;
	}

	struct h_runtime runtime = {0};

	if (prog_args != NULL) {
		struct h_instr_stack instrs = {0};
		struct h_error error        = {0};

		if ((error = h_parse_code(&instrs, prog_args)).type != H_OK) {
			h_instr_stack_free(&instrs);

			print_error(&error);
			return 1;
		}

		if ((error = h_execute_instr_stack(&instrs, &runtime)).type != H_OK) {
			h_value_stack_free(&runtime.value_stack);
			h_sumboil_stack_free(&runtime.sumboil_stack);
			h_instr_stack_free(&instrs);

			print_error(&error);
			return 1;
		}

		h_instr_stack_free(&instrs);
	}

	struct h_instr_stack instrs = {0};
	struct h_error error        = {0};
	bool is_found               = false;
	
	if ((error = read_instrs(input, &instrs, &is_found)).type != H_OK) {
		h_instr_stack_free(&instrs);

		print_error(&error);
		return 1;
	}

	if (!is_found) {
		fprintf(stderr, "h: file not found!\n");
		return 1;
	}

	if ((error = h_execute_instr_stack(&instrs, &runtime)).type != H_OK) {
		h_value_stack_free(&runtime.value_stack);
		h_sumboil_stack_free(&runtime.sumboil_stack);
		h_instr_stack_free(&instrs);

		print_error(&error);
		return 1;
	}

	h_instr_stack_free(&instrs);

	char buf[MAX_STACK_VALUE_LENGHT];
	h_value_stack_to_string_buf(&runtime.value_stack, buf, sizeof(buf));

	printf("%s", buf);

	h_value_stack_free(&runtime.value_stack);
	h_sumboil_stack_free(&runtime.sumboil_stack);

	return 0;
}
