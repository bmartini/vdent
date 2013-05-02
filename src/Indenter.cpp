/**
 * @file Indenter.cpp
 *
 * @author  Berin Martini
 * @license GPL v2+
 */

#include "Indenter.h"

#include <stack>
#include <cstring>
#include <stdlib.h>


#define IS_NAME_CHAR(x) (isalnum((x)) || ((x) == '.') || ((x) == '_') || ((x) == '$') || ((x) == '`'))
#define IS_WHITE_SPACE(x) (((x) == ' ') || ((x) == '\t'))

Indenter::Indenter()
{
}


Indenter::~Indenter()
{
}


void Indenter::process(std::istream *in, std::ostream *out)
{
	streams = new StreamHandler(in, out);

	char ch;
	streams->next_borrow(&ch);
	sanitize_char(&ch); // check for comment, normalize eol
	add_indentable_section(id_keyword(&ch), 0, &ch);

	while (!streams->eof()) {
		next_valid_char(&ch);
		add_indentable_section(id_keyword(&ch), 0, &ch);
	}

	streams->prev_return(&ch);
	streams->flush();

	delete streams;
}


int Indenter::compare_kw_token(const void * a , const void * b)
{
	const std::string buffer = *((std::string *) a);
	const token kw_token = *((token *) b);

	size_t bf_length = buffer.length();
	size_t kw_length = strlen(kw_token.literal);

	int match = (buffer.substr(0, kw_length)).compare(kw_token.literal);

	if (0 == match) {
		if ((bf_length > kw_length) && (IS_NAME_CHAR(buffer[(int) kw_length]))) {
			// not keyword as the next char is a legal name char
			return 1;
		}
	}

	return match;
}

/*
 * takes the char and determines if it's the start of a keyword. if a keyword
 * the function returns the keyword token.
 */
token Indenter::id_keyword(char *ch)
{
	std::string buffer;
	int bf_length;

	if (IS_WHITE_SPACE(*ch) || is_eol(*ch)) {
		return keywords[OTHER];
	}

	// special one char keywords
	if ('(' == *ch) {
		return keywords[BRACKET_OPEN];
	} else if (')' == *ch) {
		return keywords[BRACKET_CLOSE];
	} else if (';' == *ch) {
		return keywords[SEMICOLON];
	} else if ('{' == *ch) {
		return keywords[BRACES_OPEN];
	} else if ('}' == *ch) {
		return keywords[BRACES_CLOSE];
	}

	size_t pos = streams->position();

	if ((0 < pos) && IS_NAME_CHAR(streams->prev_peek())) {
		// not keyword as it follows a legal name char
		return keywords[OTHER];
	}


	// read ahead in stream and construct a tmp buffer, 1 greater then max
	// literal length
	for (bf_length = 0; bf_length < (MAX_LITERAL_SIZE + 1); bf_length++) {
		if (is_eol(*ch)) {
			break;
		}

		buffer.append(1, *ch);
		streams->next(ch);
	}

	// move back to the original input stream position
	streams->travel_to(pos, ch);

	token * ch_token = (token *) bsearch(
	                           (void *) &buffer,
	                           (void *) keywords,
	                           (sizeof(keywords) / sizeof(keywords[0])),
	                           sizeof(token),
	                           &compare_kw_token);

	if (NULL != ch_token) {
		// return matching keyword token
		return *ch_token;
	}

	return keywords[OTHER];
}


bool Indenter::is_at_sol() const
{
	size_t pos = streams->position();

	if (0 == pos) {
		return true;
	}

	char ch = streams->prev_peek();
	while ((0 != streams->position()) && IS_WHITE_SPACE(ch)) {
		streams->prev(&ch);
	}

	if ((0 == streams->position()) || is_eol(ch)) {
		streams->travel_to(pos, &ch);
		return true;
	} else {
		streams->travel_to(pos, &ch);
		return false;
	}
}


bool Indenter::is_eol(char ch) const
{
	return (streams->eof() || ch == '\n' || ch == '\r');
}


void Indenter::normalize_eol(char *ch)
{
	if (!is_eol(*ch)) {
		return;
	}

	// trim trailing white space
	while ((0 < streams->position()) && IS_WHITE_SPACE(streams->prev_peek())) {
		streams->prev_remove();
	}

	if (*ch == '\r') {	// CR+LF is windows otherwise Mac OS 9
		if (streams->next_peek() == '\n') {
			streams->next_remove();
		}
	} else {	// LF is Linux, allow for improbable LF/CR
		if (streams->next_peek() == '\r') {
			streams->next_remove();
		}
	}

	*ch = '\n';
}


// check for comment
void Indenter::sanitize_char(char *ch)
{
	add_if_string(ch);

	// single line comment
	if ((*ch == '/') && (streams->next_peek() == '/')) {
		while (!is_eol(*ch)) {
			streams->next(ch);
		}

		sanitize_char(ch);
		return;

	} else if ((*ch == '/') && (streams->next_peek() == '*')) { // multi-line comment
		streams->next(ch); // add '/'
		streams->next(ch); // add '*'

		normalize_eol(ch);

		while (!streams->eof() && !((*ch == '*') && (streams->next_peek() == '/'))) {
			streams->next(ch);
			normalize_eol(ch);
		}

		streams->next(ch); // add '*'
		streams->next(ch); // add '/'

		sanitize_char(ch);
		return;
	}

	normalize_eol(ch);
}


void Indenter::add_if_string(char *ch)
{
	if (0 == streams->position()) {
		return;
	}

	if (('"' != *ch) || (('"' == *ch) && ('\\' == streams->prev_peek()))) {
		return;
	}

	streams->next(ch);
	sanitize_char(ch);

	while (!streams->eof() && ('"' != *ch)) {
		streams->next(ch);

		if ('"' != *ch) {
			sanitize_char(ch);
		} else if ('\\' == streams->prev_peek()) {
			streams->next(ch);
			sanitize_char(ch);
		}
	}

	streams->next(ch);
	sanitize_char(ch);
}


void Indenter::add_indent_if_sol(int indent_level)
{
	if (is_at_sol()) {
		char ch;

		// trim leading white space
		while ((0 < streams->position()) && IS_WHITE_SPACE(streams->prev_peek())) {
			streams->prev_borrow(&ch);
		}

		for (int xx = 0; xx < indent_level; xx++) {
			streams->append(INDENT);
		}
	}
}


void Indenter::next_valid_char(char *ch)
{
	if (!is_eol(*ch) && !IS_WHITE_SPACE(*ch)) {
		streams->next(ch);
		sanitize_char(ch);
	}

	while (!streams->eof() && (is_eol(*ch) || IS_WHITE_SPACE(*ch))) {
		streams->next(ch);
		sanitize_char(ch);
	}

	add_indent_if_sol(0);
}


void Indenter::indent_statement(int indent_level, char *ch)
{
	// add indent if this is at the start of a line
	add_indent_if_sol(indent_level);

	// keep going until you get a ";"
	while (!streams->eof() && (';' != *ch)) {
		streams->next(ch);
		sanitize_char(ch);
	}

	// add till the EOL
	while (!is_eol(*ch)) {
		streams->next(ch);
		sanitize_char(ch);
	}
}


void Indenter::indent_loops(int indent_level, char *ch)
{
	bool done = false;

	add_indent_if_sol(indent_level);

	// add the 1st identifying char
	streams->next(ch);
	sanitize_char(ch);

	token ch_token = id_keyword(ch);

	while (!streams->eof() && !done) {
		if ((TA_BLOCK == ch_token.action) || (SEMICOLON == ch_token.id)) {
			add_indentable_section(ch_token, indent_level, ch);
			done = true;
		} else if (IF == ch_token.id) {
			add_indentable_section(ch_token, indent_level + 1, ch);
			done = true;
		} else {
			if (!add_indentable_section(ch_token, indent_level + 1, ch)) {
				add_indent_if_sol(indent_level + 1);
				streams->next(ch);
				sanitize_char(ch);
			}
			ch_token = id_keyword(ch);
		}
	}
}


void Indenter::indent_if(int indent_level, char *ch)
{
	bool done = false;

	add_indent_if_sol(indent_level);
	indent_level++;

	// add the 'i' char
	streams->next(ch);
	sanitize_char(ch);

	token ch_token = id_keyword(ch);

	while (!streams->eof() && !done) {
		if ((TA_BLOCK == ch_token.action) || (IF == ch_token.id) || (SEMICOLON == ch_token.id)) {
			add_indentable_section(ch_token, indent_level - 1, ch);

			size_t pos = streams->position();
			next_valid_char(ch);
			ch_token = id_keyword(ch);

			if (ELSE != ch_token.id) {
				// go back to position in stream
				streams->travel_to(pos, ch);
				done = true;
			}
		} else {
			if (ELSE == ch_token.id) {
				add_indent_if_sol(indent_level - 1);
			} else {
				add_indent_if_sol(indent_level);
			}

			if (!add_indentable_section(ch_token, indent_level, ch)) {
				streams->next(ch);
				sanitize_char(ch);
			}

			ch_token = id_keyword(ch);
		}
	}
}


void Indenter::indent_block(token end, int indent_level, char *ch)
{
	add_indent_if_sol(indent_level);
	indent_level++;

	streams->next(ch);
	sanitize_char(ch);

	if (is_eol(*ch) || IS_WHITE_SPACE(*ch)) {
		next_valid_char(ch);
	}

	token ch_token = id_keyword(ch);
	while (!streams->eof() && (end.id != ch_token.id)) {
		if (!add_indentable_section(ch_token, indent_level, ch)) {
			add_indent_if_sol(indent_level);
			next_valid_char(ch);
		}

		ch_token = id_keyword(ch);
	}

	if (!streams->eof()) {
		indent_level--;
		add_indent_if_sol(indent_level);
		for (unsigned int xx = 0; xx < strlen(end.literal); xx++) {
			streams->next(ch);
		}
		sanitize_char(ch);
	}
}


void Indenter::indent_module(int indent_level, char *ch)
{
	// bracket matching counter
	int bracket = 0;

	add_indent_if_sol(indent_level);
	indent_level++;

	while (('#' != *ch) && ('(' != *ch) && (';' != *ch)) {
		next_valid_char(ch);
	}

	if (';' != *ch) {
		if ('#' == *ch) {
			if (is_at_sol()) {
				// custom indent for '#('
				add_indent_if_sol(indent_level);
				streams->prev_remove();
				streams->prev_remove();
			}

			while (((0 != bracket) || (')' != *ch))) {
				next_valid_char(ch);
				add_indent_if_sol(indent_level);

				if ('(' == *ch) bracket++;
				if (')' == *ch) bracket--;
			}

			next_valid_char(ch);

			if ('(' != *ch) {
				add_indent_if_sol(indent_level - 1);
			}

			while ('(' != *ch) {
				next_valid_char(ch);
			}
		}

		// by this stage *ch will always be == '('
		bracket++;
		if (is_at_sol()) {
			add_indent_if_sol(indent_level);
			streams->prev_remove();
		}

		while (((0 != bracket) || (')' != *ch))) {
			next_valid_char(ch);
			add_indent_if_sol(indent_level);

			if ('(' == *ch) bracket++;
			if (')' == *ch) bracket--;
		}

		// custom indent for end ');'
		add_indent_if_sol(indent_level - 1);

		// add ')' get the next char and sanitize
		next_valid_char(ch);

		while (';' != *ch) {
			next_valid_char(ch);
		}

		// add ';' get the next char and sanitize
		streams->next(ch);
		normalize_eol(ch);
		sanitize_char(ch);
	}

	indent_block(keywords[ENDMODULE], (indent_level - 1), ch);
}


bool Indenter::add_indentable_section(token ch_token, int indent_level, char *ch)
{
	bool indent_performed = true;

	switch (ch_token.action) {
	case TA_MACRO:
		add_indent_if_sol(0);
		next_valid_char(ch);
		break;
	case TA_STATEMENT :
		indent_statement(indent_level, ch);
		streams->prev(ch);
		break;
	case TA_LOOP :
		indent_loops(indent_level, ch);
		break;
	case TA_IF :
		indent_if(indent_level, ch);
		break;
	case TA_MODULE :
		indent_module(0, ch);
		break;
	case TA_GROUP:
		switch (ch_token.id) {
		case BRACKET_OPEN :
			indent_block(keywords[BRACKET_CLOSE], indent_level, ch);
			break;
		case BRACES_OPEN :
			indent_block(keywords[BRACES_CLOSE], indent_level, ch);
			break;
		default :
			indent_performed = false;
			break;
		}
		break;
	case TA_BLOCK :
		switch (ch_token.id) {
		case BEGIN :
			indent_block(keywords[END], indent_level, ch);
			break;
		case FORK :
			indent_block(keywords[JOIN], indent_level, ch);
			break;
		case GENERATE :
			indent_block(keywords[ENDGENERATE], indent_level, ch);
			break;
		case FUNCTION :
			indent_block(keywords[ENDFUNCTION], indent_level, ch);
			break;
		case PRIMITIVE :
			indent_block(keywords[ENDPRIMITIVE], indent_level, ch);
			break;
		case SPECIFY :
			indent_block(keywords[ENDSPECIFY], indent_level, ch);
			break;
		case TABLE :
			indent_block(keywords[ENDTABLE], indent_level, ch);
			break;
		case TASK :
			indent_block(keywords[ENDTASK], indent_level, ch);
			break;
		case PRO_IFDEF :
		case PRO_IFNDEF :
			indent_block(keywords[PRO_ENDIF], 0, ch);
			break;
		case CASE :
		case CASEX :
		case CASEZ :
			indent_block(keywords[ENDCASE], indent_level, ch);
			break;
		default :
			indent_performed = false;
			break;
		}
		break;
	default :
		indent_performed = false;
		break;
	}


	return indent_performed;
}
