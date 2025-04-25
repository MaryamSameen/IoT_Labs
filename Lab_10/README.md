### Task 1:

downloaded the credentials of your MQTT protocol from the thingsSpeak in txt file.

then send data through MQTT protocol to the thingsSpeak Dashboard and Visualize it.
Note the changes.


### Task 2 (Multi-core)

We are sending data to thingsSpeak Through MQTT protocol through Publishing 
using Parallel computing.

import _thread (for using Multi-Threading) i.e OpenMp

from umqtt.simple import MQTTClient  (Library import)

MQTT_TOPIC -> Where we want to publish our data.

Lock Initiallize -> create a lock (token) , processor will -> Lock first acquire , then do work, then release the lock ,mainly for enforcement synchronization.

Then Create a Client.

Then create payload(what you want to publish).

Client.public -> builtin function which has one parameter of Topic and second and second payload.
 main thread run on core 0.
 other generated thread on core 1.
