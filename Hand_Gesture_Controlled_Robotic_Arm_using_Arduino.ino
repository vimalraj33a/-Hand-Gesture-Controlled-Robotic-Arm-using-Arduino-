#include <Wire.h>
#include <Servo.h>
#include <math.h>

Servo servo_1;      
Servo servo_2;
Servo servo_3;
Servo servo_4;

const int MPU_addr = 0x68;
int16_t aX, aY, aZ;
double xAng, yAng, zAng;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // Wake up MPU6050
  Wire.endTransmission(true);

  servo_1.attach(2); // Forward/Reverse
  servo_2.attach(3); // Up/Down
  servo_3.attach(4); // Gripper
  servo_4.attach(5); // Left/Right
  pinMode(9,INPUT);
}

void loop() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // Starting register for accelerometer
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);

  aX = Wire.read() << 8 | Wire.read();
  aY = Wire.read() << 8 | Wire.read();
  aZ = Wire.read() << 8 | Wire.read();

  // Calculate angles in degrees
  xAng = atan2(-aY, -aZ) * 180 / PI + 180;
  yAng = atan2(-aX, -aZ) * 180 / PI + 180;
  zAng = atan2(-aY, -aX) * 180 / PI + 180;

  Serial.print("x: "); Serial.print(xAng);
  Serial.print(" | y: "); Serial.print(yAng);
  Serial.print(" | z: "); Serial.println(zAng);

  // Gripper using flex sensor
    int tiltState = digitalRead(9);

  if (tiltState == HIGH) {
    Serial.println("Tilt detected! Servo to 90°");
    servo_3.write(90);       // Move servo to 90 degrees
  } else {
    Serial.println("Flat. Servo to 0°");
    servo_3.write(0);        // Move servo to 0 degrees
  }

  delay(200);

  // Forward/Reverse (servo_1)
  if (xAng >= 0 && xAng <= 180) {
    servo_1.write(map(xAng, 0, 60, 0, 90));
  } else if (xAng >= 300 && xAng <= 360) {
    servo_1.write(map(xAng, 360, 250, 0, 180));
  }

  // Up/Down (servo_2)
  if (yAng >= 0 && yAng <= 180) {
    servo_2.write(map(yAng, 0, 60, 0, 90));
  } else if (yAng >= 300 && yAng <= 360) {
    servo_2.write(map(yAng, 360, 250, 0, 180));
  }

  // Left/Right (servo_4)
  if (zAng >= 0 && zAng <= 180) {
    servo_4.write(map(zAng, 0, 60, 90, 180));
  } else if (zAng >= 300 && zAng <= 360) {
    servo_4.write(map(zAng, 360, 300, 90, 0));
  }

  delay(1000);
}
