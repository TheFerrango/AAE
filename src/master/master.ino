/* agosto 2013 gestione porte San Patrignano
 esempio di programma per gestire display, ingressi e porta eth
 programma scritto da Lotto Lorenzo e Lotto Alessandro
 */

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <string.h>
#include <Timer.h>

// timer manager
Timer t;

// definizioni di costanti statiche qui
#define nDevices 2
#define senForDev 12

// Definizione dei pin utilizzati dal display lcd
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);    

// badenia
int RingTone = -1;

// matrici di supporto per lo stato generale
boolean masterDoorStatus[nDevices][12];
boolean masterAuxStatus[nDevices][4];

// indici di supporto per il metodo smarzone di refresh
// non bloccante del display

int devIndexDoor, senIndexDoor, devIndexAux, senIndexAux;

// Definizione componenti ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168,1,100);
unsigned int listeningPort = 1701;
EthernetUDP Udp;
char udpMsg[UDP_TX_PACKET_MAX_SIZE];


/* 
	decodifica una stringa ricevuta da un pacchetto UDP
	assegnando i valori alle matrici di supporto
	contenenti lo stato generale del sistema
 */
void parseInputSeq(String s) {

  int device, pin;
  boolean newStatus;
  char type;

  device = s.substring(1,2).toInt();
  pin = s.substring(3,5).toInt();
  newStatus = s.substring(6,7).toInt();
  type = s[2];

  if(s[6] == '1')
    Serial.write("Evento apertura");
  else
    Serial.write("Evento chiusura");

  if(type == 'S')
    masterDoorStatus[device][pin] = newStatus;
  else
    masterAuxStatus[device][pin] = newStatus;
}


/*
	aggiorna gli eventi critici mostrati sulla seconda
 	riga del display lcd
 */
void UpdateEvents() {

  int begDev = devIndexAux;
  int begSen = senIndexAux;
  boolean found = false;
  do
  {
    lcd.setCursor(0,1);   
    if (masterAuxStatus[devIndexAux][senIndexAux])
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
    {
      senIndexAux = 0;
      devIndexAux = (devIndexAux+1 )% nDevices;
    }

    if(found)
      break;		
  }
  while(begDev != devIndexAux || begSen != senIndexAux);

  if(!found)
    lcd.print("Nessun evento   ");
}


/*
	aggiorna gli eventi porta mostrati sulla prima prima
 	riga del display lcd
 */
void UpdateDoors() {

  int begDev = devIndexDoor;
  int begSen = senIndexDoor;
  boolean found = false;
  do
  {
    lcd.setCursor(0,0);   
    if (masterDoorStatus[devIndexDoor][senIndexDoor])
    {
      lcd.print("                ");
      lcd.setCursor(0,0);   
      lcd.print("Cen ");
      lcd.print(devIndexDoor);
      lcd.print(" - porta ");
      lcd.print(senIndexDoor);	
      //digitalWrite(38, HIGH);
      found = true;			
    }

    senIndexDoor++;
    if(senIndexDoor == senForDev)
    {
      senIndexDoor = 0;
      devIndexDoor = (devIndexDoor+1 )% nDevices;
    }

    if(found)
      break;		
  }
  while(begDev != devIndexDoor || begSen != senIndexDoor);

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
	inizializzazione delle strutture dati e delle
 	componenti hardware utilizzate
 */
void setup() {

  // inizializzazione display
  lcd.begin(16, 2);

  // collocamento cursore in 0,0 e stampa
  lcd.setCursor(0,0);                       
  lcd.print("Inizializzazione");

  // inizializzazione scheda Ethernet
  Ethernet.begin(mac, ip);
  Udp.begin(listeningPort);

  //inizializzazione delle matrici di stati a spento
  for(int i = 0; i < nDevices; i++)
  {
    for(int j = 0; j < 12; j++)
      masterDoorStatus[i][j] = false;
    for(int j = 0; j < 4; j++)
      masterAuxStatus[i][j] = false;
  }

  //inizializzazione indici per display
  devIndexDoor = senIndexDoor = devIndexAux = senIndexAux = 0;

  //impostazione del pin di output collegato alla sirena
  pinMode (38, OUTPUT);

  //finta progressbar all'avvio
  lcd.setCursor(0,1);
  for(int i = 0; i < 16; i++)
  {
    lcd.print("*");
    delay(300);
  }

  //sequenze di input fake usate per test
  parseInputSeq("C0S04S1");
  parseInputSeq("C0S11S1");
  parseInputSeq("C1S06S1");
  parseInputSeq("C1S05S1");
  parseInputSeq("C0S03S1");
  //parseInputSeq("C1A35S1");

  //avvio del timer per l'aggiornamento del display
  t.every(5000, ProgressDisplay);

  lcd.clear();
  // apro console seriale per vedere lo stato ingresso
  Serial.begin(9600);                        
}


void loop() {

  //richiamo il metodo di aggiornamento dei timer
  t.update();


  //TODO: leggere stato 1 della chiave


  /*
		FORMATO: C<#centrale><S -> Sensore; A -> Aux><#pin>S<0 -> rientro allarme; 1 -> allarme>
   	
   		"C2D04S1" significa quindi:
   		Centrale: 2, Porta: 4, Stato: allarme
   	
   	*/

  //ricezione pacchetto udp e aggiornamento matrice se necessario
  if(Udp.parsePacket())
  {
    Serial.write("Ho ricevuto");
    Udp.read(udpMsg, UDP_TX_PACKET_MAX_SIZE);
    Serial.write (udpMsg);

    if(udpMsg[6] == '1')
    {
      //se la sirena è già accesa resetta il timer della sirena
      //fermandolo e ricreandolo
      if(RingTone != -1)
        t.stop(RingTone);

      //imposta il pin collegato alla sirena ad 1 Logico per 5 minuti
      RingTone = t.pulse(38, 5 * 60 * 1000, HIGH);
    }

    //decodifica del messaggio ricevuto per permettere la
    //visualizzazione degli eventi
    parseInputSeq(String(udpMsg));
  }
}

