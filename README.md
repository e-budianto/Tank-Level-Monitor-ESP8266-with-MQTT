# Tank Level Monitor with Remote Visual Display of Water Level

For the sending unit, this project uses an HC-SR04 Ultrasound Distance Sensor, DHT22 Temperature and Humidity Sensor, Photon Electron/Photon & Particle Cloud, and Ubidots (for data logging).

For the receiving unit, this project uses a super bright red LED, automotive gauge stepper motor (Adafruit #2424), Adafruit TB6612 driver break out board (Adafruit #2448), and Particle Photon/Electron & Particle Cloud.

I've made use of 3D printed enclosures for both the sender and receiver, with the receiver being a miniature replica of the tank being monitored.

The sending unit polls the tank level, temperature and humidity every 5 minutes. All data is sent to Ubidots for logging and trend graphing. The tank level is also "published" to the Particle Cloud.

The receiving unit "subscribes" to the Particle Cloud and updates the gauge needle as appropriate. When the tank levels goes below a defined threshold the unit slowly glows/breathes on and off with the red LED. When the tank dips to critical levels the red LED strobes.
