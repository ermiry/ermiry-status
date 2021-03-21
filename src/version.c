#include <cerver/utils/log.h>

#include "version.h"

// print full ermiry version information
void ermiry_version_print_full (void) {

	cerver_log_msg ("\nErmiry Status Version: %s\n", ERMIRY_VERSION_NAME);
	cerver_log_msg ("Release Date & time: %s - %s\n", ERMIRY_VERSION_DATE, ERMIRY_VERSION_TIME);
	cerver_log_msg ("Author: %s\n", ERMIRY_VERSION_AUTHOR);

}

// print the version id
void ermiry_version_print_version_id (void) {

	cerver_log_msg ("\nErmiry Status Version ID: %s\n", ERMIRY_VERSION);

}

// print the version name
void ermiry_version_print_version_name (void) {

	cerver_log_msg ("\nErmiry Status Version: %s\n", ERMIRY_VERSION_NAME);

}