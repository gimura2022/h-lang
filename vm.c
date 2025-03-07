#include <stdio.h>
#include <stdbool.h>
#include <complex.h>
#include <string.h>

#include "h.h"

#define continue_or_return_if_error(x) ({ struct h_error __x = (x); if (__x.type != H_OK) return __x; })
#define continue_or_return_if_pop_error(x) if (x.error.type != H_OK) return x.error;
#define continue_or_return_if_type_error(x, y, src) if (x.type != y) return (struct h_error) { \
			.type = H_ERROR_TYPE_ERROR, \
			.source = src, \
			.value.type_error.excepted = y, \
			.value.type_error.got = x.type, \
		};

#define return_ok() return (struct h_error) { .type = H_OK }

static struct h_error execute_instr(const struct h_instr* instr, struct h_runtime* runtime);

struct h_error h_execute_instr_stack(const struct h_instr_stack* instr_stack, struct h_runtime* runtime)
{
	for (int i = 0; i < instr_stack->count; i++) {
		const struct h_instr* instr = &instr_stack->instrs[i];

		continue_or_return_if_error(execute_instr(instr, runtime));
	}

	return_ok();
}

static struct h_error execute_value(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_add(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_sub(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_mul(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_div(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_array_def(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_imaginarity_const(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_pop(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_flip(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_copy(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_arr_get(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_arr_push(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_arr_pop(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_arr_flip(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_arr_copy(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_equals(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_not_equals(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_more(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_less(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_more_or_equals(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_less_or_equals(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_and(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_or(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_not(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_reduce(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_enumerate(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_range(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_create_variable(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_variable(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_real(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_imag(const struct h_instr* instr, struct h_runtime* runtime);
static struct h_error execute_pow(const struct h_instr* instr, struct h_runtime* runtime);

static struct h_error execute_instr(const struct h_instr* instr, struct h_runtime* runtime)
{
	switch (instr->type) {
	case H_VALUE:
		continue_or_return_if_error(execute_value(instr, runtime));
		break;

	case H_ADD:
		continue_or_return_if_error(execute_add(instr, runtime));
		break;

	case H_SUB:
		continue_or_return_if_error(execute_sub(instr, runtime));
		break;

	case H_MUL:
		continue_or_return_if_error(execute_mul(instr, runtime));
		break;

	case H_DIV:
		continue_or_return_if_error(execute_div(instr, runtime));
		break;

	case H_ARRAY_DEF:
		continue_or_return_if_error(execute_array_def(instr, runtime));
		break;

	case H_IMAGINARITY_CONST:
		continue_or_return_if_error(execute_imaginarity_const(instr, runtime));
		break;

	case H_POP:
		continue_or_return_if_error(execute_pop(instr, runtime));
		break;

	case H_FLIP:
		continue_or_return_if_error(execute_flip(instr, runtime));
		break;

	case H_COPY:
		continue_or_return_if_error(execute_copy(instr, runtime));
		break;

	case H_ARR_GET:
		continue_or_return_if_error(execute_arr_get(instr, runtime));
		break;

	case H_ARR_PUSH:
		continue_or_return_if_error(execute_arr_push(instr, runtime));
		break;

	case H_ARR_POP:
		continue_or_return_if_error(execute_arr_pop(instr, runtime));
		break;

	case H_ARR_FLIP:
		continue_or_return_if_error(execute_arr_flip(instr, runtime));
		break;

	case H_ARR_COPY:
		continue_or_return_if_error(execute_arr_copy(instr, runtime));
		break;

	case H_EQUALS:
		continue_or_return_if_error(execute_equals(instr, runtime));
		break;

	case H_NOT_EQUALS:
		continue_or_return_if_error(execute_not_equals(instr, runtime));
		break;

	case H_MORE:
		continue_or_return_if_error(execute_more(instr, runtime));
		break;

	case H_LESS:
		continue_or_return_if_error(execute_less(instr, runtime));
		break;

	case H_MORE_OR_EQUALS:
		continue_or_return_if_error(execute_more_or_equals(instr, runtime));
		break;

	case H_LESS_OR_EQUALS:
		continue_or_return_if_error(execute_less_or_equals(instr, runtime));
		break;

	case H_AND:
		continue_or_return_if_error(execute_and(instr, runtime));
		break;

	case H_OR:
		continue_or_return_if_error(execute_or(instr, runtime));
		break;

	case H_NOT:
		continue_or_return_if_error(execute_not(instr, runtime));
		break;

	case H_REDUCE:
		continue_or_return_if_error(execute_reduce(instr, runtime));
		break;

	case H_ENUMERATE:
		continue_or_return_if_error(execute_enumerate(instr, runtime));
		break;

	case H_RANGE:
		continue_or_return_if_error(execute_range(instr, runtime));
		break;

	case H_CREATE_VARIABLE:
		continue_or_return_if_error(execute_create_variable(instr, runtime));
		break;

	case H_CALL_SUMBOIL:
		continue_or_return_if_error(execute_variable(instr, runtime));
		break;

	case H_REAL:
		continue_or_return_if_error(execute_real(instr, runtime));
		break;

	case H_IMAG:
		continue_or_return_if_error(execute_imag(instr, runtime));
		break;

	case H_POW:
		continue_or_return_if_error(execute_pow(instr, runtime));
		break;

	default:
		return (struct h_error) {
			.type   = H_ERROR_UNDEFINED_VM_INSTRUCTION,
			.source = instr->source,
		};
	}

	return_ok();
}

static struct h_error execute_value(const struct h_instr* instr, struct h_runtime* runtime)
{
	h_value_stack_push(&runtime->value_stack, &instr->value.value);

	return_ok();
}

static struct h_error execute_add(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = value0.value.value.number + value1.value.value.number,
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_sub(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = value0.value.value.number - value1.value.value.number,
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_mul(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = value0.value.value.number * value1.value.value.number,
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_div(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	if (value1.value.value.number == 0)
		return (struct h_error) {
			.type   = H_ERROR_DIVISON_BY_ZERO,
			.source = instr->source,
		};

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = value0.value.value.number / value1.value.value.number,
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_array_def(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_runtime array_runtime = { .sumboil_stack = runtime->sumboil_stack,
		{ .root_stack = &runtime->value_stack } };

	continue_or_return_if_error(h_execute_instr_stack(&instr->value.array_def, &array_runtime));

	struct h_value value = (struct h_value) {
		.type        = H_ARRAY,
		.value.array = array_runtime.value_stack,
	};
	
	h_value_stack_push(&runtime->value_stack, &value);

	return_ok();
}

static struct h_error execute_imaginarity_const(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = I,
	};

	h_value_stack_push(&runtime->value_stack, &value);

	return_ok();
}

static struct h_error execute_pop(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value);

	h_value_stack_free_value(&value.value);

	return_ok();
}

static struct h_error execute_flip(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	h_value_stack_push(&runtime->value_stack, &value0.value);
	h_value_stack_push(&runtime->value_stack, &value1.value);

	return_ok();
}

static struct h_error execute_copy(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value);

	h_value_stack_push(&runtime->value_stack, &value.value);
	h_value_stack_push(&runtime->value_stack, &value.value);

	return_ok();
}

static struct h_error execute_arr_get(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value);
	continue_or_return_if_type_error(value.value, H_ARRAY, instr->source);

	struct h_value_stack_pop_result array_value = h_value_stack_pop(&value.value.value.array,
			&instr->source);

	continue_or_return_if_pop_error(array_value);

	h_value_stack_push(&runtime->value_stack, &value.value);
	h_value_stack_push(&runtime->value_stack, &array_value.value);
	
	return_ok();
}

static struct h_error execute_arr_push(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value1.value, H_ARRAY, instr->source);

	h_value_stack_push(&value1.value.value.array, &value0.value);
	h_value_stack_push(&runtime->value_stack, &value1.value);

	return_ok();
}

static struct h_error execute_arr_pop(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value);
	continue_or_return_if_type_error(value.value, H_ARRAY, instr->source);

	struct h_value_stack_pop_result array_value = h_value_stack_pop(&value.value.value.array,
			&instr->source);

	continue_or_return_if_pop_error(array_value);

	h_value_stack_free_value(&array_value.value);

	h_value_stack_push(&runtime->value_stack, &value.value);

	return_ok();
}

static struct h_error execute_arr_flip(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result array = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(array);
	continue_or_return_if_type_error(array.value, H_ARRAY, instr->source);

	struct h_value_stack_pop_result value0 = h_value_stack_pop(&array.value.value.array, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&array.value.value.array, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	h_value_stack_push(&array.value.value.array, &value0.value);
	h_value_stack_push(&array.value.value.array, &value1.value);

	h_value_stack_push(&runtime->value_stack, &array.value);

	return_ok();
}

static struct h_error execute_arr_copy(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result array = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(array);
	continue_or_return_if_type_error(array.value, H_ARRAY, instr->source);

	struct h_value_stack_pop_result value = h_value_stack_pop(&array.value.value.array, &instr->source);

	continue_or_return_if_pop_error(value);

	h_value_stack_push(&array.value.value.array, &value.value);
	h_value_stack_push(&array.value.value.array, &value.value);

	h_value_stack_push(&runtime->value_stack, &array.value);

	return_ok();
}

static struct h_error execute_equals(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = value0.value.value.number == value1.value.value.number,
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_not_equals(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = value0.value.value.number != value1.value.value.number,
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_more(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = creal(value0.value.value.number) > creal(value1.value.value.number),
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_less(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = creal(value0.value.value.number) < creal(value1.value.value.number),
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_more_or_equals(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = creal(value0.value.value.number) >= creal(value1.value.value.number),
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_less_or_equals(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = creal(value0.value.value.number) <= creal(value1.value.value.number),
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_and(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = value0.value.value.number && value1.value.value.number,
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_or(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = value0.value.value.number || value1.value.value.number,
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_not(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = !value0.value.value.number,
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_reduce(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result function = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result array    = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(array);
	continue_or_return_if_pop_error(function);

	continue_or_return_if_type_error(array.value, H_ARRAY, instr->source);
	continue_or_return_if_type_error(function.value, H_FUNCTION, instr->source);

	if (array.value.value.array.count < 2)
		return (struct h_error) {
			.type   = H_ERROR_APPLYING_REDUCE_TO_ONE_VALUE_ARRAY,
			.source = instr->source,
		};

	struct h_value save_value = array.value.value.array.value[0];

	for (int i = 1; i < array.value.value.array.count; i++) {
		struct h_runtime function_runtime = {
			.value_stack = (struct h_value_stack) {
				.root_stack = &runtime->value_stack,
			},
			.sumboil_stack = runtime->sumboil_stack,
		};

		struct h_value* value = &array.value.value.array.value[i];

		h_value_stack_push(&function_runtime.value_stack, &save_value);
		h_value_stack_push(&function_runtime.value_stack, value);

		continue_or_return_if_error(h_execute_instr_stack(&function.value.value.function,
					&function_runtime));

		struct h_value_stack_pop_result result_value = h_value_stack_pop(
			&function_runtime.value_stack, &instr->source);

		continue_or_return_if_pop_error(result_value);

		save_value = result_value.value;
		
		h_value_stack_free(&function_runtime.value_stack);
	}

	h_value_stack_free_value(&function.value);
	h_value_stack_free_value(&array.value);

	h_value_stack_push(&runtime->value_stack, &save_value);

	return_ok();
}

static struct h_error execute_enumerate(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result function = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result array    = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(array);
	continue_or_return_if_pop_error(function);

	continue_or_return_if_type_error(array.value, H_ARRAY, instr->source);
	continue_or_return_if_type_error(function.value, H_FUNCTION, instr->source);

	for (int i = 0; i < array.value.value.array.count; i++) {
		struct h_runtime function_runtime = {
			.value_stack = (struct h_value_stack) {
				.root_stack = &runtime->value_stack,
			},
			.sumboil_stack = runtime->sumboil_stack,
		};

		struct h_value* value = &array.value.value.array.value[i];

		h_value_stack_push(&function_runtime.value_stack, value);

		continue_or_return_if_error(h_execute_instr_stack(&function.value.value.function,
					&function_runtime));

		struct h_value_stack_pop_result result_value = h_value_stack_pop(
			&function_runtime.value_stack, &instr->source);

		continue_or_return_if_pop_error(result_value);

		array.value.value.array.value[i] = result_value.value;

		h_value_stack_free(&function_runtime.value_stack);
	}

	h_value_stack_free_value(&function.value);

	h_value_stack_push(&runtime->value_stack, &array.value);

	return_ok();
}

static struct h_error execute_range(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result from = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result to   = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(from);
	continue_or_return_if_pop_error(to);

	continue_or_return_if_type_error(from.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(to.value, H_NUMBER, instr->source);

	struct h_value_stack array = {0};

	for (int i = from.value.value.number; i < creal(to.value.value.number); i++) {
		struct h_value value = (struct h_value) {
			.type         = H_NUMBER,
			.value.number = i,
		};

		h_value_stack_push(&array, &value);
	}

	struct h_value result = (struct h_value) {
		.type        = H_ARRAY,
		.value.array = array,
	};

	h_value_stack_push(&runtime->value_stack, &result);

	return_ok();
}

static struct h_error execute_create_variable(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value);

	struct h_sumboil sumboil = (struct h_sumboil) { .value = value.value };
	snprintf(sumboil.name, sizeof(sumboil.name), "%s", instr->value.sumboil);

	h_sumboil_stack_push(&runtime->sumboil_stack, &sumboil);

	return_ok();
}

static struct h_error execute_variable(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value* value = NULL;
	for (int i = 0; i < runtime->sumboil_stack.count; i++) {
		if (strcmp(runtime->sumboil_stack.sumboils[i].name, instr->value.sumboil) == 0) {
			value = &runtime->sumboil_stack.sumboils[i].value;
			break;
		}
	}

	if (value == NULL)
		return (struct h_error) {
			.type   = H_ERROR_SUMBOIL_NOT_FOUND,
			.source = instr->source,
		};

	if (value->type == H_FUNCTION) {
		continue_or_return_if_error(h_execute_instr_stack(&value->value.function, runtime));
		return_ok();
	}

	h_value_stack_push(&runtime->value_stack, value);

	return_ok();
}

static struct h_error execute_real(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = creal(value0.value.value.number),
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_imag(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = cimag(value0.value.value.number),
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}

static struct h_error execute_pow(const struct h_instr* instr, struct h_runtime* runtime)
{
	struct h_value_stack_pop_result value0 = h_value_stack_pop(&runtime->value_stack, &instr->source);
	struct h_value_stack_pop_result value1 = h_value_stack_pop(&runtime->value_stack, &instr->source);

	continue_or_return_if_pop_error(value0);
	continue_or_return_if_pop_error(value1);

	continue_or_return_if_type_error(value0.value, H_NUMBER, instr->source);
	continue_or_return_if_type_error(value1.value, H_NUMBER, instr->source);

	struct h_value result_value = (struct h_value) {
		.type         = H_NUMBER,
		.value.number = cpow(value1.value.value.number, value0.value.value.number),
	};

	h_value_stack_push(&runtime->value_stack, &result_value);

	return_ok();
}
