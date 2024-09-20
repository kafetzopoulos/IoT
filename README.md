
Prologue:

For the implementation of our project, we used the ESP32 microcontroller along with the Node-RED web server for communication via Wi-Fi. Additionally, we utilized the following libraries: ESP32 DHT11, NeoPatterns, and PubSubClient for the ESP32. For Node-RED, we used the libraries: node-red 1.0.1, node-red-contrib-mqtt-broker 0.2.4, and node-red-dashboard 2.19.1.

Implementation:

We initially started by simulating our circuit using Tinkercad. To achieve this, we used the Arduino microcontroller and implemented our circuits there. Our goal was to create a starting point that would allow us to later adapt a similar circuit for the ESP32.

![image](https://github.com/user-attachments/assets/a29727b6-8173-4e5f-bb41-a952c692ca62)

Our final goal was to assemble a small car that could be controlled by pressing buttons through Node-RED and could also return values from various sensors and display them in Node-RED.![rc_car_scematic_image](https://github.com/user-attachments/assets/c6d53005-9558-4f34-a277-29841a37aff6)

The code we implemented for the Arduino in Tinkercad is not identical to the one for the ESP32, as it uses different libraries, which required many processes to be rewritten. One such process was for the servo, which needed a different library and included several changes in the code. The same applied to the process of retrieving temperature and humidity data. Below is the final design we arrived at (designed in Fritzing).

![image](https://github.com/user-attachments/assets/29eb9b21-de26-4d43-abe9-4f3c65a20ee0)

For wireless communication with the vehicle, we use an MQTT model, and our interface in Node-RED provides controls for movement:

- Forward
- Backward
- Right
- Left
- Stop

Additionally, the interface includes scrolling panels that display real-time data logging for movement, brightness, humidity, temperature, and distance, indicating how much free space the vehicle has.

The hardware components we used for the vehicle are:

- DOIT ESP32 DevKit v1
- Ultrasonic Sensor
- H-Bridge
- 9V Battery
- Light Sensitive Sensor
- Temperature & Humidity Sensor
- 2x PMDC Motors

Below is the layout of Node-RED, where MQTT is utilized. The "example/esp32" boxes represent the connection points between Node-RED and the ESP32. At the top, we see the connection to the control buttons, while at the bottom, we see how the data is added to the scrolling tables.

![image](https://github.com/user-attachments/assets/dc53fd10-91ea-4c1f-9a49-2b1c22428ad9)
