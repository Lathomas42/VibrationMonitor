# Vibration Monitor
## Arduino & Python scripts to control a vibration monitor using the MMA8451 accelerometer and the Raspberry pi
### Setup
Put python file onto raspberry pi/home directory and make sure rc local is configured to run the python file on start-up. Connect the accelerometer/Teensy set-up, rig the accelerometer up to whatever is being measured, and press the button
on the vibration monitor to configure and start recording. 

Recordings are made every few seconds (modified on the arduino side) and every hour the average, standard deviation and outliers are recorded in two csv files.

### TODO
- Confirm that it works for low Hz vibrations by setting it up on a table that suppresses vibrations next to something creating low vibrations (arduino motor? something else?) and change the sample recording time down to 15 minutes or so (on python side)
and ensure that in the 15 minutes the motor is on, the bin for the motor's vibration is higher than the 15 minutes the motor is off.
- Optimize the sampling time (on ard) and the sample averaging time (on python)