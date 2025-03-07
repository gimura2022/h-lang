#include <complex.h>
#include <string.h>

#include "h.h"

#define continue_or_return_if_error(x) ({ struct h_error __x = (x); if (__x.type != H_OK) return __x; })

#define return_ok() return (struct h_error) { .type = H_OK }

static struct h_error parse_tok(const struct h_lexer_tok* tok, struct h_instr* instr, struct h_lexer* lexer,
		struct h_instr_stack* instrs);

struct h_error h_parse_code(struct h_instr_stack* instr_stack, const char* text)
{
	struct h_lexer lexer   = {0};
	struct h_lexer_tok tok = {0};
	struct h_error error;
	
	h_create_lexer(&lexer, text);

	if ((error = h_next_tok(&lexer, &tok)).type != H_OK) {
		h_free_lexer(&lexer);
		return error;
	}

	while (tok.type != H_TOK_EOF) {
		struct h_instr instr;

		if ((error = parse_tok(&tok, &instr, &lexer, instr_stack)).type != H_OK) {
			h_free_lexer(&lexer);
			return error;
		}

		instr.source = tok.source;

		h_instr_stack_push(instr_stack, &instr);

		if ((error = h_next_tok(&lexer, &tok)).type != H_OK) {
			h_free_lexer(&lexer);
			return error;
		}
	}

	h_free_lexer(&lexer);

	return_ok();
}

static struct h_error parse_subcode(enum h_lexer_tok_type close_tok, struct h_instr_stack* instrs,
		struct h_lexer* lexer);

static struct h_error parse_tok(const struct h_lexer_tok* tok, struct h_instr* instr, struct h_lexer* lexer,
		struct h_instr_stack* instrs)
{
	switch (tok->type) {
	case H_TOK_ARRAY_OPEN:
	case H_TOK_FN_OPEN:
		return (struct h_error) {
			.type   = H_ERROR_NOT_CLOSED_SUBINSTR,
			.source = tok->source,
		};

	case H_TOK_NUMBER:
		instr->type                     = H_VALUE;
		instr->value.value.type         = H_NUMBER;
		instr->value.value.value.number = tok->value.number;

		break;

	case H_TOK_IMAGINARITY:
		instr->type                     = H_VALUE;
		instr->value.value.type         = H_NUMBER;
		instr->value.value.value.number = I;

		break;

	case H_TOK_FN_CLOSE: {
		struct h_instr_stack instrs = {0};

		continue_or_return_if_error(parse_subcode(H_TOK_FN_OPEN, &instrs, lexer));

		instr->type                       = H_VALUE;
		instr->value.value.type           = H_FUNCTION;
		instr->value.value.value.function = instrs;

		break;
	}
	
	case H_TOK_ARRAY_CLOSE: {
		struct h_instr_stack instrs = {0};

		continue_or_return_if_error(parse_subcode(H_TOK_ARRAY_OPEN, &instrs, lexer));

		instr->type            = H_ARRAY_DEF;
		instr->value.array_def = instrs;

		break;
	}

	case H_TOK_PLUS:
		instr->type = H_ADD;

		break;

	case H_TOK_MINUS:
		instr->type = H_SUB;

		break;

	case H_TOK_MUL:
		instr->type = H_MUL;

		break;

	case H_TOK_DIV:
		instr->type = H_DIV;

		break;

	case H_TOK_STRING:
		instr->type = H_ARRAY_DEF;

		for (const char* c = tok->value.string; *c != '\0'; c++) {
			struct h_instr char_instr = (struct h_instr) {
				.type                        = H_VALUE,
				.value.value.type            = H_CHAR,
				.value.value.value.charester = *c,
			};

			h_instr_stack_push(&instr->value.array_def, &char_instr);
		}

		break;

	case H_TOK_POP:
		instr->type = H_POP;

		break;

	case H_TOK_FLIP:
		instr->type = H_FLIP;

		break;

	case H_TOK_COPY:
		instr->type = H_COPY;

		break;

	case H_TOK_ARR_GET:
		instr->type = H_ARR_GET;

		break;

	case H_TOK_ARR_PUSH:
		instr->type = H_ARR_PUSH;

		break;

	case H_TOK_ARR_POP:
		instr->type = H_ARR_POP;

		break;

	case H_TOK_ARR_FLIP:
		instr->type = H_ARR_FLIP;

		break;

	case H_TOK_ARR_COPY:
		instr->type = H_ARR_COPY;

		break;
	
	case H_TOK_EQUALS:
		instr->type = H_EQUALS;

		break;

	case H_TOK_NOT_EQUALS:
		instr->type = H_NOT_EQUALS;

		break;

	case H_TOK_MORE:
		instr->type = H_MORE;

		break;

	case H_TOK_LESS:
		instr->type = H_LESS;

		break;

	case H_TOK_MORE_OR_EQUALS:
		instr->type = H_MORE_OR_EQUALS;

		break;

	case H_TOK_LESS_OR_EQUALS:
		instr->type = H_LESS_OR_EQUALS;

		break;

	case H_TOK_AND:
		instr->type = H_AND;

		break;

	case H_TOK_OR:
		instr->type = H_OR;

		break;

	case H_TOK_NOT:
		instr->type = H_NOT;

		break;

	case H_TOK_REDUCE:
		instr->type = H_REDUCE;

		break;

	case H_TOK_ENUMERATE:
		instr->type = H_ENUMERATE;

		break;

	case H_TOK_RANGE:
		instr->type = H_RANGE;

		break;

	case H_TOK_LOAD_VARIABLE:
		instr->type = H_LOAD_VARIABLE;

		break;

	case H_TOK_LOAD_LIBRARY:
		instr->type = H_LOAD_LIBRARY;

		break;

	case H_TOK_SUMBOIL:
		instr->type = H_CALL_SUMBOIL;
		strcpy(instr->value.sumboil, tok->value.sumboil);

		break;

	case H_TOK_CREATE_VARIABLE: {
		struct h_lexer_tok next_tok;
		continue_or_return_if_error(h_next_tok(lexer, &next_tok));

		if (next_tok.type != H_TOK_SUMBOIL)
			return (struct h_error) {
				.type   = H_ERROR_SUMBOIL_NOT_FOUND_IN_VARIABLE_CREATING,
				.source = tok->source,
			};

		instr->type = H_CREATE_VARIABLE;
		strcpy(instr->value.sumboil, next_tok.value.sumboil);

		break;
	}

	case H_TOK_POW:
		instr->type = H_POW;

		break;

	case H_TOK_REAL:
		instr->type = H_REAL;

		break;

	case H_TOK_IMAG:
		instr->type = H_IMAG;

		break;

	case H_TOK_EOF:
		break;
	}

	return_ok();
}

static struct h_error parse_subcode(enum h_lexer_tok_type close_tok, struct h_instr_stack* instrs,
		struct h_lexer* lexer)
{
	struct h_lexer_tok tok = {0};

	continue_or_return_if_error(h_next_tok(lexer, &tok));

	while (tok.type != close_tok) {
		if (tok.type == H_TOK_EOF)
			return (struct h_error) {
				.type   = H_ERROR_NOT_CLOSED_SUBINSTR,
				.source = tok.source,
			};

		struct h_instr instr;
		continue_or_return_if_error(parse_tok(&tok, &instr, lexer, instrs));

		instr.source = tok.source;

		h_instr_stack_push(instrs, &instr);

		continue_or_return_if_error(h_next_tok(lexer, &tok));
	}

	return_ok();
}
