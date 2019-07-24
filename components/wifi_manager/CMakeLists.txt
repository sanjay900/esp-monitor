
set(COMPONENT_REQUIRES nvs_flash wpa_supplicant mdns lwip tcpip_adapter)
set(COMPONENT_SRCS 
	"wifi_manager_main.c"
    "dns_server.c"
    "http_server.c"
    "json.c"
    "wifi_manager.c"
)
set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_EMBED_FILES "style.css jquery.gz code.js index.html")

register_component()