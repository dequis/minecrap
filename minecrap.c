#include "minecrap.h"

MCState *mc_state_new(char *host, int port) {
	GSocketClient *client;
	MCState *state = g_new0(MCState, 1);

	state->in = mc_buf_new();
	state->out = mc_buf_new();

	client = g_socket_client_new();
	state->conn = g_socket_client_connect_to_host(client, host, port, NULL, NULL);
	state->istream = g_io_stream_get_input_stream(G_IO_STREAM(state->conn));
	state->ostream = g_io_stream_get_output_stream(G_IO_STREAM(state->conn));

	g_object_unref(client);

	return state;
}

void mc_state_free(MCState *state) {
	mc_buf_free(state->in);
	mc_buf_free(state->out);
	g_object_unref(state->conn);
	g_free(state);
}

int main() {
	MCState *state;

	state = mc_state_new("127.0.0.1", 25565);

	mc_write_handshake(state->out);
	mc_write_status(state->out);

	mc_buf_debug(state->out, ">>>");
	mc_buf_flush_writes(state->out, state->ostream);

	mc_buf_feed_input(state->in, state->istream);
	mc_buf_debug(state->in, "<<<");

	mc_read_packet(state->in);

	mc_state_free(state);
}
