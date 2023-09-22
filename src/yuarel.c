/**
 * Copyright (C) 2016,2017 Jack Engqvist Johansson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Stripped for lwNBD use (server) by Ronan Bignaux
 */

//#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "yuarel.h"

/**
 * Find a character in a string, replace it with '\0' and return the next
 * character in the string.
 *
 * str: the string to search in.
 * find: the character to search for.
 *
 * Returns a pointer to the character after the one to search for. If not
 * found, NULL is returned.
 */
static inline char *
find_and_terminate(char *str, char find)
{
	str = strchr(str, find);
	if (NULL == str) {
		return NULL;
	}

	*str = '\0';
	return str + 1;
}

/* Yes, the following functions could be implemented as preprocessor macros
     instead of inline functions, but I think that this approach will be more
     clean in this case. */
static inline char *
find_fragment(char *str)
{
	return find_and_terminate(str, '#');
}

static inline char *
find_query(char *str)
{
	return find_and_terminate(str, '?');
}

static inline char *
find_path(char *str)
{
	char *strt = find_and_terminate(str, '/');
	return (strt == NULL) ? str : strt;
}

/**
 * Parse a URL string to a struct.
 *
 * url: pointer to the struct where to store the parsed URL parts.
 * u:   the string containing the URL to be parsed.
 *
 * Returns 0 on success, otherwise -1.
 */
int
yuarel_parse(struct yuarel *url, char *u)
{
	if (NULL == url || NULL == u) {
		return -1;
	}

	memset(url, 0, sizeof (struct yuarel));

	/* (Fragment) */
	url->fragment = find_fragment(u);

	/* (Query) */
	url->query = find_query(u);


	/* (Path) */
	url->path = find_path(u);
	return 0;
}

/**
 * Split a path into several strings.
 *
 * No data is copied, the slashed are used as null terminators and then
 * pointers to each path part will be stored in **parts. Double slashes will be
 * treated as one.
 *
 * path: the path to split.
 * parts: a pointer to an array of (char *) where to store the result.
 * max_parts: max number of parts to parse.
 */
int
yuarel_split_path(char *path, char **parts, int max_parts)
{
	int i = 0;

	if (NULL == path || '\0' == *path) {
		return -1;
	}

	do {
		/* Forward to after slashes */
		while (*path == '/') path++;

		if ('\0' == *path) {
			break;
		}

		parts[i++] = path;

		path = strchr(path, '/');
		if (NULL == path) {
			break;
		}

		*(path++) = '\0';
	} while (i < max_parts);

	return i;
}

int
yuarel_parse_query(char *query, char delimiter, struct yuarel_param *params, int max_params)
{
	int i = 0;

	if (NULL == query || '\0' == *query) {
		return -1;
	}

	params[i++].key = query;
	while (i < max_params && NULL != (query = strchr(query, delimiter))) {
		*query = '\0';
		params[i].key = ++query;
		params[i].val = NULL;

		/* Go back and split previous param */
		if (i > 0) {
			if ((params[i - 1].val = strchr(params[i - 1].key, '=')) != NULL) {
				*(params[i - 1].val)++ = '\0';
			}
		}
		i++;
	}

	/* Go back and split last param */
	if ((params[i - 1].val = strchr(params[i - 1].key, '=')) != NULL) {
		*(params[i - 1].val)++ = '\0';
	}

	return i;
}
