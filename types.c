#include "minecrap.h"

/* read functions {{{ */

gboolean mc_read_guint8 (MCBuffer *buf, guint8 *out);

gboolean mc_read_bytes(MCBuffer *buf, void *out, size_t count) {
	if (!mc_buf_read(buf, out, count, FALSE)) {
		return FALSE;
	}
	debug_dump(out, count, "<<<");
	return TRUE;
}

gboolean mc_read_varint(MCBuffer *buf, MCVarInt *out) {
	guint8 in;
	int bytes = 0;

	*out = 0;
	do {
		if (!mc_read_guint8(buf, &in)) {
			return FALSE;
		}
		*out |= (in & 0x7F) << (bytes++ * 7);
		if (bytes > 5) {
			return -1;
		}
	} while (in & 0x80);
	return TRUE;
}

gboolean mc_read_string(MCBuffer *buf, void *out) {
	GString *str = *(GString **) out;
	MCVarInt len;
	if (!mc_read_varint(buf, &len)) {
		return FALSE;
	}

	/* preallocate */
	g_string_set_size(str, len + 1);

	mc_read_bytes(buf, str->str, len);

	/* ensure it's properly null terminated */
	g_string_truncate(str, len);

	return TRUE;
}

/* }}} */

/* write functions {{{ */

void mc_write_bytes(MCBuffer *buf, void *in, size_t count) {
	mc_buf_write(buf, in, count);
	debug_dump(in, count, ">>>");
}

void mc_write_varint(MCBuffer *buf, MCVarInt *in) {
	guint32 value = *in;
	guint8 part;
	do {
		part = (guint8) (value & 0x7F);
		value >>= 7;
		if (value != 0) {
			part |= 0x80;
		}
		mc_write(buf, &part);
	} while (value);
}

void mc_write_string(MCBuffer *buf, void *in) {
	GString *str = *(GString **) in;
	MCVarInt len = str->len;
	mc_write_varint(buf, &len);
	mc_write_bytes(buf, str->str, len);
}

/* }}} */

/* generic read/write functions {{{ */

gboolean mc_read_guint8(MCBuffer *buf, guint8 *out) {
	if (mc_read_bytes(buf, out, sizeof(guint8))) {
		return TRUE;
	}
	return FALSE;
}

#define MC_READ_NUM_FUNC(T, cast) \
gboolean G_PASTE(mc_read_, T) (MCBuffer *buf, T *out) { \
	if (mc_read_bytes(buf, &out, sizeof(T))) { \
		*out = cast(*out); \
		return TRUE; \
	} \
	return FALSE; \
}

#define MC_WRITE_NUM_FUNC(T, cast) \
void G_PASTE(mc_write_, T) (MCBuffer *buf, T *in) { \
	T casted = cast(*in); \
	mc_write_bytes(buf, &casted, sizeof(T)); \
}

#define MC_GENERIC_NUM_FUNC(T, cast) MC_READ_NUM_FUNC(T, cast) \
MC_WRITE_NUM_FUNC(T, cast)

MC_GENERIC_NUM_FUNC(gint8, /* no cast */ );
MC_WRITE_NUM_FUNC(guint8, /* no cast */ );
MC_GENERIC_NUM_FUNC(gint16, GINT16_FROM_BE);
MC_GENERIC_NUM_FUNC(guint16, GUINT16_FROM_BE);
MC_GENERIC_NUM_FUNC(gint32, GINT32_FROM_BE);


/* }}} */

/* size functions {{{ */

size_t mc_size_varint(MCVarInt *in) {
	size_t bytes = 0;
	guint32 value = *in;
	do {
		value >>= 7;
		bytes += 1;
	} while (value);
	return bytes;
}

size_t mc_size_string(void *in) {
	GString *str = * (GString **) in;
	MCVarInt len = str->len;
	return mc_size_varint(&len) + str->len;
}

/* }}} */

/* runtime generic type handling {{{ */

typedef gboolean (*mc_rfunc) (MCBuffer *, void *);
typedef void (*mc_wfunc) (MCBuffer *, void *);

static const mc_rfunc mc_rfuncs[MC_LAST_TYPE] = {
	(mc_rfunc) mc_read_gint8,
	(mc_rfunc) mc_read_guint8,
	(mc_rfunc) mc_read_gint16,
	(mc_rfunc) mc_read_guint16,
	(mc_rfunc) mc_read_gint32,
	(mc_rfunc) mc_read_varint,
	(mc_rfunc) mc_read_string,
};

static const mc_wfunc mc_wfuncs[MC_LAST_TYPE] = {
	(mc_wfunc) mc_write_gint8,
	(mc_wfunc) mc_write_guint8,
	(mc_wfunc) mc_write_gint16,
	(mc_wfunc) mc_write_guint16,
	(mc_wfunc) mc_write_gint32,
	(mc_wfunc) mc_write_varint,
	(mc_wfunc) mc_write_string,
};

gboolean mc_read_t(MCBuffer *buf, void *out, MCFieldType type) {
	g_return_val_if_fail(type > 0 && type < MC_LAST_TYPE, FALSE);

	return mc_rfuncs[type - 1](buf, out);
}

void mc_write_t(MCBuffer *buf, void *in, MCFieldType type) {
	g_return_val_if_fail(type > 0 && type < MC_LAST_TYPE, NULL);

	mc_wfuncs[type - 1](buf, in);
}

size_t mc_size_t(void *in, MCFieldType type) {
	switch (type) {
		case MC_INT8:
		case MC_UINT8:
			return 1;
		case MC_INT16:
		case MC_UINT16:
			return 2;
		case MC_INT32:
			return 4;
		case MC_VARINT:
			return mc_size_varint((MCVarInt *) in);
		case MC_STRING:
			return mc_size_string(in);
	}
}

/* }}} */
