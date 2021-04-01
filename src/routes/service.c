#include <cerver/http/http.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>

#include "controllers/service.h"

// GET /api/ermiry
void ermiry_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	(void) http_response_send (ermiry_works, http_receive);

}

// GET /api/ermiry/version
void ermiry_version_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	(void) http_response_send (current_version, http_receive);

}

// GET *
void ermiry_catch_all_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	http_response_send (catch_all, http_receive);

}
