#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "h.h"

#define continue_or_return_if_error(x) ({ struct h_error __x = (x); if (__x.type != H_OK) return __x; })

#define return_ok() return (struct h_error) { .type = H_OK }

static bool next_line(struct h_lexer* lexer);
static bool next_tok(struct h_lexer* lexer);
static void reverse(char* str);
static struct h_error parse_tok(const char* text, struct h_lexer_tok* tok);

void h_create_lexer(struct h_lexer* lexer, const char* text)
{
	*lexer = (struct h_lexer) {0};
	lexer->const_text = text;
	lexer->text = malloc(strlen(text) + 1);
	strcpy(lexer->text, lexer->const_text);
	lexer->state = H_LEXER_PARSE_POSTFIX;

	lexer->crnt_line = strtok_r(lexer->text, "\n", &lexer->line_save);

	if (lexer->crnt_line == NULL)
		return;

	reverse(lexer->crnt_line);

	lexer->crnt_tok = strtok_r(lexer->crnt_line, " ", &lexer->tok_save);

	if (lexer->crnt_tok == NULL)
		return;

	reverse(lexer->crnt_tok);
}

void h_free_lexer(struct h_lexer* lexer)
{
	free(lexer->text);
}

struct h_error h_next_tok(struct h_lexer* lexer, struct h_lexer_tok* tok)
{
	if (lexer->crnt_tok == NULL) {
		tok->type = H_TOK_EOF;
		return_ok();
	}

	switch (lexer->state) {
	case H_LEXER_PARSE_POSTFIX: {
		if (strlen(lexer->crnt_tok) <= lexer->token_pos) {
			lexer->state = H_LEXER_PARSE_POSTFIX;
			lexer->token_pos = 0;

			if (!next_tok(lexer)) {
				tok->type = H_TOK_EOF;
				break;
			}

			return h_next_tok(lexer, tok);
		}

		char* tok_char = &lexer->crnt_tok[strlen(lexer->crnt_tok) - lexer->token_pos - 1];
		
		switch (*tok_char) {
		case ']':
			tok->type = H_TOK_ARRAY_CLOSE;
			break;

		case ')':
			tok->type = H_TOK_FN_CLOSE;
			break;

		default:
			lexer->state = H_LEXER_PARSE_CENTER;
			return h_next_tok(lexer, tok);
		}

		tok->source.source_type             = H_ERROR_SOURCE_TEXT_FILE;
		tok->source.source.text_source.text = lexer->const_text;
		tok->source.source.text_source.code_pos = (struct h_code_pos) { .line = lexer->line,
			.line_pos = tok_char - lexer->crnt_line };

		lexer->token_pos++;

		break;
	}

	case H_LEXER_PARSE_CENTER: {
		char tmp;
		char* old_endpoint = &lexer->crnt_tok[strlen(lexer->crnt_tok)];
		if (lexer->token_pos != 0) {
			tmp = lexer->crnt_tok[strlen(lexer->crnt_tok) - ((int) lexer->token_pos)];
			old_endpoint = &lexer->crnt_tok[strlen(lexer->crnt_tok) - ((int) lexer->token_pos)];
			*old_endpoint = '\0';
		}

		char* body_start = lexer->crnt_tok;
		while (*body_start != '\0' && (*body_start == '(' || *body_start == '['))
			body_start++;

		if (*(old_endpoint - 1) == '"' && *body_start == '"') {
			char tmp = *(old_endpoint - 1);
			*(old_endpoint - 1) = '\0';

			snprintf(tok->value.string, sizeof(tok->value.string), "%s", body_start + 1);

			*(old_endpoint - 1) = tmp;

			tok->type                           = H_TOK_STRING;
			tok->source.source_type             = H_ERROR_SOURCE_TEXT_FILE;
			tok->source.source.text_source.text = lexer->const_text;
			tok->source.source.text_source.code_pos = (struct h_code_pos) { .line = lexer->line,
				.line_pos = body_start - lexer->crnt_line };

			lexer->state     = H_LEXER_PARSE_PREFIX;
			lexer->token_pos = 0;

			break;
		}

		if (*(old_endpoint - 1) == '"') {
			lexer->state = H_LEXER_PARSE_STRING;

			char tmp = *(old_endpoint - 1);
			*(old_endpoint - 1) = '\0';

			reverse(body_start);
			snprintf(lexer->string_buffer, sizeof(lexer->string_buffer), "%s", body_start);

			*(old_endpoint - 1) = tmp;

			return h_next_tok(lexer, tok);
		}

		continue_or_return_if_error(parse_tok(body_start, tok));

		tok->source.source_type             = H_ERROR_SOURCE_TEXT_FILE;
		tok->source.source.text_source.text = lexer->const_text;
		tok->source.source.text_source.code_pos = (struct h_code_pos) { .line = lexer->line,
			.line_pos = body_start - lexer->crnt_line };

		if (lexer->token_pos != 0)
			*old_endpoint = tmp;

		lexer->state     = H_LEXER_PARSE_PREFIX;
		lexer->token_pos = 0;

		break;
	}
	
	case H_LEXER_PARSE_PREFIX: {
		char* body_start = lexer->crnt_tok;
		while (*body_start != '\0' && (*body_start == '(' || *body_start == '['))
			body_start++;

		if (body_start == lexer->crnt_tok) {
			lexer->state     = H_LEXER_PARSE_POSTFIX;
			lexer->token_pos = 0;

			if (!next_tok(lexer)) {
				tok->type = H_TOK_EOF;
				break;
			}

			return h_next_tok(lexer, tok);
		}

		char* tok_char = body_start - (lexer->token_pos + 1);

		switch (*tok_char) {
		case '(':
			tok->type = H_TOK_FN_OPEN;
			break;

		case '[':
			tok->type = H_TOK_ARRAY_OPEN;
			break;

		default:
			lexer->state     = H_LEXER_PARSE_POSTFIX;
			lexer->token_pos = 0;

			if (!next_tok(lexer)) {
				tok->type = H_TOK_EOF;
				break;
			}

			return h_next_tok(lexer, tok);
		}

		tok->source.source_type             = H_ERROR_SOURCE_TEXT_FILE;
		tok->source.source.text_source.text = lexer->const_text;
		tok->source.source.text_source.code_pos = (struct h_code_pos) { .line = lexer->line,
			.line_pos = tok_char - lexer->crnt_line };

		lexer->token_pos++;

		if (tok_char == lexer->crnt_tok) {
			lexer->state     = H_LEXER_PARSE_POSTFIX;
			lexer->token_pos = 0;

			next_tok(lexer);
		}
		
		break;
	}
	
	case H_LEXER_PARSE_STRING: {
		struct h_source source;
		source.source_type             = H_ERROR_SOURCE_TEXT_FILE;
		source.source.text_source.text = lexer->const_text;
		source.source.text_source.code_pos = (struct h_code_pos) { .line = lexer->line,
			.line_pos = lexer->crnt_tok - lexer->crnt_line };

		if (!next_tok(lexer))
			return (struct h_error) {
				.type   = H_ERROR_NOT_CLOSED_SUBINSTR,
				.source = source,
			};

		char* body_start = lexer->crnt_tok;
		while (*body_start != '\0' && (*body_start == '(' || *body_start == '['))
			body_start++;

		if (*body_start == '"') {
			if (strlen(lexer->string_buffer) + strlen(body_start) + 1 >=
					sizeof(lexer->string_buffer))
				break;

			reverse(body_start + 1);
			strcat(lexer->string_buffer, " ");
			strcat(lexer->string_buffer, body_start + 1);

			lexer->state     = H_LEXER_PARSE_PREFIX;
			lexer->token_pos = 0;

			tok->type = H_TOK_STRING;

			reverse(lexer->string_buffer);
			snprintf(tok->value.string, sizeof(tok->value.string), "%s", lexer->string_buffer);

			tok->source.source_type             = H_ERROR_SOURCE_TEXT_FILE;
			tok->source.source.text_source.text = lexer->const_text;
			tok->source.source.text_source.code_pos = (struct h_code_pos) { .line = lexer->line,
				.line_pos = body_start - lexer->crnt_line };

			break;
		}

		if (strlen(lexer->string_buffer) + strlen(body_start) + 1 >= sizeof(lexer->string_buffer))
			break;

		strcat(lexer->string_buffer, " ");

		reverse(body_start);
		strcat(lexer->string_buffer, body_start);

		return h_next_tok(lexer, tok);
	}
	}
	
	return_ok();
}

static bool next_line(struct h_lexer* lexer)
{
	lexer->crnt_line = strtok_r(NULL, "\n", &lexer->line_save);

	if (lexer->crnt_line == NULL)
		return false;

	reverse(lexer->crnt_line);

	return true;
}

static bool next_tok(struct h_lexer* lexer)
{
	if (lexer->crnt_line == NULL) {
		next_line(lexer);
		lexer->crnt_tok = strtok_r(lexer->crnt_line, " ", &lexer->tok_save);
	} else
		lexer->crnt_tok = strtok_r(NULL, " ", &lexer->tok_save);

	if (lexer->crnt_tok == NULL) {
		if (!next_line(lexer))
			return false;

		lexer->crnt_tok = strtok_r(lexer->crnt_line, " ", &lexer->tok_save);
		reverse(lexer->crnt_tok);

		return true;
	}

	reverse(lexer->crnt_tok);

	return true;
}

static void reverse(char* str)
{
	for (int i = 0; i < strlen(str) / 2; i++) {
		char tmp = str[i];
		str[i] = str[strlen(str) - i - 1];
		str[strlen(str) - i - 1] = tmp;
	}
}

static bool is_number(const char* str)
{
	bool dot_phase = false;

	for (const char* i = str; *i != '\0'; i++) {
		if (*i == '.' && dot_phase)
			return false;

		if (*i == '.') {
			dot_phase = true;
			continue;
		}

		if (!isdigit(*i))
			return false;
	}

	return true;
}

static struct h_error parse_tok(const char* text, struct h_lexer_tok* tok)
{
	if (strcmp(text, "+") == 0) {
		tok->type = H_TOK_PLUS;
		return_ok();
	}

	if (strcmp(text, "-") == 0) {
		tok->type = H_TOK_MINUS;
		return_ok();
	}

	if (strcmp(text, "*") == 0) {
		tok->type = H_TOK_MUL;
		return_ok();
	}

	if (strcmp(text, "/") == 0) {
		tok->type = H_TOK_DIV;
		return_ok();
	}

	if (strcmp(text, "i") == 0) {
		tok->type = H_TOK_IMAGINARITY;
		return_ok();
	}

	if (strcmp(text, ".") == 0) {
		tok->type = H_TOK_POP;
		return_ok();
	}

	if (strcmp(text, ":") == 0) {
		tok->type = H_TOK_FLIP;
		return_ok();
	}

	if (strcmp(text, ",") == 0) {
		tok->type = H_TOK_COPY;
		return_ok();
	}

	if (strcmp(text, "~<") == 0) {
		tok->type = H_TOK_ARR_GET;
		return_ok();
	}
	
	if (strcmp(text, "~>") == 0) {
		tok->type = H_TOK_ARR_PUSH;
		return_ok();
	}

	if (strcmp(text, "~.") == 0) {
		tok->type = H_TOK_ARR_POP;
		return_ok();
	}

	if (strcmp(text, "~,") == 0) {
		tok->type = H_TOK_ARR_COPY;
		return_ok();
	}

	if (strcmp(text, "~:") == 0) {
		tok->type = H_TOK_ARR_FLIP;
		return_ok();
	}

	if (strcmp(text, "==") == 0) {
		tok->type = H_TOK_EQUALS;
		return_ok();
	}

	if (strcmp(text, "!=") == 0) {
		tok->type = H_TOK_NOT_EQUALS;
		return_ok();
	}

	if (strcmp(text, "<") == 0) {
		tok->type = H_TOK_LESS;
		return_ok();
	}

	if (strcmp(text, ">") == 0) {
		tok->type = H_TOK_MORE;
		return_ok();
	}

	if (strcmp(text, "<=") == 0) {
		tok->type = H_TOK_LESS_OR_EQUALS;
		return_ok();
	}

	if (strcmp(text, ">=") == 0) {
		tok->type = H_TOK_MORE_OR_EQUALS;
		return_ok();
	}

	if (strcmp(text, "&&") == 0) {
		tok->type = H_TOK_AND;
		return_ok();
	}

	if (strcmp(text, "||") == 0) {
		tok->type = H_TOK_OR;
		return_ok();
	}

	if (strcmp(text, "!") == 0) {
		tok->type = H_TOK_NOT;
		return_ok();
	}

	if (strcmp(text, "\\") == 0) {
		tok->type = H_TOK_REDUCE;
		return_ok();
	}

	if (strcmp(text, "#") == 0) {
		tok->type = H_TOK_ENUMERATE;
		return_ok();
	}

	if (strcmp(text, "..") == 0) {
		tok->type = H_TOK_RANGE;
		return_ok();
	}

	if (strcmp(text, "$") == 0) {
		tok->type = H_TOK_LOAD_LIBRARY;
		return_ok();
	}

	if (strcmp(text, "&") == 0) {
		tok->type = H_TOK_LOAD_VARIABLE;
		return_ok();
	}

	if (strcmp(text, "=") == 0) {
		tok->type = H_TOK_CREATE_VARIABLE;
		return_ok();
	}

	if (strcmp(text, "^") == 0) {
		tok->type = H_TOK_POW;
		return_ok();
	}

	if (strcmp(text, "re") == 0) {
		tok->type = H_TOK_REAL;
		return_ok();
	}

	if (strcmp(text, "im") == 0) {
		tok->type = H_TOK_IMAG;
		return_ok();
	}

	if (is_number(text)) {
		tok->type = H_TOK_NUMBER;
		tok->value.number = atof(text);
		return_ok();
	}

	tok->type = H_TOK_SUMBOIL;
	snprintf(tok->value.sumboil, sizeof(tok->value.sumboil), "%s", text);

	return_ok();
}
