#include <ev.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// every watcher type has its own typedef'd struct with the name ev_TYPE
struct ev_loop *loop;

ev_timer timeout_watcher;
ev_async async_watcher;

// another callback, this time for a time-out
static void timeout_cb (ev_timer *w, int revents)
{
	puts("timeout");
	//puts("timeout02");
	// this causes the innermost ev_run to stop iterating
	//ev_break(loop, EVBREAK_ONE);
}

static void async_cb (ev_async *w) {
	puts("async");
	//ev_break(loop, EVBREAK_ONE);
}

static void invoke_cb(struct ev_loop *loop) {
    ev_invoke_pending(loop);

    puts("inovke---------------------------");
    ev_break(loop, EVBREAK_ALL);
}

void *routine(void *arg) {
    sleep(5);
    puts("routine");
    sleep(1);
    for (int i = 0; i < 10; ++i) {
        sleep(1);
        ev_async_send(loop, &async_watcher);
    }
}

int main (void)
{
	// use the default event loop unless you have special needs
	// struct ev_loop *loop = EV_DEFAULT; /* OR ev_default_loop(0) */
	//EV_P EV_DEFAULT;
    loop = EV_DEFAULT;

	// initialise a timer watcher, then start it
	// simple non-repeating 5.5 second timeout
	ev_timer_init(&timeout_watcher, timeout_cb, 2.5, 0.);
	ev_timer_start(loop, &timeout_watcher);

    ev_async_init(&async_watcher, async_cb);
    ev_async_start(loop, &async_watcher);

    ev_set_invoke_pending_cb(loop, invoke_cb);

    pthread_t tid;
    pthread_create(&tid, NULL,routine, NULL);

	ev_run(loop, 0); /* now wait for events to arrive */

	ev_loop_destroy(loop);

	return 0;
}
