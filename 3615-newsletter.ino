////////////////////////////////////////////////////////////////////////
/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
   
////////////////////////////////////////////////////////////////////////
  DEBUT DU PROGRAMME 
*///////////////////////////////////////////////////////////////////////

#include <Minitel1B_Soft.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <avr/wdt.h>

Minitel minitel(7, 6);  // RX, TX

////////////////////////////////////////////////////////////////////////
char TITRE[] = "L'ATELIER AUTONOME";
File myFile;
char texte[81] = "";  // 2 lignes de 40 caractères + 1 caractère null
int nbCaracteres = 0;
const int PREMIERE_LIGNE_EXPRESSION = 10;
const int NB_LIGNES_EXPRESSION = 2;
const char VIDE[] = ".";
Sd2Card card;
unsigned long touche;
////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(9600);
  minitel.changeSpeed(minitel.searchSpeed());
  minitel.smallMode();
  minitel.newScreen();
  
  if (!SD.begin(4)) {
    minitel.println("Erreur d'Initialisation de la carte SD");
    while (1);
  }
}

////////////////////////////////////////////////////////////////////////

void loop() {
  start();
}

void start(){
  // Affichage de la page
  newPage(TITRE);
  // Lecture du champ expression
  lectureChamp(PREMIERE_LIGNE_EXPRESSION, NB_LIGNES_EXPRESSION);
  minitel.moveCursorXY(0, PREMIERE_LIGNE_EXPRESSION + 4);
  writeSD();
  delay(2000);
}

//////////////////////////////////////////////////////////
void reset(){
  champVide(PREMIERE_LIGNE_EXPRESSION, NB_LIGNES_EXPRESSION);
}

////////////////////////////////////////////////////////////
void writeSD(){
  myFile = SD.open("mailling.csv", FILE_WRITE);

  if (myFile) {
    minitel.println("Ecriture...");
    myFile.println(texte);
    myFile.close();
    minitel.clearScreenFromCursor();
    minitel.noCursor();
    minitel.moveCursorXY(14, 24);
    minitel.print("C'est envoyé !");
  } else {
    minitel.println("Erreur sur l'écriture de mailling.csv");
    delay(2000);
  }
}

////////////////////////////////////////////////////////////////////////

void newPage(const char* titre) {
  minitel.newScreen();
  minitel.println(titre);
  
  for (int i = 1; i <= 40; i++) {
    minitel.writeByte(0x7E);
  }
  minitel.moveCursorXY(0, 5);
  //minitel.println("");
  //minitel.println("");
  minitel.println("Abonnez-vous a la newsletter !");
}

////////////////////////////////////////////////////////////////////////

void champVide(int premiereLigne, int nbLignes)
{
  minitel.noCursor();
  minitel.moveCursorXY(1, premiereLigne);
  minitel.clearScreenFromCursor();
  
  for (int j = 0; j < nbLignes; j++) {
    minitel.attributs(CARACTERE_BLEU);
    minitel.print(VIDE);
    minitel.repeat(39);
    minitel.attributs(CARACTERE_BLANC);
  }
  
  texte[0] = '\0';  // Réinitialisation de la chaîne
  nbCaracteres = 0;
  
  minitel.moveCursorXY(0, 21);
  minitel.print("Taper votre adresse mail puis ");
  minitel.attributs(INVERSION_FOND);
  minitel.print("ENVOI");
  minitel.attributs(FOND_NORMAL);
  minitel.println("");
  minitel.println("");
  minitel.attributs(INVERSION_FOND);
  minitel.print("CORRECTION");
  minitel.attributs(FOND_NORMAL);
  minitel.println(" pour corriger");
  minitel.attributs(INVERSION_FOND);
  minitel.print("ANNULATION");
  minitel.attributs(FOND_NORMAL);
  minitel.print(" pour annuler");
  
  minitel.moveCursorXY(0, premiereLigne);
  minitel.cursor();
}

////////////////////////////////////////////////////////////////////////

void correction(int nbLignes) {
  if ((nbCaracteres > 0) && (nbCaracteres <= 40 * nbLignes)) {
    if (nbCaracteres != 40 * nbLignes) { 
      minitel.moveCursorLeft(1); 
    }
    
    minitel.attributs(CARACTERE_BLEU);
    minitel.print(VIDE);
    minitel.attributs(CARACTERE_BLANC);
    minitel.moveCursorLeft(1);
    
    texte[nbCaracteres - 1] = '\0';  // Supprime le dernier caractère
    nbCaracteres--;
  }
}

////////////////////////////////////////////////////////////////////////

void lectureChamp(int premiereLigne, int nbLignes) {
  champVide(premiereLigne, nbLignes);
  bool fin = false;
  
  long snapshotTimer = millis();
  
  while (!fin) {
    if (millis() - snapshotTimer > 30000) {
      snapshotTimer = millis();
      reset();
    }
    
    touche = minitel.getKeyCode();
    
    if ((touche != 0) &&
        (touche != CONNEXION_FIN) &&
        (touche != SOMMAIRE) &&
        (touche != ANNULATION) &&
        (touche != RETOUR) &&
        (touche != REPETITION) &&
        (touche != GUIDE) &&
        (touche != CORRECTION) &&
        (touche != SUITE) &&
        (touche != ENVOI)) {
          
      if (nbCaracteres < (40 * nbLignes) - 1) {  // -1 pour éviter le dépassement
        texte[nbCaracteres] = (char)touche;
        texte[nbCaracteres + 1] = '\0';  // Toujours finir avec '\0'
        nbCaracteres++;
        snapshotTimer = millis();
      }

      if (nbCaracteres == 40 * nbLignes) {
        minitel.moveCursorXY(40, (premiereLigne - 1) + nbLignes);
      }
    }
    
    switch (touche) {
      case ENVOI:
        fin = true;
        break;
      case ANNULATION:
        snapshotTimer = millis();
        champVide(premiereLigne, nbLignes);
        break;
      case CORRECTION:
        correction(nbLignes);
        snapshotTimer = millis();
        break;
    }    
  }
}


////////////////////////////////////////////////////////////////////////
