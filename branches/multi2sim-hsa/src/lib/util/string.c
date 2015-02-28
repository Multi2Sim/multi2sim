/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>

#include <lib/mhandle/mhandle.h>

#include "debug.h"
#include "list.h"
#include "misc.h"
#include "string.h"



/*
 * String maps
 */

static void str_map_fatal(struct str_map_t *map, char *err_msg)
{
	int i;

	fprintf(stderr, "fatal: %s\n\n", err_msg);
	fprintf(stderr, "\tPossible values are:\n");
	for (i = 0; i < map->count; i++)
		fprintf(stderr, "\t* '%s'\n", map->map[i].string);
	fprintf(stderr, "\n");
	exit(1);
}


int str_map_string_err(struct str_map_t *map, char *string, int *err_ptr)
{
	int i;

	/* No error */
	if (err_ptr)
		*err_ptr = 0;

	/* Find string in map */
	for (i = 0; i < map->count; i++)
		if (!strcmp(string, map->map[i].string))
			return map->map[i].value;

	/* Error */
	if (err_ptr)
		*err_ptr = 1;
	return 0;
}


int str_map_string(struct str_map_t *map, char *string)
{
	return str_map_string_err(map, string, NULL);
}


int str_map_string_err_msg(struct str_map_t *map, char *s, char *err_msg)
{
	int err;
	int value;

	/* Map string */
	value = str_map_string_err(map, s, &err);
	if (!err)
		return value;
	
	/* On error, dump fatal message */
	str_map_fatal(map, err_msg);
	return 0;
}


int str_map_string_case_err(struct str_map_t *map, char *s, int *err_ptr)
{
	int i;

	/* No error */
	if (err_ptr)
		*err_ptr = 0;

	/* Find string in map */
	for (i = 0; i < map->count; i++)
		if (!strcasecmp(s, map->map[i].string))
			return map->map[i].value;

	/* Error */
	if (err_ptr)
		*err_ptr = 1;
	return 0;
}


int str_map_string_case(struct str_map_t *map, char *s)
{
	return str_map_string_case_err(map, s, NULL);
}


int str_map_string_case_err_msg(struct str_map_t *map, char *s, char *err_msg)
{
	int err;
	int value;

	/* Map string */
	value = str_map_string_case_err(map, s, &err);
	if (!err)
		return value;
	
	/* On error, dump fatal message */
	str_map_fatal(map, err_msg);
	return 0;
}


static char *unknown = "<unknown>";
char *str_map_value(struct str_map_t *map, int value)
{
	int i;
	for (i = 0; i < map->count; i++)
		if (map->map[i].value == value)
			return map->map[i].string;
	return unknown;
}


void str_map_value_buf(struct str_map_t *map, int value, char *buf, int size)
{
	int i;

	for (i = 0; i < map->count; i++)
	{
		if (map->map[i].value == value)
		{
			snprintf(buf, size, "%s", map->map[i].string);
			return;
		}
	}
	snprintf(buf, size, "%d", value);
}


void str_map_flags(struct str_map_t *map, int flags, char *out, int size)
{
	int i;
	char *comma = "";
	char temp[MAX_STRING_SIZE];

	str_printf(&out, &size, "{");
	for (i = 0; i < 32; i++)
	{
		if (flags & (1 << i))
		{
			str_printf(&out, &size, "%s", comma);
			str_map_value_buf(map, 1 << i, temp, sizeof temp);
			str_printf(&out, &size, "%s", temp);
			comma = "|";
		}
	}
	str_printf(&out, &size, "}");
}




/*
 * Token List
 */

struct list_t *str_token_list_create(char *str, char *delim)
{
	struct list_t *token_list;

	char *token;

	/* Create list */
	token_list = list_create();

	/* Split string into tokens */
	str = xstrdup(str);
	token = strtok(str, delim);
	while (token)
	{
		/* Insert in token list */
		token = xstrdup(token);
		list_add(token_list, token);

		/* Next token */
		token = strtok(NULL, delim);
	}

	/* Free copy of 'str' */
	free(str);

	/* Return token list */
	return token_list;
}


void str_token_list_free(struct list_t *token_list)
{
	int i;

	char *token;

	/* Free tokens */
	LIST_FOR_EACH(token_list, i)
	{
		token = list_get(token_list, i);
		str_free(token);
	}

	/* Free token list */
	list_free(token_list);
}


char *str_token_list_shift(struct list_t *token_list)
{
	char *token;

	/* Nothing is list of tokens is empty */
	if (!list_count(token_list))
		return "";

	/* Eliminate first token */
	token = list_remove_at(token_list, 0);
	str_free(token);

	/* Return the new first token */
	return str_token_list_first(token_list);
}


char *str_token_list_first(struct list_t *token_list)
{
	/* No token, return empty string */
	if (!list_count(token_list))
		return "";
	
	/* Return first token */
	return (char *) list_get(token_list, 0);
}


int str_token_list_find(struct list_t *token_list, char *token)
{
	int index;

	LIST_FOR_EACH(token_list, index)
		if (!strcmp(token, list_get(token_list, index)))
			return index;
	return -1;
}


int str_token_list_find_case(struct list_t *token_list, char *token)
{
	int index;

	LIST_FOR_EACH(token_list, index)
		if (!strcasecmp(token, list_get(token_list, index)))
			return index;
	return -1;
}


void str_token_list_dump(struct list_t *token_list, FILE *f)
{
	int i;

	char *token;

	LIST_FOR_EACH(token_list, i)
	{
		token = list_get(token_list, i);
		fprintf(f, "token[%d] = '%s'\n", i, token);
	}
}




/*
 * String functions
 */

enum str_error_enum
{
	str_err_ok = 0,
	str_err_base,
	str_err_format,
	str_err_range
};

static struct str_map_t str_error_map =
{
	4,
	{
		{ "OK", str_err_ok },
		{ "Invalid base", str_err_base },
		{ "Invalid format", str_err_format },
		{ "Integer exceeds valid range", str_err_range }
	}
};


/* Return the error message associated with an error code. */
char *str_error(int err)
{
	
	return str_map_value(&str_error_map, err);
}


/* Remove all duplicated spaces, tabs or new lines from the source string, and
 * create a new string with just single spaces, and no spaces on its sides.
 * As long as 'size' is greater than 0, the created string is guaranteed to be
 * null-terminated. */
void str_single_spaces(char *dest, int size, char *src)
{
	int prev_space;
	int curr_space;

	/* Nothing to do if invalid 'size' is given */
	if (size < 1)
		return;

	/* Remove initial spaces */
	while (isspace(*src))
		src++;
	
	/* Remove duplicated and final spaces */
	prev_space = 0;
	while (*src && size > 1)
	{
		curr_space = isspace(*src);
		if (!curr_space)
		{
			if (prev_space && size > 1)
			{
				*dest = ' ';
				size--;
				dest++;
			}
			if (size > 1)
			{
				*dest = *src;
				size--;
				dest++;
			}
		}

		/* Next character */
		prev_space = curr_space;
		src++;
	}

	/* Null-terminate */
	assert(size > 0);
	*dest = '\0';
}

/* Return 1 if 'suffix' is a suffix of 'str' */
int str_suffix(char *str, char *suffix)
{
	int str_len;
	int suffix_len;

	str_len = strlen(str);
	suffix_len = strlen(suffix);
	if (str_len < suffix_len)
		return 0;
	if (strcmp(str + str_len - suffix_len, suffix))
		return 0;
	return 1;
}


/* Return 1 if 'prefix' is a prefix of 'str' */
int str_prefix(char *str, char *prefix)
{
	int str_len;
	int prefix_len;

	str_len = strlen(str);
	prefix_len = strlen(prefix);
	if (str_len < prefix_len)
		return 0;
	if (!strncmp(str, prefix, prefix_len))
		return 1;
	return 0;
}


/* Copy a substring of 'src' into 'dest' */
void str_substr(char *dest, int dest_size, char *src, int src_pos, int src_count)
{
	int src_len;
	int count;

	src_len = strlen(src);
	count = src_count;
	if (count > dest_size - 1)
		count = dest_size - 1;
	if (count > src_len - src_pos)
		count = src_len - src_pos;
	memcpy(dest, src + src_pos, count);
	dest[count] = '\0';
}


void str_token(char *dest, int dest_size, char *src, int index, char *delim)
{
	char buf[MAX_STRING_SIZE];
	char *token;
	int i;

	snprintf(buf, sizeof buf, "%s", src);
	token = strtok(buf, delim);
	for (i = 0; i < index; i++)
		token = strtok(NULL, delim);
	snprintf(dest, dest_size, "%s", token ? token : "");
}


void str_trim(char *dest, int size, char *src)
{
	int len;
	int new_len;
	int right_trim;
	int left_trim;
	int i;

	/* Check valid 'size' */
	if (size < 1)
		panic("%s: invalid value for 'size'", __FUNCTION__);

	/* Store original string length */
	len = strlen(src);

	/* Count characters to trim on the right */
	right_trim = 0;
	for (i = len - 1; i >= 0 && isspace(src[i]); i--)
		right_trim++;
	
	/* Entire string is empty */
	assert(right_trim <= len);
	if (right_trim == len)
	{
		*dest = '\0';
		return;
	}

	/* Count characters to trim on the left */
	left_trim = 0;
	for (i = 0; i < len && isspace(src[i]); i++)
		left_trim++;
	
	/* New string length */
	new_len = len - left_trim - right_trim;
	assert(new_len > 0);

	/* Adjust to size of destination buffer */
	new_len = MIN(new_len, size - 1);
	assert(new_len >= 0);

	/* Create new string */
	dest[new_len] = '\0';
	for (i = 0; i < new_len; i++)
		dest[i] = src[i + left_trim];
}


int str_digit_to_int(char digit, int base, int *err_ptr)
{
	int result;

	/* Assume no error */
	PTR_ASSIGN(err_ptr, str_err_ok);

	/* Check base */
	if (base != 2 && base != 8 && base != 10 && base != 16)
	{
		PTR_ASSIGN(err_ptr, str_err_base);
		return 0;
	}

	/* Parse digit */
	result = 0;
	digit = tolower(digit);
	if (digit >= '0' && digit <= '9')
	{
		result = digit - '0';
	}
	else if (digit >= 'a' && digit <= 'f')
	{
		result = digit - 'a' + 10;
	}
	else
	{
		PTR_ASSIGN(err_ptr, str_err_format);
		return 0;
	}

	/* Check digit range */
	if (result >= base)
	{
		PTR_ASSIGN(err_ptr, str_err_format);
		return 0;
	}

	/* Return */
	return result;
}


int str_to_int(char *str, int *err_ptr)
{
	int sign;
	int base;
	int result;
	int digit;
	int num_digits;
	int err;
	int factor;
	int len;

	char trimmed[MAX_STRING_SIZE];

	/* Remove spaces */
	str_trim(trimmed, sizeof trimmed, str);
	str = trimmed;

	/* Assume no error initially */
	PTR_ASSIGN(err_ptr, str_err_ok);

	/* Base */
	base = 10;
	if (str_prefix(str, "0x"))
	{
		base = 16;
		str += 2;
	}
	else if (str[0] == '0' && str[1])
	{
		base = 8;
		str++;
	}

	/* Sign (only for base 10) */
	sign = 1;
	if (base == 10)
	{
		if (*str == '+')
		{
			sign = 1;
			str++;
		}
		else if (*str == '-')
		{
			sign = -1;
			str++;
		}
	}

	/* Empty string */
	if (!*str)
	{
		PTR_ASSIGN(err_ptr, str_err_format);
		return 0;
	}

	/* Suffixes (only for base 10) */
	factor = 0;
	len = strlen(str);
	assert(len > 0);
	if (base == 10)
	{
		switch (str[len - 1])
		{
		case 'k':
			factor = 1024;
			str[len - 1] = '\0';
			break;

		case 'K':
			factor = 1000;
			str[len - 1] = '\0';
			break;

		case 'm':
			factor = 1024 * 1024;
			str[len - 1] = '\0';
			break;

		case 'M':
			factor = 1000 * 1000;
			str[len - 1] = '\0';
			break;

		case 'g':
			factor = 1024 * 1024 * 1024;
			str[len - 1] = '\0';
			break;

		case 'G':
			factor = 1000 * 1000 * 1000;
			str[len - 1] = '\0';
			break;
		}
	}

	/* Remove leading 0s */
	while (*str == '0')
		str++;
	if (!*str)
		return 0;

	/* Start converting */
	result = 0;
	num_digits = 0;
	while (*str)
	{
		/* Get one digit */
		digit = str_digit_to_int(*str, base, &err);
		num_digits++;
		if (err)
		{
			PTR_ASSIGN(err_ptr, err);
			return 0;
		}

		/* Prevent overflow in base 10 */
		if (base == 10)
		{
			if (sign < 0 && INT_MIN / base > result)
			{
				PTR_ASSIGN(err_ptr, str_err_range);
				return 0;
			}
			if (sign > 0 && INT_MAX / base < result)
			{
				PTR_ASSIGN(err_ptr, str_err_range);
				return 0;
			}
		}

		/* Multiply by base */
		result *= base;

		/* Prevent overflow in base 10 */
		if (base == 10)
		{
			if (sign < 0 && INT_MIN + digit > result)
			{
				PTR_ASSIGN(err_ptr, str_err_range);
				return 0;
			}
			if (sign > 0 && INT_MAX - digit < result)
			{
				PTR_ASSIGN(err_ptr, str_err_range);
				return 0;
			}
		}

		/* Add digit */
		result += digit * sign;

		/* Prevent overflow in hexadecimal (unsigned) */
		if (base == 16 && num_digits > 8)
		{
			PTR_ASSIGN(err_ptr, str_err_range);
			return 0;
		}

		/* Next character */
		str++;
	}

	/* Multiplying factor */
	if (base == 10 && factor)
	{
		/* Prevent overflow */
		if (sign < 0 && INT_MIN / factor > result)
		{
			PTR_ASSIGN(err_ptr, str_err_range);
			return 0;
		}
		if (sign > 0 && INT_MAX / factor < result)
		{
			PTR_ASSIGN(err_ptr, str_err_range);
			return 0;
		}

		/* Multiply by factor */
		result *= factor;
	}

	/* Return */
	return result;
}


long long str_to_llint(char *str, int *err_ptr)
{
	int sign;
	int base;
	int digit;
	int num_digits;
	int err;
	int factor;
	int len;

	long long result;

	char trimmed[MAX_STRING_SIZE];

	/* Remove spaces */
	str_trim(trimmed, sizeof trimmed, str);
	str = trimmed;

	/* Assume no error initially */
	PTR_ASSIGN(err_ptr, str_err_ok);

	/* Base */
	base = 10;
	if (str_prefix(str, "0x"))
	{
		base = 16;
		str += 2;
	}
	else if (str[0] == '0' && str[1])
	{
		base = 8;
		str++;
	}

	/* Sign (only for base 10) */
	sign = 1;
	if (base == 10)
	{
		if (*str == '+')
		{
			sign = 1;
			str++;
		}
		else if (*str == '-')
		{
			sign = -1;
			str++;
		}
	}

	/* Empty string */
	if (!*str)
	{
		PTR_ASSIGN(err_ptr, str_err_format);
		return 0;
	}

	/* Suffixes (only for base 10) */
	factor = 0;
	len = strlen(str);
	assert(len > 0);
	if (base == 10)
	{
		switch (str[len - 1])
		{
		case 'k':
			factor = 1024;
			str[len - 1] = '\0';
			break;

		case 'K':
			factor = 1000;
			str[len - 1] = '\0';
			break;

		case 'm':
			factor = 1024 * 1024;
			str[len - 1] = '\0';
			break;

		case 'M':
			factor = 1000 * 1000;
			str[len - 1] = '\0';
			break;

		case 'g':
			factor = 1024 * 1024 * 1024;
			str[len - 1] = '\0';
			break;

		case 'G':
			factor = 1000 * 1000 * 1000;
			str[len - 1] = '\0';
			break;
		}
	}

	/* Remove leading 0s */
	while (*str == '0')
		str++;
	if (!*str)
		return 0;

	/* Start converting */
	result = 0;
	num_digits = 0;
	while (*str)
	{
		/* Get one digit */
		digit = str_digit_to_int(*str, base, &err);
		num_digits++;
		if (err)
		{
			PTR_ASSIGN(err_ptr, err);
			return 0;
		}

		/* Prevent overflow in base 10 */
		if (base == 10)
		{
			if (sign < 0 && LLONG_MIN / base > result)
			{
				PTR_ASSIGN(err_ptr, str_err_range);
				return 0;
			}
			if (sign > 0 && LLONG_MAX / base < result)
			{
				PTR_ASSIGN(err_ptr, str_err_range);
				return 0;
			}
		}

		/* Multiply by base */
		result *= base;

		/* Prevent overflow in base 10 */
		if (base == 10)
		{
			if (sign < 0 && LLONG_MIN + digit > result)
			{
				PTR_ASSIGN(err_ptr, str_err_range);
				return 0;
			}
			if (sign > 0 && LLONG_MAX - digit < result)
			{
				PTR_ASSIGN(err_ptr, str_err_range);
				return 0;
			}
		}

		/* Add digit */
		result += digit * sign;

		/* Prevent overflow in hexadecimal (unsigned) */
		if (base == 16 && num_digits > 16)
		{
			PTR_ASSIGN(err_ptr, str_err_range);
			return 0;
		}

		/* Next character */
		str++;
	}

	/* Multiplying factor */
	if (base == 10 && factor)
	{
		/* Prevent overflow */
		if (sign < 0 && LLONG_MIN / factor > result)
		{
			PTR_ASSIGN(err_ptr, str_err_range);
			return 0;
		}
		if (sign > 0 && LLONG_MAX / factor < result)
		{
			PTR_ASSIGN(err_ptr, str_err_range);
			return 0;
		}

		/* Multiply by factor */
		result *= factor;
	}

	/* Return */
	return result;
}


void str_int_to_alnum(char *str, int size, unsigned int value)
{
	unsigned int digit;
	unsigned int c;

	int len = 0;
	int i;

	/* Nothing if no room in output string */
	if (!size)
		return;
	
	/* Parse value */
	while (value && len < size - 1)
	{
		digit = value % 62;
		if (IN_RANGE(digit, 0, 9))
			c = '0' + digit;
		else if (IN_RANGE(digit, 10, 35))
			c = digit - 10 + 'a';
		else
			c = digit - 36 + 'A';
		str[len++] = c;
		value /= 62;
	}

	/* Null-terminate */
	str[len] = '\0';

	/* Mirror string */
	for (i = 0; i < len / 2; i++)
	{
		c = str[i];
		str[i] = str[len - i - 1];
		str[len - i - 1] = c;
	}
}


unsigned int str_alnum_to_int(char *str)
{
	unsigned int result = 0;
	unsigned int power;
	unsigned int digit;

	int i;
	int c;

	/* Empty string */
	if (!str || !*str)
		return 0;

	/* Parse string */
	power = 1;
	for (i = strlen(str) - 1; i >= 0; i--)
	{
		c = (unsigned char) str[i];
		if (IN_RANGE(c, '0', '9'))
			digit = c - '0';
		else if (IN_RANGE(c, 'a', 'z'))
			digit = c - 'a' + 10;
		else if (IN_RANGE(c, 'A', 'Z'))
			digit = c - 'A' + 36;
		else
			return 0;
		result += digit * power;
		power *= 62;
	}
	return result;
}


void str_printf(char **pbuf, int *psize, char *fmt, ...)
{
	va_list va;
	int len;

	if (*psize <= 0)
		return;
	if (*psize == 1)
	{
		**pbuf = '\0';
		return;
	}
	va_start(va, fmt);
	len = vsnprintf(*pbuf, *psize, fmt, va);
	if (len >= *psize)
		len = *psize - 1;
	*psize -= len;
	*pbuf += len;
}


void str_read_from_file(FILE *f, char *buf, int buf_size)
{
	int size;
	int count;

	/* Read size */
	count = fread(&size, 1, 4, f);
	if (count != 4)
	{
		fprintf(stderr, "%s: invalid file format", __FUNCTION__);
		abort();
	}

	/* Check buffer size */
	if (size > buf_size)
	{
		fprintf(stderr, "%s: buffer too small", __FUNCTION__);
		abort();
	}

	/* Read string */
	count = fread(buf, 1, size, f);
	if (count != size)
	{
		fprintf(stderr, "%s: invalid file format", __FUNCTION__);
		abort();
	}
}


void str_write_to_file(FILE *f, char *buf)
{
	int size;
	int count;

	/* Support NULL strings */
	if (!buf)
		buf = "";

	/* Write */
	size = strlen(buf) + 1;
	count = fwrite(&size, 1, 4, f);
	count += fwrite(buf, 1, size, f);
	if (count != size + 4)
	{
		fprintf(stderr, "%s: error writing to file", __FUNCTION__);
		exit(1);
	}
}


char *str_set(char *old_str, char *new_str)
{
	/* Free old value, only if not NULL */
	str_free(old_str);

	/* If new value is NULL, create empty string */
	if (!new_str)
		new_str = "";

	/* Duplicate */
	new_str = xstrdup(new_str);
	return new_str;
}


char *str_free(char *str)
{
	/* Free only if not NULL */
	if (str)
		free(str);

	/* Return new value for string, always NULL */
	return NULL;
}

