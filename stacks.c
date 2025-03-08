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

#include <string.h>

#include "h.h"

#define return_ok() return (struct h_error) { .type = H_OK }

void h_base_stack_push(struct h_base_stack* stack, const void* data, size_t data_size)
{
	if (stack->count == 0) {
		stack->ptr   = malloc(data_size);
		stack->count = 1;

		memcpy(stack->ptr, data, data_size);

		return;
	}

	stack->ptr = realloc(stack->ptr, data_size * ++stack->count);
	memcpy(stack->ptr + data_size * (stack->count - 1), data, data_size);
}

void h_base_stack_drop(struct h_base_stack* stack, size_t data_size)
{
	if (stack->count == 0 && stack->root_stack != NULL) {
		h_base_stack_drop(stack->root_stack, data_size);
		return;
	}

	stack->count--;

	if (stack->count == 0) {
		free(stack->ptr);
		stack->ptr = NULL;
		return;
	}

	stack->ptr = realloc(stack->ptr, data_size * stack->count);
}

void* h_base_stack_peek(const struct h_base_stack* stack, size_t data_size)
{
	if (stack->count == 0 && stack->root_stack != NULL)
		return h_base_stack_peek(stack->root_stack, data_size);

	return stack->ptr + data_size * (stack->count - 1);
}

void h_instr_stack_push(struct h_instr_stack* stack, const struct h_instr* data)
{
	h_base_stack_push((struct h_base_stack*) stack, data, sizeof(struct h_instr));
}

void h_instr_stack_drop(struct h_instr_stack* stack)
{
	h_base_stack_drop((struct h_base_stack*) stack, sizeof(struct h_instr));
}

struct h_instr* h_instr_stack_peek(const struct h_instr_stack* stack)
{
	return h_base_stack_peek((struct h_base_stack*) stack, sizeof(struct h_instr));
}

struct h_instr h_instr_stack_pop(struct h_instr_stack* stack)
{
	struct h_instr instr = *h_instr_stack_peek(stack);
	h_instr_stack_drop(stack);

	return instr;
}

void h_instr_stack_free_instr(struct h_instr* instr)
{
	switch (instr->type) {
	case H_ARRAY_DEF:
		h_instr_stack_free(&instr->value.array_def);
		break;

	default:
		break;
	}
}

void h_instr_stack_free(struct h_instr_stack* stack)
{
	for (int i = 0; i < stack->count; i++)
		h_instr_stack_free_instr(&stack->instrs[i]);

	free(stack->instrs);
}

void h_value_stack_push(struct h_value_stack* stack, const struct h_value* data)
{
	h_base_stack_push((struct h_base_stack*) stack, data, sizeof(struct h_value));
}

struct h_error h_value_stack_drop(struct h_value_stack* stack, const struct h_source* source)
{
	if (stack->count == 0 && stack->root_stack == NULL)
		return (struct h_error) {
			.type   = H_ERROR_EMPTY_STACK,
			.source = *source,
		};

	h_base_stack_drop((struct h_base_stack*) stack, sizeof(struct h_value));

	return_ok();
}

struct h_value_stack_peek_result h_value_stack_peek(const struct h_value_stack* stack,
		const struct h_source* source)
{
	if (stack->count == 0 && stack->root_stack == NULL)
		return (struct h_value_stack_peek_result) {
			.value = NULL,
			.error = (struct h_error) {
			.type   = H_ERROR_EMPTY_STACK,
				.source = *source,
			},
		};

	if (stack->count == 0)
		return h_value_stack_peek(stack->root_stack, source);

	return (struct h_value_stack_peek_result) {
		.value = h_base_stack_peek((struct h_base_stack*) stack, sizeof(struct h_value)),
		.error = (struct h_error) {
			.type   = H_OK,
			.source = *source,
		},
	};
}

struct h_value_stack_pop_result h_value_stack_pop(struct h_value_stack* stack,
		const struct h_source* source)
{
	struct h_value_stack_peek_result value = h_value_stack_peek(stack, source);
	if (value.error.type != H_OK)
		return (struct h_value_stack_pop_result) {
			.error = value.error,
			.value = {0},
		};

	struct h_value value_value = *value.value;

	h_value_stack_drop(stack, source);

	return (struct h_value_stack_pop_result) {
		.value = value_value,
		.error = (struct h_error) {
			.type   = H_OK,
			.source = *source,
		},
	};
}

void h_value_stack_free_value(struct h_value* value)
{
	switch (value->type) {
	case H_FUNCTION:
		h_instr_stack_free(&value->value.function);
		break;

	case H_ARRAY:
		h_value_stack_free(&value->value.array);
		break;

	default:
		break;
	}
}

void h_value_stack_free(struct h_value_stack* stack)
{
	for (int i = 0; i < stack->count; i++)
		h_value_stack_free_value(&stack->value[i]);

	if (stack->value != NULL)
		free(stack->value);

	stack->value = NULL;
}

void h_sumboil_stack_push(struct h_sumboil_stack* stack, const struct h_sumboil* data)
{
	h_base_stack_push((struct h_base_stack*) stack, data, sizeof(struct h_sumboil));
}

void h_sumboil_stack_drop(struct h_sumboil_stack* stack)
{
	h_base_stack_drop((struct h_base_stack*) stack, sizeof(struct h_sumboil));
}

struct h_sumboil* h_sumboil_stack_peek(const struct h_sumboil_stack* stack)
{
	return h_base_stack_peek((struct h_base_stack*) stack, sizeof(struct h_sumboil));
}

struct h_sumboil h_sumboil_stack_pop(struct h_sumboil_stack* stack)
{
	struct h_sumboil sumboil = *h_sumboil_stack_peek(stack);
	h_sumboil_stack_drop(stack);

	return sumboil;
}

void h_sumboil_stack_free_sumboil(struct h_sumboil* sumboil)
{
	h_value_stack_free_value(&sumboil->value);
}

void h_sumboil_stack_free(struct h_sumboil_stack* stack)
{
	for (int i = 0; i < stack->count; i++)
		h_sumboil_stack_free_sumboil(&stack->sumboils[i]);

	if (stack->sumboils != NULL)
		free(stack->sumboils);

	stack->sumboils = NULL;
}
