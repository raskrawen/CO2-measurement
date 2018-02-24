# CO2-measurement
Measure CO2 levels with Wemos D1 and CO2-sensor MH-Z14A.

Fresh-air measurement at 400 ppm CO2

Possible features:
* calibration value stored in EERPOM memory of the Wemos D1.
* search for default wifi connections
* 6 LED indicator
* calibrate by placing in fresh air, connect D14 to GND.
* autocalibrate if measurement gets below previous calibration.
* measure CO2 level continously, but send to Thingspeak.com every 10. minute.
* measurements filteret by sorting 50 measurements, select middle value.
