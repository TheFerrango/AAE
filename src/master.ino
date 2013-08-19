/* agosto 2013 gestione porte San Patrignano
 esempio di programma per gestire display, ingressi e porta eth
 programma scritto da Lotto Lorenzo e Lotto Alessandro
 */

#include <LiquidCrystal.h>
#include <EthernetUdp.h>
#include <util.h>
#include <Dhcp.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <Ethernet.h>
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
int RingTone = 0;

// matrici di supporto per lo stato generale
boolean masterDoorStatus[nDevices][12];
boolean masterAuxStatus[nDevices][4];


// indici di supporto per il metodo smarzone di refresh
// non bloccante del display

int devIndexDoor, senIndexDoor, devIndexAux, senIndexAux;


/* 
    decodifica una stringa ricevuta da un pacchetto UDP
    assegnando i valori alle matrici di supporto
    contenenti lo stato generale del sistema
*/
void parseInputSeq(String s)
{
	int device, pin;
	boolean newStatus;
	char type;
	
	device = s.substring(1,2).toInt();
	pin = s.substring(3,5).toInt();
	newStatus = s.substring(6,7).toInt();
	type = s[2];
	
	if(type == 'D')
		masterDoorStatus[device][pin] = newStatus;
	else
		masterAuxStatus[device][pin-34] = newStatus;
	
}


void ProgressDisplay()
{
	int begDev = devIndexDoor;
	int begSen = senIndexDoor;
	do
	{				
		boolean found = false;
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
	lcd.print("Nessun evento   ");
}

/*
	inizializzazione delle strutture dati e delle
	componenti hardware utilizzate
*/
void setup()
{
	// inizializzazione display
    lcd.begin(16, 2);

	// collocamento cursore in 0,0 e stampa
    lcd.setCursor(0,0);                       
    lcd.print("Inizializzazione");
	
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
	
	//finta progressbar all'avvio
	lcd.setCursor(0,1);
	for(int i = 0; i < 16; i++)
	{
		lcd.print("*");
		delay(300);
	}
	
	//sequenze di input fake usate per test
	parseInputSeq("C0D04S1");
	parseInputSeq("C0D11S1");
	parseInputSeq("C1D06S1");
	parseInputSeq("C1D05S1");
	parseInputSeq("C0D03S1");
	parseInputSeq("C1A35S1");
	
	t.every(5000, ProgressDisplay);
	
	//clearScreen();
	lcd.clear();
	// apro console seriale per vedere lo stato ingresso
    Serial.begin(9600);                        
}


void loop()
{
	t.update();
	/*
		TODO: leggi da ethernet per un pacchetto UDP.
		FORMATO: C<#centrale><D -> porta; A -> Aux><#pin>S<0 -> rientro allarme; 1 -> allarme>
	
		"C2D04S1" significa quindi:
		Centrale: 2, Porta: 4, Stato: allarme
	
	*/    
	
	//gestione della visualizzazione eventi relativa alle singole porte
	//per ogni centralina
  /*  boolean noDev = true;
	for(int i = 0; i < nDevices; i++)
	{
		for (int j=0; j<12; j++)
		{			
			lcd.setCursor(0,0);   
			if (masterDoorStatus[i][j])
			{
				lcd.print("                ");
				lcd.setCursor(0,0);   
				lcd.print("Cen ");
				lcd.print(i);
				lcd.print(" - porta ");
				lcd.print(j);
				noDev = false;
				//digitalWrite(38, HIGH);
				delay (5000);
				
			}
		}
		if(noDev)
			lcd.print("Nessun evento   ");
	}
	
	//gestione della visualizzazione eventi ausiliari
	//per ogni centralina
	boolean noAlert = true;
	for(int i = 0; i < nDevices; i++)
	{		
		for (int j=0; j<4; j++)
		{			
			lcd.setCursor(0,1);   
			if (masterAuxStatus[i][j])
			{
				lcd.print("                ");
				lcd.setCursor(0,1);  
				switch (j)
                {
                case 0:
                    lcd.print("BATTERIA SCARICA");
                case 1:
                    lcd.print("APERTURA SENSORE");
                case 2:
                    lcd.print("PERDITA SENSORE");
                }
				delay (1000);
				noAlert = false;				
			}
		}
		if(noAlert)
			lcd.print("Nessun evento   ");
	}*/
	
	lcd.setCursor(0,1);   
	lcd.print("                ");
	lcd.setCursor(0,1);  
	lcd.print(senIndexAux);
	delay(100);
	senIndexAux++;
}
