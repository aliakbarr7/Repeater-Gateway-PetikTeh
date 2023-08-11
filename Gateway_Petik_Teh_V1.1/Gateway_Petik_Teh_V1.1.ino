//Inisialisasi library LoRa
#include <SPI.h> 
#include <LoRa.h>
//Inisialisasi pin yang digunakan untuk modul LoRa => ESP32
#define SS   18        
#define RST  14  
#define DIO0 26     
#define SCK  5
#define MISO 19
#define MOSI 27
//Frekuensi yang digunakan
#define Band 915E6


//Inisialisasi library time
//Untuk menjalankan fungsi millis
#include <time.h>


#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define LEDRED 14
#define LEDGREEN 13
#define LEDBLUE 25


//Insialisasi variabel untuk kirim dan terima pesan pada komunikasi LoRa
String ID = "GTWYPPTK0823"; //ID Device Gateway
String RSI = "NAN";
String SNR = "NAN";
byte MasterNode = 0xFF;     //Addres Node Gateway     
byte Node_Repeater1 = 0xFA;    //Addres Node Halter 1
byte Node_Repeater2 = 0xFB;    //Addres Node Halter 2
byte Node_Repeater3 = 0xFC;   //Addres Node Kandang
String SenderNode = "";     //Variabel penampung pesan dari Node
String outgoing;            //Pesan keluar
byte msgCount = 0;          //Menghitung jumlah pesan keluar

 
//Insialisasi variabel penampung waktu millis
unsigned long previousMillis=0;
unsigned long int previoussecs = 0; 
unsigned long int currentsecs = 0; 
unsigned long currentMillis = 0;
int interval= 1 ; //Interval untuk melakukan update setiap 1 detik
int Secs = 0;     //Inisialisasi variabel secs
 
 
void setup() {
  //Memulai komunikasi serial
  Serial.begin(9600);
  
  //Memulai komunikasi SPI
  SPI.begin(SCK, MISO, MOSI, SS);
  //Setting pin LoRa
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(Band)) {            
    Serial.println("Gagal menjalankan LoRa. Periksan wiring rangkaian.");
  }
    Serial.println("Berhasil menjalankan LoRa Node Kandang.");

  pinMode(LEDRED, OUTPUT);
  pinMode(LEDGREEN, OUTPUT);
  pinMode(LEDBLUE, OUTPUT);
  
  LEDCOLOR("OFF");

  lcd.init();
  lcd.backlight();
}
 
void loop() {
  
currentMillis = millis();
   currentsecs = currentMillis / 1000; 
    if ((unsigned long)(currentsecs - previoussecs) >= interval){
       previoussecs = currentsecs;
       
       LEDCOLOR("RED");
       lcd.setCursor(0,0);
       lcd.print(String() + "  " + ID);
       lcd.setCursor(0,1);
       lcd.print(String() + "RSI=" + RSI + " SNR=" + SNR);   
    }
 
  //Parsing paket data LoRa
  onReceive(LoRa.parsePacket());
    
}
 
 
void sendMessage(String outgoing, byte MasterNode, byte otherNode) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(otherNode);              // add destination address
  LoRa.write(MasterNode);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}
 
void onReceive(int packetSize) {
  //Jika tidak ada paket atau data, maka return
  if (packetSize == 0) return;   
 
  //Membaca header paket data
  int recipient = LoRa.read();   //Addres penerima
  byte sender = LoRa.read();     //Addres pengirim
  
  byte incomingMsgId = LoRa.read();    //Menerima pesan ID Device
  byte incomingLength = LoRa.read();   //Menerima ukuran data pesan
 
  String incoming = "";

  //Jika ada pesan LoRa maka mulai membaca pesan
  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  //Memeriksa ukuran data untuk mengantisipasi error
  if (incomingLength != incoming.length()){  
    //Serial.println("Error: Ukuran pesan tidak sesuai!");
    //Skip, dan jangan terima pesan
    return;                   
  }
 
  //Jika penerima bukan dari perangkat ini atau device yang sudah diinisialisasi
  //Maka jangan terima pesan ini
  if (recipient != Node_Repeater1 && recipient != MasterNode){
    //Serial.println("Pesan tidak dikenali!");
    //Skip, dan jangan terima pesan
    return;                           
  }

  if (recipient != Node_Repeater2 && recipient != MasterNode){
    //Serial.println("Pesan tidak dikenali!");
    //Skip, dan jangan terima pesan
    return;                           
  }

  if (recipient != Node_Repeater3 && recipient != MasterNode){
    //Serial.println("Pesan tidak dikenali!");
    //Skip, dan jangan terima pesan
    return;                           
  }

  

 
   if( sender == Node_Repeater1 ){ 
      LEDCOLOR("GREEN");
      Serial.println(String() + incoming);
      RSI = String(LoRa.packetRssi());
      SNR = String(LoRa.packetSnr());
     }

   if( sender == Node_Repeater2 ){ 
      LEDCOLOR("GREEN");
      Serial.println(String() + incoming);
      RSI = String(LoRa.packetRssi());
      SNR = String(LoRa.packetSnr());
     }
   
   if( sender == Node_Repeater3 ){ 
      LEDCOLOR("BLUE");
      Serial.println(String() + incoming);
      RSI = String(LoRa.packetRssi());
      SNR = String(LoRa.packetSnr());
     }
 
}


void LEDCOLOR(String color) {
  if (color == "RED") {
    digitalWrite(LEDRED, HIGH);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, LOW);
  } else if (color == "GREEN") {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, HIGH);
    digitalWrite(LEDBLUE, LOW);
  } else if (color == "BLUE") {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, HIGH);
  } else if (color == "OFF") {
    digitalWrite(LEDRED, LOW);
    digitalWrite(LEDGREEN, LOW);
    digitalWrite(LEDBLUE, LOW);
  }
}
