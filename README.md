# SmartPlantPot

This code is made for a smart plant pot. In detail it works with a capacitive moisture Sensor, Wemos and a small pump. Thus it can be used for any other self watering project, which uses similar components.

It provides the possibility to either build an access point or connect to an existing WiFi-Network. The webserver can be accessed via the main route http://smartpot/ or its IP address (AP Mode: 192.168.4.1).

### Libraries

https://github.com/me-no-dev/ESPAsyncTCP  
https://github.com/me-no-dev/ESPAsyncWebServer  
https://github.com/Links2004/arduinoWebSockets  

Download as zip and use PIO console to install
```
pio lib -g install "path to zip file"
```
