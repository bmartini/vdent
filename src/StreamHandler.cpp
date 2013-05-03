/**
 * @file StreamHandler.cpp
 *
 * @author  Berin Martini
 * @license GPL v2+
 */

#include "StreamHandler.h"
#include <iostream>


StreamHandler::StreamHandler(std::istream *in, std::ostream *out)
{
	in_stream = in;
	out_stream = out;
}


StreamHandler::~StreamHandler()
{
	out_stream->flush();
}


bool StreamHandler::eof()
{
	return in_stream->eof();
}


void StreamHandler::next(char *ch)
{
	sbuffer.append(1, *ch);
	in_stream->get(*ch);
}


void StreamHandler::next_remove()
{
	in_stream->get();
}


void StreamHandler::next_borrow(char *ch)
{
	in_stream->get(*ch);
}


void StreamHandler::next_return(char *ch)
{
	in_stream->putback(*ch);
}


char StreamHandler::next_peek()
{
	return in_stream->peek();
}


void StreamHandler::prev(char *ch)
{
	in_stream->putback(*ch);

	size_t end = sbuffer.length() - 1;
	*ch = sbuffer.at(end);
	sbuffer.erase(end, 1);
}


void StreamHandler::prev_remove()
{
	size_t end = sbuffer.length() - 1;
	sbuffer.erase(end, 1);
}


void StreamHandler::prev_borrow(char *ch)
{
	size_t end = sbuffer.length() - 1;
	*ch = sbuffer.at(end);
	sbuffer.erase(end, 1);
}


void StreamHandler::prev_return(char *ch)
{
	sbuffer.append(1, *ch);
}


char StreamHandler::prev_peek()
{
	size_t end = sbuffer.length() - 1;
	return sbuffer.at(end);
}


void StreamHandler::travel_to(size_t position, char *ch)
{
	size_t end = sbuffer.length();

	while (end != position) {
		if (end < position) {
			next(ch);
		} else if (end > position) {
			prev(ch);
		}
		end = sbuffer.length();
	}
}


size_t StreamHandler::position()
{
	return sbuffer.length();
}


void StreamHandler::append(std::string addition)
{
	sbuffer.append(addition);
}


void StreamHandler::flush()
{
	*out_stream << sbuffer;
	out_stream->flush();
	sbuffer.clear();
}
