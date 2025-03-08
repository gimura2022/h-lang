#include <complex.h>
#include <stdint.h>
#include <stdio.h>

#include "h.h"

#define continue_or_return_if_error(x) ({ struct h_error __x = (x); if (__x.type != H_OK) return __x; })
#define return_ok() return (struct h_error) { .type = H_OK }

#define array_lenght(x) sizeof(x) / sizeof(x[0])

static const char header[] = { 'H', 'B' };

static struct h_error read_instr_stack(FILE* file, struct h_instr_stack* stack);

struct h_error h_read_bytecode(FILE* file, struct h_instr_stack* stack)
{
	char read_header[2];
	if (fread(read_header, sizeof(read_header[0]), array_lenght(read_header), file)
			!= array_lenght(read_header))
		return (struct h_error) {
			.type   = H_ERROR_BYTECODE_READ_ERROR,
			.source = { .source_type = H_ERROR_BYTECODE_FILE },
		};

	if (read_header[0] != header[0] || read_header[1] != header[1])
		return (struct h_error) {
			.type   = H_ERROR_BYTECODE_READ_ERROR,
			.source = { .source_type = H_ERROR_BYTECODE_FILE },
		};

	continue_or_return_if_error(read_instr_stack(file, stack));

	return_ok();
}

static void write_instr_stack(FILE* file, const struct h_instr_stack* stack);

void h_write_bytecode(FILE* file, const struct h_instr_stack* stack)
{
	fwrite(header, sizeof(header[0]), array_lenght(header), file);

	write_instr_stack(file, stack);
}

static void write_instr(FILE* file, const struct h_instr* instr);

static void write_instr_stack(FILE* file, const struct h_instr_stack* stack)
{
	fwrite(&stack->count, sizeof(stack->count), 1, file);

	for (int i = 0; i < stack->count; i++)
		write_instr(file, &stack->instrs[i]);
}

static void write_value(FILE* file, const struct h_value* value);

static void write_instr(FILE* file, const struct h_instr* instr)
{
	fwrite(&instr->type, sizeof(instr->type), 1, file);

	switch (instr->type) {
	case H_ARRAY_DEF:
		write_instr_stack(file, &instr->value.array_def);

		break;

	case H_VALUE:
		write_value(file, &instr->value.value);

		break;

	case H_CALL_SUMBOIL:
	case H_CREATE_VARIABLE:
		fwrite(instr->value.sumboil, sizeof(instr->value.sumboil), 1, file);

		break;

	default:
		break;
	}
}

static void write_value(FILE* file, const struct h_value* value)
{
	fwrite(&value->type, sizeof(value->type), 1, file);

	switch (value->type) {
	case H_NUMBER:
		fwrite(&value->value.number, sizeof(value->value.number), 1, file);

		break;

	case H_CHAR:
		fwrite(&value->value.charester, sizeof(value->value.charester), 1, file);

		break;

	case H_FUNCTION:
		write_instr_stack(file, &value->value.function);

		break;

	case H_ARRAY:
		break;
	}
}

static struct h_error read_instr(FILE* file, struct h_instr* instr);

static struct h_error read_instr_stack(FILE* file, struct h_instr_stack* stack)
{
	size_t stack_count;
	if (fread(&stack_count, sizeof(stack_count), 1, file) != 1)
		return (struct h_error) {
			.type   = H_ERROR_BYTECODE_READ_ERROR,
			.source = { .source_type = H_ERROR_BYTECODE_FILE },
		};

	for (int i = 0; i < stack_count; i++) {
		struct h_instr instr = {0};
		continue_or_return_if_error(read_instr(file, &instr));	

		h_instr_stack_push(stack, &instr);
	}

	return_ok();
}

static struct h_error read_value(FILE* file, struct h_value* value);

static struct h_error read_instr(FILE* file, struct h_instr* instr)
{
	if (fread(&instr->type, sizeof(instr->type), 1, file) != 1)
		return (struct h_error) {
			.type   = H_ERROR_BYTECODE_READ_ERROR,
			.source = { .source_type = H_ERROR_BYTECODE_FILE },
		};

	switch (instr->type) {
	case H_ARRAY_DEF:
		continue_or_return_if_error(read_instr_stack(file, &instr->value.array_def));

		break;

	case H_VALUE:
		continue_or_return_if_error(read_value(file, &instr->value.value));

		break;

	case H_CALL_SUMBOIL:
	case H_CREATE_VARIABLE:
		if (fread(instr->value.sumboil, sizeof(instr->value.sumboil), 1, file) != 1)
			return (struct h_error) {
				.type   = H_ERROR_BYTECODE_READ_ERROR,
				.source = { .source_type = H_ERROR_BYTECODE_FILE },
			};

		break;

	default:
		break;
	}

	return_ok();
}

static struct h_error read_value(FILE* file, struct h_value* value)
{
	if (fread(&value->type, sizeof(value->type), 1, file) != 1)
		return (struct h_error) {
			.type   = H_ERROR_BYTECODE_READ_ERROR,
			.source = { .source_type = H_ERROR_BYTECODE_FILE },
		};

	switch (value->type) {
	case H_NUMBER:
		if (fread(&value->value.number, sizeof(value->value.number), 1, file) != 1)
			return (struct h_error) {
				.type   = H_ERROR_BYTECODE_READ_ERROR,
				.source = { .source_type = H_ERROR_BYTECODE_FILE },
			};

		break;

	case H_CHAR:
		if (fread(&value->value.charester, sizeof(value->value.charester), 1, file) != 1)
			return (struct h_error) {
				.type   = H_ERROR_BYTECODE_READ_ERROR,
				.source = { .source_type = H_ERROR_BYTECODE_FILE },
			};

		break;

	case H_FUNCTION:
		continue_or_return_if_error(read_instr_stack(file, &value->value.function));
		
		break;

	case H_ARRAY:
		break;
	}

	return_ok();
}
