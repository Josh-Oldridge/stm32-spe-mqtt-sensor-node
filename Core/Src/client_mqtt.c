#include "client_mqtt.h"
#include <string.h>
#include <stdio.h>
#include "lwip/mem.h"
#include "lwip/err.h"
#include "lwip/timeouts.h"

/* Global MQTT client pointer */
mqtt_client_t *mqtt_client = NULL;

/* Broker settings: adjust these as necessary */
#define MQTT_BROKER_IP_STR "192.168.1.11"  // Broker IP (your laptop)
#define MQTT_BROKER_PORT   1883
#define MQTT_CLIENT_ID     "STM32_Client"

/* Convert IP string to ip_addr_t structure (you might use ipaddr_aton) */
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
    // Optionally set username/password if required
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

        // Optionally, subscribe to topics if needed:
        err_t err = mqtt_subscribe(client, "sensors/config", 0, mqtt_pub_request_cb, arg);
        if(err != ERR_OK) {
            printf("mqtt_subscribe failed: %d\n", err);
        }
    } else {
        printf("MQTT connection failed, status: %d\n", status);
        // Optionally, implement a reconnect strategy here.
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

/* Example function to publish sensor data */
void client_mqtt_publish_sensor_data(void) {
    char payload[128];
    // Here, format your sensor data as JSON or another string format.
    // For example, let's assume you have temperature and acceleration data:
    float temperature = 25.0f;  // Replace with actual sensor reading
    int16_t accel_x = 100;      // Replace with actual sensor reading
    int16_t accel_y = -50;      // Replace with actual sensor reading
    int16_t accel_z = 0;        // Replace with actual sensor reading

    snprintf(payload, sizeof(payload),
             "{\"temperature\":%.2f,\"accel_x\":%d,\"accel_y\":%d,\"accel_z\":%d}",
             temperature, accel_x, accel_y, accel_z);

    //HAL_Delay(1000);

    err_t err = mqtt_publish(mqtt_client, "sensors/data", payload, strlen(payload), 0, 0, mqtt_pub_request_cb, NULL);
    if(err != ERR_OK) {
        printf("mqtt_publish failed: %d\n", err);
    }
}

/* MQTT client main loop function - call this periodically */
void client_mqtt_run(void) {
    /* For a basic implementation, this function might just publish sensor data.
     * In a real application, you might handle reconnections and subscriptions here.
     */
    client_mqtt_publish_sensor_data();
}
