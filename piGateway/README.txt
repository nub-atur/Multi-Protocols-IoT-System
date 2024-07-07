USAGE**********************
-> copy this file to your Pi
-> pip3 install spidev
-> pip3 install RPi.GPIO
-> pip3 install pyLoRa
-> pip3 install paho-mqtt
-> python main.py
********************************









code****************************

from time import sleep
from SX127x.LoRa import *
from SX127x.board_config import BOARD
import random
import time
from paho.mqtt import client as mqtt_client
import socket
import threading
import queue
import struct
from datetime import datetime

BOARD.setup()

# Define ACK flags
LoRa_flag_ACK = False
UDP_flag_ACK = False

#Devices responing
dOK_count = 0;
dKO_count = 0;

class LoRaDuplex(LoRa):
    def __init__(self, classFlag, queueSend, queueReceive, verbose=False):
        super(LoRaDuplex, self).__init__(verbose)
        self.set_mode(MODE.SLEEP)
        self.set_dio_mapping([0] * 6)
        self.classFlag = classFlag
        self.queueSend = queueSend
        self.queueReceive = queueReceive

    def on_rx_done(self):
        self.clear_irq_flags(RxDone=1)
        payload = bytes(self.read_payload(nocheck=True))
        #print(payload)
        self.queueSend.put(payload)
        self.queueSend.put(self.get_pkt_snr_value())
        self.queueSend.put(self.get_pkt_rssi_value())
        if len(payload) >= 2:
            decode_data = payload.decode('utf-8')
            components = decode_data.split()
            if "OK" in decode_data: 
                self.set_ACK_true()
                self.inc_OK_val()
            elif "KO" in decode_data:
                self.set_ACK_true()
                self.inc_KO_val()
            elif components[0] != "none":
                float_temp = float(components[0])
                float_hum = float(components[1])
                obstacle = components[2]
                t_count = int(components[3])
                with open("log.txt", "a+") as file:
                    file.write(datetime.today().strftime('%Y-%m-%d %H:%M:%S') + "\n")
                    file.write(f"Temp: {float_temp}, Hum: {float_hum}, Obstacle: {obstacle}, mesh_time_count: {t_count}\n")
        else:
            print("LoRa >>>> payload too short or invalid")
        self.set_mode(MODE.STDBY)

    def send(self, cmd_char):
        self.set_mode(MODE.STDBY)
        self.write_payload(list(cmd_char.encode()))
        self.set_mode(MODE.TX)
        print(f"LoRa >>>> sent message >>>> {str(cmd_char)}")
        sleep(0.5)

    def run(self):
        while True:
            self.reset_ptr_rx()
            self.set_mode(MODE.RXCONT)
            sleep(.5)
            flag_value = self.classFlag.get_mesh_flag()
            if flag_value:
                msg = self.queueReceive.get()
                self.send(msg)
                self.classFlag.set_mesh_flag(False)
    
    def set_ACK_true(self):
        global LoRa_flag_ACK
        LoRa_flag_ACK = True
        print("LoRa >>>> ack")
        
    def inc_KO_val(self):
        global dKO_count
        dKO_count += 1
        
    def inc_OK_val(self):
        global dOK_count
        dOK_count += 1 

class UDPServer:
    def __init__(self, classFlag, queue, local_port=8888, buffer_size=32):
        self.local_port = local_port
        self.buffer_size = buffer_size
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.sock.bind(('', self.local_port))
        print("UDP >>>> server : {}:{}".format(self.get_ip_address(), self.local_port))
        self.classFlag = classFlag
        self.queue = queue

    def run(self):
        while True:
            data, addr = self.sock.recvfrom(self.buffer_size)
            print("UDP >>>> received: {} <<<< {}\n".format(data, addr))
            flag_value = self.classFlag.get_udp_flag()
            if data == b'OK':
                self.set_ACK_true()
                self.inc_OK_val()
            if data == b'KO':
                self.set_ACK_true()
                self.inc_KO_val()
            if flag_value:
                cmd = self.queue.get()
                if cmd is None:
                    break
                else:
                    self.sock.sendto(cmd, addr)
                    self.classFlag.set_udp_flag(False)
                    print("UDP >>>> {} >>>> {}".format(addr, cmd.decode()))
            else:
                time.sleep(2)

    def get_ip_address(self):
        ip_address = ''
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip_address = s.getsockname()[0]
        s.close()
        return ip_address

    def set_ACK_true(self):
        global UDP_flag_ACK
        UDP_flag_ACK = True
        print("UDP >>>> ack")
        
    def inc_OK_val(self):
        global dOK_count
        dOK_count += 1
        
    def inc_KO_val(self):
        global dKO_count 
        dKO_count += 1
        

class MQTTGateway:
    def __init__(self, queueUDP, queueMesh, queueToMesh, broker, port, topicS, topicR):
        self.broker = broker
        self.port = port
        self.topicS = topicS
        self.topicR = topicR
        self.client_id = f'publish-{random.randint(0, 1000)}'
        self.client = None
        self.meshFlag = False
        self.udpFlag = False
        self.queueUDP = queueUDP
        self.queueMesh = queueMesh
        self.queueToMesh = queueToMesh
        
    def set_mesh_flag(self, value):
        self.meshFlag = value
        
    def get_mesh_flag(self):
        return self.meshFlag
        
    def set_udp_flag(self, value):
        self.udpFlag = value
        
    def get_udp_flag(self):
        return self.udpFlag

    def connect_mqtt(self):
        def on_connect(client, userdata, flags, rc):
            if rc == 0:
                print("MQTT >>>> Connected to MQTT Broker!")
            else:
                print(f"MQTT >>>> Failed to connect, return code {rc}")

        def on_message(client, userdata, msg):
            print("MQTT <<<< " + msg.topic + " <<<< " + str(msg.payload.decode()))
            self.meshFlag = True
            self.udpFlag = True
            self.queueUDP.put(msg.payload)
            self.queueToMesh.put(str(msg.payload.decode()))
            time.sleep(0.5)
            
        self.client = mqtt_client.Client(self.client_id)
        self.client.on_connect = on_connect
        self.client.on_message = on_message
        self.client.connect(self.broker, self.port)

    def publish(self):
        while True:
            rc = self.check_ACK_UDP()
            rc2 = self.check_ACK_LoRa()  
            sleep(.5)
            if (rc2 == True) and (rc == True):
                msg = str(self.get_OK_count()).encode('utf-8')
                msg += "OK".encode('utf-8')
                msg += str(self.get_KO_count()).encode('utf-8')
                msg += "KO".encode('utf-8')
                self.set_ACK_LoRa_false()
                self.set_ACK_UDP_false()
                self.reset_ACK()
                pass
            elif (rc2 == True):
                msg = str(self.get_OK_count()).encode('utf-8')
                msg += "OK".encode('utf-8')
                msg += str(self.get_KO_count()).encode('utf-8')
                msg += "KO".encode('utf-8')
                self.set_ACK_LoRa_false()
                self.reset_ACK()
                pass
            elif (rc == True):
                msg = str(self.get_OK_count()).encode('utf-8')
                msg += "OK".encode('utf-8')
                msg += str(self.get_KO_count()).encode('utf-8')
                msg += "KO".encode('utf-8')
                self.set_ACK_UDP_false()
                self.reset_ACK()
                pass
            else:
                msg = self.queueMesh.get()
                snr = self.queueMesh.get()
                rssi = self.queueMesh.get()
            
                msg += ' '.encode('utf-8') 
                msg += str(snr).encode('utf-8')
                msg += ' '.encode('utf-8')
                msg += str(rssi).encode('utf-8')
                
            result = self.client.publish(self.topicS, msg)
            status = result[0]
            if status == 0:
                print(msg)
                pass
            else:
                print(f"MQTT >>>> Failed to send message to topic {self.topicS}")

    def run(self):
        self.connect_mqtt()
        self.client.loop_start()
        self.client.subscribe(self.topicR)
        self.publish()
        
    def check_ACK_UDP(self):
        global UDP_flag_ACK
        print(f"udpACK = {UDP_flag_ACK}")
        return UDP_flag_ACK
        
    def set_ACK_UDP_false(self):
        global UDP_flag_ACK
        UDP_flag_ACK = False
        
    def check_ACK_LoRa(self):
        global LoRa_flag_ACK
        print(f"loraACK = {LoRa_flag_ACK}")
        return LoRa_flag_ACK
        
    def set_ACK_LoRa_false(self):
        global LoRa_flag_ACK
        LoRa_flag_ACK = False
        
    def get_OK_count(self):
        global dOK_count
        return dOK_count
        
    def get_KO_count(self):
        global dKO_count
        return dKO_count
        
    def reset_ACK(self):
        global dOK_count, dKO_count
        dOK_count = 0
        dKO_count = 0
        

if __name__ == '__main__':
    CMD_queue = queue.Queue()
    fromMesh_queue = queue.Queue()
    toMesh_queue = queue.Queue()

    # Configuration for MQTT
    mqtt_broker = 'karolineserver.duckdns.org'
    mqtt_port = 1883
    mqtt_topic = "KA/fromPi"
    mqtt_topicRE = "KA/fromApp"
    
    # Init threads
    mqtt_gateway = MQTTGateway(CMD_queue, fromMesh_queue, toMesh_queue, mqtt_broker, mqtt_port, mqtt_topic, mqtt_topicRE)
    udp_server = UDPServer(mqtt_gateway, CMD_queue)

    udp_thread = threading.Thread(target=udp_server.run)
    mqtt_thread = threading.Thread(target=mqtt_gateway.run)

    lora = LoRaDuplex(mqtt_gateway, fromMesh_queue, toMesh_queue, verbose=False)
    lora.set_mode(MODE.STDBY)
    lora.set_pa_config(pa_select=1)
    lora.set_freq(433.0)
    assert(lora.get_agc_auto_on() == 1)

    lora_thread = threading.Thread(target=lora.run)
    lora_thread.start()
    udp_thread.start()
    mqtt_thread.start()

    lora_thread.join()
    fromMesh_queue.put(None)
    udp_thread.join()
    CMD_queue.put(None)
    mqtt_thread.join()
    toMesh_queue.put(None)

*********************************
