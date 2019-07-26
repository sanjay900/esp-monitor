/* HTTP Server
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include "esp_http_server.h"

static const char *TAG = "server_monitor_handler";

/* If adding or changing and site files, make sure they added or removed from component.mk file. 
 * Also, if changes are made you need to clean then rebuild for it to compile those new files.
 * It does not do it automatically. 
 */
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

extern const uint8_t style_css_start[] asm("_binary_style_css_start");
extern const uint8_t style_css_end[]   asm("_binary_style_css_end");

extern const uint8_t log_html_start[] asm("_binary_log_html_start");
extern const uint8_t log_html_end[]   asm("_binary_log_html_end");

extern const uint8_t sensor_data_html_start[] asm("_binary_sensor_data_html_start");
extern const uint8_t sensor_data_html_end[]   asm("_binary_sensor_data_html_end");

/* Handler GET request for /index.html */
esp_err_t index_html_get_handler(httpd_req_t *req)
{
	ESP_LOGD(TAG, "index.html requested");
	httpd_resp_set_type(req,"text/html");
    httpd_resp_send(req, (const char *)index_html_start, (index_html_end-1) - index_html_start);
	/* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);
	return ESP_OK;
}

/* Handler GET request for /log.html */
esp_err_t log_html_get_handler(httpd_req_t *req)
{
	ESP_LOGD(TAG, "log.html requested");
	httpd_resp_set_type(req,"text/html");
    httpd_resp_send(req, (const char *)log_html_start, (log_html_end-1) - log_html_start);
	/* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);
	return ESP_OK;
}

/* Handler GET request for /sensor_data.html */
esp_err_t sensor_data_html_get_handler(httpd_req_t *req)
{
	ESP_LOGD(TAG, "sensor_data.html requested");
	httpd_resp_set_type(req,"text/html");
    httpd_resp_send(req, (const char *)sensor_data_html_start, (sensor_data_html_end-1) - sensor_data_html_start);
	/* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);
	return ESP_OK;
}

/* Handler GET request for /style.css */
esp_err_t style_css_get_handler(httpd_req_t *req)
{
	ESP_LOGD(TAG, "style.css requested");
	httpd_resp_set_type(req,"text/css");
    httpd_resp_send(req, (const char *)style_css_start, (style_css_end-1) - style_css_start);
	/* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);
	return ESP_OK;
}

/* Function to start the file server */
esp_err_t start_server()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server");
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }
	
	httpd_uri_t index_html = {
		.uri       = "/",
		.method    = HTTP_GET,
		.handler   = index_html_get_handler,
		.user_ctx  = NULL
	};
	httpd_uri_t index_html1 = {
		.uri       = "/index.html",
		.method    = HTTP_GET,
		.handler   = index_html_get_handler,
		.user_ctx  = NULL
	};
	httpd_register_uri_handler(server, &index_html);
	httpd_register_uri_handler(server, &index_html1);
	httpd_uri_t log_html = {
		.uri       = "/log.html",
		.method    = HTTP_GET,
		.handler   = log_html_get_handler,
		.user_ctx  = NULL
	};
	httpd_register_uri_handler(server, &log_html);
	httpd_uri_t sensor_data_html = {
		.uri       = "/sensor_data.html",
		.method    = HTTP_GET,
		.handler   = sensor_data_html_get_handler,
		.user_ctx  = NULL
	};
	httpd_register_uri_handler(server, &sensor_data_html);
	httpd_uri_t style_css = {
		.uri       = "/style.css",
		.method    = HTTP_GET,
		.handler   = style_css_get_handler,
		.user_ctx  = NULL
	};
	httpd_register_uri_handler(server, &style_css);
	
    return ESP_OK;
}