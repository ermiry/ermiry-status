#ifndef _ERMIRY_ROUTES_SERVICE_H_
#define _ERMIRY_ROUTES_SERVICE_H_

// GET /api/ermiry
extern void ermiry_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/ermiry/version
extern void ermiry_version_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif