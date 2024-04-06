import socket
from SX127x.LoRa import *
from SX127x.board_config import BOARD
from datetime import datetime
import time
import struct
import paho.mqtt.client as mqtt
import queue

BOARD.setup()
UDP_IP = "0.0.0.0"
UDP_PORT = 3333
MQTT_SERVER = "karolineserver.duckdns.org"
MQTT_TOPIC_SEND = "KA/fromPi"
MQTT_TOPIC_RECEIVE = "KA/fromApp"
MQTT_PORT = 1883
LORA_FREQ = 433.0
response = "Received UDP message!"
QUEUE = queue.Queue()

class LoRaDuplex(LoRa):
    def __init__(self, verbose=False):
        super(LoRaDuplex, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0] * 6)

    def on_rx_done(self):
        self.clear_irq_flags(RxDone=1)
        payload = bytes(self.read_payload(nocheck=True))
        if len(payload) >= 28:
            float_temp = float(payload[0:8])
            float_hum = float(payload[9:17])
            string_data = payload[18:].decode().strip('\x00')
            print(f"LoRa >>>> received message >>>> Temp: {float_temp}, Hum: {float_hum}, {string_data}")
            with open("log.txt", "a+") as file:
                file.write(datetime.today().strftime('%Y-%m-%d %H:%M:%S')+ "\n")
                file.write(f"Temp: {float_temp}, Hum: {float_hum}, String: {string_data} \n")
        else:
            print("LoRa >>>> payload too short or invalid")
            print(payload)
        self.set_mode(MODE.STDBY)

    def send(self, cmd_char):
        self.set_mode(MODE.STDBY)
        self.write_payload(list(cmd_char.encode()))
        self.set_mode(MODE.TX)
        print(f"LoRa >>>> sent message >>>> {cmd_char}")
        time.sleep(0.5)

    def receive_message(self):
        self.reset_ptr_rx()
        self.set_mode(MODE.RXCONT)
        print("LoRa >>>> listening for response ...")

class MQTTDuplex:
	def __init__(self, broker_address, port):
		self.client = mqtt.Client()
		self.broker_address = broker_address
		self.port = port
		self.rcv_mess = None
		self.client.on_connect = self.on_connect
		self.client.on_message = self.on_message

	def on_connect(self, client, userdata, flags, rc):
		print("MQTT >>>> Connected with result code " + str(rc))
		client.subscribe(MQTT_TOPIC_RECEIVE)

	def on_message(self, client, userdata, msg):
		print(f"MQTT >>>> received message >>>> {msg.payload.decode()} <<<< {MQTT_TOPIC_RECEIVE}")
		self.rcv_mess = msg.payload.decode()

	def start(self):
		self.client.connect(self.broker_address, self.port, 60)
		self.client.loop_start()

	def publish(self, message):
		#message = self.on_message(self.client, self.userdata, sel)
		self.client.publish(MQTT_TOPIC_SEND, message)

	def stop(self):
		self.client.loop_stop()
		self.client.disconnect()

class UDPServer:
    def __init__(self, ip, port):
        self.ip = ip
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind((self.ip, self.port))

    def message(self):
        data, addr = self.sock.recvfrom(1024)
        with open("log.txt", "a+") as file:
            file.write(f"ESP32 UDP address: {addr} \n")
            file.write(f"ESP32 UDP data:    {data.decode()} \n")
        print(f"UDP  >>>> received message >>>> {data.decode()} <<<< {addr}")
        self.sock.sendto(response.encode(), addr)


udp = UDPServer(UDP_IP, UDP_PORT)

mqtt_client = MQTTDuplex(MQTT_SERVER, MQTT_PORT)
mqtt_client.start()

lora_communication = LoRaDuplex()    
lora_communication.set_freq(LORA_FREQ)

try:
	with open("log.txt", "a+") as file:
		while True:  
			data = file.readlines()
			lastData = ''.join(data[-5:]).strip() if data else ""
			mqtt_client.publish(str(lastData))
			if mqtt_client.rcv_mess == None:
				lora_communication.send('0')
			else:
				lora_communication.send(mqtt_client.rcv_mess) 
			
			print()
			lora_communication.receive_message()
			time.sleep(2)
			udp.message()
            #time.sleep(0.5)
except KeyboardInterrupt:
    pass

lora_communication.set_mode(MODE.SLEEP)
BOARD.teardown()
