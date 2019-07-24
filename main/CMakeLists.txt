# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES wifi_manager mqtt-handler tarablessd1306 sht3x esp8266_wrapper)
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
