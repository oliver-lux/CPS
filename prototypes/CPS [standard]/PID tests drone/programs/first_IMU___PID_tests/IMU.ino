#define LSM9DS1_SCK A5
#define LSM9DS1_MISO 12
#define LSM9DS1_MOSI A4
#define LSM9DS1_XGCS 6
#define LSM9DS1_MCS 5

float accPitch = 0;
float accRoll = 0;
float accmagYaw = 0;

/*float kalPitch = 0;
float kalRoll = 0;
float kalYaw = 0;

KalmanFilter kalmanX(0.001, 0.003, 0.03);     //(0.001, 0.003, 0.03)
KalmanFilter kalmanY(0.001, 0.003, 0.03);
KalmanFilter kalmanZ(0.001, 0.003, 0.03);*/

float compPitch = 0;
float compRoll = 0;
float compYaw = 0;

float mx_min, my_min, mz_min;
float mx_max, my_max, mz_max;

float offaccX;
float offaccY;
float offaccZ;

float offmagX;
float offmagY;
float offmagZ;

void setupIMU()
{
  //while (!Serial) {delay(1);}
  //if (!lsm.begin()){while (1);}
  
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
}

void runIMU()
{
  lsm.read();
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp);

  double kt = (double)micros();
  double dt = (double)(micros() - kt) / 1000000;
  
  
  mx_min = -0.05; //swapped
  my_min = -0.13;
  mz_min = -0.28;
  
  mx_max = 0.41;
  my_max = 0.27;
  mz_max = 0.12;
  
  float mx_offset = (mx_min + mx_max)/2;
  float my_offset = (my_min + my_max)/2;
  float mz_offset = (mz_min + mz_max)/2;

  float avg_delta_x = (mx_max - mx_min)/2;
  float avg_delta_y = (my_max - my_min)/2;
  float avg_delta_z = (mz_max - mz_min)/2;
  
  float avg_delta = (avg_delta_x + avg_delta_y + avg_delta_z)/3;

  float scale_x = avg_delta / avg_delta_x;                  // this is different
  float scale_y = avg_delta / avg_delta_y;
  float scale_z = avg_delta / avg_delta_z;
  
  float calmagX = (m.magnetic.x - mx_offset) * scale_x;
  float calmagY = (m.magnetic.y - my_offset) * scale_y;
  float calmagZ = (m.magnetic.z - mz_offset) * scale_z;//*/


  int OFFSET = ch[5];
  if (OFFSET>1500){
    offaccX = a.acceleration.x;
    offaccY = a.acceleration.y;
    offaccZ = a.acceleration.z-9.81;

    offmagX = calmagX-0.19;
    offmagY = calmagY-0.02;
    offmagZ = calmagZ+0.38;
  }
  
  // Calculate Pitch & Roll from accelerometer (deg)
  //accPitch = (atan2(a.acceleration.x, a.acceleration.z) * 180.0) / M_PI;
  accPitch = (-(atan2(a.acceleration.x-offaccX, sqrt((a.acceleration.y-offaccY) * (a.acceleration.y-offaccY) + (a.acceleration.z-offaccZ) * (a.acceleration.z-offaccZ))) * 180.0) / M_PI);
  accRoll  = ((atan2(a.acceleration.y-offaccY, a.acceleration.z-offaccZ) * 180.0) / M_PI)-3;

  double Bfy = (calmagZ-offmagZ) * sin(accRoll*DEG_TO_RAD) - (calmagY-offmagY) * cos(accRoll*DEG_TO_RAD);
  double Bfx = (calmagX-offmagX) * cos(accPitch*DEG_TO_RAD) + (calmagY-offmagY) * sin(accPitch*DEG_TO_RAD) * sin(accRoll*DEG_TO_RAD) + (calmagZ-offmagZ) * sin(accPitch*DEG_TO_RAD) * cos(accRoll*DEG_TO_RAD);
  accmagYaw = atan2(-Bfy, Bfx) * RAD_TO_DEG;

  //Complementary filter
  compPitch = 0.8 * (compPitch + g.gyro.y * dt) + 0.2 * accPitch; //had 50/50 but while quick accel its bad
  compRoll = 0.8 * (compRoll + g.gyro.x * dt) + 0.2 * accRoll;
  compYaw = 0.3 * (compYaw + g.gyro.z * dt) + 0.7 * accmagYaw;

 //ch[5]=2000;
  
 /*Serial.print(calmagX);
 Serial.print("\t");
 Serial.print(calmagY);
 Serial.print("\t");
 Serial.print(calmagZ);
 Serial.print("\t");
 Serial.print(compYaw);
 Serial.print("\t");
 Serial.print(compPitch);
 Serial.print("\t");
 Serial.print(compRoll);
 Serial.print("\t");
 Serial.println();//*/

  /*accX = a.acceleration.x - map(compRoll,-90,90,-1,1)*9.81//maybe sins and tangents
  accY = a.acceleration.y - map(compPitch,-90,90,-1,1)*9.81
  accZ = a.acceleration.z // - Pitch + Roll jakoś*/
  
  /*// Kalman filter
  kalPitch = kalmanY.update(accPitch, g.gyro.y);
  kalRoll = kalmanX.update(accRoll, g.gyro.x);
  kalYaw = kalmanZ.update(accmagYaw, g.gyro.z);

  if (kalPitch < -180 || kalPitch > 180)
    kalPitch = accPitch;
  if (kalRoll < -180 || kalRoll > 180)
    kalRoll = accRoll;
  if (kalYaw < -180 || kalYaw > 180)
    kalYaw = accmagYaw;*/


  
#if 0
  Serial.print("Pitch");
  Serial.print("\t");
  Serial.print(accPitch);
  Serial.print("\t");
  Serial.print(compPitch);
  Serial.print("\t");
  Serial.print("Roll");
  Serial.print("\t");
  Serial.print(accRoll);
  Serial.print("\t");
  Serial.print(compRoll);
  Serial.print("\t");
  Serial.print("Yaw");
  Serial.print("\t");
  Serial.print(accmagYaw);
  Serial.print("\t");
  Serial.print(compYaw);

  Serial.println();

  delay(00);
#endif

  runPID(compRoll, compPitch, compYaw, a.acceleration.x, a.acceleration.y, a.acceleration.z );
}
