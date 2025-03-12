#include "client_mqtt.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "lwip/altcp_tls.h"
#include "lwip/err.h"
#include <string.h>
#include <stdio.h>
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "tmp102.h"
#include "adxl345.h"
#include "i2c.h"
#include "sensor_data.h"
#include "certificates.h"

mqtt_client_t *mqtt_client = NULL;

#define MQTT_BROKER_IP_STR "192.168.1.5"
#define MQTT_BROKER_PORT_SECURE 8883
#define MQTT_CLIENT_ID     "STM32_Client"

static ip_addr_t broker_ip;
static struct altcp_tls_config *tls_config = NULL;

volatile bool mqtt_connected = false;
volatile bool mqtt_connecting = false;
volatile bool sensors_ready = false;

void my_debug(void *ctx, int level, const char *file, int line, const char *str) {
	const char *p = str;
	while (*p && (*p == '\r' || *p == '\n'))
		p++;
	printf("[mbedTLS debug] %d: %s:%04d: %s", level, file, line, p);
}

void client_mqtt_init(void) {
	err_t err;
	struct mqtt_connect_client_info_t ci;

	if (mqtt_client != NULL) {
		mqtt_disconnect(mqtt_client);
		if (tls_config != NULL) {
			altcp_tls_free_config(tls_config);
			tls_config = NULL;
		}
		mqtt_client_free(mqtt_client);
		mqtt_client = NULL;
	}

	mqtt_client = mqtt_client_new();
	if (mqtt_client == NULL) {
		printf("Failed to create MQTT client instance\n");
		return;
	}

	memset(&ci, 0, sizeof(ci));
	ci.client_id = MQTT_CLIENT_ID;
	ci.keep_alive = 60;
	ci.client_user = "admin";
	ci.client_pass = "admin";

#if LWIP_ALTCP && LWIP_ALTCP_TLS
	tls_config = altcp_tls_create_config_client((const u8_t*) broker_ca_cert,
			strlen(broker_ca_cert) + 1);
	if (tls_config == NULL) {
		printf("Failed to create TLS config\n");
		mqtt_client_free(mqtt_client);
		mqtt_client = NULL;
		return;
	}
	mbedtls_ssl_conf_max_frag_len((mbedtls_ssl_config*) tls_config,
			MBEDTLS_SSL_MAX_FRAG_LEN_4096);
	ci.tls_config = tls_config;
//    mbedtls_ssl_conf_dbg((mbedtls_ssl_config*) tls_config, my_debug, NULL);
//    mbedtls_debug_set_threshold(3);
#endif

	if (!ipaddr_aton(MQTT_BROKER_IP_STR, &broker_ip)) {
		printf("Invalid broker IP address\n");
		if (tls_config != NULL) {
			altcp_tls_free_config(tls_config);
			tls_config = NULL;
		}
		mqtt_client_free(mqtt_client);
		mqtt_client = NULL;
		return;
	}

	printf("Initiating MQTT connection: Client ID=%s, Broker IP=%s, Port=%d\n",
			ci.client_id, MQTT_BROKER_IP_STR, MQTT_BROKER_PORT_SECURE);

	err = mqtt_client_connect(mqtt_client, &broker_ip, MQTT_BROKER_PORT_SECURE,
			mqtt_connection_cb, NULL, &ci);
	if (err != ERR_OK) {
		printf("MQTT connect failed: %d\n", err);
		if (tls_config != NULL) {
			altcp_tls_free_config(tls_config);
			tls_config = NULL;
		}
		mqtt_client_free(mqtt_client);
		mqtt_client = NULL;
	}
}

void mqtt_connection_cb(mqtt_client_t *client, void *arg,
		mqtt_connection_status_t status) {
	mqtt_connecting = false;
	if (status == MQTT_CONNECT_ACCEPTED) {
		mqtt_connected = true;
		printf("MQTT connected successfully\n");

		mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb,
				mqtt_incoming_data_cb, arg);

		printf("Subscribing to topic: %s\n", "sensors/config");

		err_t err = mqtt_subscribe(client, "sensors/config", 0,
				mqtt_pub_request_cb, arg);
		if (err != ERR_OK) {
			printf("mqtt_subscribe failed: %d\n", err);
		}
	} else {
		printf("MQTT connection failed or disconnected, status: %d\n", status);
		mqtt_connected = false;
		mqtt_disconnect(client);
		if (tls_config != NULL) {
			altcp_tls_free_config(tls_config);
			tls_config = NULL;
		}
		mqtt_client_free(client);
		mqtt_client = NULL;
	}
}

void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
	printf("Incoming publish on topic: %s (total length %u)\n", topic,
			(unsigned int) tot_len);
}

void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
	printf("Incoming publish data: ");
	for (u16_t i = 0; i < len; i++)
		putchar(data[i]);
	printf("\n");
}

void mqtt_pub_request_cb(void *arg, err_t result) {
	if (result != ERR_OK) {
		printf("Publish request failed: %d\n", result);
		if (result == ERR_CONN) {
			mqtt_connected = false;
			mqtt_connecting = false;
			mqtt_disconnect(mqtt_client);
			if (tls_config != NULL) {
				altcp_tls_free_config(tls_config);
				tls_config = NULL;
			}
			mqtt_client_free(mqtt_client);
			mqtt_client = NULL;
			printf(
					"Connection lost, resources freed, ready for reconnection\n");
		}
	} else {
		printf("Publish request successful\n");
	}
}

err_t client_mqtt_publish_sensor_data(void) {
  if (!mqtt_connected || mqtt_client == NULL) {
    printf("Cannot publish: MQTT not connected\n");
    return ERR_CONN;
  }

  char payload[256];
  float temperature;
  int16_t accel_x, accel_y, accel_z;
  uint16_t adc_value;

  if (xSemaphoreTake(sensorDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    temperature = latestSensorData.temperature;
    accel_x = latestSensorData.accel_x;
    accel_y = latestSensorData.accel_y;
    accel_z = latestSensorData.accel_z;
    adc_value = latestSensorData.adc_value;
    xSemaphoreGive(sensorDataMutex);
  } else {
    printf("Failed to acquire sensor data mutex!\n");
    return ERR_TIMEOUT;
  }

  if (temperature < -1000 || adc_value > 4095) {
    printf("Invalid sensor data: temp=%.2f, adc=%u\n", temperature, adc_value);
    return ERR_VAL;
  }

  float voltage = (adc_value * 3.3f) / 4095.0f;
  float current = (voltage - 1.65f) / 0.185f;

  snprintf(payload, sizeof(payload),
           "{\"temperature\":%.2f,\"accel_x\":%d,\"accel_y\":%d,\"accel_z\":%d,"
           "\"voltage\":%.2f,\"current\":%.2f}", temperature, accel_x,
           accel_y, accel_z, voltage, current);

  printf("Publishing to topic: %s, Payload: %s\n", "sensors/data", payload);
  err_t err = mqtt_publish(mqtt_client, "sensors/data", payload,
                          strlen(payload), 0, 0, mqtt_pub_request_cb, NULL);
  if (err != ERR_OK) {
    printf("mqtt_publish failed: %d\n", err);
    return err;
  }
  return ERR_OK;
}

void client_mqtt_run(void) {
	client_mqtt_publish_sensor_data();
}
