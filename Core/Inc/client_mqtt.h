/**
  ******************************************************************************
  * @file    client_mqtt.h
  * @brief   MQTT Client Interface for CN0575 Project
  * @details This header defines the interface for the MQTT client in the CN0575 Single Pair
  *          Ethernet (SPE) board project on the STM32L496ZG-P Nucleo board. It declares
  *          functions and callbacks for establishing a secure TLSv1.2 connection from the
  *          ADIN1110 at IP 192.168.1.10 to the MQTT broker at IP 192.168.1.5, managing
  *          subscriptions, and publishing sensor data (temperature, acceleration, ADC) when
  *          USE_LWIP is defined. Used by freertos.c’s SensorDataMQTTTask for MQTT operations.
  * @addtogroup mqtt MQTT Configuration
  * @{
  ******************************************************************************
  */

#ifndef CLIENT_MQTT_H
#define CLIENT_MQTT_H

#include "lwip/apps/mqtt.h"
#include "lwip/err.h"

/** @brief Global MQTT client instance, managed by client_mqtt.c. */
extern mqtt_client_t *mqtt_client;

/** @brief Flag indicating if sensor data is ready for publishing, set by freertos.c tasks. */
extern volatile bool sensors_ready;


/**
  * @brief Debug logging function for mbedTLS
  * @param [in] ctx Context pointer (unused)
  * @param [in] level Debug level (1-4)
  * @param [in] file Source file name
  * @param [in] line Source line number
  * @param [in] str Debug message string
  * @details Outputs mbedTLS debug messages to LPUART1 when TLS_DEBUG is defined.
  */
void my_debug(void *ctx, int level, const char *file, int line, const char *str);

/* Function prototypes for MQTT client logic */

/**
  * @brief Initialize the MQTT client with TLS
  * @details Sets up the MQTT client and initiates a TLS connection to the broker at 192.168.1.5:8883.
  */
void client_mqtt_init(void);

/**
  * @brief Publish sensor data to MQTT broker
  * @return ERR_OK on success, lwIP error code on failure
  * @details Publishes sensor data (temperature, acceleration, ADC) to "sensors/data" as JSON.
  */
err_t client_mqtt_publish_sensor_data(void);

/**
  * @brief Run the MQTT client publishing routine
  * @details Triggers a single publish cycle, called by SensorDataMQTTTask in freertos.c.
  */
void client_mqtt_run(void);

/* Callback prototypes */

/**
  * @brief MQTT connection callback
  * @param [in] client MQTT client instance
  * @param [in] arg User argument (unused)
  * @param [in] status Connection status
  * @details Handles connection events, subscribing to "sensors/config" on success.
  */
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

/**
  * @brief MQTT incoming publish callback
  * @param [in] arg User argument (unused)
  * @param [in] topic Topic name of the incoming message
  * @param [in] tot_len Total length of the incoming message
  * @details Logs incoming publish events from "sensors/config".
  */
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);

/**
  * @brief MQTT incoming data callback
  * @param [in] arg User argument (unused)
  * @param [in] data Data payload of the incoming message
  * @param [in] len Length of the data chunk
  * @param [in] flags Flags indicating message state
  * @details Processes incoming data from "sensors/config" subscriptions.
  */
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);

/**
  * @brief MQTT publish request callback
  * @param [in] arg User argument (unused)
  * @param [in] result Result of the publish operation
  * @details Handles the outcome of publishing to "sensors/data".
  */
void mqtt_pub_request_cb(void *arg, err_t result);

/**
  * @}
  */

#endif /* CLIENT_MQTT_H */
