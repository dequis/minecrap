#include "minecrap.h"

MCBuffer *mc_buf_new() {
	MCBuffer *buf = g_new0(MCBuffer, 1);
	buf->queue = g_byte_array_sized_new(MC_BUFFER_SIZE);
	return buf;
}

void mc_buf_free(MCBuffer *buf) {
	g_byte_array_free(buf->queue, TRUE);
	g_free(buf);
}

gboolean mc_buf_read(MCBuffer *buf, void *out, size_t len, gboolean peek) {
	if ((buf->cursor + len) > buf->queue->len) {
		return FALSE;
	}
	if (out) {
		memcpy(out, buf->queue->data + buf->cursor, len);
	}
	if (!peek) {
		buf->cursor += len;
	}
	return TRUE;
}

void mc_buf_truncate(MCBuffer *buf) {
	g_byte_array_set_size(buf->queue, 0);
	buf->cursor = 0;
}

void mc_buf_cleanup(MCBuffer *buf) {
	if (buf->cursor == buf->queue->len) {
		mc_buf_truncate(buf);
	}
}

void mc_buf_write(MCBuffer *buf, guint8 *in, size_t len) {
	g_byte_array_append(buf->queue, in, len);
}

void mc_buf_flush_writes(MCBuffer *buf, GOutputStream *ostream) {
	ssize_t writelen;

	writelen = g_output_stream_write(ostream, buf->queue->data, buf->queue->len, NULL, NULL);

	buf->cursor += writelen;
	mc_buf_cleanup(buf);
}

void mc_buf_feed_input(MCBuffer *buf, GInputStream *istream) {
	char buffer[1024];
	ssize_t readlen;

	mc_buf_cleanup(buf);

	readlen = g_input_stream_read(istream, buffer, sizeof(buffer), NULL, NULL);

	mc_buf_write(buf, buffer, readlen);
}

void mc_buf_debug(MCBuffer *buf, char *prefix) {
	debug_dump(buf->queue->data, buf->queue->len, prefix);
}
