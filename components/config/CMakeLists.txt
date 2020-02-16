idf_component_register(SRCS "cgi.c" "config.c"
                    INCLUDE_DIRS "."
					REQUIRES "nvs_flash spiffs libesphttpd")
				
