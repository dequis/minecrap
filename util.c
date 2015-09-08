#include "minecrap.h"

void debug_dump(guint8 *buf, size_t count, char *prefix) {
	int i;
	fprintf(stderr, "%4zd %s ", count, prefix);
	for (i = 0; i < count; i++) {
		fprintf(stderr, "%02x ", buf[i]);
	}
	fprintf(stderr, "\n");
}

