#include <Wire.h>
#include "vctrmnpltn.h"
#include "sensor_params.h"

const int gyro_add = 0x68;
const int magmeter_add = 0x1E;

const double pi = 3.14159;

double Bx,By,Bz;
double B[3];
double Bx_e,By_e,Bz_e;
double Wx,Wy,Wz;
double W[3];
double Wx_e,Wy_e,Wz_e;
double W_prev[3] = {0,0,0};
double B_prev[3] = {0,0,0};
double current[3];

int c = 0;                                  //in error checking fn


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  Wire.beginTransmission(gyro_add);
  Wire.write(0x6B);                             //reset registry
  Wire.write(0x00);
  Wire.endTransmission(true);

  Wire.beginTransmission(magmeter_add);
  Wire.write(0x02);
  Wire.write(0x00);
  Wire.endTransmission(true);

  find_error();
  delay(50);
}

void find_error()
{
  while (c < 200) {
    Wire.beginTransmission(gyro_add);
    Wire.write(0x43);                             //0x43,44,45,46,47,48 LSB and MSB values
    Wire.endTransmission(false);
    Wire.requestFrom(gyro_add, 6, true);
    Wx = Wire.read() << 8 | Wire.read();
    Wy = Wire.read() << 8 | Wire.read();
    Wz = Wire.read() << 8 | Wire.read();
    // Sum all readings
    Wx_e = Wx_e + (Wx / 131.0);
    Wy_e = Wy_e + (Wy / 131.0);                  //LSB per unit is 131.0 for +/-250deg/sec range
    Wz_e = Wz_e + (Wz / 131.0);
    c++;
  }
  //Divide the sum by 200 to get the error value
  Wx_e = Wx_e / 200;
  Wy_e = Wy_e / 200;
  Wz_e = Wz_e / 200;
  // Print the error values on the Serial Monitor
  Serial.print("GyroErrorX: ");
  Serial.println(Wx_e);
  Serial.print("GyroErrorY: ");
  Serial.println(Wy_e);
  Serial.print("GyroErrorZ: ");
  Serial.println(Wz_e);
}

void navigation(double B[3],double W[3])
{
  if(abs(B_prev[0]) + abs(B_prev[1]) + abs(B_prev[2]) + abs(W_prev[0]) + abs(W_prev[1]) + abs(W_prev[2]) == 0)
  {
    W_prev[0] = W[0];
    W_prev[1] = W[1];
    W_prev[2] = W[2];
    B_prev[0] = B[0];
    B_prev[1] = B[1];
    B_prev[2] = B[2];
  }
  else
  {
    for(int i = 0;i < 3;i++)
    {
      W[i] = W_prev[i]*(1.0 - Sw) + Sw*W[i];
      B[i] = B_prev[i]*(1.0 - Sb) + Sb*B[i];
    } 
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Wire.beginTransmission(gyro_add);
  Wire.write(0x43);
  Wire.endTransmission(false);
  Wire.requestFrom(gyro_add,6,true);
  Wx = (Wire.read()<<8)|(Wire.read());
  Wy = (Wire.read()<<8)|(Wire.read());
  Wz = (Wire.read()<<8)|(Wire.read());
  Wx = Wx/131.0 - Wx_e;
  Wy = Wy/131.0 - Wy_e;
  Wz = Wz/131.0 - Wz_e;
  Wx = (Wx*pi)/180.00;                                    //into radians
  Wy = (Wy*pi)/180.00;
  Wz = (Wz*pi)/180.00;
  W[0] = Wx;
  W[1] = Wy;
  W[2] = Wz;
  
  //-----------------------------------------------------//

  Wire.beginTransmission(magmeter_add);
  Wire.write(0x03);
  Wire.endTransmission(false);
  Wire.requestFrom(magmeter_add,6,true);                    //0x03,04,05,06,07,08 LSB and MSB values
  Bx = ((Wire.read()<<8)|(Wire.read()))/1090.0;
  Bz = ((Wire.read()<<8)|(Wire.read()))/1090.0;
  By = ((Wire.read()<<8)|(Wire.read()))/1090.0;             //Lsb per unit = 1090 for +/-1.3Ga
  Bx = Bx*1E-4;
  By = By*1E-4;
  Bz = Bz*1E-4;                                             //into Teslas
  B[0] = Bx;
  B[1] = By;
  B[2] = Bz;
  navigation(B,W);                                          //filter
  
  //----------------------------------------------------//

  vctrmnpltn vctr1;
  vctr1.cross(W,B);
  double WxB[] = {vctr1.resultant[0],vctr1.resultant[1],vctr1.resultant[2]};
  double k = 67200.0;
  for(int i = 0;i < 3;i++)
  {
    current[i] = (k*WxB[i])/(n*A);                                                  //Bdot control
  }

  double sum_abs_current = abs(current[0])+abs(current[1])+abs(current[2]);
  double norm_current = sqrt(pow(current[0],2) + pow(current[1],2) + pow(current[2],2));
  if (sum_abs_current > saturation_current)
  {
    for(int i = 0;i < 3;i++)
    {
      current[i] = (current[i]*saturation_current)/norm_current;                    //Saturation adjustment
    }
  }

  //------------------------------------------------------//

}
