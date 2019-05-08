#include "memory_stream.h"

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

static int8_t s_native_endian = 0;

int
get_native_endian()
{
	if (0 == s_native_endian)
	{
		int x = 1;
		if (*(char*)&x == 1)
			s_native_endian = ENDIAN_LITTLE;
		else
			s_native_endian = ENDIAN_BIG;
	}
	return s_native_endian;
}

#if 1
#define HTONS(A) (A)
#define NTOHS(a) (a) 
#define HTONL(A) (A)
#define NTOHL(a) (a)
#define HTONLL(A) (A)
#define NTOHLL(a) (a)
#else
#define HTONS(A)  ((((A) & 0xff00) >> 8) | ((A) & 0x00ff) << 8)   
#define NTOHS(a)  HTONS(a)
#define HTONL(A)  ((((A) & 0xff000000) >> 24) | (((A) & 0x00ff0000) >> 8) | (((A) & 0x0000ff00) << 8) | (((A) & 0x000000ff) << 24))
#define NTOHL(a)  HTONL(a)
#endif

memory_stream_t *
create_memory_stream(size_t capacity)
{
	memory_stream_t *stream = (memory_stream_t *)malloc(sizeof(memory_stream_t));
	init_memory_stream(stream, capacity);
	return stream;
}

void
destroy_memory_stream(memory_stream_t *stream)
{
	free(stream->buf);
	free(stream);
}

void
init_memory_stream(memory_stream_t *stream, size_t capacity)
{
	memset(stream, 0, sizeof(memory_stream_t));
	stream->capacity = capacity;
	stream->buf = (char *)malloc(capacity);
	stream->cursor_w = stream->buf;
	stream->cursor_r = stream->buf;
}

void
memory_stream_reset(memory_stream_t *stream)
{
	stream->cursor_w = stream->cursor_r = stream->buf;
}

void
memory_stream_rewind(memory_stream_t *stream)
{
	if (stream->cursor_w > stream->cursor_r) {
		size_t size = stream->cursor_w - stream->cursor_r;
		memcpy(stream->buf, stream->cursor_r, size);
		stream->cursor_r = stream->buf;
		stream->cursor_w = stream->buf + size;
	}
	else {
		memory_stream_reset(stream);
	}
}

void
memory_stream_skip(memory_stream_t *stream, int len)
{
	if (stream->cursor_r + len < stream->cursor_w) {
		stream->cursor_r += len;
	}
	else {
		stream->cursor_r = stream->cursor_w;
	}
}

void
memory_stream_skip_all(memory_stream_t *stream)
{
	memory_stream_reset(stream);
}

int
memory_stream_get_used_size(memory_stream_t *stream)
{
	return stream->cursor_w - stream->buf;
}

int
memory_stream_get_free_size(memory_stream_t *stream) {
	return stream->capacity - memory_stream_get_used_size(stream);
}

bool
memory_stream_ensure_free_size(memory_stream_t *stream, int size) {
	int sz = memory_stream_get_free_size(stream);
	return (sz > 0 && sz > size);
}

int8_t
memory_stream_read_byte(memory_stream_t *stream)
{
	unsigned char d;
	memcpy(&d, stream->cursor_r, 1);
	stream->cursor_r += 1;
	return d;
}

int16_t
memory_stream_read_int16(memory_stream_t *stream)
{
	unsigned short d;
	memcpy(&d, stream->cursor_r, 2);
	d = NTOHS(d);
	stream->cursor_r += 2;
	return d;
}

int32_t
memory_stream_read_int32(memory_stream_t *stream)
{
	int32_t d;
	memcpy(&d, stream->cursor_r, 4);
	d = NTOHL(d);
	stream->cursor_r += 4;
	return d;
}

int64_t
memory_stream_read_int64(memory_stream_t *stream)
{
	int64_t d;
	memcpy(&d, stream->cursor_r, 8);
	d = NTOHLL(d);
	stream->cursor_r += 8;
	return d;
}

const char *
memory_stream_read_string(memory_stream_t *stream, uint16_t *outlen)
{
	static char __strbuf[32768];
	uint16_t len = memory_stream_read_int16(stream);
	size_t szmax = memory_stream_get_used_size(stream);
	len = (len < (uint16_t)sizeof(__strbuf)) ? len : (uint16_t)sizeof(__strbuf);
	len = (len < (uint16_t)szmax) ? len : (uint16_t)szmax;
	memcpy(__strbuf, stream->cursor_r, len);
	stream->cursor_r += len;
	*outlen = len;
	return __strbuf;
}

void
memory_stream_read_raw(memory_stream_t *stream, unsigned char *dest, int len)
{
	memcpy(dest, stream->cursor_r, len);
	stream->cursor_r += len;
}

void
memory_stream_write_byte(memory_stream_t *stream, int8_t d)
{
	memcpy(stream->cursor_w, &d, 1);
	stream->cursor_w += 1;
}

void
memory_stream_write_int16(memory_stream_t *stream, int16_t d)
{
	d = HTONS(d);
	memcpy(stream->cursor_w, &d, 2);
	stream->cursor_w += 2;
}

void
memory_stream_write_int32(memory_stream_t *stream, int32_t d)
{
	d = HTONL(d);
	memcpy(stream->cursor_w, &d, 4);
	stream->cursor_w += 4;
}

void
memory_stream_write_int64(memory_stream_t *stream, int64_t d)
{
	d = HTONLL(d);
	memcpy(stream->cursor_w, &d, 8);
	stream->cursor_w += 8;
}

void
memory_stream_write_string(memory_stream_t *stream, const char *str, uint16_t len)
{
	memory_stream_write_int16(stream, (int16_t)len);
	memcpy(stream->cursor_w, str, len);
	stream->cursor_w += len;
}

void
memory_stream_write_raw(memory_stream_t *stream, unsigned char *src, int len)
{
	memcpy(stream->cursor_w, src, len);
	stream->cursor_w += len;
}