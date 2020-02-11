set(srcs "net_handler.c" "net_eth.c" "net_wifi.c")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
					REQUIRES "esp_http_server nvs_flash libesphttpd")
				
