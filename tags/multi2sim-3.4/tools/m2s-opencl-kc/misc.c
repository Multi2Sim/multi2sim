#include <misc.h>
#include <debug.h>


static int memoryleft;

void strccat(char *dest, char *src)
{
	int destlen = strlen(dest);
	int srclen = strlen(src);
	if (memoryleft <= 1)
		return;
	srclen = MIN(srclen, memoryleft - 1);
	memcpy(dest + destlen, src, srclen);
	dest[destlen + srclen] = 0;
	memoryleft -= srclen;
}


void strccpy(char *dest, char *src, int size)
{
	int srclen = strlen(src);
	memoryleft = size;
	if (memoryleft <= 1)
		return;
	srclen = MIN(srclen, memoryleft - 1);
	memcpy(dest, src, srclen);
	dest[srclen] = 0;
	memoryleft -= srclen;
}

static char *unknown = "<unknown>";
char *map_value(struct string_map_t *map, int value)
{
	int i;
	for (i = 0; i < map->count; i++)
		if (map->map[i].value == value)
			return map->map[i].string;
	return unknown;
}


void map_value_string(struct string_map_t *map, int value, char *buf, int size)
{
	int i;
	for (i = 0; i < map->count; i++) {
		if (map->map[i].value == value) {
			snprintf(buf, size, "%s", map->map[i].string);
			return;
		}
	}
	snprintf(buf, size, "%d", value);
}


void map_flags(struct string_map_t *map, int flags, char *out, int size)
{
	int i;
	char *comma = "", temp[size];

	strccpy(out, "{", size);
	for (i = 0; i < 32; i++) {
		if (flags & (1U << i)) {
			strccat(out, comma);
			map_value_string(map, 1U << i, temp, size);
			strccat(out, temp);
			comma = "|";
		}
	}
	strccat(out, "}");
}



int write_buffer(char *file_name, void *buf, size_t size)
{
	FILE *f;
	if (!(f = fopen(file_name, "wb")))
		return 0;
	fwrite(buf, size, 1, f);
	fclose(f);
	return 1;
}


void *read_buffer(char *file_name, size_t *psize)
{
	FILE *f;
	void *buf;
	int size;

	f = fopen(file_name, "rb");
	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = malloc(size);
	fread(buf, 1, size, f);
	if (psize)
		*psize = size;
	return buf;
}

