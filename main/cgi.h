#pragma once
#include "libesphttpd/httpd.h"
#include "config.h"

CgiStatus tplCurrentConfig(HttpdConnData *connData, char *token, void **arg);