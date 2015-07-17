#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>

float MIN_DB = 70.0;

const int FFT_SIZE = 512;


Adafruit_MMA8451 mma = Adafruit_MMA8451();
unsigned long time,start_time,accum_time = 3000000;
int samp_rate = 200; //Hz
unsigned long samp_micros;
unsigned long micro = 0;
sensors_event_t event;
int LED = 12;
int BUTTON = 11;
int count;
float avg_x, avg_y, avg_z;
float avg_len;
bool button_on = false, has_run = false, accumulating = false, data_print = false;
float vel = 0;
char msg = ' ';// msg will be 'Y' when python requests data

IntervalTimer samplingTimer;
//float samples[FFT_SIZE];
//float magnitudes[FFT_SIZE];
int sampleCounter=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  if(!mma.begin()){
    Serial.println("couldnt start");
    while(1);
  }
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(LED,OUTPUT);
  samp_micros = 1000000/samp_rate;
  mma.setRange(MMA8451_RANGE_2_G);
}

void loop() {
  while(Serial.available()>0){
    msg = Serial.read();
    if(msg == 'Y'){
      data_print = true;
    }
  }
  time = micros();
  if (digitalRead(BUTTON) == LOW || accumulating) {
    if(!accumulating){
      accumulating = true;
      start_time = time;
      has_run=false;
    }
    vel = 0.f;
    if(!has_run){
      accumulate_accel();
    }
    if(time-start_time > accum_time){
      accumulating = false;
    }
  }
  else{
    average_accumulation();
    if(data_print){
      samplingBegin();
    }
  }
}

void accumulate_accel(){
  if(!button_on){
      button_on = true;
      avg_x = 0.f;
      avg_y = 0.f;
      avg_z = 0.f;
      count = 0;
    }
    mma.read();
    mma.getEvent(&event);
    avg_x += event.acceleration.x;
    avg_y += event.acceleration.y;
    avg_z += event.acceleration.z;
    count++;
    digitalWrite(LED,HIGH);
}

void average_accumulation(){
  if(button_on){
      avg_x /= count;
      avg_y /= count;
      avg_z /= count;
      avg_len = sqrt(pow(avg_x,2)+pow(avg_y,2)+pow(avg_z,2));
      button_on = false;
      has_run = true;
      Serial.println('N');
      digitalWrite(LED,LOW);
    }
    
}

float offset_relative(float ax, float ay, float az){
  float dot = ax*avg_x + ay*avg_y + az*avg_z;
  dot /= avg_len;
  return dot;
}

void samplingBegin() {
  // Reset sample buffer position and start callback at necessary rate.
  data_print = false;
  sampleCounter = 0;
  samplingTimer.begin(samplingCallback, 1000000/samp_rate);
}
void samplingCallback(){
  mma.read();
  mma.getEvent(&event);
  float d = offset_relative(event.acceleration.x-avg_x,event.acceleration.y-avg_y,event.acceleration.z-avg_z);
  vel += 9.80665f*d*samp_micros;
  Serial.println(vel);
  if(sampleCounter/64 % 2 == 0){
    digitalWrite(LED,HIGH);
  }
  else{
    digitalWrite(LED,LOW);
  }
//  samples[sampleCounter] = vel;
  sampleCounter++;
  if(sampleCounter > FFT_SIZE){
    samplingTimer.end();
    digitalWrite(LED,LOW);
  }
}
