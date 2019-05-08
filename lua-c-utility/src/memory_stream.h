#ifndef __MEMORY_STREAM_H__
#define __MEMORY_STREAM_H__

#include <stddef.h> // for "size_t" on linux
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ENDIAN_LITTLE 1
#define ENDIAN_BIG    2

/* get native endian: 0 = litte, 1 = big */
int                get_native_endian();

typedef struct memory_stream_s
{
	size_t  capacity;
	char   *buf;
	char   *cursor_r;
	char   *cursor_w;
} memory_stream_t;

memory_stream_t *  create_memory_stream(size_t capacity);
void               destroy_memory_stream(memory_stream_t *stream);
void               init_memory_stream(memory_stream_t *stream, size_t capacity);

void               memory_stream_reset(memory_stream_t *stream);
void               memory_stream_rewind(memory_stream_t *stream);
void               memory_stream_skip(memory_stream_t *stream, int len);
void               memory_stream_skip_all(memory_stream_t *stream);
int                memory_stream_get_used_size(memory_stream_t *stream);
int                memory_stream_get_free_size(memory_stream_t *stream);
bool               memory_stream_ensure_free_size(memory_stream_t *stream, int size);

int8_t             memory_stream_read_byte(memory_stream_t *stream);
int16_t            memory_stream_read_int16(memory_stream_t *stream);
int32_t            memory_stream_read_int32(memory_stream_t *stream);
int64_t            memory_stream_read_int64(memory_stream_t *stream);
const char *       memory_stream_read_string(memory_stream_t *stream, uint16_t *outlen);
void               memory_stream_read_raw(memory_stream_t *stream, unsigned char *dest, int len);

void               memory_stream_write_byte(memory_stream_t *stream, int8_t d);
void               memory_stream_write_int16(memory_stream_t *stream, int16_t d);
void               memory_stream_write_int32(memory_stream_t *stream, int32_t d);
void               memory_stream_write_int64(memory_stream_t *stream, int64_t d);
void               memory_stream_write_string(memory_stream_t *stream, const char *str, uint16_t len);
void               memory_stream_write_raw(memory_stream_t *stream, unsigned char *src, int len);

#ifdef __cplusplus
}
#endif

#endif /* __MEMORY_STREAM_H__ */