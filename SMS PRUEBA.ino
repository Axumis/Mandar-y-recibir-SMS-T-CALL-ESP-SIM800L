 //  Configuracion de la libreria TinyGSM para trabajar con el Modem del SIM800L
  #define TINY_GSM_MODEM_SIM800      // El Modem es el SIM800
  #define TINY_GSM_RX_BUFFER   1024  // Establece el bufer de RX en 1kB

#include <TinyGsmClient.h>
#include <Wire.h>

 // SIM card PIN (leave empty, if not defined)
  const char simPIN[]   = "";

//  Se define el numero de celular a enviar el SMS
  String SMS_TARGET = "+549xxxxxxxxxx";

  //Se define unas variables
  bool bandera=false;
  String respuesta;

//  Definicion de los pines del TTGO T-Call para el SIM800L
  #define MODEM_RST            5
  #define MODEM_PWKEY          4
  #define MODEM_POWER_ON       23
  #define MODEM_TX             27
  #define MODEM_RX             26
  #define I2C_SDA              21
  #define I2C_SCL              22

  // Configura una comunicacion serial para la consola (Monitor serie con velocidad por defecto 115200)
  #define SerialMon Serial
  // Configura los comandos AT (para el modulo SIM800L)
  #define SerialAT  Serial1

  #ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
  #else
  TinyGsm modem(SerialAT);
  #endif
  #define IP5306_ADDR          0x75
  #define IP5306_REG_SYS_CTL0  0x00

  //  Funcion para alimentar el sistema por la batería
  bool setPowerBoostKeepOn(int en){
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en) {
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  } else {
    Wire.write(0x35); // 0x37 is default reg value
  }
  return Wire.endTransmission() == 0;
}

void alarma()
{
  digitalWrite(32,HIGH);
  digitalWrite(33,HIGH);
}

void ApagaAlarma()
{
  digitalWrite(32,LOW);
  digitalWrite(33,LOW);
  bandera=true;
}

void smsAlarma()
{
   String MensajeSms = String("La alarma está sonando ");
                     
   if(modem.sendSMS(SMS_TARGET, MensajeSms))
   {
      SerialMon.println(MensajeSms);
      SerialAT.println("AT+CMGD=1,4"); //Comando para eliminar mensajes recibidos
      delay(500);
    }
   else
     {
      SerialMon.println("Falló al enviar el SMS");      
      SerialAT.println("AT+CMGD=1,4"); //Comando para eliminar mensajes recibidos
      delay(500);
     }
     delay(15000); 
       while(SerialAT.available()>0)
       {respuesta=SerialAT.readString();}
       if(respuesta.indexOf("+CMT:")!=-1)
     {
     if(respuesta.lastIndexOf("Cop")!=-1)
     {
        SerialMon.println("Mensaje llegado:" +respuesta);
        SerialAT.println("AT+CMGD=1,4"); //Comando para eliminar mensajes recibidos
        delay(100);
        ApagaAlarma();
     }
      }
}
void setup(){
// Set console baud rate
  SerialMon.begin(115200);

  // Keep power when running from battery
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  // Restart SIM800 module, it takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  
// Unlock your SIM card with a PIN if needed
  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }

    // Define pines como salida
  pinMode(32,OUTPUT);
  pinMode(33,OUTPUT);

}
  
  void loop() {
 int b=1;
 int cont=0;
while(b=1 && cont<3 && !bandera)
  {
  alarma();
  smsAlarma();
  cont++;
  if(cont==3)
  bandera=true;
  }
  ApagaAlarma();
  bandera=false;
  delay(10000);
  }
