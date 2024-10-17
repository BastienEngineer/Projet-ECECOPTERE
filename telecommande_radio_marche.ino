#include <SPI.h>
#include <RF24.h>
#include <Adafruit_SSD1306.h>

#define nombreDePixelsEnLargeur 128         // Taille de l'écran OLED, en pixel, au niveau de sa largeur
#define nombreDePixelsEnHauteur 64          // Taille de l'écran OLED, en pixel, au niveau de sa hauteur
#define brocheResetOLED         -1          // Reset de l'OLED partagé avec l'Arduino (d'où la valeur à -1, et non un numéro de pin)
#define adresseI2CecranOLED     0x3C        // Adresse de "mon" écran OLED sur le bus i2c (généralement égal à 0x3C ou 0x3D)
#define JOY_1_X  A0
#define JOY_1_Y  A1 
#define JOY_1_SW 2
#define JOY_2_X  A2
#define JOY_2_Y  A3
#define JOY_2_SW 3
Adafruit_SSD1306 ecranOLED(nombreDePixelsEnLargeur, nombreDePixelsEnHauteur, &Wire, brocheResetOLED);

#define pinCE     9         // On associe la broche "CE" du NRF24L01 à la sortie digitale D7 de l'arduino
#define pinCSN    10         // On associe la broche "CSN" du NRF24L01 à la sortie digitale D8 de l'arduino
        // On associe le point milieu du potentiomètre à l'entrée analogique A0 de l'arduino
//Emetteur
#define tunnel1 "PIPE0"     // On définit un premier "nom de tunnel" (5 caractères), pour pouvoir envoyer des données à l'autre NRF24
#define tunnel2 "PIPE1"     // On définit un second "nom de tunnel" (5 caractères), pour pouvoir recevoir des données de l'autre NRF24
/* ----- NOTEZ L'INVERSION de PIPE1 et PIPE2, entre celui-ci et l'autre montage ! (car chaque tunnel véhicule les infos que dans un seul sens) */

int Joy_1_X_value = 0; 
int Joy_1_Y_value = 0; 
int Joy_2_X_value = 0; 
int Joy_2_Y_value = 0;
bool JOY_1_SW_value = 0;
bool JOY_2_SW_value = 0;
int key = 1001;
const int buzzer = 6; //D6
int connextion = 0; 
RF24 radio(pinCE, pinCSN);  // Instanciation du NRF24L01
        

const byte adresses[][6] = {tunnel1, tunnel2};    // Tableau des adresses de tunnel, au format "byte array"
int message[7];
int message2[7];

void setup() {
    pinMode(buzzer, OUTPUT);
    pinMode(JOY_1_SW, INPUT_PULLUP);
    pinMode(JOY_2_SW, INPUT_PULLUP);
    Serial.begin(9600);
    Serial.println("Récepteur NRF24L01");
    Serial.println("");
    radio.begin();    
    // Initialisation du module NRF24
    radio.setChannel(85);
    radio.openWritingPipe(adresses[0]);      // Ouverture du "tunnel1" en ÉCRITURE
    radio.openReadingPipe(1, adresses[1]);   // Ouverture du "tunnel2" en LECTURE
    radio.setPALevel(RF24_PA_MIN);           // Sélection d'un niveau "MINIMAL" pour communiquer (pas besoin d'une forte puissance, pour nos essais)
    radio.setDataRate(RF24_1MBPS);
    ecranOLED.begin(SSD1306_SWITCHCAPVCC, adresseI2CecranOLED);
    ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
    ecranOLED.setTextSize(2);  
    ecranOLED.setTextColor(SSD1306_WHITE);
    ecranOLED.setCursor(0, 0);                                  // Déplacement du curseur en position (0,0), c'est à dire dans l'angle supérieur gauche
                 // Affichage du texte en "blanc" (avec la couleur principale, en fait, car l'écran monochrome peut être coloré)
    ecranOLED.print("WAIT");
     
    ecranOLED.display();                            // Transfert le buffer à l'écran
    attachInterrupt(digitalPinToInterrupt(buttonPin), ISR, CHANGE);
    delay(50);
}

void loop() 
{
  ecranOLED.clearDisplay();  
  // read analog X and Y analog values
   Joy_1_X_value = analogRead(JOY_1_X);
   Joy_1_Y_value = analogRead(JOY_1_Y);
   Joy_2_X_value = analogRead(JOY_2_X);
   Joy_2_Y_value = analogRead(JOY_2_Y);
  
  JOY_1_SW_value = digitalRead(JOY_1_SW);
  JOY_2_SW_value = digitalRead(JOY_2_SW);
  
  message[0]=key;
  message[1]=Joy_1_X_value;
  message[2]=Joy_1_Y_value;
  message[3]=Joy_2_X_value;
  message[4]=Joy_2_Y_value;
  message[5]=JOY_1_SW_value;
  message[6]=JOY_2_SW_value;
        
  delay(5);
  
  radio.stopListening();                      // On commence par arrêter le mode écoute, pour pouvoir émettre les données
  radio.write(&message, sizeof(message)); 
  delay(5);

  radio.startListening(); 
  if(radio.available()) 
  {                                        // On regarde si une donnée a été reçue
    radio.read(&message2, sizeof(message2));
      if (message2[0]==key)
      {
        if(!JOY_2_SW_value)//if(message[5]==HIGH || message[6]==HIGH) //si on appuie sur le switch d'un joystick, on active le buzzer
        {
            tone(buzzer, 7000);
            delay(200);
            noTone(buzzer);
        }
        if(!JOY_1_SW_value)
        {
            tone(buzzer, 1000);
            delay(200);
            noTone(buzzer);
        }
        on(message2[1]);
     
   }
   delay(5);
 }
   else
   {
     ecranOLED.clearDisplay();
     ecranOLED.setTextSize(2);  
     ecranOLED.setTextColor(SSD1306_WHITE);
     ecranOLED.setCursor(0, 0);                                  // Déplacement du curseur en position (0,0), c'est à dire dans l'angle supérieur gauche
     ecranOLED.print("OFF");  
     ecranOLED.display(); 
     if(!JOY_2_SW_value)//if(message[5]==HIGH || message[6]==HIGH) //si on appuie sur le switch d'un joystick, on active le buzzer
        {
            tone(buzzer, 7000);
            delay(200);
            noTone(buzzer);
        }
        if(!JOY_1_SW_value)
        {
            tone(buzzer, 1000);
            delay(200);
            noTone(buzzer);
        }
   }
   /*
  while(!radio.available())
  {
     ecranOLED.clearDisplay();
     ecranOLED.setTextSize(2);  
     ecranOLED.setTextColor(SSD1306_WHITE);
     ecranOLED.setCursor(0, 0);                                  // Déplacement du curseur en position (0,0), c'est à dire dans l'angle supérieur gauche
     ecranOLED.print("OFF");  
     ecranOLED.display(); 
     if(!JOY_2_SW_value)//if(message[5]==HIGH || message[6]==HIGH) //si on appuie sur le switch d'un joystick, on active le buzzer
     {
        tone(buzzer, 7000);
        delay(200);
        noTone(buzzer);
     }
     if(!JOY_1_SW_value)
     {
        tone(buzzer, 1000);
        delay(200);
        noTone(buzzer);
     }
  }
  */
  // delay(5);
  
}

void on(int message1)
{
   ecranOLED.clearDisplay();
   ecranOLED.setTextSize(2);  
   ecranOLED.setTextColor(SSD1306_WHITE);
   ecranOLED.setCursor(0, 0);                                  // Déplacement du curseur en position (0,0), c'est à dire dans l'angle supérieur gauche
   ecranOLED.print("ON");
   ecranOLED.setCursor(50, 50);                                  // Déplacement du curseur en position (0,0), c'est à dire dans l'angle supérieur gauche
   ecranOLED.print(message1);
   ecranOLED.display();     
}

void ISR(){
  button_time = millis();
  //check to see if increment() was called in the last 250 milliseconds
  if (button_time - last_button_time > 250){
    Serial.print("Interrupt ");
    Serial.print(y++);
    Serial.println();
    ledToggle = !ledToggle;
    digitalWrite(ledPin, ledToggle);
    last_button_time = button_time;
    }
}
