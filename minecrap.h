#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>

#define MC_BUFFER_SIZE 1024

typedef struct {
	GByteArray *queue;
	guint32 cursor;
} MCBuffer;

typedef struct {
	int fd;
	MCBuffer *in;
	MCBuffer *out;

	GSocketConnection *conn;
	GInputStream *istream;
	GOutputStream *ostream;
} MCState;

#include "types.h"

/* buf.c */
MCBuffer *mc_buf_new();
void mc_buf_free(MCBuffer *buf);
gboolean mc_buf_read(MCBuffer *buf, void *out, size_t len, gboolean peek);
void mc_buf_truncate(MCBuffer *buf);
void mc_buf_write(MCBuffer *buf, guint8 *in, size_t len);
void mc_buf_flush_writes(MCBuffer *buf, GOutputStream *ostream);
void mc_buf_feed_input(MCBuffer *buf, GInputStream *istream);
void mc_buf_debug(MCBuffer *buf, char *prefix);

/* util.c */
void debug_dump(guint8 *buf, size_t count, char *prefix);

/* packets.c */
void mc_write_packet(MCBuffer *buf, MCVarInt packet_id, ...);
void mc_write_handshake(MCBuffer *buf);
void mc_write_status(MCBuffer *buf);
gboolean mc_read_packet(MCBuffer *buf);
