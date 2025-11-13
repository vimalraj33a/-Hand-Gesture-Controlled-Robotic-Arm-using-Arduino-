#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu;

bool dmpReady = false;  
uint8_t mpuIntStatus;   
uint8_t devStatus;      
uint16_t packetSize;    
uint8_t fifoBuffer[64]; 

Quaternion q;           
VectorFloat gravity;    
float ypr[3];           

volatile bool mpuInterrupt = false;

void dmpDataReady() {
  mpuInterrupt = true;
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  pinMode(2, INPUT); // INT pin

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  Serial.println("MPU6050 connected!");

  devStatus = mpu.dmpInitialize();

  // Optional: set offsets (replace with your own if calibrated)
  mpu.setXAccelOffset(-2800);
  mpu.setYAccelOffset(-1100);
  mpu.setZAccelOffset(1680);
  mpu.setXGyroOffset(85);
  mpu.setYGyroOffset(50);
  mpu.setZGyroOffset(-20);

  if (devStatus == 0) {
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.setDMPEnabled(true);

    attachInterrupt(digitalPinToInterrupt(2), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    dmpReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize();
    Serial.println("DMP ready!");
  } else {
    Serial.print("DMP Initialization failed (code ");
    Serial.print(devStatus);
    Serial.println(")");
  }
}

void loop() {
  if (!dmpReady) return;

  if (!mpuInterrupt && mpu.getFIFOCount() < packetSize) return;

  mpuInterrupt = false;
  mpuIntStatus = mpu.getIntStatus();

  if ((mpuIntStatus & 0x10) || mpu.getFIFOCount() == 1024) {
    mpu.resetFIFO();
    Serial.println("FIFO overflow!");
    return;
  }

  if (mpuIntStatus & 0x02) {
    while (mpu.getFIFOCount() < packetSize);
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    Serial.print("Yaw: ");
    Serial.print(ypr[0] * 180/M_PI);
    Serial.print(" | Pitch: ");
    Serial.print(ypr[1] * 180/M_PI);
    Serial.print(" | Roll: ");
    Serial.println(ypr[2] * 180/M_PI);
  }
}

