# PL_JSON - Yet another single header json parser

Single-file MIT licensed library for C/C++


## About

PL_JSON decodes JSON into a structure that is easy to use. 

PL_JSON is reasonably fast, with a throughput of about 1.5 GB/sec on my 
Macbook M2. This is certainly not the fastest parser, but it's far from the
slowest.

With about 250 lines of code (sans API) PL_JSON is definitely one of the 
simplest parsers out there. The provided API for accessing the resulting
`json_t` is as easy to use as it gets.

This library does not check the JSON data for conformance. Escape sequences 
such as `\n`, `\r`, or `\\` are handled, but unicode escape sequences (`\uxxxx`)
are not - these will be replaced by a single `?`` character. So use JSON that
has utf8 strings instead of unicode escape sequences.

The caller of is responsible for memory allocation. To facilitate memory 
efficient use, the parsing is split into two steps: 
	1) tokenization into a temporary token buffer
	2) parsing of the tokens into the final structure


## Synopsis:

```c
// Define `PL_JSON_IMPLEMENTATION` in *one* C/C++ file before including this
// library to create the implementation.

#define PL_JSON_IMPLEMENTATION
#include "pl_json.h"

// Read file
FILE *fh = fopen(path, "rb");
fseek(fh, 0, SEEK_END);
int size = ftell(fh);
fseek(fh, 0, SEEK_SET);

char *json_text = malloc(size);
fread(json_text, 1, size, fh);
fclose(fh);

// Tokenize JSON. Allocate some temporary memory that is big enough to hold the 
// expected number of tokens. The maximum number of tokens for a given byte size
// can be no larger than 1 + size / 2 
// E.g. for the worst case `[0,0,0]` = 7 bytes, 4 tokens
unsigned int tokens_capacity = 1 + size / 2;
json_token_t *tokens = malloc(tokens_capacity * sizeof(json_token_t));

unsigned int size_req;
unsigned int tokens_len = json_tokenize(json_text, size, tokens, tokens_capacity, &size_req);
assert(tokens_len > 0);

// Parse tokens. Allocate memory for the final json object. The size_req 
// returned by json_tokenize is always sufficient and close to optimal.
json_t *v = malloc(size_req);
json_parse_tokens(json_text, tokens, tokens_len, v);

// The json_text and tokens are no longer required and can be freed.
free(json_text);
free(tokens);

// Assuming the input `[1, 2, 3, "hello", 4, "world"]`, print the array of 
// numbers and strings:

if (v->type == JSON_ARRAY) {
	json_t *values = v->data;
	for (int i = 0; i < v->len; i++) {
		if (values[i].type == JSON_STRING) {
			printf("string with length %d: %s\n", values[i].len, values[i].string);
		}
		else if (values[i].type == JSON_NUMBER) {
			printf("number: %f\n", values[i].number);	
		}
	}
}

// Some convenience functions can be used to access the data of json_t; these
// functions are null-safe and check the underlying type. So doing something 
// like this is always fine, as long as you check the final return value:

char *str = json_string(json_value_for_key(v, "foobar"));
if (str != null) {
	printf("string value for key foobar: %s", str);
}

// Finally, when you're done with the parsed json, just free it again.
free(v);
```

See [pl_json.h](https://github.com/phoboslab/pl_json/blob/master/pl_json.h) for
the full documentation.


## Limitations

- does not handle unicode escape sequences `\uxxxx` in strings
- limited nesting depth (256 by default)
- will happily parse invalid numbers, such as `10.20.30` (if your JSON is valid, 
this is not an issue)
