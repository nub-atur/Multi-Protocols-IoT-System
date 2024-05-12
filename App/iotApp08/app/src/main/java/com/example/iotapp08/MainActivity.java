package com.example.iotapp08;

import android.graphics.Color;
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
import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.LegendRenderer;
import com.jjoe64.graphview.Viewport;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

import org.eclipse.paho.client.mqttv3.IMqttActionListener;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.IMqttToken;
import org.eclipse.paho.client.mqttv3.MqttAsyncClient;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.util.ArrayList;
import java.util.Objects;
import java.util.Random;

public class MainActivity extends AppCompatActivity {
    private static final String BROKER_URL = "tcp://karolineserver.duckdns.org:1883";
    private static final String clientId = MqttClient.generateClientId();
//    private static final String[] topics = {
//                                    "KA/fromPi",
//                                    "KA/snr",
//                                    "KA/rssi"
//                            };
//    private static final int[] MQTT_QOS = {1, 1, 1}; // QoS levels for each topic
    private static final String TAG = "RunningService";
    private MqttClient client;
    ArrayList<Integer> timeBuff = new ArrayList<>();
    ArrayList<Number> tempBuff = new ArrayList<>();
    ArrayList<Number> humiBuff = new ArrayList<>();
    LineGraphSeries<DataPoint> seriesTemp;
    LineGraphSeries<DataPoint> seriesHumi;
    GraphView mScatterPlot;
    Button sendButton;
    String outputMessage;
    Button updateButton;
    String obs;
    String SNR;
    String RSSI;

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

        mScatterPlot = findViewById(R.id.idGraphView);
        sendButton = findViewById(R.id.sendButton);
        updateButton = findViewById(R.id.update_now);

//        timeBuff.add(0);
//        tempBuff.add(25.0);
//        humiBuff.add(70.0);

        //make xyValueArray global
        seriesTemp = new LineGraphSeries<>();
        seriesHumi = new LineGraphSeries<>();

        connect();
        subscribe();

        seriesTemp.setColor(Color.BLUE);
        seriesHumi.setColor(Color.RED);
        seriesTemp.setTitle("Temperature");
        seriesHumi.setTitle("Humidity");

        mScatterPlot.getGridLabelRenderer().setVerticalAxisTitle("temp(deg)");
        mScatterPlot.getGridLabelRenderer().setHorizontalAxisTitle("time(s)");

        mScatterPlot.addSeries(seriesTemp);
        mScatterPlot.getSecondScale().addSeries(seriesHumi);

        // Set manual Y bounds for second Y-axis
        mScatterPlot.getSecondScale().setMinY(55);
        mScatterPlot.getSecondScale().setMaxY(80);
        // Set legend to show titles and colors
        mScatterPlot.getLegendRenderer().setVisible(true);
        mScatterPlot.getLegendRenderer().setTextSize(20);
        mScatterPlot.getLegendRenderer().setAlign(LegendRenderer.LegendAlign.TOP);
//        mScatterPlot.getGridLabelRenderer().setVerticalAxisTitleTextSize(36);
        mScatterPlot.getGridLabelRenderer().setLabelsSpace(5);

        updateButton.setOnClickListener(this::onUpDateClick);
        sendButton.setOnClickListener(this::onSendCommandClick);
    }
//    @Override
//    protected void onResume() {
//        super.onResume();
////        subscribe();
//        // we're going to simulate real time with thread that append data to the graph
//        new Thread(() -> {
//            // we add 100 new entries
////            for (int i = 0; i < 100; i++) {
//                runOnUiThread(this::addEntry);
//
//                // sleep to slow down the add of entries
//                try {
//                    Thread.sleep(600);
//                } catch (InterruptedException e) {
//                    // manage error ...
//                }
////            }
//        }).start();
//    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        disconnect();
    }

    private void connect() {
        try {
            // Set up the persistence layer
            MemoryPersistence persistence = new MemoryPersistence();

            // Initialize the MQTT client
            client = new MqttClient(MainActivity.BROKER_URL, MainActivity.clientId, persistence);

            // Set up the connection options
            MqttConnectOptions connectOptions = new MqttConnectOptions();
            connectOptions.setCleanSession(true);
            connectOptions.setAutomaticReconnect(true);
            // Connect to the broker
            client.connect(connectOptions);
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
            toastMessage("Sending text: " + message);
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    private void subscribe() {
        try {
            Log.d(TAG, "client.isConnected >>>>>>>>" + client.isConnected());
            client.subscribe("KA/fromPi");

            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    toastMessage("droppedConnection");
                }
                @Override
                public void messageArrived(String topic, MqttMessage message) {
                    outputMessage = new String(message.getPayload()); //rec data
                    Log.d(TAG, "message>>" + outputMessage);

                    //Decode
                    String[] parts = outputMessage.split(" ");

                    Double temp = Double.parseDouble(parts[0]);
                    Double hum = Double.parseDouble(parts[1]);
                    obs = parts[2];
                    int tNew = Integer.parseInt(parts[3]);
                    SNR = parts[4];
                    RSSI = parts[5];

                    tempBuff.add(temp);
                    humiBuff.add(hum);
                    timeBuff.add(tNew);

//                    seriesHumi.appendData(new DataPoint(timeBuff.get(timeBuff.size()-1), (double) humiBuff.get(humiBuff.size()-1)), false, 10, true);
//                    seriesTemp.appendData(new DataPoint(timeBuff.get(timeBuff.size()-1), (double) tempBuff.get(tempBuff.size()-1)), false, 10, true);

                    // Add series to graph
                    seriesTemp.resetData(generateDataPoints(tempBuff));
                    seriesHumi.resetData(generateDataPoints(humiBuff));
                }
                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                }
            });

        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    private DataPoint[] generateDataPoints(ArrayList<Number> a) {
        DataPoint[] dataPoints = new DataPoint[timeBuff.size()];
        for (int i = 0; i < timeBuff.size(); i++) {
            dataPoints[i] = new DataPoint(timeBuff.get(i), (double) a.get(i));
        }
        return dataPoints;
    }
    public void onSendCommandClick(View view) {
        TextInputEditText editText = findViewById(R.id.textInputEditText);
        String cmd = Objects.requireNonNull(editText.getText()).toString();
        publish(cmd);
    }

    public void onUpDateClick(View view){
        final TextView mess = findViewById(R.id.message);
        final TextView obstacle = findViewById(R.id.textOb);
        final TextView calcView = findViewById(R.id.SnrAndRssi);
        String calc = SNR + "dB  " + RSSI + "dBm";
        runOnUiThread(() -> mess.setText(outputMessage)); // show data on app
        runOnUiThread(() -> obstacle.setText(obs));
        runOnUiThread(() -> calcView.setText(calc));
    }

    /**
     * customizable toast
     */
    private void toastMessage(String message){
        Toast.makeText(MainActivity.this, message, Toast.LENGTH_SHORT).show();
    }

//    private void addEntry() {
//        // here, we choose to display max 10 points on the viewport and we scroll to end
//        //seriesTemp.appendData(new DataPoint(lastX++, RANDOM.nextDouble() * 10d), false, 10);
//        //seriesHumi.appendData(new DataPoint(lastX++, RANDOM.nextDouble() * 10d), false, 10);
//
//    }
}