set(COMPONENT_ADD_INCLUDEDIRS .)

# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES esp8266_wrapper)
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS ./sht3x.c)

register_component()
