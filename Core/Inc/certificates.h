/**
  ******************************************************************************
  * @file    certificates.h
  * @brief   TLS Certificate Definitions for CN0575 MQTT Connection
  * @details This header defines the CA certificate used in the CN0575 Single Pair Ethernet
  *          (SPE) board project for establishing a secure TLSv1.2 connection between the
  *          ADIN1110 MAC-PHY at IP 192.168.1.10 and the MQTT broker at IP 192.168.1.5. The
  *          certificate is passed to the MQTT client in client_mqtt.c during initialization
  *          when USE_LWIP is defined, enabling encrypted communication over lwIP. The
  *          certificate used here is the one I created called myCA.crt. This cert was used
  *          to sign the W&T MQTT.box auto generated CSR and then was uploaded to the box
  *          via the webpage at "https://192.168.1.5/"
  * @addtogroup mqtt MQTT Configuration
  * @{
  ******************************************************************************
  */

#ifndef CERTIFICATES_H
#define CERTIFICATES_H

/**
  * @brief CA certificate for MQTT broker authentication
  * @details Contains the PEM-encoded CA certificate for the MQTT broker at 192.168.1.5,
  *          used by client_mqtt.c to verify the broker’s identity during TLS handshake.
  *          Is valid from Feb 18, 2025, to Feb 18, 2026.
  */
static const char broker_ca_cert[] = "-----BEGIN CERTIFICATE-----\r\n"
    "REPLACE_WITH_LOCAL_CA_CERT\r\n"
    "-----END CERTIFICATE-----\r\n";

#endif /* CERTIFICATES_H */

/**
  * @}
  */
