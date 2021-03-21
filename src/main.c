#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <signal.h>

#include <cerver/version.h>
#include <cerver/cerver.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

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
	// GET /api/ermiry
	HttpRoute *ermiry_route = http_route_create (REQUEST_METHOD_GET, "api/ermiry", ermiry_handler);
	http_cerver_route_register (http_cerver, ermiry_route);

	/* register ermiry children routes */
	// GET api/ermiry/version
	HttpRoute *ermiry_version_route = http_route_create (REQUEST_METHOD_GET, "version", ermiry_version_handler);
	http_route_child_add (ermiry_route, ermiry_version_route);

	// GET api/ermiry/auth
	HttpRoute *ermiry_auth_route = http_route_create (REQUEST_METHOD_GET, "auth", ermiry_auth_handler);
	http_route_set_auth (ermiry_auth_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (ermiry_auth_route, ermiry_user_parse_from_json, ermiry_user_delete);
	http_route_child_add (ermiry_route, ermiry_auth_route);

	/*** transactions ***/

	// GET api/ermiry/transactions
	HttpRoute *transactions_route = http_route_create (REQUEST_METHOD_GET, "transactions", ermiry_transactions_handler);
	http_route_set_auth (transactions_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (transactions_route, ermiry_user_parse_from_json, ermiry_user_delete);
	http_route_child_add (ermiry_route, transactions_route);

	// POST api/ermiry/transactions
	http_route_set_handler (transactions_route, REQUEST_METHOD_POST, ermiry_transaction_create_handler);

	// GET api/ermiry/transactions/:id
	HttpRoute *single_trans_route = http_route_create (REQUEST_METHOD_GET, "transactions/:id", ermiry_transaction_get_handler);
	http_route_set_auth (single_trans_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (single_trans_route, ermiry_user_parse_from_json, ermiry_user_delete);
	http_route_child_add (ermiry_route, single_trans_route);

	// PUT api/ermiry/transactions/:id
	http_route_set_handler (single_trans_route, REQUEST_METHOD_PUT, ermiry_transaction_update_handler);

	// DELETE api/ermiry/transactions/:id
	http_route_set_handler (single_trans_route, REQUEST_METHOD_DELETE, ermiry_transaction_delete_handler);

	/*** categories ***/

	// GET api/ermiry/categories
	HttpRoute *categories_route = http_route_create (REQUEST_METHOD_GET, "categories", ermiry_categories_handler);
	http_route_set_auth (categories_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (categories_route, ermiry_user_parse_from_json, ermiry_user_delete);
	http_route_child_add (ermiry_route, categories_route);

	// POST api/ermiry/categories
	http_route_set_handler (categories_route, REQUEST_METHOD_POST, ermiry_category_create_handler);

	// GET api/ermiry/categories/:id
	HttpRoute *single_category_route = http_route_create (REQUEST_METHOD_GET, "categories/:id", ermiry_category_get_handler);
	http_route_set_auth (single_category_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (single_category_route, ermiry_user_parse_from_json, ermiry_user_delete);
	http_route_child_add (ermiry_route, single_category_route);

	// PUT api/ermiry/categories/:id
	http_route_set_handler (single_category_route, REQUEST_METHOD_PUT, ermiry_category_update_handler);

	// DELETE api/ermiry/categories/:id
	http_route_set_handler (single_category_route, REQUEST_METHOD_DELETE, ermiry_category_delete_handler);

	/*** places ***/

	// GET api/ermiry/places
	HttpRoute *places_route = http_route_create (REQUEST_METHOD_GET, "places", ermiry_places_handler);
	http_route_set_auth (places_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (places_route, ermiry_user_parse_from_json, ermiry_user_delete);
	http_route_child_add (ermiry_route, places_route);

	// POST api/ermiry/places
	http_route_set_handler (places_route, REQUEST_METHOD_POST, ermiry_place_create_handler);

	// GET api/ermiry/places/:id
	HttpRoute *single_place_route = http_route_create (REQUEST_METHOD_GET, "places/:id", ermiry_place_get_handler);
	http_route_set_auth (single_place_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (single_place_route, ermiry_user_parse_from_json, ermiry_user_delete);
	http_route_child_add (ermiry_route, single_place_route);

	// PUT api/ermiry/places/:id
	http_route_set_handler (single_place_route, REQUEST_METHOD_PUT, ermiry_place_update_handler);

	// DELETE api/ermiry/places/:id
	http_route_set_handler (single_place_route, REQUEST_METHOD_DELETE, ermiry_place_delete_handler);

}

static void ermiry_set_users_routes (HttpCerver *http_cerver) {

	/* register top level route */
	// GET /api/users
	HttpRoute *users_route = http_route_create (REQUEST_METHOD_GET, "api/users", users_handler);
	http_cerver_route_register (http_cerver, users_route);

	/* register users children routes */
	// POST api/users/login
	HttpRoute *users_login_route = http_route_create (REQUEST_METHOD_POST, "login", users_login_handler);
	http_route_child_add (users_route, users_login_route);

	// POST api/users/register
	HttpRoute *users_register_route = http_route_create (REQUEST_METHOD_POST, "register", users_register_handler);
	http_route_child_add (users_route, users_register_route);

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

		http_cerver_auth_set_jwt_algorithm (http_cerver, JWT_ALG_RS256);
		if (ENABLE_USERS_ROUTES) {
			http_cerver_auth_set_jwt_priv_key_filename (http_cerver, PRIV_KEY->str);
		}
		
		http_cerver_auth_set_jwt_pub_key_filename (http_cerver, PUB_KEY->str);

		ermiry_set_ermiry_routes (http_cerver);

		if (ENABLE_USERS_ROUTES) {
			ermiry_set_users_routes (http_cerver);
		}

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