set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES mqtt tarablessd1306)
set(COMPONENT_PRIV_REQUIRES)

set(COMPONENT_SRCS ./mqtt_handler.c)

register_component()
