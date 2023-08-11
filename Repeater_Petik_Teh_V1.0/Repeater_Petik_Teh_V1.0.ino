//Nama Program    : Uji Telemetri GPS
//Nama Pembuat    : Ali Akbar
//Tanggal Di Buat : 23/07/2023


//Inisialisasi library axp20x (Power Management)
#include <axp20x.h>
AXP20X_Class axp;
//Inisialisasi variabel untuk menyimpan addres axp192
const uint8_t slave_address = AXP192_SLAVE_ADDRESS;
String VBAT;

//Inisialisasi library GPS Neo 6
#include <TinyGPS++.h>
TinyGPSPlus gps;
//Menggunakan pin serial 1 pada esp32
HardwareSerial GPS(1);
//Inisialisasi variabel untuk menampung nilai koordinat dari GPS
String Latitude, Longitude, Altitude;




//Inisialisasi library SPI
#include <SPI.h>
//Inisialisasi library LoRa
#include <LoRa.h>
//Inisialisasi pin yang digunakan LoRa => ESP32
#define SS   18     
#define RST  14   
#define DIO0 26   
#define SCK  5
#define MISO 19
#define MOSI 27
//Insialisasi variabel untuk kirim dan terima pesan pada komunikasi LoRa
String RSI;         //Variabel untuk menampung nilai RSI
String outgoing;            //Pesan keluar
byte msgCount = 0;          //Menghitung jumlah pesan keluar
byte MasterNode = 0xFF;     //Addres Node Gateway
byte Node_Repeater1 = 0xFA;
byte Node_Mesin_Petik_1 = 0xAA;
byte Node_Mesin_Petik_2 = 0xAB;
String pesanrepeater;               //Variabel untuk menampung pesan yang akan dikirim ke Gateway


//Inisialisasi variabel untuk menampung nilai millis
unsigned long lastSendTime = 0;



void setup() {
  //Memulai komunikasi serial
  Serial.begin(9600);

  //Mulai menjalankan komunikasi I2C
  Wire.begin(21, 22);


  //Memulai komunikasi SPI
  SPI.begin(SCK, MISO, MOSI, SS);
  //Setting pin LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(915E6)) {            
    Serial.println("Gagal menjalankan LoRa. Periksan wiring rangkaian.");
  }

  //Mulai menjalankan GPS
  GPS.begin(9600, SERIAL_8N1, 34, 12);


  //Mulai menjalankan library AXP (Power Management)
  int ret = axp.begin(Wire, slave_address);
  if (ret) {
    Serial.println("Ooops, AXP202/AXP192 power chip detected ... Check your wiring!");
  }
  axp.adc1Enable(AXP202_VBUS_VOL_ADC1 |
                 AXP202_VBUS_CUR_ADC1 |
                 AXP202_BATT_CUR_ADC1 |
                 AXP202_BATT_VOL_ADC1,
                 true);


}


void loop() {


  if (millis() - lastSendTime > 1000) {


    if (axp.isBatteryConnect()) {
      VBAT = axp.getBattVoltage();
    } 
    
    else {
      VBAT = "NAN";
    }

    
    if (gps.location.isValid()) {
      Latitude = String(gps.location.lat(), 9);
      Longitude = String(gps.location.lng(), 9);
      Altitude = String(gps.altitude.meters()); 
    } 
    
    else {
      Latitude = "NAN";
      Longitude = "NAN";
      Altitude = "NAN";
    }

    
    
    pesanrepeater = String() + "RPTR1" + "," + Latitude + "," + Longitude + "," + Altitude + "," + VBAT + ",*";

    lastSendTime = millis();            // timestamp the message
  
  } 

  
  else {
    while (GPS.available()) {
      gps.encode(GPS.read());
    }
  }

  onReceive(LoRa.parsePacket());
  
}


void sendMessage(String outgoing, byte Node_Repeater1, byte otherNode) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(otherNode);              // add destination address
  LoRa.write(Node_Repeater1);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}
 
void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
 
  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length
 
  String incoming = "";
 
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
 
  if (incomingLength != incoming.length()) {   // check length for error
   // Serial.println("error: message length does not match length");
    return;                             // skip rest of function
  }
 
  // if the recipient isn't this device or broadcast,
  if (recipient != Node_Mesin_Petik_1 && recipient != Node_Repeater1) {
    //Serial.println("This message is not for me.");
    return;                             // skip rest of function
  } 

  // if the recipient isn't this device or broadcast,
  if (recipient != Node_Mesin_Petik_2 && recipient != Node_Repeater1) {
    //Serial.println("This message is not for me.");
    return;                             // skip rest of function
  } 

  RSI = String(LoRa.packetRssi());
  
  if (sender == Node_Mesin_Petik_1){   
    String message = String() + pesanrepeater + incoming; 
    Serial.println(String() + message);
    sendMessage(message,Node_Repeater1,MasterNode);
    delay(1000);
  }

  
}
 
