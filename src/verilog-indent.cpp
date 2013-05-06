#include "Indenter.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <getopt.h>


int main(int argc, char* argv[])
{
	int space_per_indent = 4;
	int opt;
	while ((opt = getopt(argc, argv, "hs:")) != -1) {
		switch (opt) {
		case 's':
			space_per_indent = atoi(optarg);
			break;
		case 'h':
		default: /* '?' */
			fprintf(stderr, "Usage: %s [-h] [-s num] < input.v [ > output.v ]\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (space_per_indent < 2) {
		fprintf(stderr, "Error: Number of spaces per indent must be equal to or greater then 2.\n");
		exit(EXIT_FAILURE);
	}
	std::string indent = std::string((size_t) space_per_indent, ' ');

	Indenter* indenter = new Indenter(indent);

	indenter->process(&std::cin, &std::cout);

	delete indenter;

	return 0;
}
