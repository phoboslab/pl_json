/*

Copyright (c) 2024, Dominic Szablewski - https://phoboslab.org
SPDX-License-Identifier: MIT


fuzzing harness for pl_json.h

Compile and run with: 
	clang -fsanitize=address,fuzzer -g -O3 fuzz.c -o fuzz && ./fuzz

*/

#include <stdint.h>
#include <stdlib.h>

#define PL_JSON_IMPLEMENTATION
#include "pl_json.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	unsigned int tokens_capacity = 1 + size / 2;
	json_token_t *tokens = malloc(tokens_capacity * sizeof(json_token_t));
	unsigned int size_req;
	int tokens_len = json_tokenize((char *)data, size, tokens, tokens_capacity, &size_req);
	if (tokens_len > 0) {
		json_t *v = malloc(size_req);
		json_parse_tokens((char *)data, tokens, tokens_len, v);
		free(v);
	}
	free(tokens);
	return 0;
}