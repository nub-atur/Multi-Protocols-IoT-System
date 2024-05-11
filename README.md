# Multi-Protocols-IoT-System

This repository collects the basic functionalities of using various protocols to build an IoT system aimed at handling multiple protocols with long distance, speed, and intelligence. It includes:

    A mesh of three ESP32 nodes programmed using the Arduino framework, with the main library being 'painlessMesh'. One node serves as the root and establishes connections between the sx1278 and ESP32, aiming to achieve full-duplex communication in a peer-to-peer connection. Another node functions as a send-only node integrated with an IR sensor for obstacle detection. The last node handles received commands, drives an LCD 1602 (PCF85 I2C), and sends DHT22 sensor data. The root node receives data from the Raspberry Pi and broadcasts messages to the target nodes.

    The Raspberry Pi 3B communicates via SPI with the LoRa module sx1278 and operates as a client in the MQTT protocol. It uses the Python language with the pyLoRa library. Additionally, it connects with one more ESP32 through a UDP socket.

    ESP32 UDP coding is done on the ESP-IDF framework to control pulse width driving LED WS2812.

    An app integrated MQTT connection with the server, built on the Android Studio IDE, using the Java language. For now, it can send a string to the IoT system and monitor all sensor data from it.
    
In the hardware part, coding follows multitasking (ESP32, FreeRTOS) and multithreading (Pi, multithread).

Continue...