#set(COMPONENT_ADD_INCLUDEDIRS . data)

# Edit following two lines to set component requirements (see docs)
#set(COMPONENT_REQUIRES esp_http_server)
#set(COMPONENT_PRIV_REQUIRES)

#set(COMPONENT_SRCS ./server_monitor_handler.c)

idf_component_register(SRCS "server_monitor_handler.c"
                    INCLUDE_DIRS "."
					REQUIRES "esp_http_server"
					EMBED_TXTFILES "index.html" "log.html" "sensor_data.html" "style.css")
					
#register_component()
