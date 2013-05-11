/**
 * @file Indenter.h
 *
 * @author  Berin Martini
 * @license GPL v2+
 */

#ifndef INDENTER_H
#define INDENTER_H


#include "StreamHandler.h"
#include "KeywordToken.h"


class Indenter
{
public:
	Indenter(std::string indent_str);
	virtual ~Indenter();

	void process(std::istream *in, std::ostream *out);

private:
	std::string* indent;
	StreamHandler* streams;

	static int compare_kw_token(const void * a , const void * b);
	token id_keyword(StreamHandler* streams, char *ch);

	bool is_at_sol(StreamHandler* streams) const;
	bool is_eol(StreamHandler* streams, char ch) const;

	void normalize_eol(char *ch);
	void sanitize_char(char *ch);
	void add_if_string(StreamHandler* streams, char *ch);
	void add_indent_if_sol(StreamHandler* streams, int indentLevel);
	void next_valid_char(StreamHandler* streams, char *ch);

	void indent_statement(int indentLevel, char *ch);
	void indent_module(int indentLevel, char *ch);
	void indent_module_bracket(int indentLevel, char *ch);
	void indent_if(int indentLevel, char *ch);
	void indent_loops(int indentLevel, char *ch);
	void indent_block(token end, int indentLevel, char *ch);

	bool add_indentable_section(StreamHandler* streams, token ch_token, int indentLevel, char *ch);
};


#endif // closes INDENTER_H
