/* agosto 2013 gestione porte San Patrignano
  esempio di programma per gestire display, ingressi e porta eth
  programma scritto da Lotto Lorenzo e Lotto Alessandro */

#include <LiquidCrystal.h>
#include <EthernetUdp.h>
#include <string.h>
//#include <Timer.h>
#include <Ethernet.h>
#include <SPI.h>

// timer manager
Timer t;

//  define lcd pin, status doors, input pins, badenia, switch pins 


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
int DoorValue[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int DoorPin[12] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33 };
int auxPin[4] = { 34, 35, 36, 37 };
int auxValue[4] = { 0, 0, 0, 0 };
int RingTone = 0;
int senIndexDoor = 0;
int senIndexAux = 0;
#define senForDev 12

// Definizione componenti ethernet
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };                  // 0xEF
IPAddress ip(192, 168,1,101);                                         // first remote device .101
unsigned int listeningPort = 1701;
EthernetUDP Udp;
char udpMsg[UDP_TX_PACKET_MAX_SIZE];


 
void setup()
{
   lcd.begin(16, 2);                   // inizializzo il display
  	lcd.setCursor(1,0);            // finta progressbar all'avvio
	for(int i = 0; i < 16; i++)
	{
	  lcd.print("*");
	  delay(300);
	}

 lcd.setCursor(0,0);            // cursore in 0 0
 lcd.print("LOTTO GIANNI   ");
 lcd.setCursor(0,1);
 lcd.print("IMP. ELETTRICI ");
 delay (5000);
 
 for (int i=0; i<12; i++)         // ciclo per la dichiarazione degli ingressi di stato delle porte
    pinMode (DoorPin[i], INPUT);
 for (int i=0; i<4; i++)          // ciclo per la dichiarazione degli ingressi ausiliari
    pinMode (auxPin[i], INPUT);
 pinMode (38, OUTPUT);

  for(int i = 0; i < 12; i++)     //inizializzazione delle matrici di stati a spento
    DoorStatus[i] = false;
  for(int i = 0; i < 4; i++)
    AuxStatus[i] = false;	

  t.every(2000, ProgressDisplay);


  Serial.begin(9600);             // apro console seriale per vedere lo stato ingresso
 
}


/*	aggiorna gli eventi porta mostrati sulla prima prima
	riga del display lcd  */

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
			//digitalWrite(38, HIGH);
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

void UpdateEvents()
{
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


void loop()
{
  t.update();
  for (int i=0; i<12; i++)
    {
      int Tmp = digitalRead(DoorPin[i]);
      if (Tmp != DoorValue[i])
        {
        }
        
    }
  for (int i=0; i<4; i++)
    {
      int Tmp = digitalRead(auxPin[i]);  
      if (Tmp != auxValue[i])
        {
        }
    }
    
    
    
 
}
