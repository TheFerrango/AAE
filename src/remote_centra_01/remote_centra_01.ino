/* agosto 2013 gestione porte San Patrignano
 esempio di programma per gestire display, ingressi e porta eth
 programma scritto da Lotto Lorenzo e Lotto Alessandro */

#include <LiquidCrystal.h>
#include <EthernetUdp.h>
#include <string.h>
#include <Timer.h>
#include <Ethernet.h>
#include <SPI.h>


// Definizione delle costanti
#define senForDev 12
#define currentDevice 0

// timer manager
Timer t;

//  define lcd pin, status doors, input pins, badenia, switch pins 


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int DoorValue[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int DoorPin[12] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33 };
int auxPin[4] = { 34, 35, 36, 37 };
int auxValue[4] = { 0, 0, 0, 0 };
int RingTone = -1;
int senIndexDoor = 0;
int senIndexAux = 0;


// Definizione componenti ethernet
// mac locale: 0xEF   -   first remote device .101
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
IPAddress ip(192, 168,1,101), server(192, 168, 1, 100);
unsigned int listeningPort = 1701;
EthernetUDP Udp;
char udpMsg[UDP_TX_PACKET_MAX_SIZE];



/*	
	aggiorna gli eventi porta mostrati sulla prima prima
 	riga del display lcd  
*/
void UpdateDoors() {

  int begSen = senIndexDoor;
  boolean found = false;
  do
  {
    lcd.setCursor(0,0);   
    if (DoorValue[senIndexDoor])
    {
      lcd.print("                ");
      lcd.setCursor(0,0);   
      lcd.print("EVENTO PORTA  ");
      lcd.print(senIndexDoor);	

      found = true;			
    }

    senIndexDoor++;
    if(senIndexDoor == senForDev)
      senIndexDoor = 0;

    if(found)
      break;		
  }
  while(begSen != senIndexDoor);

  if(!found)
    lcd.print("Nessun evento   ");
}


void UpdateEvents() {

  int begSen = senIndexAux;
  boolean found = false;
  do
  {
    lcd.setCursor(0,1);   
    if (auxValue[senIndexAux])
    {
      lcd.print("                ");
      lcd.setCursor(0,1);   
      switch (senIndexAux)
      {
      case 0:
        lcd.print("BATTERIA SCARICA");
      case 1:
        lcd.print("APERTURA SENSORE");
      case 2:
        lcd.print("PERDITA SENSORE");
      }
      found = true;			
    }

    senIndexAux++;
    if(senIndexAux == 4)
      senIndexAux = 0;
    if(found)
      break;		
  }
  while(begSen != senIndexAux);

  if(!found)
    lcd.print("Nessun evento   ");
}


void ProgressDisplay() {
  UpdateDoors();
  UpdateEvents();
}


void SendUdpMessage(int sensor, char type, int senStatus) {

  char msg[6];
  String s = "C"+ currentDevice + type;
  if(sensor < 10)
    s+= "0";
  s+=String(sensor) + "S" + String(senStatus);
	
  s.toCharArray(msg, 6);
  
  Udp.beginPacket(server, listeningPort);
  Udp.write(msg);
  Udp.endPacket();
}


void setup() {

  // inizializzo il display
  lcd.begin(16, 2);              

  // finta progressbar all'avvio	
  lcd.setCursor(1,0);            
  for(int i = 0; i < 16; i++)
  {
    lcd.print("*");
    delay(300);
  }


  // stampa logo aziendale
  lcd.setCursor(0,0);            
  lcd.print("LOTTO GIANNI   ");
  lcd.setCursor(0,1);
  lcd.print("IMP. ELETTRICI ");
  delay (5000);

  // ciclo per la dichiarazione degli ingressi di stato delle porte
  for (int i=0; i<12; i++)         
    pinMode (DoorPin[i], INPUT);

  // ciclo per la dichiarazione degli ingressi ausiliari
  for (int i=0; i<4; i++)          
    pinMode (auxPin[i], INPUT);

  // impostazione del pin di controllo della sirena
  pinMode (38, OUTPUT);


  /*
  IN TEORIA QUESTO SERVE SOLO SU MASTER
   
   //inizializzazione dei vettori di stati a spento
   for(int i = 0; i < 12; i++)     
   	DoorStatus[i] = false;
   for(int i = 0; i < 4; i++)
   	AuxStatus[i] = false;	
   	
   */


  t.every(2000, ProgressDisplay);

  // apro console seriale per vedere lo stato ingresso
  Serial.begin(9600);
}

void loop() {
  
  t.update();

  for (int i=0; i<12; i++)
  {
    int Tmp = digitalRead(DoorPin[i]);
    if (Tmp != DoorValue[i])
    {
      if(Tmp == HIGH)
      {
        //se la sirena è già accesa resetta il timer della sirena
        //fermandolo e ricreandolo
        if(RingTone != -1)
          t.stop(RingTone);

        //imposta il pin collegato alla sirena ad 1 Logico per 5 minuti
        RingTone = t.pulse(38, 5 * 60 *1000, HIGH);
      }
      SendUdpMessage(i, 'S', Tmp);

    }

  }

  for (int i=0; i<4; i++)
  {
    int Tmp = digitalRead(auxPin[i]);  
    if (Tmp != auxValue[i])
    {
      if(Tmp == HIGH)
      {
        //se la sirena è già accesa resetta il timer della sirena
        //fermandolo e ricreandolo
        if(RingTone != -1)
          t.stop(RingTone);

        //imposta il pin collegato alla sirena ad 1 Logico per 5 minuti
        RingTone = t.pulse(38, 5 * 60 * 1000, HIGH);
      }
      SendUdpMessage(i, 'A', Tmp);
    }
  }
}



