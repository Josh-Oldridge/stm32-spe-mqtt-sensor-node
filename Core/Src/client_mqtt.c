/**
 ******************************************************************************
 * @file    client_mqtt.c
 * @brief   MQTT Client Implementation for CN0575 Project
 * @details This file implements the MQTT client for the CN0575 Single Pair Ethernet (SPE)
 *          board project on the STM32L496ZG-P Nucleo board. It establishes a secure TLSv1.2
 *          connection from the ADIN1110 at IP 192.168.1.10 to the MQTT broker at IP 192.168.1.5,
 *          using the CA certificate from certificates.h. Manages connection, subscription to
 *          "sensors/config", and publishing of sensor data (temperature, acceleration, ADC)
 *          to "sensors/data" when USE_LWIP is defined. Integrates with FreeRTOS tasks from
 *          freertos.c for sensor data collection and uses LPUART1 for debug logging.
 * @addtogroup mqtt MQTT Configuration
 * @{
 ******************************************************************************
 */

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

/** @brief Global MQTT client instance, initialized in client_mqtt_init. */
mqtt_client_t *mqtt_client = NULL;

/** @brief IP address of the MQTT broker (192.168.1.5). Static DHCP entry
 *         on the Pheonix Contact 2303-8SP1 Switch
 */
#define MQTT_BROKER_IP_STR "192.168.1.5"

/** @brief Secure port for MQTT over TLS (8883). */
#define MQTT_BROKER_PORT_SECURE 8883

/** @brief Client ID for MQTT connection ("STM32_Client"). */
#define MQTT_CLIENT_ID     "STM32_Client"

/** @brief IP address structure for the MQTT broker, parsed from MQTT_BROKER_IP_STR. */
static ip_addr_t broker_ip;

/** @brief TLS configuration for secure MQTT connection, using broker_ca_cert. */
static struct altcp_tls_config *tls_config = NULL;

/** @brief Indicates if the MQTT client is connected to the broker. */
volatile bool mqtt_connected = false;

/** @brief Indicates if the MQTT client is attempting to connect. */
volatile bool mqtt_connecting = false;

/** @brief Indicates if sensor data is ready for publishing (set by freertos.c tasks). */
volatile bool sensors_ready = false;

/**
 * @brief Debug logging function for mbedTLS
 * @param [in] ctx Context pointer (unused)
 * @param [in] level Debug level (1-4)
 * @param [in] file Source file name
 * @param [in] line Source line number
 * @param [in] str Debug message string
 * @details Outputs mbedTLS debug messages to LPUART1 when TLS_DEBUG is defined, skipping
 *          leading newlines for cleaner logs.
 */
#ifdef TLS_DEBUG
void my_debug(void *ctx, int level, const char *file, int line, const char *str) {
    const char *p = str;
    while (*p && (*p == '\r' || *p == '\n'))
        p++;
    printf("[mbedTLS debug] %d: %s:%04d: %s", level, file, line, p);
}
#endif

/**
 * @brief Initialize the MQTT client with TLS
 * @details Creates a new MQTT client instance, configures TLS with broker_ca_cert, and attempts
 *          to connect to the broker at 192.168.1.5:8883. Sets up client ID, credentials, and
 *          keep-alive, logging progress via LPUART1. Cleans up on failure.
 */
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

#ifdef TLS_DEBUG
    mbedtls_ssl_conf_dbg((mbedtls_ssl_config*)tls_config, my_debug, NULL);
    mbedtls_debug_set_threshold(3);
#endif
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

/**
 * @brief MQTT connection callback
 * @param [in] client MQTT client instance
 * @param [in] arg User argument (unused)
 * @param [in] status Connection status
 * @details Handles connection events: on success, subscribes to "sensors/config" and sets up
 *          incoming data callbacks; on failure, cleans up resources and logs via LPUART1.
 */
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

/**
 * @brief MQTT incoming publish callback
 * @param [in] arg User argument (unused)
 * @param [in] topic Topic name of the incoming message
 * @param [in] tot_len Total length of the incoming message
 * @details Logs the topic and length of incoming publishes to LPUART1; currently a placeholder
 *          for configuration messages on "sensors/config".
 */
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
	printf("Incoming publish on topic: %s (total length %u)\n", topic,
			(unsigned int) tot_len);
}

/**
 * @brief MQTT incoming data callback
 * @param [in] arg User argument (unused)
 * @param [in] data Data payload of the incoming message
 * @param [in] len Length of the data chunk
 * @param [in] flags Flags indicating message state (e.g., end of message)
 * @details Logs incoming data from "sensors/config" to LPUART1 as a character stream; currently
 *          a placeholder for processing configuration updates.
 */
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
	printf("Incoming publish data: ");
	for (u16_t i = 0; i < len; i++)
		putchar(data[i]);
	printf("\n");
}

/**
 * @brief MQTT publish request callback
 * @param [in] arg User argument (unused)
 * @param [in] result Result of the publish operation
 * @details Logs publish success or failure to LPUART1; on connection errors (ERR_CONN), cleans
 *          up resources and signals reconnection in freertos.c’s SensorDataMQTTTask.
 */
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
#ifdef MQTT_CLIENT_DEBUG
        printf("Publish request successful\n");
#endif
	}
}

/**
 * @brief Publish sensor data to MQTT broker
 * @return ERR_OK on success, lwIP error code on failure
 * @details Acquires sensor data (temperature, acceleration, ADC) from latestSensorData via
 *          mutex, formats it as JSON, and publishes to "sensors/data" every 20 seconds via
 *          SensorDataMQTTTask in freertos.c. Logs errors or debug info to LPUART1.
 */
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

	if (temperature < -1000) {
		printf("Invalid temperature: %.2f\n", temperature);
		return ERR_VAL;
	}
	if (adc_value > 16383) {
		printf("Invalid ADC value: %u (max 16383)\n", adc_value);
		return ERR_VAL;
	}

	float voltage = (adc_value * 3.3f) / 16383.0f;
	float current = (voltage - 1.65f) / 0.33f;

	snprintf(payload, sizeof(payload),
			"{\"temperature\":%.2f,\"accel_x\":%d,\"accel_y\":%d,\"accel_z\":%d,"
					"\"voltage\":%.2f,\"current\":%.2f}", temperature, accel_x,
			accel_y, accel_z, voltage, current);

#ifdef MQTT_CLIENT_DEBUG
  printf("Publishing to topic: %s, Payload: %s\n", "sensors/data", payload);
#endif

	err_t err = mqtt_publish(mqtt_client, "sensors/data", payload,
			strlen(payload), 0, 0, mqtt_pub_request_cb, NULL);
	if (err != ERR_OK) {
		printf("mqtt_publish failed: %d\n", err);
		return err;
	}
	return ERR_OK;
}

/**
 * @brief Run the MQTT client publishing routine
 * @details Calls client_mqtt_publish_sensor_data to trigger a single publish cycle; currently
 *          a simple wrapper used by freertos.c’s SensorDataMQTTTask in the CN0575 project.
 */
void client_mqtt_run(void) {
	client_mqtt_publish_sensor_data();
}

/**
 * @}
 */
