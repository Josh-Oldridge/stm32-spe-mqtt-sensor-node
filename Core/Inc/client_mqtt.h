#ifndef CLIENT_MQTT_H
#define CLIENT_MQTT_H

#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/altcp.h"

extern mqtt_client_t *mqtt_client;
void my_debug(void *ctx, int level, const char *file, int line, const char *str);
err_t tls_connected_cb(void *arg, struct altcp_pcb *pcb, err_t err);
void tls_error_cb(void *arg, err_t err);

/* Function prototypes for MQTT client logic */
void client_mqtt_init(void);
err_t client_mqtt_publish_sensor_data(void);
void client_mqtt_run(void);
void client_mqtt_publish_heartbeat(void);

/* Callback prototypes */
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
void mqtt_pub_request_cb(void *arg, err_t result);

#endif /* CLIENT_MQTT_H */
