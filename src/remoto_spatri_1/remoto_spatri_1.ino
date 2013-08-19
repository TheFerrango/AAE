/* agosto 2013 gestione porte San Patrignano
  esempio di programma per gestire display, ingressi e porta eth
  programma scritto da Lotto Lorenzo e Lotto Alessandro
*/

#include <LiquidCrystal.h>
//#include <EthernetUdp.h>
//#include <util.h>
//#include <Dhcp.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <Ethernet.h>
#include <SPI.h>


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);  // indico i pin di utilizzo del display lcd
int DoorValue[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //stato degli ingressi
int DoorPin[12] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33 }; //dichiarazione pin porte
int auxPin[4] = { 34, 35, 36, 37 };
int auxValue[4] = { 0, 0, 0, 0 };
int RingTone = 0;              // badenia


 
void setup()
{
 lcd.begin(16, 2);              // inizializzo il display
 lcd.setCursor(0,0);            // cursore in 0 0
 lcd.print("CONTROLLO PORTE ");
 for (int i=0; i<12; i++)         // ciclo per la dichiarazione degli ingressi di stato delle porte
    pinMode (DoorPin[i], INPUT);
 for (int i=0; i<4; i++)         // ciclo per la dichiarazione degli ingressi ausiliari
    pinMode (auxPin[i], INPUT);
 pinMode (38, OUTPUT);
 Serial.begin(9600);            // apro console seriale per vedere lo stato ingresso
 }
  
void loop()
{
  for (int i=0; i<12; i++)
      DoorValue[i] = digitalRead(DoorPin[i]);  
  for (int i=0; i<4; i++)
      auxValue[i] = digitalRead(auxPin[i]);  
 delay (500);
 
 lcd.setCursor(0,1);            // eventi porte
   for (int i=0; i<12; i++)
    {
       switch (DoorValue[i])
         {
           case LOW:
                lcd.print("Nessun evento   ");
           case HIGH:
                {       
                lcd.print("EVENTO PORTA  ");
                lcd.print(i);
                digitalWrite(38, HIGH);
                }
          }
    }

 lcd.setCursor(0,0);            // eventi tecnici
   for (int i=0; i<4; i++)
    {
       switch (auxValue[i])
         {
           case LOW:
                lcd.print("Nessun evento   ");
           case HIGH:
                {
                 switch (auxPin[i])
                  {
                   case 34:
                    lcd.print("BATTERIA SCARICA");
                   case 35:
                     lcd.print("APERTURA SENSORE");
                   case 36:
                     lcd.print("PERDITA SENSORE");
                  }
                }
          }
    }

 
 
}
