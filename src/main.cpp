//Librerias
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <Servo.h>

// La dirección del MPU6050 puede ser 0x68 o 0x69, dependiendo 
// del estado de AD0. Si no se especifica, 0x68 estará implicito
MPU6050 sensor;

//Instancia de los servomotores de rotacion
Servo servomotor1;
Servo servomotor2;

// Valores RAW (sin procesar) del acelerometro  en los ejes x,y,z
int ax, ay, az;

//Valores de rotacion de los servos en modo automatico y la señal recibida por el ADC del jostick
int Sx,Sy,Vx;

//Variable de conversion de la entrada ADC a grados entre 0 y 180
float valor;

//Variable que definira si se trabajara de modo automatico o manual
volatile bool modo;

//Variable de cambio de modo y lectura de los botones del sistema manual
int cambio,Px,Py;

int ADC_GETDATA1(){
  ADMUX =0B01000001;
  ADCSRA = 0B10000111;	//PRE-SCALER  128 - DISABLED ANALOG CONVERTER & CONVERSION NOT STARTED
  ADCSRB = 0B00000000; 	//FREE RUNNING MODE
  DIDR0 = 0b00000010;	//ENABLE ANALOG MODE ADC1
  return ADC;
}

void setup() {
  Serial.begin(57600);    //Iniciando puerto serial
  Wire.begin();           //Iniciando I2C  
  sensor.initialize();    //Iniciando el sensor
  //Definicion de las entradas
  pinMode(A1,INPUT);
  pinMode(7,INPUT);
  pinMode(4,INPUT);
  pinMode(5,INPUT);

  //Comunicacion serial del sensor mpu6050
  if (sensor.testConnection()) Serial.println("Sensor iniciado correctamente");
  else Serial.println("Error al iniciar el sensor");
  
  //Pines referencia de salida de los servomotores
  servomotor1.attach(9);
  servomotor2.attach(10);

}

void loop() {
  //lectura de los botones
  Px=digitalRead(4);
  Py=digitalRead(5);
  cambio=digitalRead(7);

  //Conversion de entrada ADC a grados de entre 0 y 180
  valor=Vx*0.176;

  //Cambio de modo de configuracion
  if(cambio==1){
    modo=!modo;
    delay(100);
  }
  if (cambio==0){
    modo=modo;
  }

  //Condicionamiento de funcionamiento manual por medio del jostick, recordar que el movimiento de los servomotores será unicamente al presionar los botones de Y y X
  if(modo){
    Vx=ADC_GETDATA1();
      ADCSRA |= 1<<6; // ENABLED ANALOG CONVERTER & START CONVERSION
      while(ADCSRA & (1<< ADSC)); // ADSC(bit 6) DISABLED? 
      delay(100);   
    //Comportamiento del servo y salida serial del valor ejecutado en X
    if(Px==1){
      servomotor1.write(int(round(valor)));
      Serial.print("X: ");
    }
    //Comportamiento del servo y salida serial del valor ejecutado en Y
    if(Py==1){
      servomotor2.write(int(round(valor)));
      Serial.print("Y: ");
    }
    //En caso de no presionar ninguno de los botones no ocurre absolutamente nada
    Serial.println(valor);
    if(Px==0 || Py==0){
      ;
    }
  }
  //Configuración en modo automatico por medio del sensor
  if(!modo){
    servomotor1.write(Sx);
    delay(100);
    servomotor2.write(Sy);
    delay(100);
  }
  Serial.println(cambio);
  // Leer las aceleraciones 
  sensor.getAcceleration(&ax, &ay, &az);
  //Calcular los angulos de inclinacion:
  float accel_ang_x=atan(ax/sqrt(pow(ay,2) + pow(az,2)))*(180.0/3.14);
  float accel_ang_y=atan(ay/sqrt(pow(ax,2) + pow(az,2)))*(180.0/3.14);
  //Mostrar los angulos separadas por un [tab]
  Serial.print("Inclinacion en X: ");
  Serial.print(round(accel_ang_x)); 
  Serial.print(", Inclinacion en Y:");
  Serial.println(round(accel_ang_y));
  delay(500);
  
  //Definicion de angulos para cada servomotor
  Sx=round(accel_ang_x)+90;
  Sy=round(accel_ang_y)+90;
  
}