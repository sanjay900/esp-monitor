 set(srcs "net_handler.c")

if(CONFIG_ACTIVE_WIFI)
    list(APPEND srcs "net_wifi.c")
endif()
if(CONFIG_ACTIVE_ETHERNET)
    list(APPEND srcs "net_eth.c")
endif()
idf_component_register(SRCS "net_handler.c"
                    INCLUDE_DIRS "."
					REQUIRES "esp_http_server")
				
