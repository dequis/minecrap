
typedef guint32 MCVarInt;

typedef enum {
	MC_INT8 = 1,
	MC_UINT8,
	MC_INT16,
	MC_UINT16,
	MC_INT32,
	MC_VARINT,
	MC_STRING,
	MC_LAST_TYPE
} MCFieldType;

#define _GET_TYPE_NUMERIC(expr) _Generic((expr), \
	gint8: MC_INT8, \
	guint8: MC_UINT8, \
	gint16: MC_INT16, \
	guint16: MC_UINT16, \
	gint32: MC_INT32, \
	MCVarInt: MC_VARINT, \
	GString *: MC_STRING \
)

#define FIELD(x) _GET_TYPE_NUMERIC(x), &x

gboolean mc_read_bytes(MCBuffer *buf, void *out, size_t count);
gboolean mc_read_varint(MCBuffer *buf, MCVarInt *out);

void mc_write_bytes(MCBuffer *buf, void *in, size_t count);
void mc_write_int8(MCBuffer *buf, guint8 *byte);
void mc_write_varint(MCBuffer *buf, MCVarInt *in);

size_t mc_size_varint(MCVarInt *in);
size_t mc_size_string(void *in);

gboolean mc_read_t(MCBuffer *buf, void *out, MCFieldType type);
void mc_write_t(MCBuffer *buf, void *in, MCFieldType type);
size_t mc_size_t(void *in, MCFieldType type);

#define mc_read(buf, val) mc_read_t(buf, val, _GET_TYPE_NUMERIC(*val))
#define mc_write(buf, val) mc_write_t(buf, val, _GET_TYPE_NUMERIC(*val))
#define mc_size(val) mc_size_t(val, _GET_TYPE_NUMERIC(*val))
