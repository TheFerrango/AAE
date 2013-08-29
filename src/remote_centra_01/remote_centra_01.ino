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
#define currentDevice 1

// timer manager
Timer t;

//  define lcd pin, status doors, input pins, badenia, switch pins 
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int DoorValue[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int DoorPin[12] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33 };
int auxPin[4] = { 34, 35, 36 };
int auxValue[4] = { 0, 0, 0 };
int RingTone = -1;
int senIndexDoor = 0;
int senIndexAux = 0;
boolean blocked = false;

// Definizione componenti ethernet
// mac locale: 0xEF   -   first remote device .101
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };
IPAddress ip(192, 168, 1, 102), server(192, 168, 1, 100);
unsigned int listeningPort = 1701;
EthernetUDP Udp;
char udpMsg[UDP_TX_PACKET_MAX_SIZE];



/*
	imposta l'identificatore del timer della sirena a -1
 	e spegne la sirena impostando il pin 38 a 0 logico
 */
void TurnOffAlert() {
  RingTone = -1;
  digitalWrite(38, LOW);
}


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


/*
	aggiorna gli eventi critici mostrati sulla seconda
 	riga del display lcd
 */
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
        break;
      case 1:
        lcd.print("APERTURA SENSORE"); 
        break;
      case 2:
        lcd.print("PERDITA SENSORE"); 
        break;
      }
      found = true;			
    }

    senIndexAux++;
    if(senIndexAux == 3)
      senIndexAux = 0;
    if(found)
      break;		
  }
  while(begSen != senIndexAux);

  if(!found)
    lcd.print("Nessun evento   ");
}


/*
	richiama le funzioni di aggiornamento del
 	display lcd
 */
void ProgressDisplay() {
  UpdateDoors();
  UpdateEvents();
}


/*
        sends an UDP packet to the defined server, providing
 informations on sensor status changes
 */
void SendUdpMessage(int sensor, char type, int senStatus) {

  char msg[8] = "\0";
  String s = "C" + String(currentDevice) + type;
  //	msg[1] = currentDevice
  if(sensor < 10)
    s = s + "0";
  s = s + sensor + "S"+ senStatus;
  s.toCharArray(msg, 8);

  Udp.beginPacket(server, listeningPort);
  Udp.write(msg);
  Udp.endPacket();	

  delay(100);
}


/*
        sends an UDP packet to the defined server, providing
 reset information for the current station
 */
void SendUdpReset() {

  char msg[8] = "\0";
  String s = "C"+ String(currentDevice) + "SUPPR";	
  s.toCharArray(msg, 8);

  Udp.beginPacket(server, listeningPort);
  Udp.write(msg);
  Udp.endPacket();

  delay(100);
}


void setup() {

  // apro console seriale per vedere lo stato ingresso
  Serial.begin(9600);

  // inizializzo il display
  lcd.begin(16, 2);              

  // collocamento cursore in 0,0 e stampa
  lcd.setCursor(0,0);                       
  lcd.print("Inizializzazione");

  // inizializzazione scheda Ethernet
  Ethernet.begin(mac, ip);
  Udp.begin(listeningPort);

  // finta progressbar all'avvio	  
  lcd.setCursor(0,1);            
  for(int i = 0; i < 16; i++)
  {
    lcd.print("*");
    delay(300);
  }

  // stampa logo aziendale
  lcd.setCursor(0,0);            
  lcd.print("LOTTO     GIANNI");
  lcd.setCursor(0,1);
  lcd.print("IMP.   ELETTRICI");  

  // ciclo per la dichiarazione degli ingressi di stato delle porte
  for (int i=0; i<senForDev; i++)         
    pinMode (DoorPin[i], INPUT);  

  // ciclo per la dichiarazione degli ingressi ausiliari
  for (int i=0; i<3; i++)          
    pinMode (auxPin[i], INPUT);

  // impostazione del pin di controllo della sirena
  pinMode (38, OUTPUT);
  digitalWrite(38, LOW);

  // impostazione del pin chiave
  pinMode (37, INPUT); 

  t.every(2000, ProgressDisplay);

}


void loop() {

  // richiamo il metodo di aggiornamento dei timer
  t.update();
  int tmpKey = digitalRead(37);  

  // in caso di blocco volontario della centrale, visualizza
  // un messaggio e ignora gli allarmi
  if(tmpKey == HIGH)
  {	
    if(tmpKey != blocked)
    {
      SendUdpReset();
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Blocco manuale");
    lcd.setCursor(0,1);
    lcd.print("in corso.");
    delay(1000);
  }
  else
  {
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

          // imposta il pin collegato alla sirena ad 1 logico per 5 minuti
          digitalWrite(38, HIGH);
          RingTone = t.after(3 * 60 * 1000, TurnOffAlert);
          SendUdpMessage(i, 'S', Tmp);
          DoorValue[i] = Tmp;
        }

        
      }
    }  

    for (int i=0; i<3; i++)
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

          //imposta il pin collegato alla sirena ad 1 logico per 5 minuti
          RingTone = t.after(3 * 60 * 1000, TurnOffAlert);              
          SendUdpMessage(i, 'A', Tmp);        
          auxValue[i] = Tmp;
        }
      }	  
    }
  }

  blocked = tmpKey;
}


