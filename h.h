#ifndef _h_h
#define _h_h

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <complex.h>

#define H_MAX_SUMBOIL_NAME 64
#define H_MAX_SUBCODE_LENGHT 512
#define H_MAX_STRING_LITERAL_LENGHT 256

struct h_base_stack {
	void* ptr;
	size_t count;

	struct h_base_stack* root_stack;
};

enum h_value_type {
	H_NUMBER = 0,
	H_FUNCTION,
	H_ARRAY,
	H_CHAR,
};

struct h_instr_stack {
	struct h_instr* instrs;
	size_t count;

	struct h_instr_stack* root_stack;
};

struct h_value_stack {
	struct h_value* value;
	size_t count;

	struct h_value_stack* root_stack;
};

struct h_value {
	enum h_value_type type;

	union {
		double complex number;
		struct h_instr_stack function;
		struct h_value_stack array;
		char charester;
	} value;
};

struct h_code_pos {
	size_t line;
	size_t line_pos;
};

enum h_error_type {
	H_OK = 0,
	H_ERROR_EMPTY_STACK,
	H_ERROR_TYPE_ERROR,
	H_ERROR_UNDEFINED_TOKEN,
	H_ERROR_NOT_CLOSED_SUBINSTR,
	H_ERROR_DIVISON_BY_ZERO,
	H_ERROR_UNDEFINED_VM_INSTRUCTION,
	H_ERROR_SUMBOIL_NOT_FOUND_IN_VARIABLE_CREATING,
	H_ERROR_APPLYING_REDUCE_TO_ONE_VALUE_ARRAY,
	H_ERROR_SUMBOIL_NOT_FOUND,
};

enum h_source_type {
	H_ERROR_SOURCE_TEXT_FILE = 0,
	H_ERROR_BYTECODE_FILE,
};

struct h_source {
	enum h_source_type source_type;

	union {
		struct {
			struct h_code_pos code_pos;
			const char* text;
		} text_source;
	} source;
};

struct h_error {
	enum h_error_type type;
	struct h_source source;

	union {
		struct {
			enum h_value_type excepted;
			enum h_value_type got;
		} type_error;
	} value;
};

struct h_value_stack_peek_result {
	struct h_value* value;
	struct h_error error;
};

struct h_value_stack_pop_result {
	struct h_value value;
	struct h_error error;
};

enum h_instr_type {
	H_VALUE = 0,

	H_ADD,
	H_SUB,
	H_MUL,
	H_DIV,
	H_POW,
	H_REAL,
	H_IMAG,

	H_IMAGINARITY_CONST,

	H_POP,
	H_FLIP,
	H_COPY,

	H_ARRAY_DEF,
	H_ARR_PUSH,
	H_ARR_GET,
	H_ARR_POP,
	H_ARR_FLIP,
	H_ARR_COPY,

	H_EQUALS,
	H_NOT_EQUALS,
	H_MORE,
	H_LESS,
	H_MORE_OR_EQUALS,
	H_LESS_OR_EQUALS,
	H_AND,
	H_OR,
	H_NOT,

	H_REDUCE,
	H_ENUMERATE,
	H_RANGE,

	H_LOAD_LIBRARY,
	H_LOAD_VARIABLE,

	H_CREATE_VARIABLE,
	H_CALL_SUMBOIL,
};

struct h_instr {
	enum h_instr_type type;
	struct h_source source;

	union {
		struct h_value value;
		struct h_instr_stack array_def;
		char sumboil[H_MAX_SUMBOIL_NAME];
	} value;
};

struct h_sumboil {
	char name[H_MAX_SUMBOIL_NAME];
	struct h_value value;
};

struct h_sumboil_stack {
	struct h_sumboil* sumboils;
	size_t count;
};

struct h_runtime {
	struct h_sumboil_stack sumboil_stack;
	struct h_value_stack value_stack;
};

enum h_lexer_state {
	H_LEXER_PARSE_POSTFIX = 0,
	H_LEXER_PARSE_PREFIX,
	H_LEXER_PARSE_CENTER,
	H_LEXER_PARSE_STRING,
};

struct h_lexer {
	char* line_save;
	char* tok_save;

	char* crnt_tok;
	char* crnt_line;

	char* text;
	const char* const_text;

	char string_buffer[H_MAX_STRING_LITERAL_LENGHT];

	size_t line;

	enum h_lexer_state state;
	size_t token_pos;
};

enum h_lexer_tok_type {
	H_TOK_EOF = 0,

	H_TOK_FN_OPEN,
	H_TOK_FN_CLOSE,

	H_TOK_ARRAY_OPEN,
	H_TOK_ARRAY_CLOSE,

	H_TOK_PLUS,
	H_TOK_MINUS,
	H_TOK_MUL,
	H_TOK_DIV,
	H_TOK_POW,
	H_TOK_REAL,
	H_TOK_IMAG,

	H_TOK_IMAGINARITY,

	H_TOK_NUMBER,
	H_TOK_STRING,

	H_TOK_POP,
	H_TOK_FLIP,
	H_TOK_COPY,

	H_TOK_ARR_GET,
	H_TOK_ARR_PUSH,
	H_TOK_ARR_POP,
	H_TOK_ARR_FLIP,
	H_TOK_ARR_COPY,

	H_TOK_EQUALS,
	H_TOK_NOT_EQUALS,
	H_TOK_MORE,
	H_TOK_LESS,
	H_TOK_MORE_OR_EQUALS,
	H_TOK_LESS_OR_EQUALS,
	H_TOK_AND,
	H_TOK_OR,
	H_TOK_NOT,

	H_TOK_REDUCE,
	H_TOK_ENUMERATE,
	H_TOK_RANGE,

	H_TOK_LOAD_LIBRARY,
	H_TOK_LOAD_VARIABLE,

	H_TOK_SUMBOIL,
	H_TOK_CREATE_VARIABLE,
};

struct h_lexer_tok {
	enum h_lexer_tok_type type;
	struct h_source source;

	union {
		double number;
		char string[H_MAX_STRING_LITERAL_LENGHT];
		char sumboil[H_MAX_SUMBOIL_NAME];
	} value;
};

void h_base_stack_push(struct h_base_stack* stack, const void* data, size_t data_size);
void h_base_stack_drop(struct h_base_stack* stack, size_t data_size);
void* h_base_stack_peek(const struct h_base_stack* stack, size_t data_size);

void h_value_stack_push(struct h_value_stack* stack, const struct h_value* data);
struct h_error h_value_stack_drop(struct h_value_stack* stack, const struct h_source* source);
struct h_value_stack_peek_result h_value_stack_peek(const struct h_value_stack* stack,
		const struct h_source* source);
struct h_value_stack_pop_result h_value_stack_pop(struct h_value_stack* stack,
		const struct h_source* source);

void h_value_stack_free_value(struct h_value* value);
void h_value_stack_free(struct h_value_stack* stack);

void h_instr_stack_push(struct h_instr_stack* stack, const struct h_instr* data);
void h_instr_stack_drop(struct h_instr_stack* stack);
struct h_instr* h_instr_stack_peek(const struct h_instr_stack* stack);
struct h_instr h_instr_stack_pop(struct h_instr_stack* stack);

void h_instr_stack_free_instr(struct h_instr* instr);
void h_instr_stack_free(struct h_instr_stack* stack);

void h_sumboil_stack_push(struct h_sumboil_stack* stack, const struct h_sumboil* data);
void h_sumboil_stack_drop(struct h_sumboil_stack* stack);
struct h_sumboil* h_sumboil_stack_peek(const struct h_sumboil_stack* stack);
struct h_sumboil h_sumboil_stack_pop(struct h_sumboil_stack* stack);

void h_sumboil_stack_free_sumboil(struct h_sumboil* sumboil);
void h_sumboil_stack_free(struct h_sumboil_stack* stack);

struct h_error h_execute_instr_stack(const struct h_instr_stack* instr_stack, struct h_runtime* runtime);

struct h_error h_parse_code(struct h_instr_stack* instr_stack, const char* text);

void h_create_lexer(struct h_lexer* lexer, const char* text);
struct h_error h_next_tok(struct h_lexer* lexer, struct h_lexer_tok* tok);
void h_free_lexer(struct h_lexer* lexer);

void h_create_error_message(const struct h_error* error, char* buf, size_t buf_len);

void h_value_to_string_buf(const struct h_value* value, char* buf, size_t buf_size);
void h_value_stack_to_string_buf(const struct h_value_stack* stack, char* buf, size_t buf_size);
bool h_is_array_string(const struct h_value_stack* stack);

#endif
