// ============================================================
// STATION ENVIRONNEMENTALE COMPLETE
// Arduino UNO + BMP280 + SGP30 + LCD I2C + OpenLog
//
// Fonctions :
// - Lecture temperature / pression / altitude
// - Lecture CO2 / TVOC
// - Affichage LCD I2C
// - Creation automatique d'un fichier texte UTF-8
// - Sauvegarde des mesures sur carte SD
//
// Encodage : UTF-8
// ============================================================


// ============================================================
// LIBRAIRIES
// ============================================================

#include <Wire.h>
#include <SoftwareSerial.h>

#include <LiquidCrystal_I2C.h>

#include <Adafruit_BMP280.h>
#include <Adafruit_SGP30.h>


// ============================================================
// CONFIGURATION
// ============================================================

// LCD I2C
#define LCD_ADDR 0x27

// BMP280
#define BMP280_I2C_ADDR 0x76

// OPENLOG
#define PIN_OPENLOG_RXI_6 6
#define PIN_OPENLOG_TXO_7 7


// ============================================================
// OBJETS
// ============================================================

LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

Adafruit_BMP280 bmp;

Adafruit_SGP30 sgp;

SoftwareSerial OpenLog(
  PIN_OPENLOG_TXO_7,
  PIN_OPENLOG_RXI_6
);


// ============================================================
// VARIABLES
// ============================================================

float altitudeReference = 0;

float temperature = 0;
float pression = 0;
float altitude = 0;

uint16_t co2 = 0;
uint16_t tvoc = 0;

String nomFichier = "MESURES.TXT";


// ============================================================
// SERIAL
// ============================================================

void serialSetup(long baudrate) {

  Serial.begin(baudrate);

  while (!Serial) {
    delay(100);
  }

  Serial.println("Serial OK");
}


// ============================================================
// MESURE SGP30
// ============================================================

bool mesureSGP30() {

  if (!sgp.IAQmeasure()) {

    Serial.println("Erreur SGP30");
    return false;
  }

  co2 = sgp.eCO2;
  tvoc = sgp.TVOC;

  return true;
}


// ============================================================
// CREATION FICHIER UTF-8
// ============================================================

void creationFichier() {

  // --------------------------------------------------------
  // IMPORTANT :
  //
  // OpenLog enregistre automatiquement
  // en UTF-8 si le texte envoye est UTF-8.
  //
  // On force un BOM UTF-8 :
  // EF BB BF
  //
  // Cela permet a Windows / Notepad
  // de reconnaitre le fichier en UTF-8
  // et NON en ISO-8859-15.
  // --------------------------------------------------------

  OpenLog.write(0xEF);
  OpenLog.write(0xBB);
  OpenLog.write(0xBF);

  OpenLog.println("====================================");
  OpenLog.println("STATION ENVIRONNEMENTALE");
  OpenLog.println("Encodage UTF-8");
  OpenLog.println("====================================");

  OpenLog.println(
    "Temperature;Pression;Altitude;CO2;TVOC"
  );

  OpenLog.println("");
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  // --------------------------------------------------------
  // SERIAL
  // --------------------------------------------------------

  serialSetup(9600);

  // --------------------------------------------------------
  // LCD
  // --------------------------------------------------------

  lcd.init();

  lcd.backlight();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Initialisation");

  // --------------------------------------------------------
  // OPENLOG
  // --------------------------------------------------------

  OpenLog.begin(9600);

  delay(2000);

  // --------------------------------------------------------
  // CREATION FICHIER UTF-8
  // --------------------------------------------------------

  creationFichier();

  // --------------------------------------------------------
  // BMP280
  // --------------------------------------------------------

  if (!bmp.begin(BMP280_I2C_ADDR)) {

    Serial.println("BMP280 absent");

    lcd.clear();
    lcd.print("BMP280 absent");

    while (1);
  }

  Serial.println("BMP280 OK");

  // --------------------------------------------------------
  // SGP30
  // --------------------------------------------------------

  if (!sgp.begin()) {

    Serial.println("SGP30 absent");

    lcd.clear();
    lcd.print("SGP30 absent");

    while (1);
  }

  Serial.println("SGP30 OK");

  // --------------------------------------------------------
  // ALTITUDE REFERENCE
  // --------------------------------------------------------

  delay(1000);

  altitudeReference =
    bmp.readAltitude(1013.25);

  // --------------------------------------------------------
  // LCD READY
  // --------------------------------------------------------

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Systeme Pret");

  delay(2000);
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  // =========================================================
  // MESURES
  // =========================================================

  temperature =
    bmp.readTemperature();

  pression =
    bmp.readPressure() / 100.0;

  altitude =
    bmp.readAltitude(1013.25)
    - altitudeReference;

  mesureSGP30();

  // =========================================================
  // PAGE LCD 1
  // =========================================================

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(temperature, 1);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("CO2:");
  lcd.print(co2);
  lcd.print("ppm");

  delay(3000);

  // =========================================================
  // PAGE LCD 2
  // =========================================================

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Press:");
  lcd.print(pression, 0);
  lcd.print("hPa");

  lcd.setCursor(0, 1);
  lcd.print("Alt:");
  lcd.print(altitude, 1);
  lcd.print("m");

  delay(3000);

  // =========================================================
  // PAGE LCD 3
  // =========================================================

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("TVOC:");
  lcd.print(tvoc);

  lcd.setCursor(0, 1);
  lcd.print("Sauvegarde");

  // =========================================================
  // CREATION TEXTE UTF-8
  // =========================================================

  String ligne = "";

  ligne +=
    "Temperature="
    + String(temperature, 1)
    + "°C ; ";

  ligne +=
    "Pression="
    + String(pression, 0)
    + "hPa ; ";

  ligne +=
    "Altitude="
    + String(altitude, 1)
    + "m ; ";

  ligne +=
    "CO2="
    + String(co2)
    + "ppm ; ";

  ligne +=
    "TVOC="
    + String(tvoc)
    + "ppb";

  // =========================================================
  // SAUVEGARDE SD UTF-8
  // =========================================================

  OpenLog.println(ligne);

  // =========================================================
  // MONITEUR SERIE
  // =========================================================

  Serial.println(ligne);

  Serial.println("--------------------------------");

  // =========================================================
  // ATTENTE
  // =========================================================

  delay(2000);

  // Le programme recommence automatiquement
}
