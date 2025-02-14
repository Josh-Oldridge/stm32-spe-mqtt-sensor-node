#include "client_mqtt.h"
#include <string.h>
#include <stdio.h>
#include "lwip/mem.h"
#include "lwip/err.h"
#include "lwip/timeouts.h"

#include "tmp102.h"
#include "adxl345.h"
#include "i2c.h"


extern uint16_t adcBuffer[ADC_BUFFER_SIZE];
mqtt_client_t *mqtt_client = NULL;

/* Broker settings: adjust these as necessary */
#define MQTT_BROKER_IP_STR "192.168.1.11"  // Broker IP
#define MQTT_BROKER_PORT   1883
#define MQTT_CLIENT_ID     "STM32_Client"

static ip_addr_t broker_ip;

/* Initialize MQTT client and connect to the broker */
void client_mqtt_init(void) {
    err_t err;
    struct mqtt_connect_client_info_t ci;

    /* Create new MQTT client */
    mqtt_client = mqtt_client_new();
    if (mqtt_client == NULL) {
        printf("Failed to create MQTT client instance\n");
        return;
    }

    /* Set up client info */
    memset(&ci, 0, sizeof(ci));
    ci.client_id = MQTT_CLIENT_ID;
    ci.keep_alive = 60;
    // username/password
    // ci.client_user = "username";
    // ci.client_pass = "password";

    /* Convert broker IP string to ip_addr_t */
    if (!ipaddr_aton(MQTT_BROKER_IP_STR, &broker_ip)) {
        printf("Invalid broker IP address\n");
        return;
    }

    /* Connect to the MQTT broker.
     * The mqtt_connection_cb function will be called with the result.
     */
    err = mqtt_client_connect(mqtt_client, &broker_ip, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &ci);
    if (err != ERR_OK) {
        printf("mqtt_client_connect failed: %d\n", err);
    }
}

/* Connection callback */
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if(status == MQTT_CONNECT_ACCEPTED) {
        printf("MQTT connected successfully\n");

        /* Set incoming publish callbacks */
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);

        // Subscribe to topics
        err_t err = mqtt_subscribe(client, "sensors/config", 0, mqtt_pub_request_cb, arg);
        if(err != ERR_OK) {
            printf("mqtt_subscribe failed: %d\n", err);
        }
    } else {
        printf("MQTT connection failed, status: %d\n", status);
        // Reconnect Strategy...
    }
}

/* Callback for incoming publish messages (called when a new publish message starts) */
void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    printf("Incoming publish on topic: %s (total length %u)\n", topic, (unsigned int)tot_len);
}

/* Callback for incoming publish data (called with each data fragment) */
void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    printf("Incoming publish data: ");
    for (u16_t i = 0; i < len; i++) {
        putchar(data[i]);
    }
    printf("\n");
}

/* Callback for publish request completion */
void mqtt_pub_request_cb(void *arg, err_t result) {
    if(result != ERR_OK) {
        printf("Publish request failed: %d\n", result);
    } else {
        printf("Publish request successful\n");
    }
}

err_t client_mqtt_publish_sensor_data(void) {
    char payload[256];
    /* Temperature Read */
    float temperature = TMP102_ReadTemperature();

    /* Accelerometer Read */
    int16_t accel_x = 0, accel_y = 0, accel_z = 0;
    HAL_StatusTypeDef ret = ADXL345_ReadAccel(&hi2c2, &accel_x, &accel_y, &accel_z);
    if(ret != HAL_OK) {
        printf("ADXL345 Read Error!\n");
        /*Default Values*/
        accel_x = accel_y = accel_z = 0;
    }

    /* Current Read */
    uint16_t adcValue = adcBuffer[0];
    float voltage = (adcValue * 3.3f) / 4095.0f;
    float current = (voltage - 1.65f) / 0.185f;

    snprintf(payload, sizeof(payload),
             "{\"temperature\":%.2f,\"accel_x\":%d,\"accel_y\":%d,\"accel_z\":%d,"
             "\"voltage\":%.2f,\"current\":%.2f}",
             temperature, accel_x, accel_y, accel_z, voltage, current);

    err_t err = mqtt_publish(mqtt_client, "sensors/data", payload, strlen(payload), 0, 0, mqtt_pub_request_cb, NULL);
    if(err != ERR_OK) {
        printf("mqtt_publish failed: %d\n", err);
    }

    return ERR_OK;
}

void client_mqtt_run(void) {

    client_mqtt_publish_sensor_data();
}

/* Heartbeat Packet to keep TCP Connection open */
//void client_mqtt_publish_heartbeat(void) {
//    const char heartbeatPayload[] = "ping";
//    err_t err = mqtt_publish(mqtt_client, "sensors/heartbeat", heartbeatPayload, sizeof(heartbeatPayload) - 1, 0, 0, mqtt_pub_request_cb, NULL);
//    if(err != ERR_OK) {
//        printf("mqtt_publish heartbeat failed: %d\n", err);
//    } else {
//        printf("Heartbeat published\n");
//    }
//}
