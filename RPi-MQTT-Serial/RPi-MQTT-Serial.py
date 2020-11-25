
import serial
import paho.mqtt.client as mqttClient
import threading

event = threading.Event()

# init serial
ser = serial.Serial(
    "/dev/ttyS0",
    baudrate = 4800,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=2)

# callback 
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to broker")
        global Connected                    #Use global variable
        Connected = True                    #Signal connection 
    else:
        print("Connection failed")

# mqtt message receive
def on_message(client, userdata, message):
    messageled = message.payload
    #event.wait(0.05)                        #defided cycle time
    print (messageled)
    if (messageled == b'R'):
        ser.write('R'.encode('ascii'))
    elif (messageled == b'Y'):
        ser.write('Y'.encode('ascii'))
    elif (messageled == b'O'):
        ser.write('O'.encode('ascii'))
    else:
        print ("Invalid message")


Connected = False                                  #global variable for the state of the connection
 
broker_address= "192.168.0.13"
port = 1883
user = "mrfmach"
password = "mosquitto"

#init broker
client = mqttClient.Client("Python")               #create new instance
client.username_pw_set(user, password=password)    #set username and password
client.on_connect= on_connect                      #attach function to callback
client.on_message= on_message

client.connect(broker_address, port=port)          #connect to broker
client.loop_start()                                #start the loop

while Connected != True:                           #Wait for connection
    event.wait(0.2)

client.subscribe("arduino/led")
client.publish("esp32/led", "o")                   #publish to start the cycle

try:
    while True:
        readArdu = ser.readline()
        print(readArdu)
        
        readStatus = readArdu[0:1]
        readDistance = readArdu[2:10]
        
        event.wait(0.15)                           #cycle time
        
        client.publish("arduino/distance", readDistance)
        
        if (readStatus == b'o'):
            client.publish("esp32/led", "o")
        elif (readStatus == b'r'):
            client.publish("esp32/led", "r")
        elif (readStatus == b'y'):
            client.publish("esp32/led", "y")
        elif (readStatus == b'g'):
            client.publish("esp32/led", "g")

except KeyboardInterrupt:
    client.disconnect()
    client.loop_stop()