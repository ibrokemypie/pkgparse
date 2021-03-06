/* Copyright (c) 2009 Sebastian Nowicki <sebnow@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utility.h"

/* Function: _strcpy_partial
Copy a substring, from start to end.

Parameters:
	string - The string to be copied.
	start - A marker indicating the beginning of the substring.
	end - A marker indicating the end of a substring.

Returns:
	A copy of the string between start and end. The returned string should be
	deallocated by the caller.
*/
static char *_strcpy_partial(char *string, char *start, char *end);

/* Function: _find_next_substitution
Locate the next word substitution within a string.

Example:
	(start code)
	char *text = "foo$bar baz";
	char *start = NULL;
	char *end = NULL;
	int status = 0;

	status = _find_next_substitution(text, &start, &end);
	// start -> '$'
	// end -> 'r'
	// status -> 1
	(end)

Parameters:
	string - The string to be searched.
	start - The address where the location of the next substitution should be
		stored. The location will always point to the sigil ('$').
	end - The address where the location of the end of the word should be stored.

Returns:
	True (1) on success, otherwise false (0).
*/
static int _find_next_substitution(char *string, char **start, char **end);

/* Function: _substitute_words
Substitute variables with their values

Parameters:
	table - A symbol table containing the values of variables
	string - The string to be parsed.

Returns:
	A string with substituted words, or NULL on error. The returned string
	should be deallocated by the caller.

See Also:
	<sh_parse_word()>
*/
static char *_substitute_words(table_t *table, char *string);

/* Function: _array_cat
Concatenate an array of strings to a single string.

Parameters:
	array - An array of strings.

Returns:
	A string consisting of all elements in array, delimited by a space, or NULL
	on error, or if array is empty. The returned string should be deallocated by
	the caller.
*/
static char *_array_cat(char **array);

static char *_strcpy_partial(char *string, char *start, char *end)
{
	char *result;
	size_t len = end - start + 1;

	result = malloc((len + 1) * sizeof(*result));
	result = strncpy(result, string, len);
	result[len] = '\0';
	return result;
}

static char *_array_cat(char **array)
{
	char *result = NULL;
	size_t size = 0;
	int i;

	if(array == NULL || array[0] == NULL) {
		return result;
	}

	for(i = 0; array[i] != NULL; i++) {
		size += strlen(array[i]);
	}
	size += i - 1; /* spaces between elements */
	result = malloc(sizeof(*result) * (size + 1));
	result[size] = '\0';

	for(i = 0; array[i] != NULL; i++) {
		result = strncat(result, array[i], size);
		if(array[i + 1] != NULL) {
			result = strncat(result, " ", size);
		}
	}
	return result;
}

static size_t _array_size(const char *array)
{
	const char *str_ptr = array;
	char quote_char;
	int in_quote = 0;
	size_t count = 0;
	int end_of_array;

	/* Skip left parenthesis, otherwise attempt to count elements anyway */
	if(*str_ptr == '(') {
		str_ptr++;
	}

	end_of_array = *array == ')' || *array == '\0';

	while(!end_of_array) {
		switch(*str_ptr) {
			/* Entering or exitting quote */
			case '\'':
			case '"':
				if(*(str_ptr - 1) != '\\') {
					in_quote = !(in_quote && *str_ptr == quote_char);
					quote_char = *str_ptr;
				}
				break;
			/* Element separator, if not in quote */
			case ' ':
			case '\t':
			case '\n':
				if(!in_quote && *(str_ptr - 1) != '\\') {
					count++;
				}
				break;
			case ')':
				if(!in_quote && *(str_ptr - 1) != '\\') {
					count++;
					end_of_array = 1;
				}
				break;
			case '\0':
				end_of_array = 1;
				break;
			default:
				break;
		}
		str_ptr++;
	}
	return count;
}

char **sh_split_array(char *string)
{
	size_t count_elem;
	int in_quote = 0;
	unsigned int elem;
	char quote_char = '\0';
	char *str_cpy;
	char *str_ptr;
	char *start_ptr;
	char **array;
	int end_of_array;

	/* Copy string as we will be mutating it */
	str_cpy = strdup(string);

	count_elem = _array_size(str_cpy);

	array = malloc((count_elem + 1) * sizeof(*array));
	array[count_elem] = NULL;
	elem = 0;

	end_of_array = count_elem == 0;

	/* Skip the left parenthesis */
	str_ptr = str_cpy + 1;
	start_ptr = str_ptr;
	/* Loop until end of string, and extract the elements */
	while(!end_of_array) {
		switch(*str_ptr) {
			/* Entering or exitting quote */
			case '\'':
			case '"':
				if(*(str_ptr - 1) != '\\') {
					in_quote = !(in_quote && *str_ptr == quote_char);
					quote_char = *str_ptr;
				}
				break;
			/* Element separator, if not in quote */
			case ' ':
			case '\t':
			case '\n':
				if(!in_quote && *(str_ptr - 1) != '\\') {
					*str_ptr = '\0';
					/* Skip multiple spaces */
					if(strlen(start_ptr) != 1 && !isspace(*start_ptr)) {
						array[elem] = strdup(start_ptr);
						elem++;
					}
					start_ptr = str_ptr + 1;
				}
				break;
			case ')':
				if(!in_quote && *(str_ptr - 1) != '\\') {
					*str_ptr = '\0';
					array[elem] = strdup(start_ptr);
					end_of_array = 1;
				}
				break;
			case '\0':
				end_of_array = 1;
				break;
			default:
				break;
		}
		str_ptr++;
	}

	free(str_cpy);

	return array;
}

/* TODO: Unquote strings in the middle of a string, e.g.
 * "foo'bar baz'zer" */
char *sh_unquote(char *string)
{
	size_t len = 0;
	char *str_ptr = string;
	char *return_string = NULL;

	if(string == NULL) {
		return NULL;
	}

	if(*str_ptr == '\'' || *str_ptr == '"') {
		str_ptr++;
		len = strlen(str_ptr);
		return_string = malloc(len * sizeof(*string));
		return_string = strncpy(return_string, str_ptr, len - 1);
		return_string[len - 1] = '\0';
	} else {
		/* Since we allocate when the string is quoted, we should do so
         * when it's not for consistency. */
		return_string = strdup(string);
	}

	return return_string;
}

static int _find_next_substitution(char *string, char **start, char **end)
{
	char *str_ptr;
	int escaped = 0;
	int in_literal_quote = 0;
	int in_brace = 0;
	int variable = 0;
	int found = 0;
	int end_of_word;

	*start = NULL;
	*end = NULL;

	for(str_ptr = string; *str_ptr != '\0' && !found; str_ptr++) {
		switch(*str_ptr) {
			case '\'':
				in_literal_quote = !escaped && !in_literal_quote;
				escaped = 0;
				break;
			case '\\':
				escaped = !escaped;
				break;
			case '$':
				if(!escaped && !in_literal_quote) {
					variable = 1;
					*start = str_ptr;
				}
				escaped = 0;
				break;
			case '{':
				in_brace = variable && !escaped;
				break;
			case '}':
				if(in_brace) {
					*end = str_ptr;
					found = 1;
				}
				break;
			default:
				end_of_word = !isalnum(*(str_ptr + 1))
					&& *(str_ptr + 1) != '_'
					|| *(str_ptr + 1) == '\0';
				if(variable && !in_brace && end_of_word) {
					*end = str_ptr;
					found = 1;
				}
				escaped = 0;
				break;
		}
	}
	return found;
}

static char *_substitute_words(table_t *table, char *string)
{
	size_t len = 0;
	size_t result_len;
	char *str_ptr = string;
	char *start = NULL;
	char *end = NULL;
	char *word = NULL;
	char *result = NULL;
	char *value = NULL;
	int free_value = 0;
	symbol_t *symbol = NULL;

	if(!_find_next_substitution(str_ptr, &start, &end)) {
		return strdup(string);
	} else {
		/* The string has to be NULL terminated for strncpy() to work */
		result = malloc(sizeof(*result));
		result[0] = '\0';
		result_len = 1;
	}

	while(_find_next_substitution(str_ptr, &start, &end)) {
		/* Skip word identifier marks ("${...}") */
		if(*(start + 1) == '{' && *end == '}') {
			word = _strcpy_partial(start + 2, start + 2, end - 1);
		} else {
			word = _strcpy_partial(start + 1, start + 1, end);
		}
		symbol = table_lookupr(table, word);
		free(word);

		if(symbol != NULL) {
			if(symbol_type(symbol) == kSymbolTypeArray) {
				value = _array_cat(symbol_array(symbol));
				free_value = 1;
			} else {
				value = symbol_string(symbol);
			}
			len = strlen(value);
			result_len += start - str_ptr + len;
			result = realloc(result, result_len * sizeof(*result));
			/* Concatenate the string preceeding substitution */
			result = strncat(result, str_ptr, (start - str_ptr) * sizeof(*result));
			result = strncat(result, value, len);
			if(free_value) {
				free(value);
				free_value = 0;
				value = NULL;
			}
		}

		str_ptr = end + 1;
	}

	/* Append the remainder of the string */
	if(strlen(str_ptr) > 0) {
		len = strlen(str_ptr);
		result_len += len;
		result = realloc(result, result_len * sizeof(*result));
		result = strncat(result, str_ptr, len);
	}
	
	return result;
}

char **sh_parse_array(table_t *table, char *string)
{
	char **result;
	char **array_ptr;
	char *parsed;
	result = sh_split_array(string);
	if(result != NULL && table != NULL) {
		for(array_ptr = result; *array_ptr != NULL; array_ptr++) {
			parsed = sh_parse_word(table, *array_ptr);
			free(*array_ptr);
			*array_ptr = parsed;
			parsed = NULL;
		}
	}
	return result;
}

char *sh_parse_word(table_t *table, char *string)
{
	char *substituted;
	char *parsed;

	substituted = _substitute_words(table, string);
	parsed = sh_unquote(substituted);
	free(substituted);
	return parsed;
}

