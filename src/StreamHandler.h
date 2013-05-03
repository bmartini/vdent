/**
 * @file StreamHandler.h
 *
 * @author  Berin Martini
 * @license GPL v2+
 */

#ifndef STREAM_HANDLER_H
#define STREAM_HANDLER_H


#include <string>
class istream;
class ostream;


class StreamHandler
{
public:
	StreamHandler(std::istream *in, std::ostream *out);
	virtual ~StreamHandler();

	bool eof();

	void next(char *ch);
	void next_borrow(char *ch);
	void next_return(char *ch);
	void next_remove();
	char next_peek();

	void prev(char *ch);
	void prev_borrow(char *ch);
	void prev_return(char *ch);
	void prev_remove();
	char prev_peek();

	void append(std::string addition);
	void travel_to(size_t position, char *ch);
	size_t position();

	void flush();
private:
	std::istream * in_stream;
	std::ostream * out_stream;

	std::string sbuffer;
};


#endif // closes STREAM_HANDLER_H
