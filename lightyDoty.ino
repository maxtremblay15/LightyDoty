/*-----------------------------------------------------------------------------------------------------
 * Produit par:
 * Maxime Tremblay
 * EDM-4640 UQAM 2017
 * 
 * DESCRIPTIF:
 *  Jeu une dimension sur une bande de LED contenant 3 boutons
 * BUT:
 * Le jouer doit avancer jusqu'au bout de la bande en bloquant et tuant les ennemis
 ------------------------------------------------------------------------------------------------------*/
//Importation des librairies1
#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>
#include <Chrono.h>
#include <AsciiMassagePacker.h>

//Chrono pour l'envoir des messages à processing pour joueur les sons
Chrono messageSendInterval = Chrono();

// ICLUDE MASSAGE PACKER
// PACKER(FOR SENDING) INSTANCE:
AsciiMassagePacker outbound;
//Msg envoyé pour la lecture des sons à processing
int msgSon;

//Correction GAMMA
const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

//Bande LED
const int nbLED = 84;
#define PIN_STRIP 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(nbLED, PIN_STRIP, NEO_GRB + NEO_KHZ800);
//Chrono pour le rafraichissement de la bande LED
Chrono chronoStrip;

//BOUTONS
//BOUTON VERT - avancer
#define BUTTON_AVANCE 2
Bounce debouncerAvance = Bounce();
bool btnAvanceON = false;

//BOUTON BLEU - bloquer
#define BUTTON_BOUCLIER 3
Bounce debouncerBouclier = Bounce(); 

//BOUTON JAUNE - tirer
#define BUTTON_SHOOT 4
Bounce debouncerSHOOT = Bounce(); 

//Index du joueur sur la LED
int indexJoueur = 0;

//Index du bouclier sur la LED
int indexBouclier;
//Activation du bouclier
bool bouclierOn = false;

//Index du projectile du joueur
int indexSHOOT;
//Activation du projectile
bool SHOOTOn = false;
//Increment du projectile
int incrementSHOOT = 0;
//Position initiale du projectile tiré
int posInitSHOOT;
//Chrono pour l'animation du projectile
Chrono timerSHOOT;

//Chrono pour l'animation de la mort
Chrono mortTimer;
//Nb de fois que l'animation de mort clignote
int mortblink = 0;

//Chrono de la victoire
Chrono gagneTimer;
//Nb de fois que l'animation de victoire clignote
int gagneblink = 0;

//Tableau d'ennemis
#define nbEnnemis 12
//Mise en mémoire de la position des ennemis
int ennemisStetup[nbEnnemis] = {10, 20, 25, 35, 38, 48, 52, 55, 62, 68, 73, 77};
//Tableau à manipuler pour les ennemis
int ennemisArray[nbEnnemis];
//Nb d'ennemis mort
int ennemisMort = 0;

//Attaque ennemi
bool shootEnnemi = false;
//Decrement du projectile ennemi
int decrementShootE;
//Position initiale du projectile ennemi
int posInitShootEnnemi;
//Chrono du projectile ennemi
Chrono ennemiShootTimer;


//Codes de couleurs
  uint32_t cEs;
uint32_t cE;
uint32_t cJ;
uint32_t cB;
uint32_t cVide = strip.Color(0,0,0);

//Etat de la partie
String etat = "jouer";

void setup() {
  //Attribution des valeurs couleurs aux variables
   cEs = strip.Color(pgm_read_byte(&gamma8[240]),0,pgm_read_byte(&gamma8[150]));
 cE = strip.Color(pgm_read_byte(&gamma8[255]),0,0);
 cJ = strip.Color(0,pgm_read_byte(&gamma8[255]),0);
 cB = strip.Color(0,0,pgm_read_byte(&gamma8[255]));

  
  //Mise en arret des timers au départ
  timerSHOOT.stop();
  mortTimer.stop();
  gagneTimer.stop();

  /*Débogage USB************/
     Serial.begin(57600);
  /*************************/
  //Initialisation de la LED à 0
  strip.begin();
  strip.clear();
  strip.show();

  //Initialisation des boutons
  pinMode(BUTTON_AVANCE,INPUT_PULLUP);
  debouncerAvance.attach(BUTTON_AVANCE);
  debouncerAvance.interval(5);

  pinMode(BUTTON_BOUCLIER,INPUT_PULLUP);
  debouncerBouclier.attach(BUTTON_BOUCLIER);
  debouncerBouclier.interval(5);

  pinMode(BUTTON_SHOOT,INPUT_PULLUP);
  debouncerSHOOT.attach(BUTTON_SHOOT);
  debouncerSHOOT.interval(5);

  //Initialisation des ennemis
  setupEnnemis();
}

void loop() {
      //Aléatoire basé sur la fluctuation analogique des broches
      randomSeed(analogRead(0)+analogRead(1)+analogRead(2)+analogRead(3)+analogRead(4)+analogRead(5));
      
      //Écouteurs d'évènement pour les boutons
      debouncerAvance.update();
      int valueAvance = debouncerAvance.read();

      debouncerBouclier.update();
      int valueBouclier = debouncerBouclier.read();

      debouncerSHOOT.update();
      int valueSHOOT = debouncerSHOOT.read();

      //Si la partie est en cours
      if(etat == "jouer"){
          //Si le bouton vert est appuyé et inactif au départ en apppuyant sur rien d'autre
          if(valueAvance == 0 && btnAvanceON == false && valueBouclier == 1 && valueSHOOT == 1){
            //Mettre le bouton actif
            btnAvanceON = true;
            //Avancer le joueur
            avancerJoueur();
          }

          //Si le bouton vert n'est pas enfoncé
          if(valueAvance == 1){
            //Mettre le bouton inactif
            btnAvanceON = false;
          }

          //Si le bouton bleu est appuyé et inactif au départ en apppuyant sur rien d'autre
          if(valueBouclier == 0 && valueSHOOT ==1 && valueAvance == 1){
            //activer le bouclier et ransmettre la valeur couleur(BLEU)
            bouclier(cB);
            //Mettre le bouton inactif
            bouclierOn = true;
            //Envoyer l'index de son à Processing
            msgSon = 1;
          }

          //Si le bouton bleu n'est pas enfoncé et qu'il était activé au départ
          if(valueBouclier == 1 && bouclierOn == true){
            //Mettre le bouton inactif
            bouclierOn = false;
            //Envoyer la valeur couleur VIDE au bouclier
            bouclier(cVide);
          }

          //Si le bouton jaune est appuyé et inactif au départ en apppuyant sur rien d'autre
          if(valueSHOOT == 0 && SHOOTOn == false && valueBouclier == 1){
            //Mettre le bouton activé
            SHOOTOn = true;
            //Envoyer l'index de son à processing
            msgSon = 2;
            //Définir le départ du projectile
            posInitSHOOT = indexJoueur+1;
            //Débuter l'animation du projectile
            timerSHOOT.start();
          }
          //Si le bouton jaune est activer
          if(SHOOTOn == true){
            //Démarrer le lancement du projectile
            activerSHOOT();
          }

          //Si le tire ennemi est désactivé
          if(shootEnnemi == false && ennemisMort < nbEnnemis){
            //Activer le tire
            shootEnnemi = true;
            //Démarrer le moteur d'animation de tire
            ennemiShootTimer.start();
          }
          //Si le projectile ennemi est activer
          if(shootEnnemi == true){
            //lancer le projectile ennemi
            activerShootEnnemi();
          }

      }
      //Si l'état de jeu est MORT
      if(etat == "mort"){
            //Vérifier le Chrono de mort
            if(mortTimer.hasPassed(200)){
               //Colorer toutes les LEDS en rouge
              for(int indexStrip = 0; indexStrip < nbLED; indexStrip++){
                strip.setPixelColor(indexStrip, cE);
              }
            }
            //Si le chrono dépasse la limite
            if(mortTimer.hasPassed(400)){
              //Éteindre toutes les LEDS
              strip.clear();
              mortTimer.restart();
              //Incrémenter le compteur de clignotement
              mortblink++;
              //Si la bande clignote plus que 5 fois
              if(mortblink>5){
                //Redémarrer le jeu
                resetJeu();
              }
            }
            
        }
        //Si le joueur gagne
        if(etat == "gagne"){
          //Verifier le Chrono de victoire
            if(gagneTimer.hasPassed(200)){
              //Mettre toutes les LEDS d'une couleur aléatoire
              for(int indexStrip = 0; indexStrip < nbLED; indexStrip++){
                strip.setPixelColor(indexStrip, strip.Color(pgm_read_byte(&gamma8[random(0,255)]), pgm_read_byte(&gamma8[random(0,255)]), pgm_read_byte(&gamma8[random(0,255)])));
              }
            }
            //Si le chrono depasse
            if(gagneTimer.hasPassed(300)){
              //Eteindre les LEDS
              strip.clear();
              //Redemarrer le Chrono
              gagneTimer.restart();
              //Incrementer le compteur de clignotement
              gagneblink++;
            }

            //Si le compteur depasse 20
            if(gagneblink>30){
              //mettre le 
              gagneTimer.restart();
              gagneTimer.stop();
                resetJeu();
              }
        }

    //Envoie des index pour la lecture sonore par processing
    if ( messageSendInterval.hasPassed(100) ) {
                messageSendInterval.restart();
                outbound.beginPacket("msg");
                outbound.addInt(msgSon);
                outbound.endPacket();
                Serial.write(outbound.buffer(), outbound.size());
                //Remettre le msg à 0
                if(msgSon!=0){
                  msgSon = 0;
                }
                
   }

    if(chronoStrip.hasPassed(10)){
      strip.show();
      chronoStrip.restart();
      }
   
}
//Avancer le point vert sur la bande
void avancerJoueur(){
  //Eteindre l'index présent sur la LED
  strip.setPixelColor(indexJoueur, cVide);

  //Mise en memoire de la couleur du prochain index
  uint32_t colorNext = strip.getPixelColor(indexJoueur+1);

      //Incrémentation de l'index du joueur
      indexJoueur++;
      //Envoie du son de deplacement
      msgSon = 6;
      //Mettre la LED cible en vert
      strip.setPixelColor(indexJoueur, cJ);
      //Si le joueur atteint le bout de la bande
      if(indexJoueur >= nbLED){
        //Demarrer le chrono de victoire
        gagneTimer.start();
        //Envoie de l'index du son de victoire à processing
        msgSon = 4;
        //Définir l'état de victoire
        etat = "gagne";
      }

    //Si la couleur suivant était un ennemi
   if(colorNext == cE || colorNext == cEs){
    //Déclancher la mort
     mort();
   }
}

//Activation du bouclier
void bouclier(uint32_t c){
  //Positionnement à l'avant du joueur
  indexBouclier = indexJoueur+1;
  //Mettre la LED cible selon la couleur transmise
  strip.setPixelColor(indexJoueur+1, c);
}

//Activation du projectile
void activerSHOOT(){
  //Nb de LED que le projectile peut parcourir
  int shootRange = 4;
  //Définir la couleur jaune
  uint32_t c = strip.Color(pgm_read_byte(&gamma8[255]),pgm_read_byte(&gamma8[140]),0);
  //Déplacer le projectile selon l'increment depuis la position initiale
  strip.setPixelColor(posInitSHOOT+incrementSHOOT, c);
        //Si le chrono est dépassé un temps
        if ( timerSHOOT.hasPassed(75) ){
              //Si l'increment est plus grand ou egale à 0
              if(incrementSHOOT>=0){
                //Definir la LED comme vide
                strip.setPixelColor(posInitSHOOT+incrementSHOOT, cVide);
                //redemarrer le chrono
                timerSHOOT.restart();
              }

              //Incrementer la position du projectile
              incrementSHOOT++;

              //Si le projectile touche un ennemi rouge et l'increment n'atteint pas la limite de déplacement
              if(strip.getPixelColor(posInitSHOOT+incrementSHOOT) == cE && incrementSHOOT<=shootRange){
                //Envoie de l'index pour le son à processing
                msgSon = 5;
                //Mettre la LED comme vide
                strip.setPixelColor(posInitShootEnnemi-decrementShootE, cVide);
                //Mettre l'ennemi à 0 dans le tableau des ennemis
                ennemisArray[ennemisMort] = 0;
                //Incrementer le nb d'ennemis mort pour activer le prochain
                ennemisMort++;
                
              }
              
        }
            //Si le projectile atteint sa limite
            if(incrementSHOOT > shootRange){
             //Mettre la LED comme vide
              strip.setPixelColor(posInitSHOOT+incrementSHOOT-1, cVide);
              //Redemarrer le chrono
              timerSHOOT.restart();
              //Arreter le chrono
              timerSHOOT.stop();
              //Désactiver le tire
              SHOOTOn = false;
              //remettre l'increment de tire à 0
              incrementSHOOT = 0;
            }
        
         
 }



//Initialisation des ennemis dans un tableau dédié à la manipulation pour garder l'original intact
void setupEnnemis(){
  //Parcour du tableau de références
  for(int iTabE = 0; iTabE<nbEnnemis; iTabE++){
    //Copie de chaque index vers le tableau manipulable
      ennemisArray[iTabE] = ennemisStetup[iTabE];
   }
    //Pour chaque ennemi, le créer
     for(int indexEnnemi=0; indexEnnemi<nbEnnemis; indexEnnemi++){
              ennemi(ennemisArray[indexEnnemi]);
           }
}

//Création d'un ennemi
 void ennemi(int position){
    //Si la position n'est pas 0
    if(position!=0){
      //Colorier la LED à partir de la position et la couleur transmises
      strip.setPixelColor(position, cE);
    }
 }

//Activation du projectile ennemi
void activerShootEnnemi(){
  //LED limite pour le déplacement du projectile de droite à gauche
  int limite = 0;
  //Vitesse de base du projectile
  int vitesse = 2500;
  //Definir la position de depart 1 index de moin que l'ennemi
  posInitShootEnnemi = ennemisArray[ennemisMort]-1;
  //Colorier la LED selon le positionnement initiale et l'index de decrementation avec la couleur transmise
  strip.setPixelColor(posInitShootEnnemi-decrementShootE, cEs);
  //Si le chrono dépasse la vitesse qui devient plus petite avec le nb d'ennemis morts
  if(ennemiShootTimer.hasPassed(vitesse/(ennemisMort+1))){
              //Definir la LED comme vide
              strip.setPixelColor(posInitShootEnnemi-decrementShootE, cVide);
              //Augmenter le decrement
              decrementShootE++;

              //Si le projectile rencontre le bouclier
              if(strip.getPixelColor(posInitShootEnnemi-decrementShootE) == cB){
                //Arreter le projectile
                stopShootE();
              }

              //Si le projectile touche le joueur pendant qu'il se trouve sur la LED
              if(posInitShootEnnemi-decrementShootE == indexJoueur && indexJoueur >0){
                //Arreter le chrono
                ennemiShootTimer.stop();
                //remettre à 0 le decrement
                decrementShootE = 0;
                //Activer la mort
                mort();
              }
     //Redemarrage du chrono
    ennemiShootTimer.restart();
   }

  //Si le projectile atteint la limite
  if(posInitShootEnnemi-decrementShootE == limite){
    //Arreter le projectile
      stopShootE();
   }
}

//Arret du projectile
void stopShootE(){
   //Remettre de decrement à 0
  decrementShootE = 0;
      //Desactiver le tire ennemi
      shootEnnemi = false;
      //Redemarrer le chrono
      ennemiShootTimer.restart();
      //Arreter le chrono
      ennemiShootTimer.stop();
  
  }

//Activation de la mort
void mort(){
  //Envoie de l'index du son à processing
  msgSon = 3;
  //Nettoyer la bande
  strip.clear();
    //Mettre l'état à mort
     etat = "mort";
     //Activer le chrono de mort pour l'animation de défaite
     mortTimer.start();  
  
}

//Réinitialisation du jeu
void resetJeu(){
      //Remise à 0 des variables
      mortblink = 0;
      gagneblink = 0;
      indexJoueur = 0;
      //incrementSHOOT = 0;
      ennemisMort = 0;
      decrementShootE = 0;
      setupEnnemis();

      //Remise à zéro  et arret préventif des timers
      //timerSHOOT.restart();
      //timerSHOOT.stop();
      mortTimer.restart();
      mortTimer.stop();
      gagneTimer.restart();
      gagneTimer.stop();
      

      //Mettre l'état à jouer
      etat = "jouer";
      ennemiShootTimer.restart();
}


