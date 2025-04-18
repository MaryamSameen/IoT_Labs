### Task 1:

We connected ESP32 to ThingsSpeak through things speak API(write) and sent Sensor data collected by ESP32 to Cloud ThingsSpeak dashboard and visuallize the data updation patterns.
Additionally added the channel Id to android ThingsView app to visualize the channel.
CheckInterval = 20 is because we are using cloud for Free that doesn't allow us to send data for less than 15 sec.

### Task 2:

Free Thingspeak can't give Text Alert but we can deal with Numeric Value Alerts.
Created a React, Alert for High Temperature in ThingsSpeak Cloud and added MATLAB Analysis code.
We are doing calculations on Thingspeak cloud and ESP32 will do its work as calculations will be done on cloud.

In thing2.py give your own read,write API, edit your channel ID in the url.