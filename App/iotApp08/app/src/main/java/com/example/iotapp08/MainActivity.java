package com.example.iotapp08;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.google.android.material.textfield.TextInputEditText;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.util.Objects;


public class MainActivity extends AppCompatActivity {

    private static final String BROKER_URL = "tcp://karolineserver.duckdns.org:1883";
    private static final String CLIENT_ID = "Im App";
    private MqttClient client;
    TextInputEditText editText;
    String outputMessage;
    Button sendButton;
    Button updateButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        editText = findViewById(R.id.textInputEditText);
        sendButton = findViewById(R.id.sendButton);
        updateButton = findViewById(R.id.myButton);

        connect();
        subscribe();

        updateButton.setOnClickListener(v -> subscribe());
        sendButton.setOnClickListener(this::onSendCommandClick);
    }

    @Override
    protected void onDestroy() {
        disconnect();
        super.onDestroy();
    }

    private void connect() {
        try {
            // Set up the persistence layer
            MemoryPersistence persistence = new MemoryPersistence();

            // Initialize the MQTT client
            client = new MqttClient(MainActivity.BROKER_URL, MainActivity.CLIENT_ID, persistence);

            // Set up the connection options
            MqttConnectOptions connectOptions = new MqttConnectOptions();
            connectOptions.setCleanSession(true);

            // Connect to the broker
            client.connect(connectOptions);

            if (connectOptions.isAutomaticReconnect()) Log.e("tag", "Connected to MQTT server");
            else Log.e("tag", "Failed to connect to MQTT server");
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    private void disconnect() {
        try {
            client.disconnect();
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    private void publish(String message) {
        try {
            MqttMessage mqttMessage = new MqttMessage(message.getBytes());
            client.publish("KA/fromApp", mqttMessage);
            Toast.makeText(MainActivity.this, "Sending input text to backend: " + message, Toast.LENGTH_SHORT).show();
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    private void subscribe() {
        try {
            Log.d("tag", "mqtt channel name >>>>>>>>" + "KA/fromPi");
            Log.d("tag", "client.isConnected >>>>>>>>" + client.isConnected());
            client.subscribe("KA/fromPi");
            client.setCallback(new MqttCallback() {
                final TextView mess = findViewById(R.id.message);
                   @Override
                   public void connectionLost(Throwable cause) {
                       Toast.makeText(MainActivity.this, "droppedConnection", Toast.LENGTH_SHORT).show();
                   }

                   @Override
                   public void messageArrived(String topic, MqttMessage message) {
                       outputMessage = new String(message.getPayload());
                       Log.d("tag", "message>>" + outputMessage);

                       runOnUiThread(() -> mess.setText(outputMessage));
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

    public void onPauseClick(View view) {
    }

    public void onSendCommandClick(View view) {
        String cmd = editText.getText().toString();
        publish(cmd);
    }

    public void onUpDateClick(View view) {
    }

}