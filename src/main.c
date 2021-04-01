#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <signal.h>

#include <cerver/version.h>
#include <cerver/cerver.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>

#include <cerver/utils/log.h>
#include <cerver/utils/utils.h>

#include "ermiry.h"
#include "version.h"

#include "routes/service.h"

static Cerver *ermiry_api = NULL;

void end (int dummy) {
	
	if (ermiry_api) {
		cerver_stats_print (ermiry_api, false, false);
		cerver_log_msg ("\nHTTP Cerver stats:\n");
		http_cerver_all_stats_print ((HttpCerver *) ermiry_api->cerver_data);
		cerver_log_line_break ();
		cerver_teardown (ermiry_api);
	}

	(void) ermiry_end ();

	cerver_end ();

	exit (0);

}

static void ermiry_set_ermiry_routes (HttpCerver *http_cerver) {

	/* register top level route */
	// GET /api/status
	HttpRoute *ermiry_route = http_route_create (REQUEST_METHOD_GET, "api/status", ermiry_handler);
	http_cerver_route_register (http_cerver, ermiry_route);

	/* register ermiry children routes */
	// GET api/status/version
	HttpRoute *ermiry_version_route = http_route_create (REQUEST_METHOD_GET, "version", ermiry_version_handler);
	http_route_child_add (ermiry_route, ermiry_version_route);

}

static void start (void) {

	ermiry_api = cerver_create (
		CERVER_TYPE_WEB,
		"ermiry-api",
		PORT,
		PROTOCOL_TCP,
		false,
		CERVER_CONNECTION_QUEUE
	);

	if (ermiry_api) {
		/*** cerver configuration ***/
		cerver_set_receive_buffer_size (ermiry_api, CERVER_RECEIVE_BUFFER_SIZE);
		cerver_set_thpool_n_threads (ermiry_api, CERVER_TH_THREADS);
		cerver_set_handler_type (ermiry_api, CERVER_HANDLER_TYPE_THREADS);

		/*** web cerver configuration ***/
		HttpCerver *http_cerver = (HttpCerver *) ermiry_api->cerver_data;

		ermiry_set_ermiry_routes (http_cerver);

		// add a catch all route
		http_cerver_set_catch_all_route (http_cerver, ermiry_catch_all_handler);

		if (cerver_start (ermiry_api)) {
			cerver_log_error (
				"Failed to start %s!",
				ermiry_api->info->name->str
			);

			cerver_delete (ermiry_api);
		}
	}

	else {
		cerver_log_error ("Failed to create cerver!");

		cerver_delete (ermiry_api);
	}

}

int main (int argc, char const **argv) {

	srand (time (NULL));

	// register to the quit signal
	(void) signal (SIGINT, end);
	(void) signal (SIGTERM, end);

	// to prevent SIGPIPE when writting to socket
	(void) signal (SIGPIPE, SIG_IGN);

	cerver_init ();

	cerver_version_print_full ();

	ermiry_version_print_full ();

	if (!ermiry_init ()) {
		start ();
	}

	else {
		cerver_log_error ("Failed to init ermiry!");
	}

	(void) ermiry_end ();

	cerver_end ();

	return 0;

}