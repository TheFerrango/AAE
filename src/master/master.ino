/* 
 	agosto 2013 gestione porte San Patrignano
 	esempio di programma per gestire display, ingressi e porta eth
 	programma scritto da Lotto Lorenzo e Lotto Alessandro
 */

#include <Ethernet.h>
#include <EthernetUdp.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <string.h>
#include <Timer.h>


// definizioni di costanti statiche qui
#define nDevices 2
#define senForDev 12

// timer manager
Timer t;

// Definizione dei pin utilizzati dal display lcd
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);    

// sirena
int RingTone = -1;

// matrici di supporto per lo stato generale
boolean masterDoorStatus[nDevices][12];
boolean masterAuxStatus[nDevices][4];

// indici di supporto per il metodo smarzone di refresh
// non bloccante del display
int devIndexDoor, senIndexDoor, devIndexAux, senIndexAux;

// Definizione componenti ethernet
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168,1,100);
unsigned int listeningPort = 1701;
EthernetUDP Udp;



/*
	imposta l'identificatore del timer della sirena a -1
 	e spegne la sirena impostando il pin 38 a 0 logico
 */
void TurnOffAlert() {
  RingTone = -1;
  digitalWrite(38, LOW);
}


/*
	in caso di stringa di spegnimento di una centrale,
 	effettua il reset degli stati relativi alla centrale
 	indicata e controlla gli stati delle altre centrali.
 	Se almeno una di queste ha un evento in corso ritorna true,
 	altrimenti false
 */
boolean execReset(String s) {

  // ottiene l'indice del device
  int device = s.substring(1,2).toInt();

  // imposta a 0 gli stati della centrale indicata
  for(int i = 0; i < senForDev; i++)
  {
    masterDoorStatus[device][i] = 0;
  }
  for(int i = 0; i < 4; i++)
  {
    masterAuxStatus[device][i] = 0;
  }

  // effettua il controllo sulle altre centrali
  for(int j = 0; j < nDevices; j++)
  {
    if(device == j)
      continue;
    for(int i = 0; i < senForDev; i++)
    {
      if(masterDoorStatus[j][i])
        return true;
    }

    for(int i = 0; i < 4; i++)
    {
      if(masterAuxStatus[j][i])
        return true;
    }
  }

  return false;
}

/* 
 	decodifica una stringa ricevuta da un pacchetto UDP
 	assegnando i valori alle matrici di supporto
 	contenenti lo stato generale del sistema
 */
void parseInputSeq(String s) {

  int device, pin;
  boolean newStatus;
  char type;  

  if(s.substring(2,7) == "SUPPR")
  {
    if(!execReset(s))
    {
      t.stop(RingTone);
      TurnOffAlert();
    }
  }
  else
  {	  
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

  // inizializzazione delle matrici di stati a spento
  for(int i = 0; i < nDevices; i++)
  {
    for(int j = 0; j < senForDev; j++)
      masterDoorStatus[i][j] = false;
    for(int j = 0; j < 4; j++)
      masterAuxStatus[i][j] = false;
  }

  // inizializzazione indici per display
  devIndexDoor = senIndexDoor = devIndexAux = senIndexAux = 0;

  // impostazione del pin di output collegato alla sirena
  pinMode (38, OUTPUT);
  digitalWrite(38, LOW);

  // finta progressbar all'avvio
  lcd.setCursor(0,1);
  for(int i = 0; i < 16; i++)
  {
    lcd.print("*");
    delay(300);
  }

  // avvio del timer per l'aggiornamento del display
  t.every(2000, ProgressDisplay);

  // stampa del logo aziendale
  lcd.setCursor(0,0);            
  lcd.print("LOTTO     GIANNI");
  lcd.setCursor(0,1);
  lcd.print("IMP.   ELETTRICI");

  // apro console seriale per vedere lo stato ingresso
  Serial.begin(9600);                        
}


/*
	metodo di loop richiamato costantemente dal 
 	microcontrollore
 */
void loop() {

  // richiamo il metodo di aggiornamento dei timer
  t.update();

  // ricezione pacchetto udp e aggiornamento matrice se necessario
  if(Udp.parsePacket())
  {
    char udpMsg[UDP_TX_PACKET_MAX_SIZE];
    Serial.write("Ho ricevuto");
    Udp.read(udpMsg, UDP_TX_PACKET_MAX_SIZE);
    Serial.write (udpMsg);

    if(udpMsg[6] == '1')
    {
      // se la sirena è già accesa resetta il timer della sirena
      // fermandolo e ricreandolo
      if(RingTone != -1)
        t.stop(RingTone);

      // imposta il pin collegato alla sirena ad 1 logico per 5 minuti
      digitalWrite(38, HIGH);
      RingTone = t.after(3 * 60 * 1000, TurnOffAlert);
    }

    // decodifica del messaggio ricevuto per permettere la
    // visualizzazione degli eventi
    parseInputSeq(String(udpMsg));	
  }
}

