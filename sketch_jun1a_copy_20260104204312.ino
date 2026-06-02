#include <Wire.h>
#include <SoftwareSerial.h>

#include <rgb_lcd.h>
#include <Adafruit_BMP280_I2C.h>
#include <Adafruit_SGP30.h>

// =======================
// CONFIGURATION
// =======================

#define BMP280_I2C_ADDR 0x76

#define PIN_OPENLOG_RXI_6 6
#define PIN_OPENLOG_TXO_7 7

// =======================
// OBJETS
// =======================

rgb_lcd lcdRgb;

Adafruit_BMP280 bmp280;
Adafruit_SGP30 sgp30;

SoftwareSerial OpenLog_7(PIN_OPENLOG_TXO_7, PIN_OPENLOG_RXI_6);

// =======================
// VARIABLES
// =======================

float h0;

float temp;
float pression;
float alt;

uint16_t tauxCO2;
uint16_t tauxTVOC;

// =======================
// FONCTIONS
// =======================

void serial_setupConnection(long baudrate) {

  Serial.begin(baudrate);

  while (!Serial) {
    delay(100);
  }

  Serial.println("Port serie active");
}

bool sgp30_measure() {

  if (!sgp30.IAQmeasure()) {

    Serial.println("Erreur mesure SGP30");
    return false;
  }

  tauxCO2 = sgp30.eCO2;
  tauxTVOC = sgp30.TVOC;

  return true;
}

// =======================
// SETUP
// =======================

void setup() {

  // -------- SERIAL --------

  serial_setupConnection(9600);

  // -------- LCD --------

  lcdRgb.begin(16, 2);

  // Couleur verte
  lcdRgb.setRGB(0, 255, 0);

  lcdRgb.clear();
  lcdRgb.print("Initialisation");

  // -------- OPENLOG --------

  OpenLog_7.begin(4800);

  // -------- BMP280 --------

  while (!bmp280.begin(BMP280_I2C_ADDR)) {

    Serial.println("En attente BMP280...");
    
    lcdRgb.clear();
    lcdRgb.print("BMP280 absent");

    delay(1000);
  }

  Serial.println("BMP280 detecte");

  // -------- SGP30 --------

  while (!sgp30.begin()) {

    Serial.println("En attente SGP30...");

    lcdRgb.clear();
    lcdRgb.print("SGP30 absent");

    delay(1000);
  }

  Serial.println("SGP30 detecte");

  // -------- ALTITUDE DE REFERENCE --------

  delay(500);

  h0 = bmp280.readAltitude(1013.25);

  // -------- VALEURS INIT --------

  temp = 0;
  pression = 0;
  alt = 0;

  tauxCO2 = 0;
  tauxTVOC = 0;

  // -------- ENTETE CSV --------

  OpenLog_7.println("Temperature;Pression;Altitude;CO2;TVOC");

  lcdRgb.clear();
  lcdRgb.print("Systeme Pret");

  delay(2000);
}

// =======================
// LOOP
// =======================

void loop() {

  // -------- MESURES BMP280 --------

  temp = bmp280.readTemperature();

  pression = bmp280.readPressure() / 100.0; // hPa

  alt = bmp280.readAltitude(1013.25) - h0;

  // -------- MESURES SGP30 --------

  sgp30_measure();

  // =======================
  // AFFICHAGE LCD PAGE 1
  // =======================

  lcdRgb.clear();

  lcdRgb.setCursor(0, 0);
  lcdRgb.print("T:");
  lcdRgb.print(temp, 1);
  lcdRgb.print((char)223);
  lcdRgb.print("C");

  lcdRgb.setCursor(9, 0);
  lcdRgb.print("CO2:");
  lcdRgb.print(tauxCO2);

  lcdRgb.setCursor(0, 1);
  lcdRgb.print("P:");
  lcdRgb.print(pression, 0);
  lcdRgb.print("hPa");

  delay(3000);

  // =======================
  // AFFICHAGE LCD PAGE 2
  // =======================

  lcdRgb.clear();

  lcdRgb.setCursor(0, 0);
  lcdRgb.print("Alt:");
  lcdRgb.print(alt, 1);
  lcdRgb.print("m");

  lcdRgb.setCursor(0, 1);
  lcdRgb.print("TVOC:");
  lcdRgb.print(tauxTVOC);
  lcdRgb.print("ppb");

  delay(3000);

  // =======================
  // ENREGISTREMENT SD
  // =======================

  OpenLog_7.print(temp);
  OpenLog_7.print(";");

  OpenLog_7.print(pression);
  OpenLog_7.print(";");

  OpenLog_7.print(alt);
  OpenLog_7.print(";");

  OpenLog_7.print(tauxCO2);
  OpenLog_7.print(";");

  OpenLog_7.println(tauxTVOC);

  // =======================
  // MONITEUR SERIE
  // =======================

  Serial.print("Temperature : ");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("Pression : ");
  Serial.print(pression);
  Serial.println(" hPa");

  Serial.print("Altitude : ");
  Serial.print(alt);
  Serial.println(" m");

  Serial.print("CO2 : ");
  Serial.print(tauxCO2);
  Serial.println(" ppm");

  Serial.print("TVOC : ");
  Serial.print(tauxTVOC);
  Serial.println(" ppb");

  Serial.println("----------------------");
}
