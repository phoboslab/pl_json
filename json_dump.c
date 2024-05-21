/*

Copyright (c) 2024, Dominic Szablewski - https://phoboslab.org
SPDX-License-Identifier: MIT


Example for pl_json.h: dumps the structure of a json file

Compile with: 
	gcc json_dump.c -std=c99 -O3 -o json_dump

*/

#include <stdio.h>
#include <stdlib.h>

#define PL_JSON_IMPLEMENTATION
#include "pl_json.h"

void json_dump(json_t *v, int depth) {
	if (v->type == JSON_TRUE) {
		printf("true\n");
	}
	else if (v->type == JSON_FALSE) {
		printf("false\n");
	}
	else if (v->type == JSON_NULL) {
		printf("null\n");
	}
	else if (v->type == JSON_NUMBER) {
		printf("number: %f\n", v->number);
	}
	else if (v->type == JSON_STRING) {
		printf("string (%d): \"%s\"\n", v->len, v->string);
	}
	else if (v->type == JSON_ARRAY) { 
		printf("array (%d):\n", v->len);
		json_t *values = json_values(v);
		for (int i = 0; i < v->len; i++) {
			for (int i = 0; i < depth; i++) {
				printf("    ");
			}
			printf("[%d] ", i);
			json_dump(&values[i], depth+1);
		}
	}
	else if (v->type == JSON_OBJECT) {
		printf("object (%d):\n", v->len);
		json_t *values = json_values(v);
		char **keys = json_keys(v);
		for (int i = 0; i < v->len; i++) {
			for (int i = 0; i < depth; i++) {
				printf("    ");
			}
			printf("[%s] ", keys[i]);
			json_dump(&values[i], depth+1);
		}
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		puts("Usage: json_dump <infile.json>");
		return 1;
	}

	// Read file
	FILE *fh = fopen(argv[1], "rb");
	if (!fh) {
		printf("Failed to open %s\n", argv[1]);
		return 2;
	}

	fseek(fh, 0, SEEK_END);
	int size = ftell(fh);
	fseek(fh, 0, SEEK_SET);

	char *json_text = malloc(size);
	if (fread(json_text, 1, size, fh) != size) {
		printf("Failed to read %s\n", argv[1]);
		return 2;
	}
	fclose(fh);

	// Tokenize JSON. Allocate some temporary memory that is big enough to hold the 
	// expected number of tokens. The maximum number of tokens for a given byte size
	// can be no larger than 1 + size / 2 
	// E.g. for the worst case `[0,0,0]` = 7 bytes, 4 tokens
	unsigned int tokens_capacity = 1 + size / 2;
	json_token_t *tokens = malloc(tokens_capacity * sizeof(json_token_t));

	unsigned int size_req;
	int tokens_len = json_tokenize(json_text, size, tokens, tokens_capacity, &size_req);
	if (tokens_len <= 0) {
		printf("tokenize error %d\n", tokens_len);
		return 3;
	}

	printf("tokens: %d\n", tokens_len);


	// Parse tokens. Allocate memory for the final json object. The size_req 
	// returned by json_tokenize is always sufficient and close to optimal.
	json_t *v = malloc(size_req);
	json_parse_tokens(json_text, tokens, tokens_len, v);

	// The json_text and tokens are no longer required and can be freed.
	free(json_text);
	free(tokens);

	// Dump json
	json_dump(v, 0);
	printf("\n%d tokens\n%d bytes required for parsed json_t\n", tokens_len, size_req);


	free(v);

	return 0;
}