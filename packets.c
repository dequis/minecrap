#include "minecrap.h"

/* varargs are a list of pairs (type, value) */
void mc_write_packet(MCBuffer *buf, MCVarInt packet_id, ...) {
	va_list ap, aq;
	int i = 0;
	MCVarInt len = 0;
	MCFieldType type;

	va_start(ap, packet_id);
	G_VA_COPY(aq, ap);

	/* go through the list once to calculate packet size */
	len += mc_size_varint(&packet_id);
	while ((type = va_arg(aq, int))) {
		len += mc_size_t(va_arg(aq, void *), type);
	}

	va_end(aq);

	/* actually write it */
	mc_write(buf, &len);
	mc_write(buf, &packet_id);
	while ((type = va_arg(ap, int))) {
		mc_write_t(buf, va_arg(ap, void *), type);
	}
	va_end(ap);
}

void mc_write_handshake(MCBuffer *buf) {
	MCVarInt protocol_version = 47;
	GString *server_host = g_string_new("127.0.0.1");
	guint16 server_port = 25565;
	MCVarInt next_state = 1;

	mc_write_packet(buf, 0,
		FIELD(protocol_version),
		FIELD(server_host),
		FIELD(server_port),
		FIELD(next_state),
		NULL
	);

	g_string_free(server_host, TRUE);
}

void mc_write_status(MCBuffer *buf) {
	mc_write_packet(buf, 0, NULL);
}

void mc_read_status(MCBuffer *buf) {
	GString *status_json = g_string_new("");

	mc_read(buf, &status_json);

	fprintf(stderr, "Status JSON: %s\n", status_json->str);

	g_string_free(status_json, TRUE);
}

gboolean mc_read_packet(MCBuffer *buf) {
	MCVarInt packet_id, size;
	guint32 orig_cursor = buf->cursor;
	guint32 packet_end;
	size_t packet_id_size;

	if (!mc_read(buf, &size) ||
	    !mc_buf_read(buf, NULL, size, TRUE)) {
		/* not enough read to process this packet */
		goto reset;
	}

	packet_end = buf->cursor + size;

	mc_read(buf, &packet_id);

	if (packet_id == 0) {
		mc_read_status(buf);
	} else {
		buf->cursor = packet_end;
	}

	return TRUE;
reset:
	buf->cursor = orig_cursor;
	return FALSE;
}
