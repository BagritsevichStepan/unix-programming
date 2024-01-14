#include <stdlib.h>
#include <string.h>
#include <uv.h>

uv_tcp_t server;
uv_loop_t* loop;

void read_cb(uv_stream_t* stream, ssize_t n, const uv_buf_t* buf) {
    if (n <= 0) {
        free(stream);
        free(buf->base);
        return;
    }
    uv_write_t* req = (uv_write_t*) malloc(sizeof(uv_write_t));
    memset(req, 0, sizeof(uv_write_t));
    uv_write(req, stream, buf, 1, NULL);
    free(buf->base);
    free(req);
}

void alloc_buffer(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
    buf->base = malloc(size);
    buf->len = size;
    memset(buf->base, 0, size);
}

void connection_cb(uv_stream_t* stream, int status) {
    uv_tcp_t* client = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    uv_accept(stream, (uv_stream_t*) client);
    uv_read_start((uv_stream_t*) client, alloc_buffer, read_cb);
}

int main() {
    loop = uv_default_loop();
    struct sockaddr_in addr;
    uv_ip4_addr("127.0.0.1", 12345, &addr);
    uv_tcp_init(loop, &server);
    uv_tcp_bind(&server, (struct sockaddr*) &addr, 0);
    uv_listen((uv_stream_t*)&server, 128, connection_cb);

    return uv_run(loop, UV_RUN_DEFAULT);
}
