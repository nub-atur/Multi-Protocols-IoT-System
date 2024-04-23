package com.example.iotapp08;

import android.util.Log;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

public class MqttHandler {

    private MqttClient client;

    public void connect(String brokerUrl, String clientId) {
        try {
            // Set up the persistence layer
            MemoryPersistence persistence = new MemoryPersistence();

            // Initialize the MQTT client
            client = new MqttClient(brokerUrl, clientId, persistence);

            // Set up the connection options
            MqttConnectOptions connectOptions = new MqttConnectOptions();
            connectOptions.setCleanSession(true);

            // Connect to the broker
            client.connect(connectOptions);

            if (connectOptions.isAutomaticReconnect()) Log.e("MQTT", "Connected to MQTT server");
            else Log.e("MQTT", "Failed to connect to MQTT server");
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public void disconnect() {
        try {
            client.disconnect();
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public void publish(String topic, String message) {
        try {
            MqttMessage mqttMessage = new MqttMessage(message.getBytes());
            client.publish(topic, mqttMessage);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    StringBuilder outputMessage;
    public void subscribe(String topic) {
        try {
            Log.d("tag", "mqtt channel name>>>>>>>>" + topic);
            Log.d("tag", "client.isConnected()>>>>>>>>" + client.isConnected());
            client.subscribe(topic);
            client.setCallback(new MqttCallback() {
                   @Override
                   public void connectionLost(Throwable cause) {
                   }

                   @Override
                   public void messageArrived(String topic, MqttMessage message) throws Exception {
                       outputMessage = new StringBuilder(new String(message.getPayload()));
                       Log.d("tag", "message>>" + outputMessage);
                       Log.d("tag", "topic>>" + topic);
                   }

                   @Override
                   public void deliveryComplete(IMqttDeliveryToken token) {
                   }
               }
            );
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public StringBuilder getOutputMessage() {
        return outputMessage;
    }
}
