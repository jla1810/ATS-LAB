/*
  ============================================================================
  SI4735 2.8 TFT WIFI V5.3oc - SOURCE RESTRUCTUREE / NETTOYEE
  ----------------------------------------------------------------------------
  Nettoyage conservatif :
  - aucune fonction deplacee ;
  - aucune logique radio modifiee ;
  - aucun bloc de code supprime ;
  - espaces et lignes vides normalises ;
  - sections principales clairement identifiees.

  Projet ATS LAB / O.H.M
  ============================================================================
*/

//  V.5.3b-mjs  17.09.2022+   Better displayed stored configuration over serial port upon startup. Established timeout

//  Vadim Afonkin on his Dropbox repository. It is important to note that the author of the SI473x library does not encourage anyone to use the SSB patches

//  ATTENTION: The author of this project does not guarantee that procedures shown here will work in your development environment.

#include <WiFi.h>
#include "BluetoothSerial.h"
#include <ESPmDNS.h>
#include <Preferences.h>
#include "time.h"

// Prototypes explicites requis par le preprocesseur Arduino.
void FreqDispl();
void FreqDraw(float freq, int d);
void Segment(String freq, String mask, int d);
bool pcStartScan(uint32_t requestedBaseKHz = 0,
                 float requestedStepKHz = 0);

String WIFI_SSID = "";
String WIFI_PASS = "";

bool wifiConnectPending = false;
unsigned long wifiConnectStarted = 0;
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 20000;

bool pcRdsEnabled = false;
String pcRdsPS = "";
String pcRdsRT = "";
String pcRdsCT = "";
uint16_t pcRdsPI = 0;
uint8_t pcRdsPTY = 0;
bool pcRdsHaveData = false;
unsigned long pcRdsLastPoll = 0;
const unsigned long PC_RDS_POLL_MS = 120;
Preferences atsPreferences;

const uint16_t ATS_TCP_PORT = 3333;
WiFiServer atsTcpServer(ATS_TCP_PORT);
WiFiClient atsTcpClient;
bool atsTcpStarted = false;
BluetoothSerial SerialBT;
bool atsBluetoothStarted = false;

const char* ntpServer = "ntp.midway.ovh";
const long  gmtOffset_sec =3600 * 0;
const int   daylightOffset_sec = 0;

#define NTP_MIN_VALID_EPOCH 1533081600

// =====================PINS========================
#define ESP32_I2C_SDA    21  // I2C bus pin on ESP32
#define ESP32_I2C_SCL    22  // I2C bus pin on ESP32
#define RESET_PIN        12
#define ENCODER_PIN_A    17  // http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
#define ENCODER_PIN_B    16
#define ENCODER_SWITCH   33
#define BAT_INFO         35
#define BEEPER           32
#define DISPLAY_LED      14
#define AUDIO_MUTE       27
// =================================================

// ====================Display======================
#define SCREEN_V      0
#define SCREEN_H      1
uint16_t calDataV[5] = { 258, 3566, 413, 3512, 4 };
uint16_t calDataH[5] = { 387, 3530, 246, 3555, 7 };
// =================================================

// ==================Oscillator=====================
#define IhaveCrystal
// =================================================

#include "Tahoma_10x12_ru.h"
#include "Tahoma_15x16_ru.h"

#define TFT_TRANS   -1 //transparent color

#define BL_T -1 //left alignment
#define BC_T 0 //center alignment
#define BR_T 1 //right alignment

#define REG_T 0 //regular
#define BOL_T 1 //bold
#define NRG_T 2 //dense
#define NBL_T 3 //dense bold
#define CUR_T 4 //italic
#define CUB_T 5 //bold italic
#define NCR_T 6 //dense italic
#define NCB_T 7 //dense bold italic

#define T1012_T 0 //font Tahoma_10x12_ru
#define T1516_T 1 //font Tahoma_15x16_ru

float tftRusSize = 1; //font size
uint16_t tftRusColor = 0xFFFF; //font color is white
int32_t tftRusBack = 0x0000; //background color is black
int tftRusDatum = 0; //center alignment
int tftRusStyle = 0; //regular font
int tftRusFont = 0; //font
int tftRusBeginChar = 0; //start output from character number
int tftRusContChar = 0; //number of characters to display. 0 - all

int tftRusWidth = 10; //current character width
bool tftRusBottomCut = false; // cut below the writing line
int tftRusCursiveLevel = 4; //degree of italic slant 1 - strong, 2 - medium, 4 - weak

#include <TFT_eSPI.h> // NOTE: *REQUIRES* TFT_eSPI by Bodmer version 2.3.70 - Attempting to upgrade to newer versions causes this
#include <SPI.h>
#include <SI4735.h>
#include "EEPROM.h"
#include "Rotary.h"

#ifdef IhaveSI5351
#endif

#include "DSEG7_Classic_Mini_Regular_34.h"
#include "TFT_Colors.h"
#include "Button.h"
#include "logo.h"
#include "ohm_startup_logo.h"
//======================================================= BUTTON ===========================================
#define B_NORMAL  0
#define B_JAM     1
#define B_SELECT  2
#define B_BLOCK   3

struct But {
  const uint8_t     num;
  const uint8_t  layout;
  const uint8_t    type;
  const char      *Name;
  const uint16_t  xPosV;
  const uint16_t  yPosV;
  const uint16_t  xPosH;
  const uint16_t  yPosH;
};

#include "key.h"

const int lastBut = (sizeof but / sizeof(But)) - 1;
bool butBlock[lastBut + 1];
//======================================================= Retro Bands    ===================================
typedef struct // Retro bands data
{
  const char      *BandName;
  const char      *BandNameRU;
  const char      *RetroBandTime;
  const char      *RetroBandTimeRU;
  const uint16_t  xPosV;
  const uint16_t  yPosV;
  const uint16_t  xPosH;
  const uint16_t  yPosH;
  const int       band;
  const float     minimumFreq;
  const float     maximumFreq;
  float           currentFreq;
  const float     scale; //pixels in one freq
  const float     mark; //digit marks on scale in one freq
  const float     hardStep;
  const float     softStep;
} RetroBand ;

RetroBand bandRetro[] {
  "FM",   "FM",   "",             "",                 10, 40,   240,200,    0,  87.50, 108.00,  87.50,   50,    1, 10,   10,
  "VHF",  "VHF",  "",             "",                 10, 80,   240,150,    0,  64.00,  87.00,  64.00,   50,    1, 10,    1,
  "LW",   "DV",   "",             "",                150, 40,   240, 40,    1,    153,    279,    153,   10,    9,  1,    1,
  "MW",   "СВ",   "",             "",                150, 80,   240, 90,    2,    522,   1701,    522,    1,   90,  9,    1,
  "SW1",  "КВ1",  "NIGHT WINTER", "Notte Inverno",      10,120,     0, 40,   29,   1800,   5060,   2300,    1,  100,  5,    1,
  "SW2",  "КВ2",  "NIGHT",        "Notte",             10,160,     0, 80,   29,   5300,   7600,   5900,    1,  100,  5,    1,
  "SW3",  "КВ3",  "MOSTLY NIGHT", "Soprattutto Notte",  10,200,     0,120,   29,   9400,  12160,   9400,    1,  100,  5,    1,
  "SW4",  "КВ4",  "MOSTLY DAY",   "Soprattutto Giorno",  10,240,     0,160,   29,  13570,  18168,  13570,    1,  100,  5,    1,
  "SW5",  "КВ5",  "DAY",          "Giorno",             10,280,     0,200,   29,  18900,  26100,  18900,    1,  100,  5,    1,
  };
const int lastBandRetro  = (sizeof bandRetro / sizeof (RetroBand)) - 1;
//======================================================= END Retro Bands     ==============================
typedef struct // Group data
{
  const uint16_t groupIdx;
  const char    *PresetName;
} Group ;

typedef struct // Preset data
{
  const float    memoryIdx;
  char          *MemoryName;
  char          *memoryGroup;
} Memory ;

#include "Preset.h"

const int lastGroup  = (sizeof group / sizeof (Group)) - 1;
const int lastMemory = (sizeof memory / sizeof (Memory)) - 1;

uint16_t PresetId; //CITY ID
uint16_t prevPresetId;
int lastPreset;

typedef struct // Preset data
{
  float      presetIdx;
  char      *PresetName;
  int        presetPos;
} Preset ;

int textScroll;
long elapsedScroll;
int directScroll = 0;
// ============================================================================
#include "patch_full.h"    // SSB patch for whole SSBRX full download

const uint16_t size_content = sizeof ssb_patch_content; // see ssb_patch_content in patch_full.h or patch_init.h

#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

#define MIN_ELAPSED_TIME             100
#define MIN_ELAPSED_RSSI_TIME        150
#define MIN_ELAPSED_AudMut_TIME        0  // Noise surpression SSB in mSec. 0 mSec = off
#define MIN_ELAPSED_RDS_TIME           5
#define DEFAULT_VOLUME                15   // change it for your favorite start sound volume
#define MIN_ELAPSED_VOLbut_TIME     1000
#define CLK_Xtal                    SI5351wire_CLK0

#define FM          0
#define LSB         1
#define USB         2
#define AM          3
#define CW          4

#define TFT_GREY 0x5AEB
#define TFT_LIGTHYELLOW 0xFF10

bool bfoTr          = false;
bool bfoOn          = false;
bool ssbLoaded      = false;
bool FirstLayer     = true;

bool SecondLayer    = false;
bool ThirdLayer     = false;
bool ForthLayer     = false;
bool HamBand        = false;
bool BroadcastOCbut = false; // Menu des bandes de radiodiffusion ondes courtes
bool Modebut        = false;
bool FREQbut        = false;
bool Decipoint      = false;
bool STEPbut        = false;

bool BroadBand;
bool BandWidth;

bool PRESbut        = false;
bool VOLbut         = false;
bool AudioMut       = false;
bool Mutestat       = false;
bool AGCgainbut     = false;
bool writingEeprom  = false;

// =============== Squelch Functionality ============ LWH
bool SquelchUsesRSSI = true; // When true, the squelch uses RSSI, when false the squelch uses SNR
bool SQUELCHbut = false;
long SQUELCHbutOnTime       = millis();
int previousSquelch;
int currentSquelch;
int SignalQuality = 0;
long squelchDecay            = 0;
#define squelchDecayTime     500  //  1000
bool squelch = false;
uint8_t currentSQUELCHStep     =  1;
uint8_t MaxSQUELCH             = 50;
uint8_t MinSQUELCH             =  0;
uint8_t encoderBtnState        =  0;
#define MIN_ELAPSED_SQUELCHbut_TIME     1000
// ==================================================
bool batVolt        = true;
long elapsedBat     = 0;

bool  SCANbut             = false;
int   currentScanFreq;
int   pcScanDataCount = 0;
unsigned long scanTuneStarted = 0;
const unsigned long SCAN_SETTLE_FM_MS = 35;
const unsigned long SCAN_SETTLE_HAM_MS = 60;
int   posScanFreq;
int   posScan;
int   posScanLast;
float SCANstep;
bool  SCANpause = true; // LWH - SCANpause must be initialized to a value else the squelch function will not work correctly.;
float currentScanLine;
int   ScanValueRSSI[320];
int   ScanValueSNR[320];
bool  ScanMark[320];
uint8_t ScanScaleLine[320];
uint8_t ScanMarkSNR       = 3;
int   ScanBeginBand;
int   ScanEndBand;
uint8_t ScanAGC;
bool  ScanEmpty           = true;
float deltaScanLine       = 0;
float currentMinScanStep;
float currentMaxScanStep;
int   countScanSignal     = 3;
float signalScale;
bool  prevScaleLine       = false;
// ============================================================================
bool  RETRObut            = false;
float currentRetroFreq;
float currentRetroScale;
const uint8_t RetroStationPos[] = {43, 55, 67, 79, 91, 103, 131, 143, 155, 167, 179, 191};
uint8_t RETROband         = 0;
bool  bandRETRObut        = false;
bool  cityRETRObut        = false;
int   cityRetroRotary     = 0;
int   scrollRetro         = 0;
int   bandHamRetro;
// ============================================================================
bool  MEMObut             = false;
int   currentMemo         = 0;
bool  MEMOadd             = false;
bool  MEMOdel             = false;
char  addMemoName[21];
uint16_t addMemoFreq;
uint8_t addMemoBand;
uint8_t addMemoMode;
uint8_t posMemoName;
uint8_t charMemoName;
long elapsedCursor        = millis();
bool  presetBank          = false;
// ============================================================================
bool    SETUPbut          = false;
int     pageSetup         = 0;
uint8_t maxPageSetup      = 5;
// ============================================================================
bool      PREtap          = false;
bool      PRE             = false;
uint16_t  PREfreq;
uint8_t   PREband;
uint8_t   PREmode;
int       PREbfo;
uint8_t   PREstep;
uint8_t   PREbw;
long      elapsedPRE      = millis();
// ============================================================================
bool VHFon;
bool langRetroEN;
bool digitLigth;
bool beeperOn;
bool memoPreset;
bool batShow;
bool loadMemory;
bool loadDefault;
bool saverOn;
uint8_t saverTime;
bool displayOff;
float minSCANstep;
float maxSCANstep;
bool  autoSCANstep;
int  SCANscale;
bool SCANaccuracy;
bool screenV;
bool displayPower;
uint16_t boolOpt;
bool RDSalways;
bool seekAccuracy;

bool prevdigitLigth;
bool prevlangRetroEN;
bool prevVHFon;
bool prevbeeperOn;
bool prevloadMemory;
bool prevbatShow;
bool prevmemoPreset;
bool prevloadDefault;
bool prevsaverOn;
uint8_t prevsaverTime;
bool prevdisplayOff;
float prevminSCANstep;
float prevmaxSCANstep;
bool prevautoSCANstep;
bool prevSCANaccuracy;
bool prevscreenV;
bool prevdisplayPower;
bool prevRDSalways;
bool prevseekAccuracy;
// ============================================================================
long elapsedSaver = millis();
bool Saver = false;
int saverX;
int saverY;
int posSaver = 0;
// ============================================================================
bool pressed;
bool presStat;
bool audioMuteOn  = true;
bool audioMuteOff = false;
bool RDS          = true; // RDS on / off
bool SEEK         = false;
bool bright       = false;
bool CWShift      = false;

bool calibratSI5351 = false;

int currentBFO;
int currentBFOmanu;
int previousBFO     = 0;
int previousBFOmanu = 0;
int OldRSSI;
int NewRSSI;
int NewSNR;
int encBut;
uint8_t AGCgain;

long elapsedRSSI        = millis();
long elapsedAudMut      = millis();
long stationNameElapsed = millis();
long VOLbutOnTime       = millis();

volatile int encoderCount  = 0;
volatile int encoderButton = 0;

bool volDisp               = false;

uint16_t previousFrequency;
uint8_t currentBFOStep     = 25;
uint8_t currentPRES        =  0;
int     previousPRES;
uint8_t currentPRESStep    =  1;

uint8_t currentAGCgain     =  1;
uint8_t previousAGCgain    =  1;
uint8_t currentAGCgainStep =  1;
uint8_t MaxAGCgain;
uint8_t MaxAGCgainFM       = 26;
uint8_t MaxAGCgainAM       = 37;
uint8_t MinAGCgain         =  1;

int     currentVOL         =  0;
int     previousVOL        =  0;
uint8_t currentVOLStep     =  1;
uint8_t MaxVOL             = 63;
uint8_t MinVOL             =  0;

uint8_t bwIdxSSB;
uint8_t bwIdxAM;
uint8_t bwIdxFM;
uint8_t ssIdxMW;
uint8_t ssIdxAM;
uint8_t ssIdxFM;
uint8_t bandIdx;
uint8_t currentMode        = FM;
uint8_t previousMode       =  0;
uint16_t x = 0, y = 0; // To store the touch coordinates
uint8_t encoderStatus;
uint16_t freqstep;
uint8_t freqstepnr         = 0; //1kHz
int freqDec                = 0;

const int LedFreq          = 5000;
const int LedResol         = 8;
const int LedChannelforTFT = 0;
uint16_t currentBrightness;
uint16_t previousBrightness = 65535;
uint16_t MaxBrightness     = 16;
uint16_t MinBrightness     = 256;
uint8_t stepsizesynth      = 10;

float DisplayfreqNew       = 0;
float Displayfreq          = 0;
float currentFrequency     = 0;

float dpfrq                = 0;
float fact                 = 1;
float RSSIfact             = 3;

String BWtext;
String Modtext;

float mjs_currentFrequency     = 999; // mjs
uint8_t mjs_currentMode        = 999; // mjs
uint8_t mjs_bandIdx            = 999; // mjs
int mjs_currentBFO             = 999; // mjs

// ============================================================================
struct tm timeinfo;

// ============================================================================
const char *bandwidthSSB[] = {"1.2", "2.2", "3.0", "4.0", "0.5", "1.0"};
const char *bandwidthAM[]  = {"6.0", "4.0", "3.0", "2.0", "1.0", "1.8", "2.5"};
const char *bandwidthFM[]  = {"AUTO", "110", "84", "60", "40"};
const char *stepsize[]     = {"1", "5", "9", "10"};
const char *stepsizeFM[]   = {"100", "10"};

const char *Keypathtext[]  = {"1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "OK", "DEL", "CLS", "X"};
const char *bandModeDesc[] = {"FM", "LSB", "USB", "AM", "CW"};

char buffer[64]; // Useful to handle string

char *stationName;
char bufferStatioName[50]; // 40

char *rdsMsg;
char bufferRdsMsg[100]; //  100

char *rdsTime;
char bufferRdsTime[32]; //  32

unsigned long FreqSI5351 = 3276800;
unsigned long calibratvalSI5351;

int Xsmtr   =   0;
int Ysmtr   =  80; // S meter

int XVolInd =   0;
int YVolInd = 135; // Volume indicator

int XFreqDispl  =   0;
int YFreqDispl  =   0; // display

#define B_HAM       0
#define B_BFO       1
#define B_FREQ      2
#define B_AGC       3
#define B_MUTE      4
#define B_VOL       5
#define B_MODE      6
#define B_BANDW     7
#define B_STEP      8
#define B_BAND      9
#define B_ATT      10
#define B_NEXT     11

#define B_SEEKUP    0
#define B_SEEKDN    1
#define B_INFO      2
#define B_RDS       3
#define B_FM        4
#define B_MEMO      5
#define B_LIGHT     6
#define B_SETUP     7
#define B_SQUELCH   8  // LWH - was B_CHIP
#define B_SCAN      9
#define B_RETRO    10
#define B_BACK     11
//======================================================= THE Band Definitions     ============================
typedef struct // Band data
{
  const char *bandName; // Bandname
  uint8_t  bandType; // Band type (FM, MW or SW)
  uint16_t prefmod; // Pref. modulation
  uint16_t minimumFreq; // Minimum frequency of the band
  uint16_t maximumFreq; // maximum frequency of the band
  uint16_t currentFreq; // Default frequency or current frequency
  uint8_t  currentStep; // Default step (increment and decrement)
  int          lastBFO; // Last BFO per band
  int      lastmanuBFO; // Last Manual BFO per band using X-Tal

} Band;

Band band[] = {
  {   "FM", FM_BAND_TYPE,  FM,  6400, 10800,  9920, 10, 0, 0}, //  FM          0
  {   "LW", LW_BAND_TYPE,  AM,   100,   514,   198,  9, 0, 0}, //  LW          1
  {   "MW", MW_BAND_TYPE,  AM,   514,  1800,  1395,  9, 0, 0}, //  MW          2
  { "800M", LW_BAND_TYPE,  AM,  280,   470,   284,  1, 0, 0}, // Ham  800M    3
  { "630M", SW_BAND_TYPE, LSB,   470,   480,   475,  1, 0, 0}, // Ham  630M    4
  { "160M", SW_BAND_TYPE, LSB,  1800,  2000,  1850,  1, 0, 0}, // Ham  160M    5
  { "120M", SW_BAND_TYPE,  AM,  2000,  3200,  2400,  5, 0, 0}, //      120M    6
  {  "90M", SW_BAND_TYPE,  AM,  3200,  3500,  3300,  5, 0, 0}, //       90M    7
  {  "80M", SW_BAND_TYPE, LSB,  3500,  3900,  3630,  1, 0, 0}, // Ham   80M    8
  {  "75M", SW_BAND_TYPE,  AM,  3900,  5300,  3950,  5, 0, 0}, //       75M    9
  {  "60M", SW_BAND_TYPE, USB,  5300,  5900,  5375,  1, 0, 0}, // Ham   60M   10
  {  "49M", SW_BAND_TYPE,  AM,  5900,  7000,  6000,  5, 0, 0}, //       49M   11
  {  "40M", SW_BAND_TYPE, LSB,  7000,  7500,  7074,  1, 0, 0}, // Ham   40M   12
  {  "41M", SW_BAND_TYPE,  AM,  7200,  9000,  7210,  5, 0, 0}, //       41M   13
  {  "31M", SW_BAND_TYPE,  AM,  9000, 10000,  9600,  5, 0, 0}, //       31M   14
  {  "30M", SW_BAND_TYPE, USB, 10000, 10200, 10099,  1, 0, 0}, // Ham   30M   15
  {  "25M", SW_BAND_TYPE,  AM, 10200, 13500, 11700,  5, 0, 0}, //       25M   16
  {  "22M", SW_BAND_TYPE,  AM, 13500, 14000, 13700,  5, 0, 0}, //       22M   17
  {  "20M", SW_BAND_TYPE, USB, 14000, 14500, 14074,  1, 0, 0}, // Ham   20M   18
  {  "19M", SW_BAND_TYPE,  AM, 14500, 17500, 15700,  5, 0, 0}, //       19M   19
  {  "17M", SW_BAND_TYPE,  AM, 17500, 18000, 17600,  5, 0, 0}, //       17M   20
  {  "16M", SW_BAND_TYPE, USB, 18000, 18500, 18100,  1, 0, 0}, // Ham   16M   21
  {  "15M", SW_BAND_TYPE,  AM, 18500, 21000, 18950,  5, 0, 0}, //       15M   22
  {  "14M", SW_BAND_TYPE, USB, 21000, 21500, 21074,  1, 0, 0}, // Ham   14M   23
  {  "13M", SW_BAND_TYPE,  AM, 21500, 24000, 21500,  5, 0, 0}, //       13M   24
  {  "12M", SW_BAND_TYPE, USB, 24000, 25500, 24940,  1, 0, 0}, // Ham   12M   25
  {  "11M", SW_BAND_TYPE,  AM, 25500, 26100, 25800,  5, 0, 0}, //       11M   26
  {   "CB", SW_BAND_TYPE,  AM, 26100, 28000, 27200,  1, 0, 0}, // CB band     27
  {  "10M", SW_BAND_TYPE, USB, 28000, 30000, 28500,  1, 0, 0}, // Ham   10M   28
  {   "SW", SW_BAND_TYPE,  AM,   100, 30000, 15500,  5, 0, 0} // Whole SW    29
};

#define BAND_FM     0
#define BAND_LW     1
#define BAND_MW     2
#define BAND_800M   3
#define BAND_630M   4
#define BAND_160M   5
#define BAND_120M   6
#define BAND_90M    7
#define BAND_80M    8
#define BAND_75M    9
#define BAND_60M    10
#define BAND_49M    11
#define BAND_40M    12
#define BAND_41M    13
#define BAND_31M    14
#define BAND_30M    15
#define BAND_25M    16
#define BAND_22M    17
#define BAND_20M    18
#define BAND_19M    19
#define BAND_17M    20
#define BAND_16M    21
#define BAND_15M    22
#define BAND_14M    23
#define BAND_13M    24
#define BAND_12M    25
#define BAND_11M    26
#define BAND_CB     27
#define BAND_10M    28
#define BAND_SW     29
//======================================================= End THE Band Definitions     ========================

//======================================================= Tuning Digit selection ====================
typedef struct // Tuning digit
{
  uint8_t  digit;
  uint16_t Xdignumos; //Xoffset
  uint16_t Xdignumsr; //X size rectang
  uint16_t Ydignumos; //Yoffset
  uint16_t Ydignumsr; //Y size rectang
  uint16_t Xdignumnr; //X next rectang
} DigNum;

uint8_t Xdignum = 139;
uint8_t Ydignum = 25;

DigNum dn[] = {
  { 0 , Xdignum, 21, Ydignum, 35,  0},
  { 1 , Xdignum, 21, Ydignum, 35, 30},
  { 2 , Xdignum, 21, Ydignum, 35, 59}

};
//======================================================= End Tuning Digit selection     ===============================
const int lastBand      = (sizeof band / sizeof(Band)) - 1;
const int lastdignum    = (sizeof dn / sizeof(DigNum)) - 1;

uint16_t bandMode[(lastBand + 1)];

#define offsetEEPROM       32
#define EEPROM_SIZE        2048

struct StoreStruct {
  byte     chkDigit;
  byte     bandIdx;
  uint16_t Freq;
  uint8_t  currentMode;
  uint8_t  bwIdxSSB;
  uint8_t  bwIdxAM;
  uint8_t  bwIdxFM;
  uint8_t  ssIdxMW;
  uint8_t  ssIdxAM;
  uint8_t  ssIdxFM;
  int      currentBFO;
  int      currentBFOmanu;
  uint8_t  AGCgain;
  uint8_t  currentVOL;
  uint8_t  currentBFOStep;
  uint8_t  RDS;
  unsigned long FreqSI5351;
  uint16_t currentBrightness;
  uint8_t  currentAGCgain;
  unsigned long calibratvalSI5351;
  int  BFOLW;
  int  BFOMW;
  int  BFO600M;
  int  BFO630M;
  int  BFO160M;
  int  BFO120M;
  int  BFO90M;
  int  BFO80M;
  int  BFO75M;
  int  BFO60M;
  int  BFO49M;
  int  BFO40M;
  int  BFO41M;
  int  BFO31M;
  int  BFO30M;
  int  BFO25M;
  int  BFO22M;
  int  BFO20M;
  int  BFO19M;
  int  BFO17M;
  int  BFO16M;
  int  BFO15M;
  int  BFO15H;
  int  BFO13M;
  int  BFO12M;
  int  BFO11M;
  int  BFOCB;
  int  BFO10M;
  int  BFOSW;
  byte     chk4;
  uint16_t PresetId;
  uint8_t  currentPRES;
  uint16_t currentFreqRetro0;
  uint16_t currentFreqRetro1;
  uint16_t currentFreqRetro2;
  uint16_t currentFreqRetro3;
  uint16_t currentFreqRetro4;
  uint16_t currentFreqRetro5;
  uint16_t currentFreqRetro6;
  uint16_t currentFreqRetro7;
  uint16_t currentFreqRetro8;
  uint8_t  saverTime;
  uint8_t  RETROband;
  uint8_t  SCANscale;
  uint16_t boolOpt;
  byte    chk5;
  int     SquelchVal;
};

StoreStruct storage = {
  '@', //First time check
  0, //bandIdx
  10230, //Freq
  0, //mode
  1, //bwIdxSSB
  1, //bwIdxAM
  0, //bwIdxFM
  9, //ssIdxMW
  5, //ssIdxAM
  10, //ssIdxFM
  0, //currentBFO
  0, //currentBFOmanu
  0, //AGCgain
  45, //currentVOL
  25, //currentBFOStep
  1, //RDS
  3276800, //FreqSI5351
  0, //currentBrightness
  1, //currentAGCgain
  0, //calibratvalSI5351
  0, //BFOLW;
  0, //BFOMW
  0, //BFO600M
  0, //BFO630M
  0, //BFO160M
  0, //BFO120M
  0, //BFO90M
  0, //BFO80M
  0, //BFO75M
  0, //BFO60M
  0, //BFO49M
  0, //BFO40M
  0, //BFO41M
  0, //BFO31M
  0, //BFO30M
  0, //BFO25M
  0, //BFO22M
  0, //BFO20M
  0, //BFO19M
  0, //BFO17M
  0, //BFO16M
  0, //BFO15M
  0, //BFO15H
  0, //BFO13M
  0, //BFO12M
  0, //BFO11M
  0, //BFOCB
  0, //BFO10M
  0, //BFOSW

  '@', //V4 or higer bild first time check
  777, //PresetId
  0, //currentPRES
  8750, //currentFreqRetro0
  6400, //currentFreqRetro1
  153, //currentFreqRetro2
  522, //currentFreqRetro3
  2300, //currentFreqRetro4
  5900, //currentFreqRetro5
  9400, //currentFreqRetro6
  13570, //currentFreqRetro7
  18900, //currentFreqRetro8
  10, //saverTime
  0, //RETROband
  193, //SCANscale
  1181, //boolOpt
// ========================================== LWH
  '@', //V5 or higher build first time check
  0, // SquelchVal
// ==========================================
};
//MEMO BANK===============================================================
#define offsetMemoEEPROM       248

typedef struct // MemoBank data
{
  uint16_t        freq;
  uint8_t         band;
  char            Name[21];
} MemoryBank;
MemoryBank MemoBank[75]; // 75 slots for Memory Station
const int lastMemoBank = (sizeof MemoBank / sizeof(MemoryBank)) - 1;
Preset preset [lastMemory + lastMemoBank + 1];

typedef struct // MemoBank data for Memory.h file
{
  uint16_t        freq;
  uint8_t         band;
  uint8_t         mode;
  char           *Name;
} MemoryBankFile;
#include "Memory.h"
const int lastMemoBankFile = (sizeof MemoBankFile / sizeof(MemoryBankFile)) - 1;
// ============================================================================
uint8_t rssi = 0;

uint8_t volume = DEFAULT_VOLUME;

Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

TFT_eSPI tft    = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
SI4735 si4735; // PU2CLR_SI4735 library

#ifdef IhaveSI5351
Si5351wire si5351wire;
#endif

// ============================================================================
void IRAM_ATTR RotaryEncFreq() {
// ============================================================================
  if (!writingEeprom) {
    encoderStatus = encoder.process();
    if (encoderStatus) {
      if (encoderStatus == DIR_CW) encoderCount = 1; else encoderCount = -1; // Direction clockwise
    }
  }
}

// ============================================================================

// ============================================================================
// PACKING / OPTIONS / STOCKAGE
// ============================================================================
void OPTpack() {
// ============================================================================
  boolOpt = 0;
  boolOpt += digitLigth   * 1;
  boolOpt += batShow      * 2;
  boolOpt += langRetroEN  * 4;
  boolOpt += beeperOn     * 8;
  boolOpt += VHFon        * 16;
  boolOpt += loadMemory   * 32;
  boolOpt += memoPreset   * 64;
  boolOpt += saverOn      * 128;
  boolOpt += screenV      * 256;
  boolOpt += displayOff   * 512;
  boolOpt += SCANaccuracy * 1024;
  boolOpt += displayPower * 2048;
  boolOpt += RDSalways    * 4096;
  boolOpt += seekAccuracy * 8192;
}

// ============================================================================
void OPTunpack() {
// ============================================================================
  digitLigth    = bool((boolOpt >>  0) & 1);
  batShow       = bool((boolOpt >>  1) & 1);
  langRetroEN   = bool((boolOpt >>  2) & 1);
  beeperOn      = bool((boolOpt >>  3) & 1);
  VHFon         = bool((boolOpt >>  4) & 1);
  loadMemory    = bool((boolOpt >>  5) & 1);
  memoPreset    = bool((boolOpt >>  6) & 1);
  saverOn       = bool((boolOpt >>  7) & 1);
  screenV       = bool((boolOpt >>  8) & 1);
  displayOff    = bool((boolOpt >>  9) & 1);
  SCANaccuracy  = bool((boolOpt >> 10) & 1);
  displayPower  = bool((boolOpt >> 11) & 1);
  RDSalways     = bool((boolOpt >> 12) & 1);
  seekAccuracy  = bool((boolOpt >> 13) & 1);
}

// ============================================================================
void scanOPTpack() {
// ============================================================================
  SCANscale = uint8_t((autoSCANstep * 128) + (maxSCANstep * 8) + (minSCANstep * 8));
  if (countScanSignal == 3) SCANaccuracy = true; else SCANaccuracy = false;
}

// ============================================================================
void scanOPTunpack() {
// ============================================================================
  if (SCANscale > 128) autoSCANstep = true; else autoSCANstep = false;
  minSCANstep = float(SCANscale & 0x07) / 8;
  maxSCANstep = float(SCANscale & 0x78) / 8;
  if (SCANaccuracy) countScanSignal = 3; else countScanSignal = 1;
}

// ============================================================================

// ============================================================================
// WIFI / IDENTIFIANTS / NTP
// ============================================================================
void loadWifiCredentials() {
  atsPreferences.begin("atswifi", true);
  WIFI_SSID = atsPreferences.getString("ssid", "");
  WIFI_PASS = atsPreferences.getString("pass", "");
  atsPreferences.end();

  if (WIFI_SSID.length() > 0) {
    Serial.print("WiFi credentials loaded for SSID: ");
    Serial.println(WIFI_SSID);
  } else {
    Serial.println("No saved WiFi credentials.");
  }
}

bool saveWifiCredentials() {
  if (WIFI_SSID.length() == 0)
    return false;

  if (!atsPreferences.begin("atswifi", false))
    return false;

  atsPreferences.clear();
  size_t ssidWritten = atsPreferences.putString("ssid", WIFI_SSID);
  size_t passWritten = atsPreferences.putString("pass", WIFI_PASS);
  atsPreferences.end();

  if (ssidWritten == 0 || passWritten == 0)
    return false;

  if (!atsPreferences.begin("atswifi", true))
    return false;

  String savedSsid = atsPreferences.getString("ssid", "");
  String savedPass = atsPreferences.getString("pass", "");
  atsPreferences.end();

  return (savedSsid == WIFI_SSID) && (savedPass == WIFI_PASS);
}

void clearWifiCredentials() {
  atsPreferences.begin("atswifi", false);
  atsPreferences.clear();
  atsPreferences.end();
  WIFI_SSID = "";
  WIFI_PASS = "";
  WiFi.disconnect(true);
}

void serviceWifiConnection() {
  if (!wifiConnectPending)
    return;

  wl_status_t st = WiFi.status();

  static int lastWifiDiagStatus = -999;
  if ((int)st != lastWifiDiagStatus) {
    Serial.print(F("WIFI DIAG: status="));
    Serial.print((int)st);
    Serial.print(F(" ("));
    Serial.print(wifiStatusName(st));
    Serial.println(')');
    lastWifiDiagStatus = (int)st;
  }

  if (st == WL_CONNECTED) {
    wifiConnectPending = false;

    Serial.println(F("WiFi connected."));
    Serial.print(F("IP: "));
    Serial.println(WiFi.localIP());

    startAtsTcpServer();
    initTime();
    returnLayer();
    return;
  }

  if ((millis() - wifiConnectStarted) >= WIFI_CONNECT_TIMEOUT_MS) {
    wifiConnectPending = false;
    Serial.print(F("WiFi connection timeout, status="));
    Serial.print((int)st);
    Serial.print(F(" ("));
    Serial.print(wifiStatusName(st));
    Serial.println(')');
    Serial.println(F("WIFI DIAG: see preceding WIFI EVENT reason for exact failure"));
    WiFi.disconnect(false, false);
    returnLayer();
  }
}

void sendWifiStatus(Print &out) {
  out.print("WIFI,STATE=");
  if (WiFi.status() == WL_CONNECTED) {
    out.print("CONNECTED,SSID=");
    out.print(WiFi.SSID());
    out.print(",IP=");
    out.print(WiFi.localIP());
    out.print(",RSSI=");
    out.println(WiFi.RSSI());
  } else if (wifiConnectPending) {
    out.print("CONNECTING,SSID=");
    out.print(WIFI_SSID);
    out.println(",IP=0.0.0.0,RSSI=0");
  } else {
    out.print("DISCONNECTED,SSID=");
    out.print(WIFI_SSID);
    out.println(",IP=0.0.0.0,RSSI=0");
  }
}

const char* wifiStatusName(wl_status_t st) {
  switch (st) {
    case WL_IDLE_STATUS:     return "IDLE";
    case WL_NO_SSID_AVAIL:   return "NO_SSID";
    case WL_SCAN_COMPLETED:  return "SCAN_DONE";
    case WL_CONNECTED:       return "CONNECTED";
    case WL_CONNECT_FAILED:  return "CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "CONNECTION_LOST";
    case WL_DISCONNECTED:    return "DISCONNECTED";
    default:                 return "UNKNOWN";
  }
}

const char* wifiEncryptionName(uint8_t enc) {
  switch (enc) {
    case WIFI_AUTH_OPEN:         return "OPEN";
    case WIFI_AUTH_WEP:          return "WEP";
    case WIFI_AUTH_WPA_PSK:      return "WPA";
    case WIFI_AUTH_WPA2_PSK:     return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
#ifdef WIFI_AUTH_WPA3_PSK
    case WIFI_AUTH_WPA3_PSK:     return "WPA3";
#endif
#ifdef WIFI_AUTH_WPA2_WPA3_PSK
    case WIFI_AUTH_WPA2_WPA3_PSK:return "WPA2/WPA3";
#endif
    default:                     return "OTHER";
  }
}

bool diagnoseTargetWifi() {
  Serial.print(F("WIFI DIAG: scanning for SSID '"));
  Serial.print(WIFI_SSID);
  Serial.println('\'');

  WiFi.mode(WIFI_STA);
  int count = WiFi.scanNetworks(false, true);

  if (count <= 0) {
    Serial.println(F("WIFI DIAG: no networks found"));
    WiFi.scanDelete();
    return false;
  }

  bool found = false;
  for (int i = 0; i < count; i++) {
    if (WiFi.SSID(i) == WIFI_SSID) {
      found = true;
      Serial.print(F("WIFI DIAG: FOUND SSID="));
      Serial.print(WiFi.SSID(i));
      Serial.print(F(" RSSI="));
      Serial.print(WiFi.RSSI(i));
      Serial.print(F(" CH="));
      Serial.print(WiFi.channel(i));
      Serial.print(F(" AUTH="));
      Serial.println(wifiEncryptionName((uint8_t)WiFi.encryptionType(i)));
    }
  }

  if (!found)
    Serial.println(F("WIFI DIAG: TARGET SSID NOT FOUND"));

  WiFi.scanDelete();
  return found;
}

const char* wifiDisconnectReasonName(uint8_t reason) {
  switch (reason) {
    case 2:   return "AUTH_EXPIRE";
    case 3:   return "AUTH_LEAVE";
    case 4:   return "ASSOC_EXPIRE";
    case 8:   return "ASSOC_LEAVE";
    case 15:  return "4WAY_HANDSHAKE_TIMEOUT";
    case 23:  return "8021X_AUTH_FAILED";
    case 24:  return "CIPHER_SUITE_REJECTED";
    case 200: return "BEACON_TIMEOUT";
    case 201: return "NO_AP_FOUND";
    case 202: return "AUTH_FAIL";
    case 203: return "ASSOC_FAIL";
    case 204: return "HANDSHAKE_TIMEOUT";
    case 205: return "CONNECTION_FAIL";
    default:  return "OTHER";
  }
}

void onWifiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  uint8_t reason = info.wifi_sta_disconnected.reason;

  Serial.print(F("WIFI EVENT: DISCONNECTED reason="));
  Serial.print(reason);
  Serial.print(F(" ("));
  Serial.print(wifiDisconnectReasonName(reason));
  Serial.println(')');
}

void installWifiDiagnostics() {
  static bool installed = false;
  if (installed)
    return;

  WiFi.onEvent(onWifiDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  installed = true;
  Serial.println(F("WIFI DIAG: disconnect reason handler installed"));
}

void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnectPending = false;
    return;
  }

  if (WIFI_SSID.length() == 0) {
    Serial.println(F("WiFi not configured - missing SSID"));
    wifiConnectPending = false;
    return;
  }

  Serial.print(F("Connecting to WiFi SSID: "));
  Serial.println(WIFI_SSID);

  installWifiDiagnostics();

  WiFi.disconnect(true, true);
  delay(200);
  WiFi.mode(WIFI_STA);
  delay(100);

  diagnoseTargetWifi();

  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());

  wifiConnectStarted = millis();
  wifiConnectPending = true;

  Serial.print(F("WIFI DIAG: begin status="));
  Serial.print((int)WiFi.status());
  Serial.print(F(" ("));
  Serial.print(wifiStatusName(WiFi.status()));
  Serial.println(')');
}

void initTime() {
  // NTP avec serveurs de secours.
  // L'ESP32 essaie les serveurs dans l'ordre indique.
  configTime(
    gmtOffset_sec,
    daylightOffset_sec,
    "pool.ntp.org",
    "time.google.com",
    "time.cloudflare.com"
  );

  Serial.println(F("NTP: pool.ntp.org / time.google.com / time.cloudflare.com"));
}

// ============================================================================

// ============================================================================
// INITIALISATION - SETUP
// ============================================================================
// ============================================================================
// PETITS HELPERS COMMUNS - NIVEAU 5
// ============================================================================

inline bool isSSBLikeMode() {
  return (currentMode == LSB || currentMode == USB || currentMode == CW);
}

inline void applyCurrentBFO() {
  si4735.setSSBBfo(currentBFO + currentBFOmanu);
}

inline void restoreMuteIfNeeded() {
  if (Mutestat)
    si4735.setAudioMute(audioMuteOn);
}

inline void clearScanMarks() {
  for (int i = 0; i < 320; i++)
    ScanMark[i] = false;
}

inline void pauseScanIfNeeded() {
  if (!SCANpause) {
    SCANpause = true;
    pauseSCAN();
  }
}

void setup() {
  //=======================================================================================
  Serial.begin(115200);

  // Bluetooth Classic SPP : meme protocole de commandes que USB/Wi-Fi
  if (SerialBT.begin("ATS-25X2")) {
    atsBluetoothStarted = true;
    Serial.println("Bluetooth SPP ready: ATS-25X2");
  } else {
    Serial.println("Bluetooth SPP start failed");
  }
  Serial.println(F("ATS LAB Firmware V6.0 - O.H.M"));

/*  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
  }
  tft.println("");
  tft.println("WiFi connected."); */

  pinMode(DISPLAY_LED, OUTPUT);
  pinMode(BEEPER, OUTPUT);
  digitalWrite(DISPLAY_LED, 0);

  ledcSetup(LedChannelforTFT, LedFreq, LedResol);
  ledcAttachPin(DISPLAY_LED, LedChannelforTFT);

  int16_t si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);
  Beep(1, 200);

 tft.init();

  tft.setRotation(1);
  uint16_t calData[5] = { 387, 3530, 246, 3555, 7 };
  tft.setTouch(calData);

#ifdef IhaveSI5351
  si5351wire.output_enable(CLK_Xtal, 1);
  if (si5351wire.init(SI5351wire_CRYSTAL_LOAD_8PF, CLK_Xtal, 0) == false)
  {
    Serial.println ( "SI5351 not found" );
  }
  si5351wire.set_correction(0, SI5351wire_PLL_INPUT_XO);
  si5351wire.set_freq(FreqSI5351, CLK_Xtal);
#endif

  if (!EEPROM.begin(EEPROM_SIZE))
  {
    while (1) {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.println(F("failed to initialise EEPROM"));
        Serial.println(F("failed to initialise EEPROM"));
        delay(1000);
    }
  }

  tft.fillScreen(TFT_BLACK);

  // Charge les identifiants memorises, mais ne lance plus de connexion
  // automatiquement au demarrage. ATS LAB pilotera WIFI_CONNECT.
  loadWifiCredentials();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
  wifiConnectPending = false;

  delay(1500);
  tft.setCursor(20, 10); //(7, 50);
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);

  Serial.println("     SI4735/32 Radio");
  Serial.println("Version 6.0 ATS LAB / O.H.M 17-08-2026");
  Serial.println(F("*** V6.0 ATS LAB - STABLE ***"));
  Serial.println(F("*** WiFi status mirrored on TFT ***"));
  Serial.println(F("*** V1.1 RDS PC: PS/RT/PI/PTY/CT ***"));
  Serial.println(F("*** TFT RESTORE FIX after Wi-Fi messages ***"));

  tft.pushImage(0, 0,
    OHM_STARTUP_LOGO_WIDTH,
    OHM_STARTUP_LOGO_HEIGHT,
    (uint16_t *)ohmStartupLogo);

  delay(5000);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 180);
  tft.println("TAP & HOLD ON DISPLAY FOR ROTATE SCREEN");
  tft.setTextSize(2);
  tft.setCursor(20, 130); //(7, 170);
  delay(2500);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  if ( si4735Addr == 0 ) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.print("Si4735 not detected");
    Serial.println("Si4735 not detected");
    while (1);
  } else {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("Si473X addr :  ");
    tft.println(si4735Addr, HEX);
  }

  delay(1500);

  if (EEPROM.read(offsetEEPROM) != storage.chkDigit || analogRead(ENCODER_SWITCH) < 500) {
    ErrorBeep();
    Serial.println(F("Writing defaults...."));
    saveConfig();
  }
  loadConfig();
  printConfig();

  pinMode(ENCODER_PIN_A , INPUT_PULLUP); //Rotary encoder Freqency/bfo/preset
  pinMode(ENCODER_PIN_B , INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), RotaryEncFreq, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), RotaryEncFreq, CHANGE);
  si4735.setAudioMuteMcuPin(AUDIO_MUTE);

  for (int i = 0; i <= lastBand; i++) bandMode[i] = band[i].prefmod;

  if (si4735Addr == 17)
  {
    si4735.setDeviceI2CAddress(0);
  }
  else
  {
    si4735.setDeviceI2CAddress(1);
  }

  bandIdx                   =  storage.bandIdx;
  band[bandIdx].currentFreq =  storage.Freq;
  currentMode               =  storage.currentMode;
  bwIdxSSB                  =  storage.bwIdxSSB; // band width
  bwIdxAM                   =  storage.bwIdxAM;
  bwIdxFM                   =  storage.bwIdxFM;
  ssIdxMW                   =  storage.ssIdxMW; // step size
  ssIdxAM                   =  storage.ssIdxAM;
  ssIdxFM                   =  storage.ssIdxFM;
  currentBFO                =  storage.currentBFO;
  currentBFOmanu            =  storage.currentBFOmanu;
  AGCgain                   =  storage.AGCgain;
  currentVOL                =  storage.currentVOL;
  currentBFOStep            =  storage.currentBFOStep;
  RDS                       =  storage.RDS;
  FreqSI5351                =  storage.FreqSI5351;
  currentBrightness         =  storage.currentBrightness;
  currentAGCgain            =  storage.currentAGCgain;
  calibratvalSI5351         =  storage.calibratvalSI5351;
  band[ 1].lastmanuBFO      =  storage.BFOLW;
  band[ 2].lastmanuBFO      =  storage.BFOMW;
  band[ 3].lastmanuBFO      =  storage.BFO600M;
  band[ 4].lastmanuBFO      =  storage.BFO630M;
  band[ 5].lastmanuBFO      =  storage.BFO160M;
  band[ 6].lastmanuBFO      =  storage.BFO120M;
  band[ 7].lastmanuBFO      =  storage.BFO90M;
  band[ 8].lastmanuBFO      =  storage.BFO80M;
  band[ 9].lastmanuBFO      =  storage.BFO75M;
  band[10].lastmanuBFO      =  storage.BFO60M;
  band[11].lastmanuBFO      =  storage.BFO49M;
  band[12].lastmanuBFO      =  storage.BFO40M;
  band[13].lastmanuBFO      =  storage.BFO41M;
  band[14].lastmanuBFO      =  storage.BFO31M;
  band[15].lastmanuBFO      =  storage.BFO30M;
  band[16].lastmanuBFO      =  storage.BFO25M;
  band[17].lastmanuBFO      =  storage.BFO22M;
  band[18].lastmanuBFO      =  storage.BFO20M;
  band[19].lastmanuBFO      =  storage.BFO19M;
  band[20].lastmanuBFO      =  storage.BFO17M;
  band[21].lastmanuBFO      =  storage.BFO16M;
  band[22].lastmanuBFO      =  storage.BFO15M;
  band[23].lastmanuBFO      =  storage.BFO15H;
  band[24].lastmanuBFO      =  storage.BFO13M;
  band[25].lastmanuBFO      =  storage.BFO12M;
  band[26].lastmanuBFO      =  storage.BFO11M;
  band[27].lastmanuBFO      =  storage.BFOCB;
  band[28].lastmanuBFO      =  storage.BFO10M;
  band[29].lastmanuBFO      =  storage.BFOSW;

  if (storage.chk4 == '@') {
    PresetId                  =  storage.PresetId;
    currentPRES               =  storage.currentPRES;
    bandRetro[0].currentFreq  =  float(storage.currentFreqRetro0) / 100;
    bandRetro[1].currentFreq  =  float(storage.currentFreqRetro1) / 100;
    bandRetro[2].currentFreq  =  float(storage.currentFreqRetro2);
    bandRetro[3].currentFreq  =  float(storage.currentFreqRetro3);
    bandRetro[4].currentFreq  =  float(storage.currentFreqRetro4);
    bandRetro[5].currentFreq  =  float(storage.currentFreqRetro5);
    bandRetro[6].currentFreq  =  float(storage.currentFreqRetro6);
    bandRetro[7].currentFreq  =  float(storage.currentFreqRetro7);
    bandRetro[8].currentFreq  =  float(storage.currentFreqRetro8);
    saverTime                 =  storage.saverTime;
    RETROband                 =  storage.RETROband;
    SCANscale                 =  storage.SCANscale;
    boolOpt                   =  storage.boolOpt;
  } else {
    storage.chk4              =  '@';
    PresetId                  =  777;
    currentPRES               =  0;
    bandRetro[0].currentFreq  =  8750;
    bandRetro[1].currentFreq  =  6400;
    bandRetro[2].currentFreq  =  153;
    bandRetro[3].currentFreq  =  522;
    bandRetro[4].currentFreq  =  2300;
    bandRetro[5].currentFreq  =  5900;
    bandRetro[6].currentFreq  =  9400;
    bandRetro[7].currentFreq  =  13570;
    bandRetro[8].currentFreq  =  18900;
    saverTime                 =  10;
    RETROband                 =  0;
    SCANscale                 =  193;
    boolOpt                   =  1181;
  }
// =============================================== LWH
  if (storage.chk5 == '@') {
    currentSquelch =  storage.SquelchVal;
  } else {
    storage.chk5              = '@';
    storage.SquelchVal        = 0;
  }
// ===============================================
  OPTunpack();
  scanOPTunpack();

  pressed = tft.getTouch(&x, &y);
  if (pressed) {
    ErrorBeep();
    screenV = !screenV;
  }
  screenRotate();

  if (VHFon) band[0].minimumFreq = 6400; else band[0].minimumFreq = 8750;

  for (int i = 0; i <= lastMemoBank; i++) {
    if (i > lastMemoBankFile) {
      MemoBank[i].freq = 0;
      MemoBank[i].band = 0;
      for (int j = 0; j < 21; j++) MemoBank[i].Name[j] = char(32);
    } else {
      MemoBank[i].freq = MemoBankFile[i].freq;
      MemoBank[i].band = MemoBankFile[i].band + (MemoBankFile[i].mode * 32);
      int n = 0;
      int j = 0;
      while (MemoBankFile[i].Name[j] != NULL && n < 21) {
        if (char(MemoBankFile[i].Name[j]) < 208) {
          MemoBank[i].Name[n] = MemoBankFile[i].Name[j];
          n++;
        }
        j++;
      }
      if (n < 21) for (int j = n; j < 21; j++) MemoBank[i].Name[j] = char(32);
    }
  }
  if (loadMemory) {
    saveMemo();
    loadMemory = false;
  }
  loadMemo();

#ifdef IhaveCrystal
  if (bandIdx == 0) si4735.setup(RESET_PIN, FM_BAND_TYPE); //Start in FM
  else si4735.setup(RESET_PIN, 1);
  if (bandIdx != 0) si4735.setAM(); // Start in AM
#endif

#ifdef IhaveSI5351
  si5351wire.set_freq(FreqSI5351, CLK_Xtal);
  si4735.setRefClock(32768);
  si4735.setRefClockPrescaler(1); // will work with 32768 Hz
  if (bandIdx == 0)  si4735.setup(RESET_PIN, -1, POWER_UP_FM, SI473X_ANALOG_AUDIO, XOSCEN_RCLK); // Start in FM
  else si4735.setup(RESET_PIN, -1, POWER_UP_AM, SI473X_ANALOG_AUDIO, XOSCEN_RCLK); // Start in AM
  if (bandIdx != 0) si4735.setAM();
#endif

  if (!displayPower) ledcWrite(LedChannelforTFT, currentBrightness);
  freqstep = 1000; //hz
  previousBFO = -1;
  band[bandIdx].lastBFO  = currentBFO;
  freqDec = currentBFO;
  band[bandIdx].prefmod = currentMode;
  si4735.setVolume(currentVOL);
  previousVOL = currentVOL;
  previousAGCgain = currentAGCgain;
  BandSet();
  currentFrequency = previousFrequency = band[bandIdx].currentFreq;
  Beep(2, 200);
  encBut = 600;
  x = y = 0;
  DrawFila();
  si4735.setSeekFmSpacing(10);
  si4735.setSeekFmLimits(band[0].minimumFreq, band[0].maximumFreq);
  si4735.setSeekAmRssiThreshold(50);
  si4735.setSeekAmSrnThreshold(20);
  si4735.setSeekFmRssiThreshold(5);
  si4735.setSeekFmSrnThreshold(5);
  xTaskCreate(SaveInEeprom, "SaveInEeprom", 2048, NULL, 1, NULL);
  delay(10);

  commStuff();
} // end setup

// ============================================================================
void drawProgress(uint8_t percentage, String text) {
// ============================================================================
  spr.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(text, 60, 198);
  tft.drawRect(10, 208, 320 - 20, 15, TFT_WHITE);
  tft.fillRect(12, 210, 296 * percentage / 100, 12, TFT_BLUE);

}

// ============================================================================

// ============================================================================
// EEPROM / MEMOIRES / CONFIGURATION
// ============================================================================
void SaveInEeprom (void* arg)  {
// ============================================================================
  while (1) {
    OPTpack();
    scanOPTpack();

    storage.bandIdx           = bandIdx;
    storage.Freq              = band[bandIdx].currentFreq;
    storage.currentMode       = currentMode;
    storage.bwIdxSSB          = bwIdxSSB;
    storage.bwIdxAM           = bwIdxAM;
    storage.bwIdxFM           = bwIdxFM;
    storage.ssIdxMW           = ssIdxMW;
    storage.ssIdxAM           = ssIdxAM;
    storage.ssIdxFM           = ssIdxFM;
    storage.currentBFO        = currentBFO;
    storage.currentBFOmanu    = currentBFOmanu;
    storage.AGCgain           = AGCgain;
    storage.currentVOL        = currentVOL;
    storage.currentBFOStep    = currentBFOStep;
    storage.RDS               = RDS;
    storage.FreqSI5351        = FreqSI5351;
    storage.currentBrightness = currentBrightness;
    storage.currentAGCgain    = currentAGCgain;
    storage.calibratvalSI5351 = calibratvalSI5351;
    storage.BFOLW = band[1].lastmanuBFO;
    storage.BFOMW = band[2].lastmanuBFO;
    storage.BFO600M = band[3].lastmanuBFO;
    storage.BFO630M = band[4].lastmanuBFO;
    storage.BFO160M = band[5].lastmanuBFO;
    storage.BFO120M = band[6].lastmanuBFO;
    storage.BFO90M = band[7].lastmanuBFO;
    storage.BFO80M = band[8].lastmanuBFO;
    storage.BFO75M = band[9].lastmanuBFO;
    storage.BFO60M = band[10].lastmanuBFO;
    storage.BFO49M = band[11].lastmanuBFO;
    storage.BFO40M = band[12].lastmanuBFO;
    storage.BFO41M = band[13].lastmanuBFO;
    storage.BFO31M = band[14].lastmanuBFO;
    storage.BFO30M = band[15].lastmanuBFO;
    storage.BFO25M = band[16].lastmanuBFO;
    storage.BFO22M = band[17].lastmanuBFO;
    storage.BFO20M = band[18].lastmanuBFO;
    storage.BFO19M = band[19].lastmanuBFO;
    storage.BFO17M = band[20].lastmanuBFO;
    storage.BFO16M = band[21].lastmanuBFO;
    storage.BFO15M = band[22].lastmanuBFO;
    storage.BFO15H = band[23].lastmanuBFO;
    storage.BFO13M = band[24].lastmanuBFO;
    storage.BFO12M = band[25].lastmanuBFO;
    storage.BFO11M = band[26].lastmanuBFO;
    storage.BFOCB  = band[27].lastmanuBFO;
    storage.BFO10M = band[28].lastmanuBFO;
    storage.BFOSW  = band[29].lastmanuBFO;
    storage.PresetId = PresetId;
    storage.currentPRES = currentPRES;
    storage.currentFreqRetro0 = uint16_t(bandRetro[0].currentFreq * 100);
    storage.currentFreqRetro1 = uint16_t(bandRetro[1].currentFreq * 100);
    storage.currentFreqRetro2 = uint16_t(bandRetro[2].currentFreq);
    storage.currentFreqRetro3 = uint16_t(bandRetro[3].currentFreq);
    storage.currentFreqRetro4 = uint16_t(bandRetro[4].currentFreq);
    storage.currentFreqRetro5 = uint16_t(bandRetro[5].currentFreq);
    storage.currentFreqRetro6 = uint16_t(bandRetro[6].currentFreq);
    storage.currentFreqRetro7 = uint16_t(bandRetro[7].currentFreq);
    storage.currentFreqRetro8 = uint16_t(bandRetro[8].currentFreq);
    storage.saverTime = saverTime;
    storage.RETROband = RETROband;
    storage.SCANscale = SCANscale;
    storage.boolOpt = boolOpt;
      storage.SquelchVal = currentSquelch; //LWH

    for (unsigned int t = 0; t < sizeof(storage); t++) {
      delay(1);
      if (EEPROM.read(offsetEEPROM + t) != *((char*)&storage + t)) {
        delay(1);
        EEPROM.write(offsetEEPROM + t, *((char*)&storage + t));
      }
    }

    for (unsigned int t = 0; t < sizeof(MemoBank); t++) {
      delay(1);
      if (EEPROM.read(offsetMemoEEPROM + t) != *((char*)&MemoBank + t)) {
        delay(1);
        EEPROM.write(offsetMemoEEPROM + t, *((char*)&MemoBank + t));
      }
    }

    writingEeprom = true;
    EEPROM.commit();
    writingEeprom = false;
    vTaskDelay(5000 / portTICK_RATE_MS);
  }

}

// ============================================================================
void saveMemo() {
// ============================================================================
  delay(10);
  for (unsigned int t = 0; t < sizeof(MemoBank); t++) {
    if (EEPROM.read(offsetMemoEEPROM + t) != *((char*)&MemoBank + t)) {
      EEPROM.write(offsetMemoEEPROM + t, *((char*)&MemoBank + t));
    }
  }
  EEPROM.commit();
}

// ============================================================================
void loadMemo() {
// ============================================================================
  if (EEPROM.read(offsetEEPROM + 0) == storage.chkDigit) {
    for (unsigned int t = 0; t < sizeof(MemoBank); t++)
      *((char*)&MemoBank + t) = EEPROM.read(offsetMemoEEPROM + t);
    Serial.println("Load config done");
  }
}

// ============================================================================
void saveConfig() {
// ============================================================================
  delay(10);
  for (unsigned int t = 0; t < sizeof(storage); t++) {
    if (EEPROM.read(offsetEEPROM + t) != *((char*)&storage + t)) {
      EEPROM.write(offsetEEPROM + t, *((char*)&storage + t));
    }
  }
  EEPROM.commit();
}

// ============================================================================
void loadConfig() {
// ============================================================================
  if (EEPROM.read(offsetEEPROM + 0) == storage.chkDigit) {
    for (unsigned int t = 0; t < sizeof(storage); t++)
      *((char*)&storage + t) = EEPROM.read(offsetEEPROM + t);
    Serial.println("Load config done");
  }
}

// ============================================================================
void printConfig() {
// ============================================================================
  unsigned int x;
  Serial.print("Storage size = ");
  Serial.println(sizeof(storage));
  Serial.println("Storage contents - ");
  if (EEPROM.read(offsetEEPROM) == storage.chkDigit) {
    for (unsigned int t = 0; t < sizeof(storage); t++) {
      x = EEPROM.read(offsetEEPROM + t);
      Serial.print(" 0x");
      if (x < 16) {
        Serial.print("0");
      }
      Serial.print(x,HEX);
      if (((t+1) % 8) == 0) {
        Serial.print("  ");
      }
      if (((t+1) % 16) == 0) {
        Serial.println();
      }
    }
    Serial.println();
  }
}

// ============================================================================

// ============================================================================
// RADIO - BANDES / MODES / FILTRES
// ============================================================================
void BandSet()  {
// ============================================================================
  if (bandIdx == 0) currentMode = FM; // only mod FM in FM band
  if ((currentMode == AM) || (currentMode == FM)) {
    ssbLoaded = false;
  }
  if ((currentMode == LSB) ||  (currentMode == USB))
  {
    if (ssbLoaded == false) {
      loadSSB();
    }
  }
  useBand();
  setBandWidth();
  checkAGC();
}

// ============================================================================
void useBand()  {
// ============================================================================
  if ((band[bandIdx].bandType == MW_BAND_TYPE) || (band[bandIdx].bandType == LW_BAND_TYPE)) {
    band[bandIdx].currentStep = ssIdxMW;
  }
  if (band[bandIdx].bandType == SW_BAND_TYPE) {
    band[bandIdx].currentStep = ssIdxAM;
  }
  if (band[bandIdx].bandType == FM_BAND_TYPE)
  {
    bfoOn = false;
    si4735.setTuneFrequencyAntennaCapacitor(0);
    delay(100);
    band[bandIdx].currentStep = ssIdxFM;
    si4735.setFM(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, band[bandIdx].currentStep);
    si4735.setFMDeEmphasis(1);
    ssbLoaded = false;
    si4735.RdsInit();
    si4735.setRdsConfig(1, 2, 2, 2, 2);
  }
  else
  {
    if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
      si4735.setTuneFrequencyAntennaCapacitor(0);
    } else {
      si4735.setTuneFrequencyAntennaCapacitor(1);
    }
    if (ssbLoaded)
    {
      si4735.setSSB(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, band[bandIdx].currentStep, currentMode);
      applyCurrentBFO();
      int temp = 1; // SSB ONLY 1KHz stepsize
      si4735.setFrequencyStep(temp);
      band[bandIdx].currentStep = temp;
    }
    else
    {
      si4735.setAM(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq, band[bandIdx].currentFreq, band[bandIdx].currentStep);
      bfoOn = false;
    }
  }
  delay(100);

} // end useband

// ============================================================================
void setBandWidth()  {
// ============================================================================
  if (currentMode == LSB || currentMode == USB)
  {
    si4735.setSSBAudioBandwidth(bwIdxSSB);
    if (bwIdxSSB == 0 || bwIdxSSB == 4 || bwIdxSSB == 5)
      si4735.setSBBSidebandCutoffFilter(0);
    else
      si4735.setSBBSidebandCutoffFilter(1);
  }
  if (currentMode == AM)
  {
    si4735.setBandwidth(bwIdxAM, 0);
  }
  if (currentMode == FM)
  {
    si4735.setFmBandwidth(bwIdxFM);
  }
}

// ============================================================================
void loadSSB()  {
// ============================================================================
  si4735.reset();
  si4735.queryLibraryId(); // Is it really necessary here? I will check it.
  si4735.patchPowerUp();
  delay(50);
  si4735.setI2CFastMode(); // Recommended
  si4735.downloadPatch(ssb_patch_content, size_content);
  si4735.setI2CStandardMode(); // goes back to default (100KHz)

  si4735.setSSBConfig(bwIdxSSB, 1, 0, 1, 0, 1);
  delay(25);
  ssbLoaded = true;
}

// ============================================================================
void Freqcalq(int keyval)  {
// ============================================================================
  if (keyval > 11) {
    tft.fillRect(0, 80, 240, 40, TFT_NAVY);
    if (keyval == 12) {
      if (fact == 1) DisplayfreqNew = float(int(DisplayfreqNew / 100)) * 10;
      if (fact == 10) {
        Decipoint = false;
        fact = 1;
      }
      if (fact == 100) {
        dpfrq = 0;
        fact = 10;
      }
      if (fact == 1000) {
        dpfrq = float(int(dpfrq * 10)) / 10;
        fact = 100;
      }
    } else {
      DisplayfreqNew = 0;
      dpfrq = 0;
      fact = 1;
      Decipoint = false;
    }
  } else {
    if (Decipoint) {
      if (keyval < 10 && fact < 1000) {
        dpfrq = dpfrq + keyval / fact;
        fact = fact * 10;
      }
    } else {
      if ((DisplayfreqNew + keyval) <= 30000) DisplayfreqNew = (DisplayfreqNew + keyval) * 10;
    }
  }
  tft.setTextDatum(BL_DATUM);
  tft.setTextSize(3);
  float realFreq = (DisplayfreqNew / 10) + dpfrq;
  if ((realFreq > 0 && realFreq < 31) || (realFreq >= 64 && realFreq <= 108) || (realFreq > 152 && realFreq <= 30000)) tft.setTextColor(TFT_WHITE, TFT_NAVY); else tft.setTextColor(TFT_RED, TFT_NAVY);
  tft.setCursor(10, 90);
  String hhz = "    ";
  if (DisplayfreqNew > 0) {
    if (Decipoint) {
      hhz = " MHz";
      if (fact == 10) {
        tft.print((DisplayfreqNew / 10), 0);
        tft.print(".");
      }
      if (fact == 100) tft.print(((DisplayfreqNew / 10) + dpfrq), 1);
      if (fact == 1000) tft.print(((DisplayfreqNew / 10) + dpfrq), 2);
    } else {
      int khz = trunc(DisplayfreqNew / 10000);
      int hz = (DisplayfreqNew - (khz * 10000)) / 10;
      char s[6] = {'\0'};
      if (realFreq < 153) {
        sprintf(s, "%i", hz);
        if ((realFreq > 0 && realFreq < 31) || (realFreq >= 64 && realFreq <= 108)) hhz = " MHz";
      } else {
        hhz = " KHz";
        if (realFreq < 1000) sprintf(s, "%i", hz); else sprintf(s, "%i %03i", khz, hz);
      }
      tft.drawString(s, 10, 114);
    }
  } else tft.print("0");
  if (!Decipoint && DisplayfreqNew >= (band[0].minimumFreq / 10) && DisplayfreqNew <= (band[0].maximumFreq / 10)) drawButton(L_FREQ, 9, B_NORMAL); else drawButton(L_FREQ, 9, B_BLOCK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW, TFT_NAVY);
  tft.drawString(hhz, 186, 114);
}

// ============================================================================

// ============================================================================
// MESURES / AUDIO / INDICATEURS
// ============================================================================
void Smeter() {
// ============================================================================
  int spoint;
  if (currentMode != FM) {
    if ((rssi >= 0) && (rssi <=  1)) spoint =  12; // S0
    if ((rssi >  1) && (rssi <=  2)) spoint =  24; // S1
    if ((rssi >  2) && (rssi <=  3)) spoint =  36; // S2
    if ((rssi >  3) && (rssi <=  4)) spoint =  48; // S3
    if ((rssi >  4) && (rssi <= 10)) spoint =  48 + (rssi - 4) * 2; // S4
    if ((rssi > 10) && (rssi <= 16)) spoint =  60 + (rssi - 10) * 2; // S5
    if ((rssi > 16) && (rssi <= 22)) spoint =  72 + (rssi - 16) * 2; // S6
    if ((rssi > 22) && (rssi <= 28)) spoint =  84 + (rssi - 22) * 2; // S7
    if ((rssi > 28) && (rssi <= 34)) spoint =  96 + (rssi - 28) * 2; // S8
    if ((rssi > 34) && (rssi <= 44)) spoint = 108 + (rssi - 34) * 2; // S9
    if ((rssi > 44) && (rssi <= 54)) spoint = 124 + (rssi - 44) * 2; // S9 +10
    if ((rssi > 54) && (rssi <= 64)) spoint = 140 + (rssi - 54) * 2; // S9 +20
    if ((rssi > 64) && (rssi <= 74)) spoint = 156 + (rssi - 64) * 2; // S9 +30
    if ((rssi > 74) && (rssi <= 84)) spoint = 172 + (rssi - 74) * 2; // S9 +40
    if ((rssi > 84) && (rssi <= 94)) spoint = 188 + (rssi - 84) * 2; // S9 +50
    if  (rssi > 94)                   spoint = 204; // S9 +60
    if  (rssi > 95)                   spoint = 208; //>S9 +60
  }
  else
  {
    if  (rssi <  1) spoint = 36;
    if ((rssi >  1) && (rssi <=  2)) spoint =  60; // S6
    if ((rssi >  2) && (rssi <=  8)) spoint =  84 + (rssi - 2) * 2; // S7
    if ((rssi >  8) && (rssi <= 14)) spoint =  96 + (rssi - 8) * 2; // S8
    if ((rssi > 14) && (rssi <= 24)) spoint = 108 + (rssi - 14) * 2; // S9
    if ((rssi > 24) && (rssi <= 34)) spoint = 124 + (rssi - 24) * 2; // S9 +10
    if ((rssi > 34) && (rssi <= 44)) spoint = 140 + (rssi - 34) * 2; // S9 +20
    if ((rssi > 44) && (rssi <= 54)) spoint = 156 + (rssi - 44) * 2; // S9 +30
    if ((rssi > 54) && (rssi <= 64)) spoint = 172 + (rssi - 54) * 2; // S9 +40
    if ((rssi > 64) && (rssi <= 74)) spoint = 188 + (rssi - 64) * 2; // S9 +50
    if  (rssi > 74)                   spoint = 204; // S9 +60
    if  (rssi > 76)                   spoint = 208; //>S9 +60
  }

  int tik = 0;
  int met = spoint + 2;
  while (met > 11 && tik < 9) {
    if (tik) tft.fillRect(Xsmtr + 20 + (tik * 12), Ysmtr + 38 , 10, 6, TFT_ORANGE); else tft.fillRect(Xsmtr + 15, Ysmtr + 38 , 15, 6, TFT_RED);
    met -= 12;
    tik++;
  }
  while (met > 15 && tik < 15) {
    tft.fillRect(Xsmtr + 20 + ((tik - 9) * 16) + 108, Ysmtr + 38 , 14, 6, TFT_GREEN);
    met -= 16;
    tik++;
  }
  if (tik == 15 && met > 4) {
    tft.fillRect(Xsmtr + 20 + 204, Ysmtr + 38, 3, 6, TFT_ORANGE);
  } else {
    tft.fillRect(Xsmtr + 22 + spoint - met, Ysmtr + 38, 207 - (2 + spoint) + met, 6, TFT_BLACK);
  }
}

// ============================================================================
void Battery() { //battery info
// ============================================================================
  float vsupply = 3.724 * analogRead(BAT_INFO) / 2047; //3.3v
  int bat = map(int(vsupply * 100), 270, 405, 0, 100);
  if ((FirstLayer || ThirdLayer) && ((elapsedBat + 10000) < millis())) {
    if (bat < 0) bat = 0;
    if (bat > 100) bat = 100;
    int colorBatt = TFT_WHITE;
    if (bat < 15) colorBatt = TFT_ORANGE;
    if (bat < 5) colorBatt = 64528;
    tft.drawRect(XVolInd + 175, YVolInd + 4, 48, 18, colorBatt);
    tft.drawRect(XVolInd + 224, YVolInd + 8, 2, 10, colorBatt);
    tftRusSetFont(T1012_T);
    tftRusSetSize(1);
    tftRusSetColor(TFT_WHITE, TFT_NAVY);
    tftRusSetDatum(BC_T);
    tftRusSetStyle(NRG_T);
    if (batVolt) {
      tftRusPrint(String(vsupply, 2) + "V", XVolInd + 199, YVolInd + 19);
    } else {
      int tmp = (46 - tftRusTextWidth(String(bat) + "%")) / 2;
      tft.fillRect(XVolInd + 176, YVolInd + 5, tmp, 16, TFT_NAVY);
      tft.fillRect(XVolInd + 221 - tmp, YVolInd + 5, tmp, 16, TFT_NAVY);
      tftRusPrint(String(bat) + "%", XVolInd + 199, YVolInd + 19);
    }
  }
  if ((elapsedBat + 10000) < millis()) {
    elapsedBat = millis();
    if (bat < 5) ErrorBeep();
  }
}

// ============================================================================
void VolumeIndicator(int val) {
// ============================================================================
  tft.setTextColor(TFT_WHITE, TFT_GREY);
  tft.setTextSize(1);
  tft.setCursor(XVolInd + 57, YVolInd + 3);
  tft.print("VOLUME");
  val = map(val, MinVOL, MaxVOL, 0, 128);
  tft.fillRect(XVolInd + 15, YVolInd + 16 , (2 + val), 6, TFT_GREEN);
  tft.fillRect(XVolInd + 17 + val, YVolInd + 16 , 130 - (2 + val), 6, TFT_NAVY);
}

// ============================================================================
void brightnessIndicator(int val) {
// ============================================================================
  tft.setTextColor(TFT_WHITE, TFT_GREY);
  tft.setTextSize(1);
  tft.setCursor(XVolInd + 57, YVolInd + 3);
  tft.print("BRIGHT");
  val = map(val, MinBrightness, MaxBrightness, 0, 128);
  tft.fillRect(XVolInd + 15, YVolInd + 16 , (2 + val), 6, TFT_CYAN);
  tft.fillRect(XVolInd + 17 + val, YVolInd + 16 , 130 - (2 + val), 6, TFT_MAROON);
}

// ============================================================================
void squelchIndicator(int val) {
// ============================================================================
  tft.setTextColor(TFT_WHITE, TFT_GREY);
  tft.setTextSize(1);
  tft.setCursor(XVolInd + 57, YVolInd + 3);
  tft.print("SQUELCH");
  val = map(val, MinSQUELCH, MaxSQUELCH, 0, 128);
  tft.fillRect(XVolInd + 15, YVolInd + 16 , (2 + val), 6, TFT_ORANGE);
  tft.fillRect(XVolInd + 17 + val, YVolInd + 16 , 130 - (2 + val), 6, TFT_MAROON);
  }
// ============================================================================
void saver() {
// ============================================================================
  float freq;
  tft.fillScreen(TFT_BLACK);
  if (displayOff) {
    if (displayPower) digitalWrite(DISPLAY_LED, 1); else ledcWrite(LedChannelforTFT, MinBrightness);
  }
  if (saverOn) {
    Saver = true;
    SEEK = false;
    bright = false;
    AGCgainbut = false;
    freq = si4735.getFrequency();
    if (SCANbut) freq = currentScanFreq + int((currentScanLine - 159 + deltaScanLine) * SCANstep);
    else if (isSSBLikeMode()) freq -= int(currentBFO / 1000);
  }
  while ((((pressed == false) && (encoderCount == 0) && (encoderButton == 0) && (analogRead(ENCODER_SWITCH) > 500) && !Serial.available())) || (writingEeprom)) { // wait loop
    pressed = tft.getTouch(&x, &y);
    processPcAndWifiControl();
    if (saverOn) {
      int t = posSaver;
      posSaver++;
      if (posSaver == 500) posSaver = 0;
      for (int i = 0; i < 63; i++) {
        int c = 31 - abs(i - 31);
        if (t < 200) {
          tft.drawPixel(saverX - 10 + t, saverY - 5, (c * 64) + c);
        } else if (t >= 200 && t < 250) {
          tft.drawPixel(saverX + 189, saverY - 205 + t, (c * 64) + c);
        } else if (t >= 250 && t < 450) {
          tft.drawPixel(saverX + 439 - t, saverY + 44, (c * 64) + c);
        } else {
          tft.drawPixel(saverX - 10, saverY + 494 - t, (c * 64) + c);
        }
        t += 3;
        if (t > 499) t -= 500;
      }
      if ((elapsedSaver + 15000) < millis()) {
        elapsedSaver = millis();
        tft.fillScreen(TFT_BLACK);
        if (screenV) {
          saverX = random(40) + 10;
          saverY = random(270) + 5;
        } else {
          saverX = random(120) + 10;
          saverY = random(190) + 5;
        }
        FreqDraw(freq, 0);
        if (batShow) {
          float vsupply = 3.724 * analogRead(BAT_INFO) / 2047; //3.3v
          int bat = map(int(vsupply * 100), 270, 405, 0, 100);
          if (bat < 0) bat = 0;
          if (bat > 100) bat = 100;
          int colorBatt = TFT_DARKCYAN;
          if (bat < 15) colorBatt = TFT_ORANGE;
          if (bat < 5) colorBatt = 64528;
      tft.drawRect(saverX + 142, saverY, 43, 18, colorBatt);
          tftRusSetFont(T1012_T);
          tftRusSetSize(1);
          tftRusSetColor(TFT_CYAN, TFT_TRANS);
          tftRusSetDatum(BC_T);
          tftRusSetStyle(NRG_T);

         char timeHour[3];
         strftime(timeHour,3, "%H", &timeinfo);

         char timeMin[3];
         strftime(timeMin,3, "%M", &timeinfo);

         if(getLocalTime(&timeinfo)){
         tftRusPrint(String(timeHour)+":"+String(timeMin), saverX + 164, saverY + 15);
          }
        }
      }
    }
    if (SCANbut && !SCANpause) DisplaySCAN();
  }
  Saver = false;
  pressed = false;
  encoderCount = 0;
  encoderButton = 0;
  if (displayOff) {
    if (displayPower) digitalWrite(DISPLAY_LED, 0); else ledcWrite(LedChannelforTFT, currentBrightness);
  }
  returnLayer();
  elapsedSaver = millis();
}

// ============================================================================
void returnLayer() {
// ============================================================================
  if (FirstLayer) DrawFila();
  else if (ThirdLayer) DrawThla();
  if (HamBand) drawList(L_HAM, "HAM RADIO BAND");
  if (FREQbut) {
    drawList(L_FREQ, "FREQUENCY");
    tft.fillRect(0, 80, 240, 40, TFT_NAVY);
    Freqcalq(0);
  }
  if (Modebut) drawList(L_MODE, "MODULATION");
  if (BandWidth) {
    if (currentMode == AM) drawList(L_BANDW_AM, "AM Filter in KHz");
    else if (currentMode == FM) drawList(L_BANDW_FM, "FM Filter in KHz");
    else drawList(L_BANDW_SSB, "SSB Filter in KHz");
  }
  if (STEPbut) {
    if (currentMode == AM) drawList(L_STEP_AM, "STEP TUNE AM"); else drawList(L_STEP_FM, "STEP TUNE FM");
  }
  if (BroadBand) drawList(L_BAND, "BAND");
  if (cityRETRObut) drawRetroCity();
  if (bandRETRObut) drawRetroBand();
  if (RETRObut) {
    drawRETRO();
    drawRETROscale();
  }
  if (MEMObut) {
    drawList(L_MEMO, "MEMORY BANK");
    if (MEMOadd) {
      drawButton(L_MEMO, 0, B_NORMAL, "OK");
      drawButton(L_MEMO, 1, B_SELECT);
      drawButton(L_MEMO, 3, B_NORMAL, "X");
    }
    if (MEMOdel) {
      drawButton(L_MEMO, 0, B_NORMAL, "OK");
      drawButton(L_MEMO, 1, B_BLOCK);
      drawButton(L_MEMO, 2, B_SELECT);
      drawButton(L_MEMO, 3, B_NORMAL, "X");
    }
    displMEMO();
  }
  if (SETUPbut) {
    drawList(L_SETUP,"SETUP");
    if (!pageSetup) drawButton(L_SETUP, 0, B_BLOCK);
    if (pageSetup == maxPageSetup) drawButton(L_SETUP, 1, B_BLOCK);
    displSETUP();
  }
  if (SCANbut && STEPbut == false) {
    drawSCAN();
    if (SCANpause) drawButton(L_SCAN, 1, B_JAM);
    drawSCANgraf(false);
    DrawSCANtxt(true);
  }
  if (PRESbut) {
    tft.fillRect(XFreqDispl, YFreqDispl + 20 , 239, 65, TFT_DARKCYAN);
    drawButton(L_THIRD, B_FM, B_SELECT);
    tft.setTextSize(1);
    tft.setTextDatum(BL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_DARKCYAN );
    tft.drawString(String(preset[currentPRES].presetIdx, 2) + " MHz ", 5, 83);
    if (!directScroll) {
      tftRusSetFont(T1516_T);
      tftRusSetSize(1);
      tftRusSetColor(TFT_WHITE, TFT_TRANS);
      tftRusSetDatum(BC_T);
      tftRusSetStyle(NBL_T);
      tftRusSetCut(0, 18);
      tftRusPrint(String(preset[currentPRES].PresetName), 120, 50);
      tftRusSetCut(0, 0);
    }
  }
  if (FirstLayer || ThirdLayer) VolumeIndicator(si4735.getVolume());
}

// ============================================================================
// ======================= PC + WIFI CONTROL =======================

// ============================================================================
// PROTOCOLE ATS LAB - COMMANDES PC / WIFI
// ============================================================================
const char* pcModeName(uint8_t mode) {
  switch (mode) {
    case FM:  return "FM";
    case AM:  return "AM";
    case LSB: return "LSB";
    case USB: return "USB";
    case CW:  return "CW";
    default:  return "?";
  }
}

uint32_t pcGetTunedFrequencyHz() {
  if (currentMode == FM) {
    return (uint32_t)si4735.getFrequency() * 10000UL;
  }

  int64_t freqHz = (int64_t)si4735.getFrequency() * 1000LL;

  if (isSSBLikeMode()) {
    freqHz -= (int64_t)band[bandIdx].lastBFO;
    if (currentMode == CW && CWShift) freqHz += 700LL;
  }

  if (freqHz < 0) freqHz = 0;
  return (uint32_t)freqHz;
}

void pcSendStatus(Print &out) {
  uint8_t pcRssi = NewRSSI;
  uint8_t pcSnr  = NewSNR;

  if ((pcRssi == 0) && (pcSnr == 0)) {
    si4735.getCurrentReceivedSignalQuality();
    pcRssi = si4735.getCurrentRSSI();
    pcSnr  = si4735.getCurrentSNR();
  }

  uint32_t freqHz = pcGetTunedFrequencyHz();
  uint32_t freqKHz = (freqHz + 500UL) / 1000UL; // compatible integer field

  out.print("STATUS,FREQ_KHZ="); out.print(freqKHz);
  out.print(",FREQ_HZ="); out.print(freqHz);
  out.print(",MODE="); out.print(pcModeName(currentMode));
  out.print(",VOL="); out.print(currentVOL);
  out.print(",SQ="); out.print(currentSquelch);
  out.print(",RSSI="); out.print(pcRssi);
  out.print(",SNR="); out.print(pcSnr);
  out.print(",BAND="); out.println(band[bandIdx].bandName);
}

int pcFindBandForFrequency(uint32_t freqKHz, uint16_t internalFreq) {

  if (freqKHz >= 64000UL && freqKHz <= 108000UL)
    return BAND_FM;

  for (int i = 1; i < BAND_SW; i++) {
    if (internalFreq >= band[i].minimumFreq &&
        internalFreq <= band[i].maximumFreq)
      return i;
  }

  return BAND_SW;
}

bool pcSetFrequencyKHz(uint32_t freqKHz) {
  uint16_t internalFreq;
  if (freqKHz >= 64000UL && freqKHz <= 108000UL) internalFreq = (uint16_t)(freqKHz / 10UL);
  else if (freqKHz >= 100UL && freqKHz <= 30000UL) internalFreq = (uint16_t)freqKHz;
  else return false;

  int newBand = pcFindBandForFrequency(freqKHz, internalFreq);
  bandIdx = newBand;
  currentMode = (newBand == BAND_FM) ? FM : band[newBand].prefmod;
  band[bandIdx].currentFreq = internalFreq;
  currentFrequency = internalFreq;
  previousFrequency = internalFreq;
  BandSet();

  if (Mutestat)
    si4735.setAudioMute(audioMuteOn);
  FreqDispl();
  return true;
}

bool pcSetMode(const String &modeText) {
  String m = modeText;
  m.trim();
  m.toUpperCase();
  uint8_t newMode;
  if      (m == "FM")  newMode = FM;
  else if (m == "AM")  newMode = AM;
  else if (m == "LSB") newMode = LSB;
  else if (m == "USB") newMode = USB;
  else if (m == "CW")  newMode = CW;
  else return false;
  if (bandIdx == BAND_FM && newMode != FM) return false;
  if (bandIdx != BAND_FM && newMode == FM) return false;
  currentMode = newMode;
  band[bandIdx].prefmod = currentMode;
  ssbLoaded = false;
  BandSet();
  FreqDispl();
  return true;
}

bool pcStartScan(uint32_t requestedBaseKHz, float requestedStepKHz) {
  if (SCANbut) {
    if (SCANpause) {
      SCANpause = false;
      pauseSCAN();
    }
    return true;
  }

  int d = screenV * 40;
  const int visibleCount = 320 - (d * 2);
  if (requestedBaseKHz != 0 || requestedStepKHz != 0) {
    const bool fmRequest = currentMode == FM && bandIdx == BAND_FM;
    const bool hamRequest = currentMode != FM &&
      (bandIdx == BAND_40M || bandIdx == BAND_20M || bandIdx == BAND_14M);
    const float bandMinKHz = fmRequest ?
      band[bandIdx].minimumFreq * 10.0f : band[bandIdx].minimumFreq;
    const float bandMaxKHz = fmRequest ?
      band[bandIdx].maximumFreq * 10.0f : band[bandIdx].maximumFreq;
    const bool invalidFmStep = fmRequest &&
      (requestedStepKHz < 10 || requestedStepKHz > 1000 ||
       fabsf((requestedStepKHz / 10.0f) -
             roundf(requestedStepKHz / 10.0f)) > 0.0001f);
    const bool invalidHamStep = hamRequest &&
      (requestedStepKHz < 1.0f || requestedStepKHz > 100.0f ||
       fabsf(requestedStepKHz - roundf(requestedStepKHz)) > 0.0001f);
    if ((!fmRequest && !hamRequest) ||
        requestedBaseKHz < bandMinKHz || requestedBaseKHz > bandMaxKHz ||
        invalidFmStep || invalidHamStep) {
      return false;
    }

    pcScanDataCount = min(visibleCount,
      int(floorf((bandMaxKHz - requestedBaseKHz) / requestedStepKHz)) + 1);
    if (pcScanDataCount < 2) return false;
  } else {
    pcScanDataCount = visibleCount;
  }

  SCANbut = true;
  ScanAGC = AGCgain;
  currentScanFreq = si4735.getFrequency();

  if (isSSBLikeMode()) {
    currentScanFreq -= (currentBFO / 1000);
    currentBFO = freqDec = band[bandIdx].lastBFO = 0;
    si4735.setSSBBfo(currentBFOmanu);
    commSetFreq(currentScanFreq);
  }

  if (autoSCANstep) {
    float tmp = float(band[bandIdx].maximumFreq - band[bandIdx].minimumFreq) /
                (320 - (d * 2));
    float i = maxSCANstep / 2;
    while (i >= minSCANstep) {
      if (tmp > i) {
        currentMinScanStep = i / 4;
        currentMaxScanStep = i * 2;
        i = 0;
      }
      i /= 2;
    }
    if (currentMinScanStep > 0.5) currentMinScanStep = 0.5;
    if ((currentMinScanStep < minSCANstep) ||
        (isSSBLikeMode()))
      currentMinScanStep = minSCANstep;
    if (currentMaxScanStep == minSCANstep) currentMaxScanStep *= 2;
  } else {
    currentMinScanStep = minSCANstep;
    currentMaxScanStep = maxSCANstep;
  }

  if (requestedBaseKHz != 0 && requestedStepKHz > 0) {
    if (currentMode == FM) {
      // Le SI4735 exprime la frequence FM en pas de 10 kHz.
      SCANstep = requestedStepKHz / 10.0f;
      currentScanFreq = int((requestedBaseKHz / 10.0f) +
        ((159 - d) * SCANstep) + 0.5f);
    } else {
      SCANstep = requestedStepKHz;
      currentScanFreq = int(requestedBaseKHz +
        ((159 - d) * SCANstep) + 0.5f);
    }
    currentMinScanStep = SCANstep;
    currentMaxScanStep = SCANstep;
  } else {
    SCANstep = currentMaxScanStep / 2;
  }
  currentScanLine = 159 - d;
  deltaScanLine = 0;

  drawSCAN();
  SCANpause = false;
  pauseSCAN();
  drawSCANgraf(true);
  DrawSCANtxt(true);
  signalScale = 1.5 + (d / 80);
  ThirdLayer = false;
  SecondLayer = true;
  return true;
}

bool pcPauseScan() {
  if (!SCANbut) return false;
  if (!SCANpause) {
    SCANpause = true;
    pauseSCAN();
  }
  return true;
}

bool pcResumeScan() {
  if (!SCANbut) return false;
  if (SCANpause) {
    SCANpause = false;
    pauseSCAN();
  }
  return true;
}

bool pcStopScan() {
  if (!SCANbut) return true;

  SCANpause = true;
  pauseSCAN();
  band[bandIdx].currentFreq = si4735.getFrequency();
  if (isSSBLikeMode())
    band[bandIdx].lastBFO = currentBFO = 0;
  restoreMuteIfNeeded();
  SCANbut = false;
  SCANstep = 0;
  pcScanDataCount = 0;
  DrawThla();
  return true;
}

void pcSendScanStatus(Print &out) {
  out.print("SCAN,STATE=");
  if (!SCANbut) out.print("STOPPED");
  else if (SCANpause) out.print("PAUSED");
  else out.print("RUNNING");

  out.print(",FREQ_KHZ=");
  out.print((pcGetTunedFrequencyHz() + 500UL) / 1000UL);
  out.print(",STEP=");
  out.println(SCANstep, 3);
}

void pcSendScanData(Print &out) {
  if (!SCANbut || SCANstep <= 0) {
    out.println("ERR,SCAN,NO_DATA");
    return;
  }

  // Attendre que le premier balayage soit complet. Envoyer SCANEND avec un
  // tableau encore partiel ferait arreter le scan par ATS LAB.
  if (ScanEmpty) {
    out.println("SCAN,STATE=RUNNING");
    return;
  }

  const int dCoord = screenV * 40;
  const int nativeVisibleCount = 320 - (screenV * 80);
  const int visibleCount = (pcScanDataCount > 0) ?
    min(pcScanDataCount, nativeVisibleCount) : nativeVisibleCount;
  float baseFreqKHz = currentScanFreq +
    ((0 - 159 + dCoord + deltaScanLine) * SCANstep);
  float stepKHz = SCANstep;
  if (currentMode == FM) {
    // Les valeurs internes FM du SI4735 sont des unites de 10 kHz.
    baseFreqKHz *= 10.0f;
    stepKHz *= 10.0f;
  }

  out.print("SCANBEGIN,COUNT=");
  out.print(visibleCount);
  out.print(",BASE_KHZ=");
  out.print(baseFreqKHz, 3);
  out.print(",STEP_KHZ=");
  out.print(stepKHz, 3);
  out.print(",STATE=");
  if (SCANpause) out.println("PAUSED"); else out.println("RUNNING");

  for (int start = 0; start < visibleCount; start += 20) {
    out.print("SCANDATA,");
    out.print(start);
    int last = start + 20;
    if (last > visibleCount) last = visibleCount;

    for (int i = start; i < last; i++) {
      int rssi = int(((198.0f - ScanValueRSSI[i]) / signalScale) + 0.5f);
      if (rssi < 0) rssi = 0;
      out.print(',');
      out.print(rssi);
      out.print(',');
      out.print(ScanValueSNR[i]);
    }
    out.println();
  }
  out.println("SCANEND");
}

bool pcBfoModeAvailable() {
  return isSSBLikeMode();
}

void pcSendBfoStatus(Print &out) {
  out.print("BFO,STATE=");
  out.print(pcBfoModeAvailable() ? "READY" : "UNAVAILABLE");
  out.print(",VALUE=");
  out.print(currentBFOmanu);
  out.print(",STEP=");
  out.println(currentBFOStep);
}

bool pcSetManualBfo(int valueHz) {
  if (!pcBfoModeAvailable()) return false;

#ifdef IhaveCrystal
  if (valueHz > 999) valueHz = 999;
  if (valueHz < -999) valueHz = -999;

  currentBFOmanu = valueHz;
  band[bandIdx].lastmanuBFO = currentBFOmanu;

  applyCurrentBFO();
  previousBFO = currentBFO;
  previousBFOmanu = currentBFOmanu;
  FreqDispl();
  return true;
#else
  return false;
#endif
}

bool pcSetBfoStep(int stepHz) {
  if (stepHz != 1 && stepHz != 10 && stepHz != 25) return false;
  currentBFOStep = stepHz;
  return true;
}

void pcPollRdsCache() {
  if (!pcRdsEnabled || currentMode != FM) return;
  unsigned long now = millis();
  if ((now - pcRdsLastPoll) < PC_RDS_POLL_MS) return;
  pcRdsLastPoll = now;

  si4735.getRdsStatus();
  if (!(si4735.getRdsReceived() && si4735.getRdsSync() && si4735.getRdsSyncFound()))
    return;

  char *ps = si4735.getRdsText0A();
  char *rt = si4735.getRdsText2A();
  char *ct = si4735.getRdsTime();
  uint16_t pi = si4735.getRdsPI();
  uint8_t pty = si4735.getRdsProgramType();

  if (ps != NULL && strlen(ps) > 0) {
    String v = String(ps); v.trim();
    if (v.length() > 0) pcRdsPS = v;
  }
  if (rt != NULL && strlen(rt) > 0) {
    String v = String(rt); v.trim();
    if (v.length() > 0) pcRdsRT = v;
  }
  if (ct != NULL && strlen(ct) > 0) {
    String v = String(ct); v.trim();
    if (v.length() > 0) pcRdsCT = v;
  }

  if (pi > 1) pcRdsPI = pi;
  if (pty > 0) pcRdsPTY = pty;

  pcRdsHaveData = (pcRdsPS.length() > 0) ||
                  (pcRdsRT.length() > 0) ||
                  (pcRdsCT.length() > 0) ||
                  (pcRdsPI != 0) ||
                  (pcRdsPTY != 0);
}

void pcSendRdsStatus(Print &out) {
  if (!pcRdsEnabled) {
    out.println("RDS,STATE=OFF");
    return;
  }
  if (currentMode != FM) {
    out.println("RDS,STATE=NOT_FM");
    return;
  }

  pcPollRdsCache();
  si4735.getRdsStatus();

  bool received = si4735.getRdsReceived();
  bool sync = si4735.getRdsSync();
  bool syncFound = si4735.getRdsSyncFound();

  if (!(received && sync && syncFound)) {
    out.println("RDS,STATE=NO_DATA");
    return;
  }

  if (pcRdsPS.length() > 0) {
    out.print("RDSPS,"); out.println(pcRdsPS);
  }
  if (pcRdsRT.length() > 0) {
    out.print("RDSRT,"); out.println(pcRdsRT);
  }
  if (pcRdsPI != 0) {
    char piHex[5];
    snprintf(piHex, sizeof(piHex), "%04X", pcRdsPI);
    out.print("RDSPI,"); out.println(piHex);
  }
  out.print("RDSPTY,"); out.println(pcRdsPTY);
  if (pcRdsCT.length() > 0) {
    out.print("RDSCT,"); out.println(pcRdsCT);
  }
  if (!pcRdsHaveData)
    out.println("RDS,STATE=WAITING");
}

// ============================================================================
// PROTOCOLE ATS LAB - PARSEUR DE COMMANDES
// ============================================================================
void processControlCommand(String cmd, Print &out) {
  if (cmd == "ID?" || cmd == "IDENT?" || cmd == "VERSION?") {
    out.println(F("ID,ATS-25X2,FW=6.0,HW=ESP32,CHIP=SI4735,BT=SPP"));
    return;
  }

  cmd.trim();
  if (cmd.length() == 0) return;
  String upper = cmd;
  upper.toUpperCase();

  if (upper == "PING") {
    out.println("OK,PONG");
  } else if (upper == "STATUS?" || upper == "GETSTATUS") {
    pcSendStatus(out);
  } else if (upper.startsWith("FREQ=")) {
    uint32_t f = (uint32_t)cmd.substring(5).toInt();
    out.println(pcSetFrequencyKHz(f) ? "OK,FREQ" : "ERR,FREQ");
    pcSendStatus(out);
  } else if (upper.startsWith("MODE=")) {
    out.println(pcSetMode(cmd.substring(5)) ? "OK,MODE" : "ERR,MODE");
    pcSendStatus(out);
  } else if (upper.startsWith("VOL=")) {
    int v = cmd.substring(4).toInt();
    if (v < MinVOL) v = MinVOL;
    if (v > MaxVOL) v = MaxVOL;
    currentVOL = v;
    previousVOL = v;
    si4735.setVolume(currentVOL);
    FreqDispl();
    out.println("OK,VOL");
    pcSendStatus(out);
  } else if (upper.startsWith("SQ=")) {
    int sq = cmd.substring(3).toInt();
    if (sq < MinSQUELCH) sq = MinSQUELCH;
    if (sq > MaxSQUELCH) sq = MaxSQUELCH;
    currentSquelch = sq;
    previousSquelch = sq;
    storage.SquelchVal = sq;
    out.println("OK,SQ");
    pcSendStatus(out);
  } else if (upper == "SQ?" || upper == "SQUELCH?") {
    out.print("SQ,");
    out.println(currentSquelch);
    pcSendStatus(out);
  } else if (upper == "BFO?" || upper == "BFO=STATUS") {
    pcSendBfoStatus(out);
  } else if (upper.startsWith("BFOSTEP=")) {
    int stepHz = cmd.substring(8).toInt();
    out.println(pcSetBfoStep(stepHz) ? "OK,BFO,STEP" : "ERR,BFO,STEP");
    pcSendBfoStatus(out);
  } else if (upper.startsWith("BFO=")) {
    int bfoHz = cmd.substring(4).toInt();
    out.println(pcSetManualBfo(bfoHz) ? "OK,BFO" : "ERR,BFO,MODE");
    pcSendBfoStatus(out);
  } else if (upper.startsWith("SCAN=START,BASE_KHZ=")) {
    int stepToken = upper.indexOf(",STEP_KHZ=");
    uint32_t baseKHz = 0;
    float stepKHz = 0;
    if (stepToken > 20) {
      baseKHz = (uint32_t)cmd.substring(20, stepToken).toInt();
      stepKHz = cmd.substring(stepToken + 10).toFloat();
    }
    out.println(pcStartScan(baseKHz, stepKHz) ?
      "OK,SCAN,START" : "ERR,SCAN,PARAMETERS");
    pcSendScanStatus(out);
  } else if (upper == "SCAN=START") {
    out.println(pcStartScan() ? "OK,SCAN,START" : "ERR,SCAN,START");
    pcSendScanStatus(out);
  } else if (upper == "SCAN=STOP") {
    out.println(pcStopScan() ? "OK,SCAN,STOP" : "ERR,SCAN,STOP");
    pcSendScanStatus(out);
  } else if (upper == "SCAN=PAUSE") {
    out.println(pcPauseScan() ? "OK,SCAN,PAUSE" : "ERR,SCAN,NOT_RUNNING");
    pcSendScanStatus(out);
  } else if (upper == "SCAN=RESUME") {
    out.println(pcResumeScan() ? "OK,SCAN,RESUME" : "ERR,SCAN,NOT_RUNNING");
    pcSendScanStatus(out);
  } else if (upper == "SCAN?" || upper == "SCAN=STATUS") {
    pcSendScanStatus(out);
  } else if (upper == "SCANDATA?" || upper == "SCAN=DATA") {
    pcSendScanData(out);
  } else if (upper == "MUTE=ON" || upper == "MUTE=1") {
    Mutestat = true;
    si4735.setAudioMute(audioMuteOn);
    out.println("OK,MUTE,ON");
  } else if (upper == "MUTE=OFF" || upper == "MUTE=0") {
    Mutestat = false;
    si4735.setAudioMute(audioMuteOff);
    out.println("OK,MUTE,OFF");
  } else if (upper == "MUTE?") {
    out.print("MUTE,");
    out.println(Mutestat ? "ON" : "OFF");
  } else if (upper == "RDS=1" || upper == "RDS=ON") {
    pcRdsEnabled = true;
    RDS = true;
    pcRdsPS = ""; pcRdsRT = ""; pcRdsCT = "";
    pcRdsPI = 0; pcRdsPTY = 0; pcRdsHaveData = false; pcRdsLastPoll = 0;
    if (currentMode == FM) {
      si4735.RdsInit();
      si4735.setRdsConfig(1, 2, 2, 2, 2);
      delay(50);
    }
    out.println("OK,RDS,ON");
    pcSendRdsStatus(out);
  } else if (upper == "RDS=0" || upper == "RDS=OFF") {
    pcRdsEnabled = false;
    RDS = false;
    out.println("OK,RDS,OFF");
  } else if (upper == "RDS?" || upper == "RDS=STATUS") {
    pcSendRdsStatus(out);
  } else if (upper.startsWith("WIFI_SSID=")) {
    WIFI_SSID = cmd.substring(10);
    WIFI_SSID.trim();
    out.println("OK,WIFI,SSID");
  } else if (upper.startsWith("WIFI_PASS=")) {
    WIFI_PASS = cmd.substring(10);
    WIFI_PASS.trim();
    out.println("OK,WIFI,PASS");
  } else if (upper == "WIFI_SAVE") {
    out.println(saveWifiCredentials() ? "OK,WIFI,SAVED" : "ERR,WIFI,SAVE");
  } else if (upper == "WIFI_CONNECT") {
    connectWifi();
    out.println("OK,WIFI,CONNECTING");
    sendWifiStatus(out);
  } else if (upper == "WIFI?" || upper == "WIFI_STATUS?") {
    sendWifiStatus(out);
  } else if (upper == "WIFI_DIAG?" || upper == "WIFI_SCAN?") {
    bool found = diagnoseTargetWifi();
    out.println(found ? "OK,WIFI,FOUND" : "ERR,WIFI,NOT_FOUND");
  } else if (upper == "WIFI_CLEAR") {
    clearWifiCredentials();
    out.println("OK,WIFI,CLEARED");
    sendWifiStatus(out);
  } else {
    out.println("ERR,UNKNOWN_COMMAND");
  }
}

void processControlStream(Stream &input, Print &output, String &buffer) {
  while (input.available()) {
    char c = (char)input.read();
    if (c == '\r') continue;
    if (c == '\n') {
      processControlCommand(buffer, output);
      buffer = "";
    } else if (buffer.length() < 80) {
      buffer += c;
    }
  }
}

void startAtsTcpServer() {
  if (atsTcpStarted || WiFi.status() != WL_CONNECTED) return;
  atsTcpServer.begin();
  atsTcpServer.setNoDelay(true);
  atsTcpStarted = true;
  Serial.print("ATS TCP server listening on port ");
  Serial.println(ATS_TCP_PORT);
  if (MDNS.begin("ats25")) {
    MDNS.addService("ats-control", "tcp", ATS_TCP_PORT);
    Serial.println("mDNS ready: ats25.local");
  } else {
    Serial.println("mDNS start failed; use the IP address");
  }
}

void processPcAndWifiControl() {
  serviceWifiConnection();

  if (Mutestat)
    si4735.setAudioMute(audioMuteOn);
  pcPollRdsCache();
  static String serialBuffer;
  static String wifiBuffer;
  static String bluetoothBuffer;

  processControlStream(Serial, Serial, serialBuffer);

  // Bluetooth SPP reutilise exactement le meme parseur de commandes.
  if (atsBluetoothStarted && SerialBT.hasClient()) {
    processControlStream(SerialBT, SerialBT, bluetoothBuffer);
  }

  if (WiFi.status() == WL_CONNECTED) startAtsTcpServer();
  if (!atsTcpStarted) return;

  if (!atsTcpClient || !atsTcpClient.connected()) {
    WiFiClient newClient = atsTcpServer.available();
    if (newClient) {
      if (atsTcpClient) atsTcpClient.stop();
      atsTcpClient = newClient;
      atsTcpClient.setNoDelay(true);
      wifiBuffer = "";
      atsTcpClient.println("ATS25,READY");
      Serial.print("ATS Studio Wi-Fi connected: ");
      Serial.println(atsTcpClient.remoteIP());
    }
  }

  if (atsTcpClient && atsTcpClient.connected()) {
    processControlStream(atsTcpClient, atsTcpClient, wifiBuffer);
  }
}
// ===================== END PC + WIFI CONTROL =====================

// ============================================================================
// BOUCLE PRINCIPALE
// ============================================================================

// ============================================================================
// DECOUPAGE DE LOOP - NIVEAU 7
// ============================================================================

inline void updateMainIndicators() {
  if (((FirstLayer == true) || (ThirdLayer == true)) &&
      (bright == false) && (squelch == false))
    VolumeIndicator(si4735.getVolume());

  if ((ThirdLayer == true) && bright)
    brightnessIndicator(currentBrightness);

  if ((ThirdLayer == true) && squelch)
    squelchIndicator(currentSquelch);
}

inline void manageSquelch() {
  if (Mutestat)
    return;

  si4735.getCurrentReceivedSignalQuality();

  if (SquelchUsesRSSI)
    SignalQuality = si4735.getCurrentRSSI();
  else
    SignalQuality = si4735.getCurrentSNR();

  if (SignalQuality >= currentSquelch) {
    if (SCANpause) {
      si4735.setAudioMute(audioMuteOff);
      squelchDecay = millis();
    }
  } else if (millis() > (squelchDecay + squelchDecayTime)) {
    si4735.setAudioMute(audioMuteOn);
  }
}

inline bool shouldStayInIdleLoop() {
  return ((((pressed == false) && (encoderCount == 0) && (encoderButton == 0) &&
           (analogRead(ENCODER_SWITCH) > 500) && !Serial.available())) ||
         (writingEeprom));
}

void processIdleLoop() {
 // wait loop
    pressed = tft.getTouch(&x, &y);
    processPcAndWifiControl();
    if ((elapsedSaver + (saverTime * 60000)) < millis() && (saverOn || displayOff)) saver();
    showtimeRSSI();
    if (batShow) Battery();
    // RDS doit etre decode en permanence en FM lorsque l'option RDS est active.
    // L'ancien test PRESbut || RDSalways bloquait completement le RDS en usage normal.
    if (RDS && currentMode == FM)
      DisplayRDS();
    if (PREtap && (elapsedPRE + 500) < millis()) {
      PREtap = false;
      FreqDispl();
    }
    if (SCANbut) {
      if (SCANpause) DisplaySCANsignal(); else DisplaySCAN();
    }
    if (RETRObut) {
      if (tftRusLength(String(presetNameLoad())) > (10 + (screenV * 8)) && (elapsedScroll + 200) < millis()) {
        textScroll++;
        if (textScroll == tftRusLength(String(presetNameLoad())) + 5) textScroll = 0;
        tftRusSetFont(T1516_T);
        tftRusSetSize(1);
        tftRusSetColor(TFT_OLIVE, TFT_BLACK);
        tftRusSetDatum(BL_T);
        tftRusSetStyle(NCB_T);
        tftRusSetCut(textScroll, 10 + (screenV * 8));
        tftRusPrint(String(presetNameLoad() + " "), 0, 18);
        tftRusSetCut(0, 0);
        if (textScroll) elapsedScroll = millis(); else elapsedScroll = millis() + 3000;
      }
    }
    if (scrollRetro) {
      int i = 0;
      while (i <= lastPreset && scrollRetro) {
        float freq = preset[i].presetIdx;
        if (RETROband < 2) freq = int(freq * 100);
        if ((scrollRetro == 1 && freq > currentRetroFreq && freq <= (currentRetroFreq + band[bandIdx].currentStep)) ||
            (scrollRetro == -1 && freq < currentRetroFreq && freq >= (currentRetroFreq - band[bandIdx].currentStep))) {
          commSetFreq(freq);
          scrollRetro = false;
        } else i++;
      }
      if (scrollRetro) {
        float tmpmax = bandRetro[RETROband].maximumFreq;
        float tmpmin = bandRetro[RETROband].minimumFreq;
        if (bandIdx == 0) {
          tmpmax *= 100;
          tmpmin *= 100;
        }
        if ((scrollRetro == 1 && (currentRetroFreq + band[bandIdx].currentStep) <= tmpmax) || (scrollRetro != 1 && (currentRetroFreq - band[bandIdx].currentStep) >= tmpmin)) {
          if (scrollRetro == 1) commFreqUp(); else commFreqDown();
        } else scrollRetro = false;
      }
      currentRetroFreq = si4735.getFrequency();
      if (RETROband < 2) bandRetro[RETROband].currentFreq = currentRetroFreq / 100; else bandRetro[RETROband].currentFreq = currentRetroFreq;
      drawRETROscale();
    }
    if (PRESbut) {
      if (directScroll && (elapsedScroll + 200) < millis()) {
        textScroll += directScroll;
        tftRusSetFont(T1516_T);
        tftRusSetSize(1);
        tftRusSetColor(TFT_WHITE, TFT_DARKCYAN);
        tftRusSetDatum(BC_T);
        tftRusSetStyle(NBL_T);
        tftRusSetCut(textScroll, 18);
        tftRusPrint(String(preset[currentPRES].PresetName), 120, 50);
        tftRusSetCut(0, 0);
        if (textScroll == (tftRusLength(String(preset[currentPRES].PresetName)) - 18)) {
          elapsedScroll = millis() + 3000;
          directScroll = -1;
        } else
        if (!textScroll) {
          elapsedScroll = millis() + 3000;
          directScroll = 1;
        } else elapsedScroll = millis();
      }
    }
    if (MEMOadd) {
      if (posMemoName == 20) {
        MEMOadd = false;
        MemoBank[currentMemo].freq = addMemoFreq;
        MemoBank[currentMemo].band = addMemoBand;
        for (int i = 0; i < 21; i++) MemoBank[currentMemo].Name[i] = addMemoName[i];
        drawButtons(L_MEMO);
        displMEMO();
      } else {
        int d = !screenV * 40;
        tftRusSetDatum(BL_T);
        tftRusSetFont(T1516_T);
        tftRusSetColor(TFT_WHITE, TFT_DARKCYAN);
        tftRusSetStyle(NBL_T);
        tftRusWidth = 12;
        if (!charMemoName) charMemoName = 32;
        tftRusPrint(String(char(charMemoName)),(posMemoName * 12) + d, 157);
        if ((elapsedCursor + 200) > millis()) {
          tft.fillRect((posMemoName * 12) + d, 154, 10, 3, TFT_WHITE);
        } else {
          if ((elapsedCursor + 400) < millis()) elapsedCursor = millis();
        }
        if (posMemoName) tftRusPrint(String(addMemoName[posMemoName - 1]),(posMemoName * 12) - 12 + d, 157);
        if (posMemoName < 19) tftRusPrint(String(addMemoName[posMemoName + 1]),(posMemoName * 12) + 12 + d, 157);
        addMemoName[posMemoName] = char(charMemoName);
      }
    }
    MuteAud();

}

// ============================================================================
// GESTION TACTILE EXTRAITE DE LOOP - NIVEAU 8
// ============================================================================

// ============================================================================
// GESTION TACTILE PAR COUCHE - NIVEAU 9
// ============================================================================

// ============================================================================
// TOUCH COMMUN FIRST / THIRD - NIVEAU 11
// ============================================================================

void processDigitSelectionTouch() {
  for (int n = 0; n <= lastdignum; n++) {
    if ((x > dn[n].Xdignumos + dn[n].Xdignumnr) &&
        (x < dn[n].Xdignumos + dn[n].Xdignumsr + dn[n].Xdignumnr) &&
        (y > dn[n].Ydignumos) &&
        (y < dn[n].Ydignumos + dn[n].Ydignumsr)) {
      Beep(1, 0);
      delay(200);

      if (!bfoOn) {
        freqstepnr = n;
        if (freqstepnr == 0) freqstep = 1000;
        if (freqstepnr == 1) freqstep = 100;
        if (freqstepnr == 2) freqstep = 10;
      } else {
        if (n == 1) stepsizesynth = 10;
        if (n == 2) stepsizesynth = 1;
      }

      FreqDispl();
    }
  }

  x = y = 0;
}

void processBatterySelectionTouch() {
  Beep(1, 0);
  delay(200);
  batVolt = !batVolt;
  elapsedBat = 0;
}

// ============================================================================
// FIRST LAYER - SOUS-FONCTIONS TACTILES - NIVEAU 14
// ============================================================================

void processFirstPresetTouch() {
  if (x < (150 + (bfoOn * 80)) && y > 25 && y < 60 && !VOLbut && !AGCgainbut && !SQUELCHbut) { // LWH - Added "and !SQUELCHbut"
    Beep(1, 0);
    delay(200);
    if (PREtap) {
      Beep(1, 0);
      PREtap = false;
      uint16_t  tmpFreq = PREfreq;
      uint8_t   tmpBand = PREband;
      uint8_t   tmpMode = PREmode;
      int       tmpBfo  = PREbfo;
      uint8_t   tmpStep = PREstep;
      uint8_t   tmpBw   = PREbw;

      PREfreq = si4735.getFrequency();
      PREband = bandIdx;
      PREmode = currentMode;
      if (isSSBLikeMode()) PREbfo  = currentBFOmanu; else PREbfo  = 0;
      PREstep = band[bandIdx].currentStep;
      if (currentMode == AM) PREbw = bwIdxAM; else if (currentMode == FM) PREbw = bwIdxFM; else PREbw = bwIdxSSB;

      if (PRE) {
        bandIdx = tmpBand;
        commSetFreq(tmpFreq);
        band[bandIdx].currentFreq = tmpFreq;
        currentMode = tmpMode;
        currentBFOmanu = tmpBfo;
        band[bandIdx].currentStep = tmpStep;
        si4735.setFrequencyStep(tmpStep);
        if (((band[bandIdx].bandType == MW_BAND_TYPE) || (band[bandIdx].bandType == LW_BAND_TYPE)) && ( currentMode == AM)) ssIdxMW = tmpStep;
        if ((band[bandIdx].bandType == SW_BAND_TYPE) && ( currentMode == AM)) ssIdxAM = tmpStep;
        if (currentMode == FM) ssIdxFM = tmpStep;
        if (currentMode == AM) bwIdxAM = tmpBw; else if (currentMode == FM) bwIdxFM = tmpBw; else bwIdxSSB = tmpBw;
        BandSet();
      } else PRE = true;
      DrawDispl();
      delay(200);
    } else {
      PREtap = true;
      elapsedPRE = millis();
      FreqDispl();
    }
  }
}

// ============================================================================
// FIRST LAYER - BOUTONS SEPARES - NIVEAU 15
// ============================================================================

void processFirstMainOCButton() {
  if (jamMainOCButton()) {
    BroadcastOCbut = true;
    drawBroadcastOCMenu();
    FirstLayer = false;
    SecondLayer = true;
  }
}

void prepareFirstButtonState(int n) {
  if ((VOLbut) && (n != B_VOL)) {
    VOLbut = false;
    drawButton(L_FIRST, B_VOL, B_NORMAL);
    DrawDispl ();
  }

  if (AGCgainbut && (n != B_ATT)) {
    AGCgainbut = false;
    drawButton(L_FIRST, B_ATT, B_NORMAL);
    FreqDispl();
  }
}

void processFirstAttButton(int n) {
  if (n == B_ATT) {
    if (AGCgainbut) AGCgainbut = false;
    else {
      AGCgainbut = true;
      si4735.getAutomaticGainControl();
      previousAGCgain = 38; // force to setup AGC gain
    }
    FreqDispl();
    if (AGCgainbut) drawButton(L_FIRST, B_ATT, B_SELECT); else drawButton(L_FIRST, B_ATT, B_NORMAL);
    drawButton(L_FIRST, B_AGC, B_NORMAL);
  }
}

void processFirstAgcButton(int n) {
  if (n == B_AGC) { //============================== AGC switch
    if  (AGCgain == 1) {
      AGCgain = 0; // disabled
      drawButton(L_FIRST, B_AGC, B_NORMAL);
    } else AGCgain = 1; //  enabled
    checkAGC();
    AGCfreqdisp();
  }
}

void processFirstHamButton(int n) {
  if (n == B_HAM) { //============================== HAM button
    HamBand = true;
    drawList(L_HAM, "HAM RADIO BAND");
    FirstLayer = false;
    SecondLayer = true;
  }
}

void processFirstBfoButton(int n) {
  if (n == B_BFO) { //============================== BFO button
    if (isSSBLikeMode())  {
      if (bfoOn) bfoOn = false; else bfoOn = true;
      if (bfoOn) {
        drawButton(L_FIRST, B_BFO, B_SELECT);
        drawButton(L_FIRST, B_STEP, B_NORMAL);
     } else {
        drawButton(L_FIRST, B_BFO, B_NORMAL);
        drawButton(L_FIRST, B_STEP, B_BLOCK);
      }
      bfoTr = true;
      DrawDispl ();
    } else ErrorBeep();
  }
}

void processFirstFreqButton(int n) {
  if (n == B_FREQ) { //============================ Frequency input
    FREQbut = true;
    drawList(L_FREQ, "FREQUENCY");
    tft.fillRect(0, 80, 240, 40, TFT_NAVY);
    Decipoint = false;
    DisplayfreqNew = 0;
    dpfrq = 0;
    Freqcalq(0);
    FirstLayer = false;
    SecondLayer = true;
  }
}

void processFirstModeButton(int n) {
  if (n == B_MODE) { //============================= MODE
    if (currentMode != FM)  {
      Modebut = true;
      drawList(L_MODE, "MODULATION");
      FirstLayer = false;
      SecondLayer = true;
    } else ErrorBeep();
  }
}

void processFirstBandwidthButton(int n) {
  if (n == B_BANDW) { //========================= BANDWIDTH
    BandWidth = true;
    if (currentMode == AM) drawList(L_BANDW_AM, "AM Filter in KHz");
    else if (currentMode == FM) drawList(L_BANDW_FM, "FM Filter in KHz");
    else drawList(L_BANDW_SSB, "SSB Filter in KHz");
    FirstLayer = false;
    SecondLayer = true;
  }
}

void processFirstStepButton(int n) {
  if (n == B_STEP) { //========================== STEPS for tune and bfo
    if (bfoOn) {
      drawButton(L_FIRST, B_STEP, B_NORMAL);
      setStep();
    } else if (isSSBLikeMode()) {
      ErrorBeep();
    } else {
      STEPbut = true;
      FirstLayer = false;
      SecondLayer = true;
      if (currentMode == AM) drawList(L_STEP_AM, "STEP TUNE AM"); else drawList(L_STEP_FM, "STEP TUNE FM");
    }
  }
}

void processFirstBandButton(int n) {
  if (n == B_BAND)  { //========================== BAND button
    BroadBand = true;
    drawList(L_BAND, "BAND");
    FirstLayer = false;
    SecondLayer = true;
  }
}

void processFirstVolumeButton(int n) {
  if (n == B_VOL) { //================================== VOL button
    if (bfoOn) {
      bfoOn = false;
      drawButton(L_FIRST, B_BFO, B_NORMAL);
    }
    if (VOLbut == false) {
      VOLbut = true;
      currentVOL = si4735.getVolume();
      previousVOL = currentVOL;
    } else {
      VOLbut = false;
    }
    FreqDispl();
    if (VOLbut) drawButton(L_FIRST, B_VOL, B_SELECT); else drawButton(L_FIRST, B_VOL, B_NORMAL);
  }
}

void processFirstMuteButton(int n) {
  if (n == B_MUTE) { //================================ MUTE button
    if (Mutestat) Mutestat = false; else Mutestat = true;
    if (!Mutestat) drawButton(L_FIRST, B_MUTE, B_NORMAL);
    if (Mutestat)
      si4735.setAudioMute(audioMuteOn);
    else
      si4735.setAudioMute(audioMuteOff);
  }
}

void processFirstNextButton(int n) {
  if (n == B_NEXT) { //================================ NEXT button
    FirstLayer  = false;
    SecondLayer = false;
    ThirdLayer  = true;
    ForthLayer  = false;
    DrawThla();
  }
}

void processFirstButtonsTouch() {
  processFirstMainOCButton();

  int n = BroadcastOCbut ? -1 : jamButton(L_FIRST);
  if (n < 0)
    return;

  prepareFirstButtonState(n);
  processFirstAttButton(n);
  processFirstAgcButton(n);
  processFirstHamButton(n);
  processFirstBfoButton(n);
  processFirstFreqButton(n);
  processFirstModeButton(n);
  processFirstBandwidthButton(n);
  processFirstStepButton(n);
  processFirstBandButton(n);
  processFirstVolumeButton(n);
  processFirstMuteButton(n);
  processFirstNextButton(n);
}

void processFirstLayerTouch() {
  if (!FirstLayer)
    return;

  if (isSSBLikeMode() && (x > 139) && (x < 219) && (y > 25) && (y < 60))
    processDigitSelectionTouch();

  if ((x > XVolInd + 161) && (x < XVolInd + 237) &&
      (y > YVolInd - 1) && (y < YVolInd + 26))
    processBatterySelectionTouch();

  processFirstPresetTouch();
  processFirstButtonsTouch();
}

// ============================================================================
// SECOND LAYER - SOUS-FONCTIONS TACTILES - NIVEAU 10
// ============================================================================

void processSecondModeTouch() {
  if (Modebut) {
    if (x > 20 && x < 220 && y > 20 && y < 60 && !VOLbut) {
      Modebut = false;
      DrawFila(); //Draw first layer
      delay(400);
    } else {
      int n = jamButton(L_MODE);
      if (n >= 0) {
        currentMode = n;
        drawListBut(L_MODE);
        delay(400);
        if ((CWShift == true) && (previousMode == USB)  ) {
          currentBFO = currentBFO - 700;
          band[bandIdx].lastBFO = currentBFO;
          freqDec = currentBFO;
          CWShift = false;
        }
        if ((currentMode !=  previousMode) && (currentMode == CW) && (CWShift == false)) {
          currentMode = USB;
          CWShift = true;
          currentBFO = currentBFO + 700;
          band[bandIdx].lastBFO = currentBFO;
          freqDec = currentBFO;
        }
        Modebut = false;
        previousMode = currentMode;
        band[bandIdx].prefmod = currentMode;
        BandSet();
        DrawFila();
      }
    }
  }
}

void processSecondBandwidthTouch() {
  if (BandWidth) {
    if (x > 20 && x < 220 && y > 20 && y < 60 && !VOLbut) {
      BandWidth = false;
      DrawFila(); //Draw first layer
      delay(200);
    } else {
      int b = L_BANDW_SSB;
      if (currentMode == AM) b = L_BANDW_AM;
      else if (currentMode == FM) b = L_BANDW_FM;
      int n = jamButton(b);
      if (n >= 0) {
        if (b == L_BANDW_AM) bwIdxAM = n;
        else if (b == L_BANDW_FM) bwIdxFM = n;
        else bwIdxSSB = n;
        drawListBut(b);
        delay(400);
        BandWidth = false;
        BandSet();
        DrawFila();
      }
    }
  }
}

void processSecondRetroCityTouch() {
  if (cityRETRObut) {
    Beep(1, 0);
    x = 0;
    y = 0;

    cityRETRObut = false;
    RETRObut = true;
    if (prevPresetId != PresetId) {
      prevPresetId = PresetId;
      currentPRES = 0;
      presetLoad();
      presetSort();
      presetSetPos();
    }
    drawRETRO();
    currentRetroFreq = 0;
  }
}

void processSecondRetroBandTouch() {
  if (bandRETRObut) {
    for (int n = 0 ; n <= lastBandRetro; n++) {
      if ((screenV && x > bandRetro[n].xPosV && x < (bandRetro[n].xPosV + But_Width) && y > bandRetro[n].yPosV && y < (bandRetro[n].yPosV + But_Height)) ||
          (!screenV && x > bandRetro[n].xPosH && x < (bandRetro[n].xPosH + But_Width) && y > bandRetro[n].yPosH && y < (bandRetro[n].yPosH + But_Height))) {
        Beep(1, 0);
        x = 0;
        y = 0;

        if (VHFon || n != 1) {
          if (n != RETROband) {
            RETROband = n;
            drawRetroBandBut();
            delay(200);
            presetSetPos();
            initRetro();
          } else drawRETRO();
          bandRETRObut = false;
          RETRObut = true;
          currentRetroFreq = 0;
        } else ErrorBeep();
      }
    }
  }
}

void processSecondRetroTouch() {
  if (RETRObut) {
    if (y > 40 && y < 200) {
      Beep(1, 0);
      x = 0;
      y = 0;
      if (bandRetro[RETROband].hardStep != bandRetro[RETROband].softStep || RETROband > 3) {
        if (bandHamRetro) {
          if (bfoOn) bfoOn = false; else bfoOn = true;
          if (VOLbut) {
            VOLbut = false;
            drawButton(L_RETRO, 2, B_NORMAL);
          }
        } else {
          if (band[bandIdx].currentStep == bandRetro[RETROband].hardStep) {
            si4735.setFrequencyStep(bandRetro[RETROband].softStep);
            band[bandIdx].currentStep = bandRetro[RETROband].softStep;
          } else {
            if (RETROband > 3 && band[bandIdx].currentStep == bandRetro[RETROband].softStep) {
              si4735.setFrequencyStep(10);
              band[bandIdx].currentStep = 10;
            } else {
              si4735.setFrequencyStep(bandRetro[RETROband].hardStep);
              band[bandIdx].currentStep = bandRetro[RETROband].hardStep;
            }
          }
        }
        drawRETROscale();
      } else ErrorBeep();
    }
      int n = jamButton(L_RETRO);
      if (n >= 0) {
        if (n == 0) { //CITY
          cityRETRObut = true;
          RETRObut = false;
          VOLbut = false;
          prevPresetId = PresetId;
          drawRetroCity();
        }

        if (n == 1) { //BAND
          bandRETRObut = true;
          RETRObut = false;
          VOLbut = false;
          drawRetroBand();
        }

        if (n == 2) { //VOL
          if (bfoOn) {
            bfoOn = false;
            drawRETROscale();
          }
          if (VOLbut) VOLbut = false; else VOLbut = true;
          if (VOLbut) drawButton(L_RETRO, 2, B_SELECT); else drawButton(L_RETRO, 2, B_NORMAL);
          delay(100);
        }

        if (n == 3) { //BACK
          band[bandIdx].currentFreq = si4735.getFrequency();
          RETRObut = false;
          Beep(1, 0);
          DrawThla(); //Draw third layer
        }
      }
  }
}

void processSecondMemoTouch() {
  if (MEMObut) {
   if (x > 20 && x < 220 && y > 20 && y < 60) {
      MEMObut = false;
      DrawThla(); //Draw third layer
      delay(200);
   } else {
    int d = !screenV * 40;
    if (x > d && x < (240 + d) && y > 80 && y < (240 - d)) {
      if ((int(y / 40) - 3 + currentMemo) >= 0 && (int(y / 40) - 3 + currentMemo) <= lastMemoBank) {
        tftTransRect(d, int(y / 40) * 40, 240, 40, TFT_OLIVE);
        if (presetBank) {
          int tmpBand = bandFreq(preset[int(y / 40) - 3 + currentMemo].presetIdx);
          if (tmpBand == 29 && (preset[int(y / 40) - 3 + currentMemo].presetIdx < 153 || preset[int(y / 40) - 3 + currentMemo].presetIdx > 30000)) {
            ErrorBeep();
          } else {
            Beep(1, 0);
            if (bandIdx != tmpBand || currentMode != bandMode[tmpBand]) {
              bandIdx = tmpBand;
              currentMode = bandMode[tmpBand];
              BandSet();
            }
            if (tmpBand) commSetFreq(preset[int(y / 40) - 3 + currentMemo].presetIdx); else commSetFreq(preset[int(y / 40) - 3 + currentMemo].presetIdx * 100);
            band[bandIdx].currentFreq = si4735.getFrequency();
            FreqDispl();
          }
        } else {
          if (MemoBank[int(y / 40) - 3 + currentMemo].freq < 153 || MemoBank[int(y / 40) - 3 + currentMemo].freq > 30000) {
            ErrorBeep();
          } else {
            Beep(1, 0);
            if (bandIdx != (MemoBank[int(y / 40) - 3 + currentMemo].band & 0x1F) || currentMode != trunc(MemoBank[int(y / 40) - 3 + currentMemo].band / 32)) {
              bandIdx = MemoBank[int(y / 40) - 3 + currentMemo].band & 0x1F;
              currentMode = trunc(MemoBank[int(y / 40) - 3 + currentMemo].band / 32);
              BandSet();
            }
            commSetFreq(MemoBank[int(y / 40) - 3 + currentMemo].freq);
            band[bandIdx].currentFreq = MemoBank[int(y / 40) - 3 + currentMemo].freq;
            FreqDispl();
          }
        }
        displMEMO();
      }
    }
      int n = jamButton(L_MEMO);
      if (n >= 0) {
        if (n == 0 && !presetBank) { //EDIT
          if (MEMOadd || MEMOdel) {
            drawButton(L_MEMO, 0, B_NORMAL);
            drawButton(L_MEMO, 3, B_NORMAL);
            if (MEMOadd) {
              MEMOadd = false;
              drawButton(L_MEMO, 1, B_NORMAL);
              MemoBank[currentMemo].freq = addMemoFreq;
              MemoBank[currentMemo].band = addMemoBand + (addMemoMode * 32);
              for (int i = 0; i < 21; i++) MemoBank[currentMemo].Name[i] = addMemoName[i];
            } else {
              MEMOdel = false;
              drawButton(L_MEMO, 2, B_NORMAL);
              drawButton(L_MEMO, 1, B_NORMAL);
              MemoBank[currentMemo].freq = 0;
            }
            displMEMO();
          } else {
            if (MemoBank[currentMemo].freq < 153 || MemoBank[currentMemo].freq > 30000) {
              drawButton(L_MEMO, 0, B_NORMAL);
              ErrorBeep();
            } else {
              MEMOadd = true;
              drawButton(L_MEMO, 0, B_NORMAL, "OK");
              drawButton(L_MEMO, 3, B_NORMAL, "X");
              drawButton(L_MEMO, 1, B_SELECT);
              addMemoFreq = MemoBank[currentMemo].freq;
              addMemoBand = MemoBank[currentMemo].band & 0x1F;
              addMemoMode = trunc(MemoBank[currentMemo].band / 32);
              for (int i = 0; i < 21; i++) addMemoName[i] = MemoBank[currentMemo].Name[i];
              posMemoName = 0;
              charMemoName = addMemoName[0];
              displMEMO();
            }
          }
        }

        if (n == 1 && !presetBank) { //ADD
          if (!MEMOadd && !MEMOdel) {
            MEMOadd = true;
            drawButton(L_MEMO, 0, B_NORMAL, "OK");
            drawButton(L_MEMO, 3, B_NORMAL, "X");
            drawButton(L_MEMO, 1, B_SELECT);
            for (int i = 0; i < 21; i++) addMemoName[i] = char(32);
            addMemoFreq = uint16_t(si4735.getFrequency());
            addMemoBand = bandIdx;
            addMemoMode = currentMode;
            posMemoName = 0;
            charMemoName = 32;
            displMEMO();
          } else {
            ErrorBeep();
            if (MEMOadd) drawButton(L_MEMO, 1, B_SELECT);
          }
        }

        if (n == 2 && !presetBank) { //DEL
          if (!MEMOadd && !MEMOdel) {
            MEMOdel = true;
            drawButton(L_MEMO, 0, B_NORMAL, "OK");
            drawButton(L_MEMO, 3, B_NORMAL, "X");
            drawButton(L_MEMO, 1, B_BLOCK);
            drawButton(L_MEMO, 2, B_SELECT);
            displMEMO();
            if (!MEMOdel) {
              displMEMO();
              drawButtons(L_MEMO);
            }
          } else if (MEMOadd) {
            if (charMemoName == 32 && posMemoName) {
              posMemoName--;
            } else {
              charMemoName = 32;
            }
            drawButton(L_MEMO, 2, B_NORMAL);
          } else {
            ErrorBeep();
            drawButton(L_MEMO, 2, B_SELECT);
          }
        }

        if (n == 3) { //BACK
          if (MEMOadd || MEMOdel) {
            drawButtons(L_MEMO);
            MEMOadd = false;
            MEMOdel = false;
            displMEMO();
          } else {
            MEMObut = false;
            Beep(1, 0);
            DrawThla(); //Draw third layer
          }
        }
      }
    }
  }
}

void processSecondSetupTouch() {
  if (SETUPbut) {
    if (x > (!screenV * 40) && x < (240 + (!screenV * 40)) && y > 40 && y < 200) {
      Beep(1, 0);
      changeSETUP(int(y / 40) - 1);
      displSETUP();
    }
      int n = jamButton(L_SETUP);
      if (n >= 0) {
        if (n == 0) { //PREV
          if (!pageSetup) {
            ErrorBeep();
            drawButton(L_SETUP, 0, B_BLOCK);
          } else {
            pageSetup--;
            displSETUP();
            if (pageSetup) drawButton(L_SETUP, 0, B_NORMAL); else drawButton(L_SETUP, 0, B_BLOCK);
            drawButton(L_SETUP, 1, B_NORMAL);
          }
        }

        if (n == 1) { //NEXT
          if (pageSetup == maxPageSetup) {
            ErrorBeep();
            drawButton(L_SETUP, 1, B_BLOCK);
          } else {
            pageSetup++;
            displSETUP();
            if (pageSetup < maxPageSetup) drawButton(L_SETUP, 1, B_NORMAL); else drawButton(L_SETUP, 1, B_BLOCK);
            drawButton(L_SETUP, 0, B_NORMAL);
          }
        }

        if (n == 2) { //RESET
          defaultSETUP();
          displSETUP();
          drawButton(L_SETUP, 2, B_NORMAL);
        }

        if (n == 3) { //BACK
          SETUPbut = false;
          saveSETUP();
          if (!SETUPbut) {
            DrawThla();
            Beep(1, 0);
          }
        }
      }
  }
}

void processSecondScanTouch() {
  if (SCANbut && STEPbut == false) {
    int d = screenV * 40;
    if (x > 20 && x < 220 && y > 20 && y < 60 && !VOLbut) {
      SCANstep = 0;
      SCANpause = true;
      pauseSCAN();
      if (isSSBLikeMode()) band[bandIdx].lastBFO = currentBFO = 0;
      restoreMuteIfNeeded();
      SCANbut = false;
      Beep(1, 0);
      DrawThla(); //Draw third layer
      x = y = 0;
      delay(200);
    }
    if (y > 80 && y < 200) {
      Beep(1, 0);
      if (SCANpause) {
        float tmpdelta = deltaScanLine;
        if (x < 40 && (currentScanFreq + int((deltaScanLine - 159 + d) * SCANstep)) > band[bandIdx].minimumFreq) deltaScanLine -= (40 - x);
        if (x > (280 - (d * 2)) && (currentScanFreq + int((deltaScanLine + 160 - d) * SCANstep)) < band[bandIdx].maximumFreq) deltaScanLine += (x - 280 + (d * 2));
        float tmpfreq = currentScanFreq + int((x - 159 + d + deltaScanLine) * SCANstep);
        int tmpline = currentScanLine;
        if (tmpfreq <= band[bandIdx].maximumFreq && tmpfreq >= band[bandIdx].minimumFreq) {
          currentScanLine = x;
        } else {
          if (tmpdelta != deltaScanLine) {
            if (tmpfreq > band[bandIdx].maximumFreq) {
              deltaScanLine -= ((tmpfreq - band[bandIdx].maximumFreq) / SCANstep);
            } else {
              deltaScanLine += ((band[bandIdx].minimumFreq - tmpfreq) / SCANstep);
            }
            tmpfreq = currentScanFreq + int((x - 159 + d + deltaScanLine) * SCANstep);
            currentScanLine = x;
          }
        }
        if (tmpline != currentScanLine || tmpdelta != deltaScanLine) {
          setFreq(tmpfreq);
          if (isSSBLikeMode()) band[bandIdx].lastBFO = currentBFO = 0;
          if (tmpdelta == deltaScanLine) {
            if (currentScanLine < tmpline) currentScanLine -= 10;
            drawSCANline(tmpline);
            if (currentScanLine < tmpline) currentScanLine += 10;
            drawSCANline(currentScanLine);
          } else {
            tmpdelta = abs(tmpdelta - deltaScanLine);
            if (x < 40) {
              for (int i = 319 - (d * 2); i > tmpdelta - 1; i--) {
                ScanValueRSSI[i] = ScanValueRSSI[i - int(tmpdelta)];
                ScanValueSNR[i] = ScanValueSNR[i - int(tmpdelta)];
              }
              for (int i = 0; i < tmpdelta; i++) {
                ScanValueRSSI[i] = 198 + d;
                ScanValueSNR[i] = 0;
              }
            } else {
              for (int i = tmpdelta; i < (320 - (d * 2)); i++) {
                ScanValueRSSI[i - int(tmpdelta)] = ScanValueRSSI[i];
                ScanValueSNR[i - int(tmpdelta)] = ScanValueSNR[i];
              }
              for (int i = 320 - (d * 2) - tmpdelta; i < (320 - (d * 2)); i++) {
                ScanValueRSSI[i] = 198 + d;
                ScanValueSNR[i] = 0;
              }
            }
            ScanEmpty = true;
            clearScanMarks();
            drawSCANgraf(false);
            posScanFreq = currentScanFreq + int((deltaScanLine - 159 + d) * SCANstep);
            posScan = 0;
          }
          FreqDispl();
          DrawSCANtxt(true);
          DisplaySCANsignal();
          DrawSCANind();
        }
      } else {
        if (ScanEmpty) {
          ErrorBeep();
        } else {
          Beep(1, 0);
          int tmpMax = 198 + d;
          float tmpMid = 0;
          for (int i = 0; i < (320 - (d * 2)); i++) {
            tmpMid += 198 + d - ScanValueRSSI[i];
            if (ScanValueRSSI[i] < tmpMax) tmpMax = ScanValueRSSI[i];
          }
          tmpMid = (140 + (d / 2)) / (float(tmpMid) / (320 - (d * 2)));
          if ((198 + d - ((198 + d - tmpMax) * tmpMid)) < (100 + (d / 2))) tmpMid = (98 + (d / 2)) / float(198 + d - tmpMax);
          if (tmpMid != 1 && tmpMid > 0) {
            if ((signalScale * tmpMid) > 10) tmpMid = 10 / signalScale;
            signalScale *= tmpMid;
            for (int i = 0; i < (320 - (d * 2)); i++) ScanValueRSSI[i] = 198 + d - ((198 + d - ScanValueRSSI[i]) * tmpMid);
            drawSCANgraf(false);
            DrawSCANtxt(true);
          }
        }
      }
    }
      int n = jamButton(L_SCAN);
      if (n >= 0) {
        if (n == 0) { //SCALE
          drawButton(L_SCAN, 0, B_NORMAL);
          deltaScanLine += currentScanLine - 159 + d;
          currentScanLine = 159 - d;
          SCANstep *= 2;
          if (SCANstep > currentMaxScanStep) SCANstep = currentMinScanStep;
          if (SCANstep == currentMinScanStep) deltaScanLine *= (currentMaxScanStep / currentMinScanStep); else deltaScanLine /= 2;
          SCANpause = false;
          pauseSCAN();
          drawSCANgraf(true);
          DrawSCANtxt(true);
        }

        if (n == 1) { //PAUSE
          SCANpause = !SCANpause;
          pauseSCAN();
        }

        if (n == 2) { //STEP
          if (isSSBLikeMode()) {
            if (bfoOn) setStep(); else ErrorBeep();
            drawButton(L_SCAN, 2, B_NORMAL);
          } else {
            SCANpause = true;
            pauseSCAN();
            STEPbut = true;
            if (currentMode == AM) drawList(L_STEP_AM, "STEP TUNE AM"); else drawList(L_STEP_FM, "STEP TUNE FM");
          }
        }

        if (n == 3) { //BACK
          SCANpause = true;
          pauseSCAN();
          band[bandIdx].currentFreq = si4735.getFrequency();
          if (isSSBLikeMode()) band[bandIdx].lastBFO = currentBFO = 0;
          restoreMuteIfNeeded();
          SCANbut = false;
          Beep(1, 0);
          DrawThla(); //Draw third layer
        }
      }
  }
}

void processSecondStepTouch() {
  if (STEPbut) {
    if (x > 20 && x < 220 && y > 20 && y < 60 && !VOLbut) {
      STEPbut = false;
      if (SCANbut) {
        SCANpause = false;
        pauseSCAN();
        drawSCAN();
        drawSCANgraf(false);
        DrawSCANtxt(true);
        x = y = 0;
      } else DrawFila(); //Draw first layer
      delay(200);
    } else {
      int b = L_STEP_AM;
      if (currentMode == FM) b = L_STEP_FM;
      int n = jamButton(b);
      if (n >= 0) {
        if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
          ssIdxMW = n;
          si4735.setFrequencyStep(ssIdxMW);
          band[bandIdx].currentStep = ssIdxMW;
        } else if (currentMode == FM) {
          ssIdxFM = n;
          si4735.setFrequencyStep(ssIdxFM);
          band[bandIdx].currentStep = ssIdxFM;
        } else {
          ssIdxAM = n;
          si4735.setFrequencyStep(ssIdxAM);
          band[bandIdx].currentStep = ssIdxAM;
        }
        setStep();
        drawListBut(b);
        delay(400);
        STEPbut = false;
        if (SCANbut) {
          drawSCAN();
          SCANpause = false;
          pauseSCAN();
          drawSCANgraf(false);
          DrawSCANtxt(true);
        } else DrawFila();
      }
    }
  }
}

void processSecondBroadcastOCTouch() {
  if (BroadcastOCbut) {
    int selectedBand = jamBroadcastOCMenu();
    if (selectedBand == -2) { // BACK ou appui sur le titre
      BroadcastOCbut = false;
      DrawThla();
      delay(200);
    } else if (selectedBand >= 0) {
      if (CWShift == true) {
        currentBFO = currentBFO - 700;
        band[bandIdx].lastBFO = currentBFO;
        CWShift = false;
      }
  #ifdef IhaveCrystal
      band[bandIdx].lastmanuBFO = currentBFOmanu;
  #endif
      bandIdx = selectedBand;
      si4735.setAM();
      delay(50);
      currentMode = AM;
      currentBFO = band[bandIdx].lastBFO;
      freqDec = currentBFO;
  #ifdef IhaveCrystal
      currentBFOmanu = band[bandIdx].lastmanuBFO;
  #endif
      ssbLoaded = false;
      BroadcastOCbut = false;
      BandSet();
      DrawFila();
      delay(250);
    }
  }
}

void processSecondBroadcastBandTouch() {
  if (BroadBand) {
    if (CWShift == true)  { // CW reset
      currentBFO = currentBFO - 700;
      band[bandIdx].lastBFO = currentBFO;
      CWShift = false;
    }
  #ifdef IhaveCrystal
    band[bandIdx].lastmanuBFO = currentBFOmanu;
  #endif

    if (x > 20 && x < 220 && y > 20 && y < 60 && !VOLbut) {
      BroadBand = false;
      DrawFila(); //Draw first layer
      delay(200);
    } else {
      int n = jamButton(L_BAND);
      if (n >= 0) {
        bandIdx = n;
        drawListBut(L_BAND);
        delay(400);
        BroadBand = false;
        if (bandIdx == 0 && currentAGCgain > 26) currentAGCgain = previousAGCgain = 26; // currentAGCgain in FM max. 26
        si4735.setAM();
        delay(50);
        currentBFO = band[bandIdx].lastBFO;
        freqDec = currentBFO;
        currentMode = band[bandIdx].prefmod;
  #ifdef IhaveCrystal
        currentBFOmanu = band[bandIdx].lastmanuBFO;
  #endif
        ssbLoaded = false;
        BandSet();
        DrawFila(); //Draw first layer
      }
    }
  }
}

void processSecondHamBandTouch() {
  if (HamBand) {
    if (CWShift == true)  { // CW reset
      currentBFO = currentBFO - 700;
      band[bandIdx].lastBFO = currentBFO;
      CWShift = false;
    }
  #ifdef IhaveCrystal
    band[bandIdx].lastmanuBFO = currentBFOmanu;
  #endif

    if (x > 20 && x < 220 && y > 20 && y < 60 && !VOLbut) {
      HamBand = false;
      DrawFila(); //Draw first layer
      delay(200);
    } else {
      int n = jamButton(L_HAM);
      if (n >= 0) {
        bandIdx = n;
        drawListBut(L_HAM);
        delay(400);
        HamBand = false;
        if (ssbLoaded == false) {
          si4735.setAM();
          delay(50);
        }
  #ifdef IhaveCrystal
        currentBFOmanu = band[bandIdx].lastmanuBFO;
  #endif
        currentBFO = band[bandIdx].lastBFO;
        freqDec = currentBFO;
        currentMode = band[bandIdx].prefmod;
        BandSet();
        DrawFila();
      }
    }
  }
}

void processSecondFrequencyTouch() {
  if (FREQbut) {
    if (CWShift == true)  { // CW reset
      currentBFO = currentBFO - 700;
      band[bandIdx].lastBFO = currentBFO;
      CWShift = false;
    }
  #ifdef IhaveCrystal
    band[bandIdx].lastmanuBFO = currentBFOmanu;
  #endif

    if (x > 20 && x < 220 && y > 20 && y < 60 && !VOLbut) {
      FREQbut = false;
      DrawFila(); //Draw first layer
      delay(200);
    } else {
      int n = jamButton(L_FREQ);
      if (n >= 0) {
       if (n != 9 || (!Decipoint && DisplayfreqNew >= (band[0].minimumFreq / 10) && DisplayfreqNew <= (band[0].maximumFreq / 10))) {
        drawButton(L_FREQ, n, B_NORMAL);

        if ((n >= 0) && (n <= 8)) Freqcalq(n + 1);
        if (n == 10) Freqcalq(0);
        if (n == 9 && Decipoint == false && DisplayfreqNew >= (band[0].minimumFreq / 10) && DisplayfreqNew <= (band[0].maximumFreq / 10)) {
          Decipoint = true;
          fact = 10;
          Freqcalq(10);
        }
        if (n > 11 && n < 14) Freqcalq(n); // DEL or CLS button
        if (n == 14) { // X button
          FREQbut = false;
          DrawFila(); //Draw first layer
        }
        if (n == 11) { // SET button
            FREQbut = false;
            DisplayfreqNew = (DisplayfreqNew / 10) + dpfrq;
            if ((DisplayfreqNew > 30 && DisplayfreqNew < (band[0].minimumFreq / 100)) || (DisplayfreqNew > 108 && DisplayfreqNew < 153 ) || DisplayfreqNew == 0) {
              tft.setTextSize(2);
              tft.setTextColor(TFT_WHITE, TFT_RED);
              tft.setCursor(0, 97);
              tft.print("Freqency not support");
              ErrorBeep();
            } else {
                if ((DisplayfreqNew >= (band[0].minimumFreq / 100)) && (DisplayfreqNew <= (band[0].maximumFreq / 100))) {
                  currentFrequency = DisplayfreqNew * 100;
                  bandIdx = 0;
                  band[bandIdx].currentFreq = currentFrequency;
                } else {
                  if (DisplayfreqNew < 153) currentFrequency = DisplayfreqNew * 1000; else currentFrequency = DisplayfreqNew;
                  for (int q = 1 ; q <= lastBand; q++) {
                    if (((currentFrequency) >= band[q].minimumFreq) && ((currentFrequency) <= band[q].maximumFreq)) {
                      bandIdx = q;
                      currentMode = band[q].prefmod;
                      if (((band[bandIdx].bandType == MW_BAND_TYPE) || (band[bandIdx].bandType == LW_BAND_TYPE)) && ( currentMode == AM)) {
                        ssIdxMW = band[bandIdx].currentStep;
                      }

                      if ((band[bandIdx].bandType == SW_BAND_TYPE) && ( currentMode == AM)) {
                        ssIdxAM = band[bandIdx].currentStep;
                      }

                      if (currentMode == FM) {
                        ssIdxFM = band[bandIdx].currentStep;
                      }
                      break;
                    }
                  }
                  delay(100);
                  band[bandIdx].currentFreq = currentFrequency;
                  freqDec = currentBFO = band[bandIdx].lastBFO = 0;
                }
            }
  #ifdef IhaveCrystal
          currentBFOmanu = band[bandIdx].lastmanuBFO;
  #endif
          BandSet();
          DrawFila();
        } //   End   n=11 Send button
       }
      }
    }
  }
}

void processSecondVolumeTouch() {
  if (VOLbut && !RETRObut) {
    VOLbut = false;
    FreqDispl();
    if (SCANbut) {
      DrawSCANind();
      DisplaySCANsignal();
    }
  }
}

void processSecondLayerTouch() {
  if (!SecondLayer)
    return;

  processSecondModeTouch();
  processSecondBandwidthTouch();
  processSecondRetroCityTouch();
  processSecondRetroBandTouch();
  processSecondRetroTouch();
  processSecondMemoTouch();
  processSecondSetupTouch();
  processSecondScanTouch();
  processSecondStepTouch();
  processSecondBroadcastOCTouch();
  processSecondBroadcastBandTouch();
  processSecondHamBandTouch();
  processSecondFrequencyTouch();
  processSecondVolumeTouch();
}

// ============================================================================
// THIRD LAYER - SOUS-FONCTIONS TACTILES - NIVEAU 12
// ============================================================================

void processThirdPresetTouch() {
  if (x < (150 + (bfoOn * 80)) && y > 25 && y < 60 && !VOLbut && !PRESbut) { // PRE selection
    Beep(1, 0);
    delay(200);
    if (PREtap) {
      Beep(1, 0);
      PREtap = false;
      uint16_t  tmpFreq = PREfreq;
      uint8_t   tmpBand = PREband;
      uint8_t   tmpMode = PREmode;
      int       tmpBfo  = PREbfo;
      uint8_t   tmpStep = PREstep;
      uint8_t   tmpBw   = PREbw;

      PREfreq = si4735.getFrequency();
      PREband = bandIdx;
      PREmode = currentMode;
      if (isSSBLikeMode()) PREbfo  = currentBFOmanu; else PREbfo  = 0;
      PREstep = band[bandIdx].currentStep;
      if (currentMode == AM) PREbw = bwIdxAM; else if (currentMode == FM) PREbw = bwIdxFM; else PREbw = bwIdxSSB;

      if (PRE) {
        bandIdx = tmpBand;
        commSetFreq(tmpFreq);
        band[bandIdx].currentFreq = tmpFreq;
        currentMode = tmpMode;
        currentBFOmanu = tmpBfo;
        band[bandIdx].currentStep = tmpStep;
        si4735.setFrequencyStep(tmpStep);
        if (((band[bandIdx].bandType == MW_BAND_TYPE) || (band[bandIdx].bandType == LW_BAND_TYPE)) && ( currentMode == AM)) ssIdxMW = tmpStep;
        if ((band[bandIdx].bandType == SW_BAND_TYPE) && ( currentMode == AM)) ssIdxAM = tmpStep;
        if (currentMode == FM) ssIdxFM = tmpStep;
        if (currentMode == AM) bwIdxAM = tmpBw; else if (currentMode == FM) bwIdxFM = tmpBw; else bwIdxSSB = tmpBw;
        BandSet();
      } else PRE = true;
      DrawDispl();
      delay(400);
    } else {
      PREtap = true;
      elapsedPRE = millis();
      FreqDispl();
    }
  }
}

// ============================================================================
// THIRD LAYER - BOUTONS SEPARES - NIVEAU 13
// ============================================================================

bool prepareThirdButtonState(int n) {
  boolean PRESoff = false;
    // ==================== Squelch Button Disable =================== LWH
  if (SQUELCHbut && (n != B_SQUELCH)) {
    SQUELCHbut = false;
    squelch = false;
    drawButton(L_FIRST, B_SQUELCH, B_NORMAL);
    FreqDispl();
  }
    // ===============================================================
  if ((bright) && (n != B_LIGHT)) {
    bright = false;
    drawButton(L_THIRD, B_LIGHT, B_NORMAL);
  }

  if (VOLbut) {
    VOLbut = false;
    DrawDispl ();
  }

  if (PRESbut && n != B_RDS && n != B_LIGHT) {
    PRESbut = false; // Preset stopped after other button is pressed
    drawButton(L_THIRD, B_FM, B_NORMAL);
    DrawDispl();
    PRESoff = true;
  }

  if (bfoOn && (n == B_SCAN || n == B_RETRO || n == B_FM || n == B_MEMO || n == B_SETUP)) {
    bfoOn = false;
    if (n == B_FM) drawButton(L_FIRST, B_BFO, B_BLOCK); else drawButton(L_FIRST, B_BFO, B_NORMAL);
    DrawDispl ();
  }
  return PRESoff;
}

void processThirdFmButton(int n, bool PRESoff) {
  if (n == B_FM) { //============================== FM button
    if (!PRESoff) {
      if (currentMode != 0) { // geen fm ?
        bandIdx = 0;
        currentMode = 0;
        BandSet();
        DrawDispl();
        DrawButThla();
      }
      PRESbut = true;
      drawButton(L_THIRD, B_FM, B_SELECT);
      presetBank = false;
      presetLoad();
      presetSort();
      bool flag = false;
      for (int i = 0; i <= lastPreset; i++) if ((preset[i].presetIdx * 100) >= band[0].minimumFreq && (preset[i].presetIdx * 100) <= band[0].maximumFreq) flag = true;
      if (flag) {
        tft.fillRect(XFreqDispl, YFreqDispl + 20 , 239, 65, TFT_DARKCYAN);
        previousPRES = -1;
        previousFrequency = 0;
      } else {
        delay(200);
        PRESbut = false;
        drawButton(L_THIRD, B_FM, B_NORMAL);
      }
    }
  }
}

void processThirdRetroButton(int n) {
  if (n == B_RETRO) {
    RETRObut = true;
    presetLoad();
    presetSort();
    presetSetPos();
    initRetro();
    currentRetroFreq = 0;
    ThirdLayer = false;
    SecondLayer  = true;
  }
}

void processThirdMemoButton(int n) {
  if (n == B_MEMO) {
    MEMObut = true;
    drawList(L_MEMO, "MEMORY BANK");
    displMEMO();
    ThirdLayer = false;
    SecondLayer  = true;
  }
}

void processThirdSetupButton(int n) {
  if (n == B_SETUP) {
    SETUPbut = true;
    drawList(L_SETUP,"SETUP");
    drawButton(L_SETUP, 0, B_BLOCK);
    pageSetup = 0;

    prevVHFon = VHFon;
    prevlangRetroEN = langRetroEN;
    prevbeeperOn = beeperOn;
    prevdigitLigth = digitLigth;
    prevloadMemory = loadMemory;
    prevbatShow = batShow;
    prevmemoPreset = memoPreset;
    prevloadDefault = loadDefault;
    prevsaverOn = saverOn;
    prevsaverTime = saverTime;
    prevdisplayOff = displayOff;
    prevminSCANstep = minSCANstep;
    prevmaxSCANstep = maxSCANstep;
    prevautoSCANstep = autoSCANstep;
    prevSCANaccuracy = SCANaccuracy;
    prevscreenV = screenV;
    prevdisplayPower = displayPower;
    prevRDSalways = RDSalways;
    prevseekAccuracy = seekAccuracy;

    displSETUP();
    ThirdLayer = false;
    SecondLayer  = true;
  }
}

void processThirdScanButton(int n) {
  if (n == B_SCAN){
    int d = screenV * 40;
    SCANbut = true;
    ScanAGC = AGCgain;
    currentScanFreq = si4735.getFrequency();
    if (isSSBLikeMode()) {
      currentScanFreq -= (currentBFO / 1000);
      currentBFO = freqDec = band[bandIdx].lastBFO = 0;
      si4735.setSSBBfo(currentBFOmanu);
      commSetFreq(currentScanFreq);
    }

    if (autoSCANstep) {
      float tmp = float(band[bandIdx].maximumFreq - band[bandIdx].minimumFreq) / (320 - (d * 2));
      float i = maxSCANstep / 2;
      while (i >= minSCANstep) {
        if (tmp > i) {
          currentMinScanStep = i / 4;
          currentMaxScanStep = i * 2;
          i = 0;
        }
        i /= 2;
      }
      if (currentMinScanStep > 0.5) currentMinScanStep = 0.5;
      if ((currentMinScanStep < minSCANstep) || (isSSBLikeMode())) currentMinScanStep = minSCANstep;
      if (currentMaxScanStep == minSCANstep) currentMaxScanStep *= 2;
    } else {
      currentMinScanStep = minSCANstep;
      currentMaxScanStep = maxSCANstep;
    }
    SCANstep = currentMaxScanStep / 2;

    currentScanLine = 159 - d;
    deltaScanLine = 0;
    drawSCAN();
    SCANpause = false;
    pauseSCAN();
    drawSCANgraf(true);
    DrawSCANtxt(true);
    signalScale = 1.5 + (d / 80);
    ThirdLayer = false;
    SecondLayer  = true;
  }
}

void processThirdSeekUpButton(int n) {
  if (n == B_SEEKUP) {
      SEEK = true;
      SEEKdispl(0);
      drawButton(L_THIRD, B_SEEKUP, B_SELECT);
      if (currentMode != FM) { // No FM
        if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
          if (seekAccuracy) si4735.setSeekAmSpacing(1); else si4735.setSeekAmSpacing(9);
          si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
        }
        else {
          bandIdx = 29; // all sw
          if (seekAccuracy) si4735.setSeekAmSpacing(1); else si4735.setSeekAmSpacing(5);
          si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
        }
      }
      si4735.seekStationProgress(SeekFreq, checkStopSeeking,  SEEK_UP);
      delay(300);
      SEEK = false;
      drawButton(L_THIRD, B_SEEKUP, B_NORMAL);
      currentFrequency = si4735.getFrequency();
      band[bandIdx].currentFreq = currentFrequency ;
      previousFrequency = currentFrequency;
      FreqDraw(currentFrequency, 0);
      delay(300);
  }
}

void processThirdSeekDownButton(int n) {
  if (n == B_SEEKDN) {
      SEEK = true;
      SEEKdispl(1);
      drawButton(L_THIRD, B_SEEKDN, B_SELECT);
      if (currentMode != FM) { // No FM
        if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
          if (seekAccuracy) si4735.setSeekAmSpacing(1); else si4735.setSeekAmSpacing(9);
          si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
        } else {
          bandIdx = 29; // all sw
          if (seekAccuracy) si4735.setSeekAmSpacing(1); else si4735.setSeekAmSpacing(5);
          si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
        }
      }
      si4735.seekStationProgress(SeekFreq, checkStopSeeking,  SEEK_DOWN);
      delay(300);
      SEEK = false;
      drawButton(L_THIRD, B_SEEKDN, B_NORMAL);
      currentFrequency = si4735.getFrequency();
      band[bandIdx].currentFreq = currentFrequency ;
      previousFrequency = currentFrequency;
      FreqDraw(currentFrequency, 0);
      delay(300);
  }
}

void processThirdInfoButton(int n) {
  if (n == B_INFO) {
    subrstatus();
    DrawThla();
  }
}

void processThirdLightButton(int n) {
  if (n == B_LIGHT) {
    if (displayPower) {
      ErrorBeep();
    } else {
      if (bright == false)  {
        bright = true;
        if (bfoOn) bfoOn = false;
        if (PRESbut) drawButton(L_THIRD, B_FM, B_JAM);
        drawButton(L_THIRD, B_LIGHT, B_SELECT);
        previousBrightness = currentBrightness;
      }
      else {
        bright = false;
        if (PRESbut) drawButton(L_THIRD, B_FM, B_SELECT);
        drawButton(L_THIRD, B_LIGHT, B_NORMAL);
      }
    }
  }
}

void processThirdBackButton(int n) {
  if (n == B_BACK) {
    FirstLayer  = true;
    SecondLayer = false;
    ThirdLayer  = false;
    ForthLayer  = false;
    DrawFila();
  }
}

void processThirdRdsButton(int n) {
  if (n == B_RDS) {
    if (RDS) RDS = false;
    else RDS = true;
    if (!RDS) drawButton(L_THIRD, B_RDS, B_NORMAL);
  }
}

void processThirdSquelchButton(int n) {
  if (n == B_SQUELCH) {
    if (displayPower) {
      ErrorBeep();
    } else {
      if (squelch == false)  {
        squelch = true;
        SQUELCHbut = true;
        if (bfoOn) bfoOn = false;
        if (PRESbut) drawButton(L_THIRD, B_FM, B_JAM);
        drawButton(L_THIRD, B_SQUELCH, B_SELECT);
        previousSquelch = currentSquelch;
        DrawDispl();
      }
      else {
        squelch = false;
        SQUELCHbut = false;
        if (PRESbut) drawButton(L_THIRD, B_FM, B_SELECT);
        drawButton(L_THIRD, B_SQUELCH, B_NORMAL);
        FreqDispl();
      }
    }
  }
}

void processThirdButtonsTouch() {
  int n = jamButton(L_THIRD);
  if (n < 0)
    return;

  bool PRESoff = prepareThirdButtonState(n);
  processThirdFmButton(n, PRESoff);
  processThirdRetroButton(n);
  processThirdMemoButton(n);
  processThirdSetupButton(n);
  processThirdScanButton(n);
  processThirdSeekUpButton(n);
  processThirdSeekDownButton(n);
  processThirdInfoButton(n);
  processThirdLightButton(n);
  processThirdBackButton(n);
  processThirdRdsButton(n);
  processThirdSquelchButton(n);
}

void processThirdLayerTouch() {
  if (!ThirdLayer)
    return;

  if (isSSBLikeMode() && (x > 139) && (x < 219) && (y > 25) && (y < 60))
    processDigitSelectionTouch();

  if ((x > XVolInd + 161) && (x < XVolInd + 237) &&
      (y > YVolInd - 1) && (y < YVolInd + 26))
    processBatterySelectionTouch();

  processThirdPresetTouch();
  processThirdButtonsTouch();
}

void processForthLayerTouch() {
  if (!ForthLayer)
    return;
// ============================================================================
}

void processTouchInput() {
  if (!pressed)
    return;

  pressed = false;

  if (scrollRetro) {
    scrollRetro = 0;
    x = y = 0;
  }

  // Conserver strictement l'ordre historique des couches :
  // une couche peut en activer une autre pendant le meme appui.
  processFirstLayerTouch();
  processSecondLayerTouch();
  processThirdLayerTouch();
  processForthLayerTouch();
}

// ============================================================================
// SYNCHRONISATION DES ETATS DE LOOP - NIVEAU 18 FINAL
// ============================================================================

void updateRetroState() {
  if (currentRetroFreq != si4735.getFrequency() && RETRObut) {
    if (bandHamRetro) {
      if ((currentRetroFreq - (currentBFO / 1000)) < band[bandHamRetro].minimumFreq || (currentRetroFreq - (currentBFO / 1000)) > band[bandHamRetro].maximumFreq) {
        bandHamRetro = 0;
        currentRetroFreq = si4735.getFrequency() - (currentBFO / 1000);
        bandIdx = bandRetro[RETROband].band;
        currentMode = AM;
        BandSet();
        commSetFreq(currentRetroFreq);
        si4735.setFrequencyStep(bandRetro[RETROband].hardStep);
        band[bandIdx].currentStep = bandRetro[RETROband].hardStep;
        currentBFO = 0;
      }
      if (bandHamRetro) {
        currentBFO += ((si4735.getFrequency() - currentRetroFreq) * 1000);
        currentRetroFreq = si4735.getFrequency();
        if (currentBFO == 16000) {
          currentBFO = 0;
          currentRetroFreq -= 16;
          commSetFreq(currentRetroFreq);
        }
        if (currentBFO == -16000) {
          currentBFO = 0;
          currentRetroFreq += 16;
          commSetFreq(currentRetroFreq);
        }
      }
    } else currentRetroFreq = si4735.getFrequency();
    if (RETROband < 2) bandRetro[RETROband].currentFreq = currentRetroFreq / 100; else bandRetro[RETROband].currentFreq = currentRetroFreq;
    if (bandHamRetro) bandRetro[RETROband].currentFreq -= (currentBFO / 1000);
    drawRETROscale();
  }
}

void updateBfoState() {
  if (isSSBLikeMode()) // set BFO value in si4735
  {
    if ((currentBFO != previousBFO) || (currentBFOmanu != previousBFOmanu))
    {
      previousBFO = currentBFO;
      previousBFOmanu = currentBFOmanu;
      applyCurrentBFO();
      if (bfoOn && !RETRObut) FreqDispl();
    }
  }
}

void updatePresetState() {
  if (currentPRES != previousPRES && PRESbut) {
    if (currentPRES > lastPreset) currentPRES = 0;
    while ((preset[currentPRES].presetIdx * 100) < band[0].minimumFreq || (preset[currentPRES].presetIdx * 100) > band[0].maximumFreq) currentPRES++;

    previousPRES = currentPRES;
    tft.fillRect(XFreqDispl, YFreqDispl + 20 , 239, 36, TFT_DARKCYAN);
    tft.setTextSize(1);
    tft.setTextDatum(BL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_DARKCYAN );
    tft.drawString(String(preset[currentPRES].presetIdx, 2) + " MHz ", 1, 33); // 5 , 83 posizione freq su schermata FM (preset)

    tftRusSetFont(T1516_T);
    tftRusSetSize(1);
    tftRusSetColor(TFT_WHITE, TFT_TRANS);
    tftRusSetDatum(BC_T);
    tftRusSetStyle(NBL_T);
    tftRusSetCut(0, 18);
    tftRusPrint(String(preset[currentPRES].PresetName), 120, 50);
    tftRusSetCut(0, 0);
    if (tftRusLength(String(preset[currentPRES].PresetName)) > 18) {
      textScroll = 0;
      elapsedScroll = millis() + 3000;
      directScroll = 1;
    } else directScroll = 0;

    bandIdx = 0;
    commSetFreq((preset[currentPRES].presetIdx) * 100);
    band[bandIdx].currentFreq = si4735.getFrequency();
  }
}

void updateVolumeState() {
  if (currentVOL != previousVOL)
  {
    if (currentVOL > MaxVOL) currentVOL = MaxVOL;
    if (currentVOL < MinVOL) currentVOL = MinVOL;
    if (currentVOL == MinVOL) {
      Mutestat = true;
      if (FirstLayer) drawButton(L_FIRST, B_MUTE, B_JAM);
      si4735.setAudioMute(audioMuteOn);
    } else {
      if (Mutestat) {
        Mutestat = false;
        if (FirstLayer) drawButton(L_FIRST, B_MUTE, B_NORMAL);
        si4735.setAudioMute(audioMuteOff);
      }
    }
    previousVOL = currentVOL;
    si4735.setVolume(currentVOL);
    if (RETRObut) drawRetroVol(); else FreqDispl();
  }
}

void updateSquelchState() {
  if (currentSquelch != previousSquelch)
  {
    if (currentSquelch > MaxSQUELCH) currentSquelch = MaxSQUELCH;
    if (currentSquelch < MinSQUELCH) currentSquelch = MinSQUELCH;
    previousSquelch = currentSquelch;
    ledcWrite(LedChannelforTFT, currentSquelch);
    if (RETRObut) drawRetroVol(); else FreqDispl();
  }
}

void updateBrightnessState() {
  if (currentBrightness != previousBrightness)
  {
    if (currentBrightness < MaxBrightness) currentBrightness = MaxBrightness;
    if (currentBrightness > MinBrightness) currentBrightness = MinBrightness;
    previousBrightness = currentBrightness;
    ledcWrite(LedChannelforTFT, currentBrightness);
  }
}

void updateAgcState() {
  if (currentAGCgain != previousAGCgain)
  {
    AGCgain = 2;
    if (si4735.isCurrentTuneFM())  MaxAGCgain = MaxAGCgainFM;
    else MaxAGCgain = MaxAGCgainAM;

    if (currentAGCgain > MaxAGCgain) currentAGCgain = MaxAGCgain;
    if (currentAGCgain < MinAGCgain) currentAGCgain = MinAGCgain;

    previousAGCgain = currentAGCgain;
    si4735.setAutomaticGainControl(1, currentAGCgain);
    FreqDispl();
  }
}

void syncCommState() {
  if ((mjs_currentFrequency != currentFrequency) || (mjs_currentMode != currentMode) || (mjs_bandIdx != bandIdx) || (mjs_currentBFO != currentBFO))
  {
    commStuff();
  }
}

void loop() {
  //=======================================================================================
  processPcAndWifiControl();
  updateMainIndicators();

  manageSquelch();

  while (shouldStayInIdleLoop()) {
    processIdleLoop();
  }
  elapsedSaver = millis();

  encoderCheck(); // Check if the encoder has moved.
  encoderButtonCheck(); // Check if encoderbutton is pressed

  processTouchInput(); // end pressed

  if (currentRetroFreq != si4735.getFrequency() && RETRObut) {
    if (bandHamRetro) {
      if ((currentRetroFreq - (currentBFO / 1000)) < band[bandHamRetro].minimumFreq || (currentRetroFreq - (currentBFO / 1000)) > band[bandHamRetro].maximumFreq) {
        bandHamRetro = 0;
        currentRetroFreq = si4735.getFrequency() - (currentBFO / 1000);
        bandIdx = bandRetro[RETROband].band;
        currentMode = AM;
        BandSet();
        commSetFreq(currentRetroFreq);
        si4735.setFrequencyStep(bandRetro[RETROband].hardStep);
        band[bandIdx].currentStep = bandRetro[RETROband].hardStep;
        currentBFO = 0;
      }
      if (bandHamRetro) {
        currentBFO += ((si4735.getFrequency() - currentRetroFreq) * 1000);
        currentRetroFreq = si4735.getFrequency();
        if (currentBFO == 16000) {
          currentBFO = 0;
          currentRetroFreq -= 16;
          commSetFreq(currentRetroFreq);
        }
        if (currentBFO == -16000) {
          currentBFO = 0;
          currentRetroFreq += 16;
          commSetFreq(currentRetroFreq);
        }
      }
    } else currentRetroFreq = si4735.getFrequency();
    if (RETROband < 2) bandRetro[RETROband].currentFreq = currentRetroFreq / 100; else bandRetro[RETROband].currentFreq = currentRetroFreq;
    if (bandHamRetro) bandRetro[RETROband].currentFreq -= (currentBFO / 1000);
    drawRETROscale();
  }

  if (isSSBLikeMode()) // set BFO value in si4735
  {
    if ((currentBFO != previousBFO) || (currentBFOmanu != previousBFOmanu))
    {
      previousBFO = currentBFO;
      previousBFOmanu = currentBFOmanu;
      applyCurrentBFO();
      if (bfoOn && !RETRObut) FreqDispl();
    }
  }

  if (currentPRES != previousPRES && PRESbut) {
    if (currentPRES > lastPreset) currentPRES = 0;
    while ((preset[currentPRES].presetIdx * 100) < band[0].minimumFreq || (preset[currentPRES].presetIdx * 100) > band[0].maximumFreq) currentPRES++;

    previousPRES = currentPRES;
    tft.fillRect(XFreqDispl, YFreqDispl + 20 , 239, 36, TFT_DARKCYAN);
    tft.setTextSize(1);
    tft.setTextDatum(BL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_DARKCYAN );
    tft.drawString(String(preset[currentPRES].presetIdx, 2) + " MHz ", 1, 33); // 5 , 83 posizione freq su schermata FM (preset)

    tftRusSetFont(T1516_T);
    tftRusSetSize(1);
    tftRusSetColor(TFT_WHITE, TFT_TRANS);
    tftRusSetDatum(BC_T);
    tftRusSetStyle(NBL_T);
    tftRusSetCut(0, 18);
    tftRusPrint(String(preset[currentPRES].PresetName), 120, 50);
    tftRusSetCut(0, 0);
    if (tftRusLength(String(preset[currentPRES].PresetName)) > 18) {
      textScroll = 0;
      elapsedScroll = millis() + 3000;
      directScroll = 1;
    } else directScroll = 0;

    bandIdx = 0;
    commSetFreq((preset[currentPRES].presetIdx) * 100);
    band[bandIdx].currentFreq = si4735.getFrequency();
  }

  if (currentVOL != previousVOL)
  {
    if (currentVOL > MaxVOL) currentVOL = MaxVOL;
    if (currentVOL < MinVOL) currentVOL = MinVOL;
    if (currentVOL == MinVOL) {
      Mutestat = true;
      if (FirstLayer) drawButton(L_FIRST, B_MUTE, B_JAM);
      si4735.setAudioMute(audioMuteOn);
    } else {
      if (Mutestat) {
        Mutestat = false;
        if (FirstLayer) drawButton(L_FIRST, B_MUTE, B_NORMAL);
        si4735.setAudioMute(audioMuteOff);
      }
    }
    previousVOL = currentVOL;
    si4735.setVolume(currentVOL);
    if (RETRObut) drawRetroVol(); else FreqDispl();
  }

// ====================== Squelch Adjustment ======================= LWH
  if (currentSquelch != previousSquelch)
  {
    if (currentSquelch > MaxSQUELCH) currentSquelch = MaxSQUELCH;
    if (currentSquelch < MinSQUELCH) currentSquelch = MinSQUELCH;
    previousSquelch = currentSquelch;
    ledcWrite(LedChannelforTFT, currentSquelch);
    if (RETRObut) drawRetroVol(); else FreqDispl();
  }
// ================================================================
  if (currentBrightness != previousBrightness)
  {
    if (currentBrightness < MaxBrightness) currentBrightness = MaxBrightness;
    if (currentBrightness > MinBrightness) currentBrightness = MinBrightness;
    previousBrightness = currentBrightness;
    ledcWrite(LedChannelforTFT, currentBrightness);
  }

  if (currentAGCgain != previousAGCgain)
  {
    AGCgain = 2;
    if (si4735.isCurrentTuneFM())  MaxAGCgain = MaxAGCgainFM;
    else MaxAGCgain = MaxAGCgainAM;

    if (currentAGCgain > MaxAGCgain) currentAGCgain = MaxAGCgain;
    if (currentAGCgain < MinAGCgain) currentAGCgain = MinAGCgain;

    previousAGCgain = currentAGCgain;
    si4735.setAutomaticGainControl(1, currentAGCgain);
    FreqDispl();
  }

  if ((mjs_currentFrequency != currentFrequency) || (mjs_currentMode != currentMode) || (mjs_bandIdx != bandIdx) || (mjs_currentBFO != currentBFO))
  {
    commStuff();
  }

  //=======================================================================================
} // end loop
// ============================================================================

// ============================================================================

// ============================================================================
// INTERFACE RADIO / COMMANDES LOCALES
// ============================================================================
void commStuff()  {
// ============================================================================

/****** CALCULATIONS FOR DISPLAYING TUNED FREQUENCY -
      if (currentMode == LSB || currentMode == USB  || currentMode == CW ) {

       Displayfreq = (currentFrequency * 1000) - (band[bandIdx].lastBFO);
       if (CWShift) Displayfreq = Displayfreq + 700;
       int mhz = trunc(Displayfreq / 1000000);
       int khz = Displayfreq - (mhz * 1000000);
       khz = trunc(khz / 1000);
       int hz = Displayfreq - (mhz * 1000000) - (khz * 1000);
       char s[12] = {'\0'};
       if (mhz > 0) sprintf(s, "%i %03i.%02i", mhz, khz, hz / 10); else sprintf(s, "%i.%02i", khz, hz / 10);
*/

  // NOTE: There is so much bullshit in this code. E.g. the "currentFrequency" is often NOT the same
  float mjs_Frequency = 999;
  mjs_Frequency = si4735.getFrequency();
  if (bandIdx == 0) mjs_Frequency *= 10.0; // mjs FM band is bandIdx == 0, and is NOT in kHz
  Serial.print("F," + String((float)(mjs_Frequency - (band[bandIdx].lastBFO / 1000.0)),3) + ",M,"); // mjs
  if (currentMode == AM) Serial.print("AM,B,"); // mjs
  else if (currentMode == FM) Serial.print("FM,B,"); // mjs
  else if (currentMode == CW) Serial.print("CW,B,"); // mjs
  else if (currentMode == USB) Serial.print("USB,B,"); // mjs
  else if (currentMode == LSB) Serial.print("LSB,B,"); // mjs
  else Serial.print(String(currentMode) + ",B,"); // mjs

#if 1
  if ((currentMode == LSB || currentMode == USB  || currentMode == CW ) && (CWShift)) Serial.print(String((band[bandIdx].lastBFO * -1.0) + 700) + ",X,");
  else Serial.print(String(band[bandIdx].lastBFO * -1.0) + ",X,");
#else
  if ((currentMode == LSB || currentMode == USB  || currentMode == CW ) && (CWShift)) Serial.print(String((currentBFO * -1.0) + 700) + ",X,");
  else Serial.print(String(currentBFO * -1.0) + ",X,");
#endif

  Serial.println(String(bandIdx));

  mjs_currentFrequency = currentFrequency;

  mjs_currentMode = currentMode;

  mjs_bandIdx = bandIdx;

  mjs_currentBFO = currentBFO;

// ============================================================================
} // end loop
// ============================================================================

// ============================================================================
void checkAGC()  {
// ============================================================================
  si4735.getAutomaticGainControl();
  if (si4735.isAgcEnabled()) {
    if (AGCgain == 2) si4735.setAutomaticGainControl(1, currentAGCgain);
    if (AGCgain == 0) si4735.setAutomaticGainControl(1, 0); // disabled
  } else if (AGCgain == 1) si4735.setAutomaticGainControl(0, 0); // enabled
}

// ============================================================================
void VOLbutoff()  {
// ============================================================================
  if (((millis() - VOLbutOnTime) > MIN_ELAPSED_VOLbut_TIME * 30) && (VOLbut == true)) {
    VOLbut = false;
    if (FirstLayer) drawButton(L_FIRST, B_VOL, B_NORMAL);
    if (RETRObut) drawButton(L_RETRO, 2, B_NORMAL);
    if (!RETRObut) FreqDispl();
  }
  if (VOLbut == false) VOLbutOnTime = millis();
}

// ============================================================================

// ============================================================================
// RDS / HORLOGE / RSSI
// ============================================================================
void DisplayRDS() {
  if (currentMode != FM || !RDS)
    return;

  // Toujours interroger le decodeur RDS, meme si une autre couche est affichee.
  // checkRDS() decide ensuite quoi dessiner.
  checkRDS();
}

// ============================================================================
void showtimeRSSI() {
  //=======================================================================================
  if ((millis() - elapsedRSSI) > MIN_ELAPSED_RSSI_TIME * RSSIfact) // 150 * 1  = 150 msec refresh time RSSI
  {
    si4735.getCurrentReceivedSignalQuality();
    NewRSSI = si4735.getCurrentRSSI();
    NewSNR = si4735.getCurrentSNR();
   // ======================= Manage Squelch ========================= LWH
    if (SquelchUsesRSSI) {
      SignalQuality = NewRSSI;
    } else {
      SignalQuality = NewSNR;
    }
    if (!Mutestat) {
      if (SignalQuality >= currentSquelch){
        if (SCANpause == true) {
            si4735.setAudioMute(audioMuteOff);
            squelchDecay = millis();
        }
      } else {
        if (millis() > (squelchDecay + squelchDecayTime)) {
          si4735.setAudioMute(audioMuteOn);
        }
      }
   }
   // ================================================================
    OldRSSI = NewRSSI;
    showRSSI();
    elapsedRSSI = millis();
  }
}

// ============================================================================
void DisplayClock() {
  static unsigned long lastClockUpdate = 0;

  if (!((FirstLayer || ThirdLayer) && !PRESbut))
    return;

  if (WiFi.status() != WL_CONNECTED)
    return;

  if ((millis() - lastClockUpdate) < 1000)
    return;

  if (!getLocalTime(&timeinfo, 500))
    return;

  lastClockUpdate = millis();

  char timeHour[3];
  char timeMin[3];
  char timeDay[3];
  char timeMonth[3];
  char timeYear[5];

  strftime(timeHour, sizeof(timeHour), "%H", &timeinfo);
  strftime(timeMin, sizeof(timeMin), "%M", &timeinfo);
  strftime(timeDay, sizeof(timeDay), "%d", &timeinfo);
  strftime(timeMonth, sizeof(timeMonth), "%m", &timeinfo);
  strftime(timeYear, sizeof(timeYear), "%Y", &timeinfo);

  tft.fillRect(XFreqDispl, YFreqDispl + 23, 72, 44, TFT_BLACK);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM);
  tft.setTextPadding(0);
  tft.drawString(String(timeHour) + ":" + String(timeMin),
                 XFreqDispl + 3, YFreqDispl + 25);

  tft.setTextColor(TFT_GOLD, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString(String(timeDay) + "/" + String(timeMonth),
                 XFreqDispl + 8, YFreqDispl + 47);
  tft.drawString(String(timeYear),
                 XFreqDispl + 18, YFreqDispl + 59);
}

void showRSSI() {
// ============================================================================

  if ((  currentMode == FM ) && ((FirstLayer) || (ThirdLayer) || (SecondLayer && RETRObut && !RETROband))) {
    if (RETRObut) {
      int d = screenV * 80;
      if (si4735.getCurrentPilot()) tft.fillRect(298 - d, 25, 14, 5, TFT_RED); else tft.fillRect(298 - d, 25, 14, 5, TFT_BLACK);
    } else {
      sprintf(buffer, "%s", (si4735.getCurrentPilot()) ? "STEREO" : "MONO");
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.setTextSize(1);
      tft.setTextDatum(BC_DATUM);
      tft.setTextPadding(0);
      tft.fillRect(XFreqDispl + 195, YFreqDispl + 25 , 38, 12, TFT_RED); // STEREO MONO
      tft.drawString(buffer, XFreqDispl + 214, YFreqDispl + 36); //72);
    }
  }

  rssi = NewRSSI;
  if ((FirstLayer) || (ThirdLayer)) Smeter();
  if ((  currentMode == AM ) || (  currentMode == LSB )||  (  currentMode == USB )|| (  currentMode == CW )&& ((FirstLayer) || (ThirdLayer) || (SecondLayer && RETRObut && !RETROband)))
    {
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  if ((FirstLayer || ThirdLayer) && !PRESbut) { // dBuV and dB at freq. display
    tft.setTextDatum(TL_DATUM);
    tft.drawString("RSSI " + String(NewRSSI) + " dBuV " , XFreqDispl + 8, YFreqDispl + 75);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(" SNR " + String(NewSNR) + " dB", XFreqDispl + 180, YFreqDispl + 75);
    }
  }
  VOLbutoff();
  DisplayClock();
}

// ============================================================================

// ============================================================================
// GESTION ENCODEUR / BOUTONS
// ============================================================================
// ============================================================================
// ENCODEUR - SOUS-FONCTIONS - NIVEAU 16
// ============================================================================

void processEncoderScan(bool &mainpurp) {
  if (SCANbut && VOLbut == false && bfoOn == false) {
    int d = screenV * 40;
    pauseScanIfNeeded();
    float tmpcurline = (encoderCount == 1) ? (currentScanLine + (float(band[bandIdx].currentStep) / SCANstep)) : (currentScanLine - (float(band[bandIdx].currentStep) / SCANstep));
    if ((encoderCount == 1 && (currentScanFreq + ((tmpcurline - 159 + d + deltaScanLine) * SCANstep)) <= band[bandIdx].maximumFreq) || (encoderCount != 1 && (currentScanFreq + ((tmpcurline - 159 + d + deltaScanLine) * SCANstep)) >= band[bandIdx].minimumFreq)) {
      int tmpline = currentScanLine;
      currentScanLine = tmpcurline;

      //set frequency ========================================================
      if (isSSBLikeMode()) {
        if (encoderCount == 1) currentBFO -= 1000; else currentBFO += 1000;
        if (currentBFO <= -16000) {
          currentBFO += 16000;
          commSetFreq(si4735.getFrequency() + 16);
          checkAGC();
        }
        if (currentBFO >= 16000) {
          currentBFO -= 16000;
          commSetFreq(si4735.getFrequency() - 16);
          checkAGC();
        }
        applyCurrentBFO();
        band[bandIdx].lastBFO = currentBFO;
      } else {
        if (encoderCount == 1) commFreqUp(); else commFreqDown();
      }
// ============================================================================

      if (trunc(currentScanLine) < 0) {
        for (int i = 319 - (d * 2); i > (abs(currentScanLine) - 1); i--) {
          ScanValueRSSI[i] = ScanValueRSSI[int(i + currentScanLine)];
          ScanValueSNR[i] = ScanValueSNR[int(i + currentScanLine)];
        }
        for (int i = 0; i < abs(currentScanLine); i++) {
          ScanValueRSSI[i] = 198 + d;
          ScanValueSNR[i] = 0;
        }
        deltaScanLine += currentScanLine;
        currentScanLine = 0;
        ScanEmpty = true;
        clearScanMarks();
        drawSCANgraf(false);
        DrawSCANtxt(true);
        posScan = 0;
      } else {
        if (trunc(currentScanLine) > (319 - (d * 2))) {
          for (int i = currentScanLine - 319 + (d * 2); i < (320 - (d * 2)); i++) {
            ScanValueRSSI[int(i - currentScanLine + 319 - (d * 2))] = ScanValueRSSI[i];
            ScanValueSNR[int(i - currentScanLine + 319 - (d * 2))] = ScanValueSNR[i];
          }
          for (int i = 639 - (d * 4) - currentScanLine; i < (320 - (d * 2)); i++) {
            ScanValueRSSI[i] = 198 + d;
            ScanValueSNR[i] = 0;
          }
          deltaScanLine += currentScanLine - 319 + (d * 2);
          currentScanLine = 319 - (d * 2);
          ScanEmpty = true;
          clearScanMarks();
          drawSCANgraf(false);
          DrawSCANtxt(true);
          posScan = 0;
        } else {
          if (encoderCount != 1) currentScanLine -= 10;
          drawSCANline(tmpline);
          if (encoderCount != 1) currentScanLine += 10;
          drawSCANline(currentScanLine);
          DrawSCANtxt(true);
        }
      }
    }
    mainpurp = false;
  }
}

void processEncoderBfo(bool &mainpurp) {
  if (bfoOn)  {
  #ifdef IhaveSI5351
    currentBFOmanu = 0;
    if (encoderCount == 1) {
      FreqSI5351 = FreqSI5351 + stepsizesynth;
    } else {
      FreqSI5351 = FreqSI5351 - stepsizesynth;
    }
    calibratSI5351 = false;
    si5351wire.set_freq(FreqSI5351, CLK_Xtal);
    mainpurp = false;
  #endif

  #ifdef IhaveCrystal
    if (RETRObut) {
      currentBFOmanu = (encoderCount == 1) ? (currentBFOmanu - currentBFOStep) : (currentBFOmanu + currentBFOStep);
      if (currentBFOmanu == 1000) {
        currentBFOmanu = 0;
        currentRetroFreq--;
      }
      if (currentBFOmanu == -1000) {
        currentBFOmanu = 0;
        currentRetroFreq++;
      }
    } else {
      currentBFOmanu = (encoderCount == 1) ? (currentBFOmanu + currentBFOStep) : (currentBFOmanu - currentBFOStep);
      if (currentBFOmanu > 999 || currentBFOmanu < -999) currentBFOmanu = previousBFOmanu;
    }
    mainpurp = false;
  #endif
  }
}

void processEncoderRetro(bool &mainpurp) {
  if (RETRObut && !VOLbut && !bfoOn) { //retro scale
    if (scrollRetro) {
      if (encoderCount == 1) scrollRetro = 1; else scrollRetro = -1;
    } else {
      float tmpmax = bandRetro[RETROband].maximumFreq;
      float tmpmin = bandRetro[RETROband].minimumFreq;
      if (bandIdx == 0) {
        tmpmax *= 100;
        tmpmin *= 100;
      }
      if ((encoderCount == 1 && (currentRetroFreq + band[bandIdx].currentStep) <= tmpmax) || (encoderCount != 1 && (currentRetroFreq - band[bandIdx].currentStep) >= tmpmin)) {
        if (bandHamRetro) {
          currentRetroFreq += encoderCount;
        } else {
          if (encoderCount == 1) commFreqUp(); else commFreqDown();
        }
      }
    }
    mainpurp = false;
  }
}

void processEncoderRetroCity(bool &mainpurp) {
  if (cityRETRObut) {
    if (encoderCount == 1) cityRetroRotary = 1; else cityRetroRotary = -1;
    drawRetroCity();
    mainpurp = false;
  }
}

void processEncoderPreset(bool &mainpurp) {
  if (PRESbut && !bright) { // FM preset
    currentPRES = (encoderCount == 1) ? (currentPRES + currentPRESStep) : (currentPRES - currentPRESStep);
    if (currentPRES == 255) currentPRES = lastPreset;
    if (currentPRES > lastPreset) currentPRES = 0;
    while ((preset[currentPRES].presetIdx * 100) < band[0].minimumFreq || (preset[currentPRES].presetIdx * 100) > band[0].maximumFreq) {
      currentPRES = (encoderCount == 1) ? (currentPRES + currentPRESStep) : (currentPRES - currentPRESStep);
      if (currentPRES > lastPreset) {
        if (encoderCount == 1) currentPRES = 0; else currentPRES = lastPreset;
      }
    }
    mainpurp = false;
  }
}

void processEncoderBrightness(bool &mainpurp) {
  if (bright) { // Brightness control
    currentBrightness = (encoderCount == 1) ? (currentBrightness - 10) : (currentBrightness + 10);
    mainpurp = false;
  }
}

void processEncoderSquelch(bool &mainpurp) {
  if (SQUELCHbut) { // Squelch control
    currentSquelch = (encoderCount == 1) ? (currentSquelch + currentSQUELCHStep) : (currentSquelch - currentSQUELCHStep);
    SQUELCHbutOnTime = millis();
    squelchDecay = 0;
    mainpurp = false;
    si4735.getCurrentReceivedSignalQuality();
    if (SquelchUsesRSSI) {
      SignalQuality = NewRSSI;
    } else {
      SignalQuality = NewSNR;
    }
    if (SignalQuality >= currentSquelch){
      if (SCANpause == true) {
        si4735.setAudioMute(audioMuteOff);
      }
    } else {
      si4735.setAudioMute(audioMuteOn);
    }
  }
}

void processEncoderVolume(bool &mainpurp) {
  if (VOLbut) { // Volume control
    currentVOL = (encoderCount == 1) ? (currentVOL + currentVOLStep) : (currentVOL - currentVOLStep);
    VOLbutOnTime = millis();
    mainpurp = false;
  }
}

void processEncoderAgcGain(bool &mainpurp) {
  if (AGCgainbut) { // AGC gain control
    currentAGCgain = (encoderCount == 1) ? (currentAGCgain + currentAGCgainStep) : (currentAGCgain - currentAGCgainStep);
    mainpurp = false;
  }
}

void processEncoderSetup(bool &mainpurp) {
  if (SETUPbut) {
    pageSetup = (encoderCount == 1) ? (pageSetup + 1) : (pageSetup - 1);
    if (pageSetup < 0) pageSetup = 0;
    if (!pageSetup) drawButton(L_SETUP, 0, B_BLOCK); else drawButton(L_SETUP, 0, B_NORMAL);
    if (pageSetup > maxPageSetup) pageSetup = maxPageSetup;
    if (pageSetup == maxPageSetup) drawButton(L_SETUP, 1, B_BLOCK); else drawButton(L_SETUP, 1, B_NORMAL);
    displSETUP();
    mainpurp = false;
  }
}

void processEncoderMemo(bool &mainpurp) {
  if (MEMObut) {
    if (MEMOadd) {
      charMemoName = (encoderCount == 1) ? (charMemoName + 1) : (charMemoName - 1);
      if (charMemoName == 144 && encoderCount == 1) charMemoName = 32;
      if (charMemoName == 128 && encoderCount == 1) charMemoName = 144;
      if (charMemoName == 192) charMemoName = 128;
      if (charMemoName == 127 && encoderCount != 1) charMemoName = 191;
      if (charMemoName == 143 && encoderCount != 1) charMemoName = 127;
      if (charMemoName == 31) charMemoName = 143;
    } else {
      currentMemo = (encoderCount == 1) ? (currentMemo + 1) : (currentMemo - 1);
      if (currentMemo < 0) currentMemo = 0;
      if (presetBank) {
        if (currentMemo > lastPreset) currentMemo = lastPreset;
      } else {
        if (currentMemo > lastMemoBank) currentMemo = lastMemoBank;
      }
      displMEMO();
    }
    mainpurp = false;
  }
}

void processEncoderMainTune(bool &mainpurp) {
  if (mainpurp)
  {

  // ======================= Manage Squelch ========================= LWH
   if (!Mutestat) {
    si4735.getCurrentReceivedSignalQuality();
    if (SquelchUsesRSSI){
      SignalQuality = si4735.getCurrentRSSI();
    } else {
      SignalQuality = si4735.getCurrentSNR();
    }
    if (SignalQuality >= currentSquelch){
      if (SCANpause == true) {
        si4735.setAudioMute(audioMuteOff);
        squelchDecay = millis();
      }
    } else {
      if (millis() > (squelchDecay + squelchDecayTime)) {
        si4735.setAudioMute(audioMuteOn);
      }
    }
  }
  // ================================================================

    if (isSSBLikeMode()) {
      if (encoderCount == 1) {
        freqDec = freqDec - freqstep;
        int freqTot = (si4735.getFrequency() * 1000) + (freqDec * -1);
        if ( freqTot > (band[bandIdx].maximumFreq * 1000)) {
          commSetFreq(band[bandIdx].maximumFreq);
          freqDec = 0;
        }
        if (freqDec <= -16000)  {
          freqDec = freqDec + 16000;
          int freqPlus16 = currentFrequency + 16;
          MuteAudOn();
          commSetFreq(freqPlus16);
        }
        currentBFO = freqDec;
      } else {
        freqDec = freqDec + freqstep;
        int freqTot = (si4735.getFrequency() * 1000) - freqDec;
        if ( freqTot < (band[bandIdx].minimumFreq * 1000)) {
          commSetFreq(band[bandIdx].minimumFreq);
          freqDec = 0;
        }
        if (freqDec >= 16000)  {
          freqDec = freqDec - 16000;
          int freqMin16 = currentFrequency - 16;
          MuteAudOn();
          commSetFreq(freqMin16);
        }
        currentBFO = freqDec;
      }
      band[bandIdx].lastBFO = currentBFO;
      checkAGC();
    } else  {
      if (encoderCount == 1) {
        commFreqUp();
      } else {
        commFreqDown();
      }
    }
    band[bandIdx].currentFreq = si4735.getFrequency();
  }
}

void finishEncoderUpdate() {
  if (!PRESbut && !RETRObut && !VOLbut && !AGCgainbut && !bright && !cityRETRObut && !bandRETRObut && !SETUPbut && !MEMObut && !SQUELCHbut) FreqDispl(); // LWH - added "and SQUELCHbut"
  if (SCANbut) {
    DrawSCANind();
    DisplaySCANsignal();
  }
  encoderCount = 0;
}

void encoderCheck() {
  if (encoderCount != 0) {
    bool mainpurp = true;

    processEncoderScan(mainpurp);
    processEncoderBfo(mainpurp);
    processEncoderRetro(mainpurp);
    processEncoderRetroCity(mainpurp);
    processEncoderPreset(mainpurp);
    processEncoderBrightness(mainpurp);
    processEncoderSquelch(mainpurp);
    processEncoderVolume(mainpurp);
    processEncoderAgcGain(mainpurp);
    processEncoderSetup(mainpurp);
    processEncoderMemo(mainpurp);
    processEncoderMainTune(mainpurp);
    finishEncoderUpdate();
  }
}

// ============================================================================
// ============================================================================
// BOUTON ENCODEUR - APPUI LONG / COURT - NIVEAU 17
// ============================================================================

void processEncoderLongPress() {
  Beep(1, 0);
  pressed = tft.getTouch(&x, &y);
  if (pressed) {
  Beep(1, 0);
  x = y = 0;
  bool tmp;
  if (SCANbut) {
    tmp = SCANpause;
    SCANpause = true;
    pauseSCAN();
    deltaScanLine += currentScanLine - 159 + (screenV * 40);
  }
  screenV = !screenV;
  prevscreenV = screenV;
  if (SCANbut) {
    currentScanLine = 159 - (screenV * 40);
    ScanEmpty = true;
    clearScanMarks();
    drawSCANgraf(true);
    DrawSCANtxt(true);
    posScan = 0;
  }
  screenRotate();
  returnLayer();
  if (SCANbut) {
    SCANpause = false;
    pauseSCAN();
    SCANpause = tmp;
    pauseSCAN();
  }
  delay(500);
  } else if (MEMObut) {
  presetBank = !presetBank;
  if (presetBank) {
    presetLoad();
    presetSort();
    if (lastPreset < 0) presetBank = false; else currentMemo = 0;
  } else currentMemo = 0;
  drawButtons(L_MEMO);
  if (presetBank) {
    drawButton(L_MEMO, 0, B_BLOCK);
    drawButton(L_MEMO, 1, B_BLOCK);
    drawButton(L_MEMO, 2, B_BLOCK);
  }
  displMEMO();
  } else {
  if (isSSBLikeMode() && !RETRObut) {
    if (bfoOn) bfoOn = false; else bfoOn = true;
    if (FirstLayer) {
      if (bfoOn) {
        drawButton(L_FIRST, B_BFO, B_SELECT);
        drawButton(L_FIRST, B_STEP, B_NORMAL);
      } else {
        drawButton(L_FIRST, B_BFO, B_NORMAL);
        drawButton(L_FIRST, B_STEP, B_BLOCK);
      }
    }
    if (VOLbut) {
      VOLbut = false;
      if (FirstLayer) drawButton(L_FIRST, B_VOL, B_NORMAL);
    }
    bfoTr = true;
    FreqDispl();
    if (SCANbut) {
      if (bfoOn) drawButton(L_SCAN, 2, B_NORMAL); else drawButton(L_SCAN, 2, B_BLOCK);
      if (bfoOn) {
        pauseScanIfNeeded();
      } else {
        DrawSCANind();
        DisplaySCANsignal();
      }
    }
  } else if (RETRObut && RETROband > 3) {
    if (currentMode == LSB || currentMode == USB) {
      bandHamRetro = 0;
      currentRetroFreq = si4735.getFrequency() - (currentBFO / 1000);
      bandIdx = bandRetro[RETROband].band;
      currentMode = AM;
      BandSet();
      commSetFreq(currentRetroFreq);
      si4735.setFrequencyStep(5);
      band[bandIdx].currentStep = 5;
      currentBFO = 0;
      drawRETROscale();
    } else {
      int i = 1;
      while (currentRetroFreq < band[i].minimumFreq || currentRetroFreq > band[i].maximumFreq) i++;
      if (bandMode[i] == LSB || bandMode[i] == USB) {
        bandIdx = i;
        currentMode = bandMode[i];
        previousMode = currentMode;
        band[bandIdx].prefmod = currentMode;
        BandSet();
        currentBFO = 0;
        currentBFOmanu = 0;
        currentBFOStep = 25;
        commSetFreq(currentRetroFreq);
        bandHamRetro = i;
        drawRETROscale();
      }
    }
    checkAGC();
  } else ErrorBeep();
  }
}

// ============================================================================
// BOUTON ENCODEUR COURT - DOUBLE / SIMPLE - NIVEAU 18 FINAL
// ============================================================================

void processEncoderDoublePress() {
  while (analogRead(ENCODER_SWITCH) < 500);
  long encTime = millis() + 400;
  while (analogRead(ENCODER_SWITCH) > 500 && encTime > millis());
  if (analogRead(ENCODER_SWITCH) < 500) {
    if (RETRObut) scrollRetro = -1;
    if ((currentMode != LSB) && (currentMode != USB) && (currentMode != CW) && (ThirdLayer || FirstLayer))   {
      SEEK = true;
      SEEKdispl(1);
      if (ThirdLayer) drawButton(L_THIRD, B_SEEKDN, B_SELECT);
      if (currentMode != FM) {
        if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
          si4735.setSeekAmSpacing(1);
          si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
        }
        else {
          bandIdx = 29; // all sw
          si4735.setSeekAmSpacing(1);
          si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
        }
      }
      si4735.seekStationProgress(SeekFreq, checkStopSeeking,  SEEK_DOWN);
      delay(300);
      SEEK = false;
      if (ThirdLayer) drawButton(L_THIRD, B_SEEKDN, B_NORMAL);
      currentFrequency = si4735.getFrequency();
      band[bandIdx].currentFreq = currentFrequency ;
      previousFrequency = currentFrequency;
      FreqDraw(currentFrequency, 0);
      delay(300);
    }
  } else {
    if (RETRObut) scrollRetro = 1;
    if ((currentMode != LSB) && (currentMode != USB) && (currentMode != CW) && (ThirdLayer || FirstLayer))   {
      SEEK = true;
      SEEKdispl(0);
      if (ThirdLayer) drawButton(L_THIRD, B_SEEKUP, B_SELECT);
      if (currentMode != FM) {
        if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
          si4735.setSeekAmSpacing(1);
          si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
        } else {
          bandIdx = 29; // all sw
          si4735.setSeekAmSpacing(1);
          si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
        }
      }
      si4735.seekStationProgress(SeekFreq, checkStopSeeking,  SEEK_UP);
      delay(300);
      SEEK = false;
      if (ThirdLayer) drawButton(L_THIRD, B_SEEKUP, B_NORMAL);
      currentFrequency = si4735.getFrequency();
      band[bandIdx].currentFreq = currentFrequency ;
      previousFrequency = currentFrequency;
      FreqDraw(currentFrequency, 0);
      delay(300);
    }
  }
}

void processEncoderSinglePress() {
  bool mainpurp = true;

  if (scrollRetro) {
    scrollRetro = 0;
    mainpurp = false;
  }

  if (cityRETRObut) {
    cityRETRObut = false;
    RETRObut = true;
    if (prevPresetId != PresetId) {
      prevPresetId = PresetId;
      currentPRES = 0;
      presetLoad();
      presetSort();
      presetSetPos();
    }
    drawRETRO();
    currentRetroFreq = 0;
    mainpurp = false;
  }

  if (PRESbut) { // FM preset selection
    PRESbut = false;
    drawButton(L_THIRD, B_FM, B_NORMAL);
    DrawDispl();
    mainpurp = false;
  }

  if (bright) {
    bright = false;
    drawButton(L_THIRD, B_LIGHT, B_NORMAL);
    mainpurp = false;
  }

  if (AGCgainbut) {
    AGCgainbut = false;
    FreqDispl();
    drawButton(L_THIRD, B_ATT, B_NORMAL);
    mainpurp = false;
  }

  if (SETUPbut) mainpurp = false;

  if (MEMObut) {
    if (MEMOadd) {
      posMemoName++;
      if (posMemoName < 20) charMemoName = addMemoName[posMemoName];
    } else {
      if (presetBank) {
        int tmpBand = bandFreq(preset[currentMemo].presetIdx);
        if (tmpBand == 29 && (preset[currentMemo].presetIdx < 153 || preset[currentMemo].presetIdx > 30000)) {
          ErrorBeep();
        } else {
          Beep(1, 0);
          if (bandIdx != tmpBand || currentMode != bandMode[tmpBand]) {
            bandIdx = tmpBand;
            currentMode = bandMode[tmpBand];
            BandSet();
          }
          if (tmpBand) commSetFreq(preset[currentMemo].presetIdx); else commSetFreq(preset[currentMemo].presetIdx * 100);
          band[bandIdx].currentFreq = si4735.getFrequency();
          FreqDispl();
          displMEMO();
        }
      } else {
        if (MemoBank[currentMemo].freq < 153 || MemoBank[currentMemo].freq > 30000) {
          ErrorBeep();
        } else {
          if (bandIdx != (MemoBank[currentMemo].band & 0x1F) || currentMode != trunc(MemoBank[currentMemo].band / 32)) {
            bandIdx = MemoBank[currentMemo].band & 0x1F;
            currentMode = trunc(MemoBank[currentMemo].band / 32);
            BandSet();
          }
          commSetFreq(MemoBank[currentMemo].freq);
          band[bandIdx].currentFreq = MemoBank[currentMemo].freq;
          FreqDispl();
        }
      }
      checkAGC();
    }
    mainpurp = false;
  }

  #ifdef IhaveSI5351
  if (FirstLayer) {
    if ((bfoOn) && (calibratSI5351 == false)) {
      calibratSI5351 = true;
      calibratvalSI5351 = 3276800 - FreqSI5351;
      FreqDispl();
      mainpurp = false;
    }
  }
  #endif
  if (mainpurp) {
  // ================== Toggle between the volume control and the squelch control ========== LWH
  if (encoderBtnState == 0) {
    SQUELCHbut = false;
    squelchDecay = 0;
    if (VOLbut) VOLbut = false; else VOLbut = true;
  }
  if (encoderBtnState == 1) {
    VOLbut = false;
    if (SQUELCHbut) SQUELCHbut = false; else SQUELCHbut = true;
  }
  if (encoderBtnState == 2) {
    VOLbut = false;
    SQUELCHbut = false;
    squelchDecay = 0;
  }
  encoderBtnState = encoderBtnState + 1;
  if (encoderBtnState == 3) encoderBtnState = 0;
  // ==========================================================================
  if (FirstLayer) {
    if (VOLbut) drawButton(L_FIRST, B_VOL, B_SELECT); else drawButton(L_FIRST, B_VOL, B_NORMAL);
  }
  if (RETRObut) {
    if (VOLbut) drawButton(L_RETRO, 2, B_SELECT); else drawButton(L_RETRO, 2, B_NORMAL);
  }
  if (bfoOn) {
    bfoOn = false;
    if (FirstLayer) drawButton(L_FIRST, B_BFO, B_NORMAL);
    if (RETRObut) drawRETROscale();
  }
  if (RETRObut) drawRetroVol(); else FreqDispl();
  if (SCANbut) {
    if (VOLbut) {
      pauseScanIfNeeded();
    } else {
      DrawSCANind();
      DisplaySCANsignal();
    }
  }
  }
}

void processEncoderShortPress() {
  long encTime = millis() + 400;
  while (analogRead(ENCODER_SWITCH) > 500 && encTime > millis());
  if (analogRead(ENCODER_SWITCH) < 500) {
    while (analogRead(ENCODER_SWITCH) < 500);
    long encTime = millis() + 400;
    while (analogRead(ENCODER_SWITCH) > 500 && encTime > millis());
    if (analogRead(ENCODER_SWITCH) < 500) {
      if (RETRObut) scrollRetro = -1;
      if ((currentMode != LSB) && (currentMode != USB) && (currentMode != CW) && (ThirdLayer || FirstLayer))   {
        SEEK = true;
        SEEKdispl(1);
        if (ThirdLayer) drawButton(L_THIRD, B_SEEKDN, B_SELECT);
        if (currentMode != FM) {
          if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
            si4735.setSeekAmSpacing(1);
            si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
          }
          else {
            bandIdx = 29; // all sw
            si4735.setSeekAmSpacing(1);
            si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
          }
        }
        si4735.seekStationProgress(SeekFreq, checkStopSeeking,  SEEK_DOWN);
        delay(300);
        SEEK = false;
        if (ThirdLayer) drawButton(L_THIRD, B_SEEKDN, B_NORMAL);
        currentFrequency = si4735.getFrequency();
        band[bandIdx].currentFreq = currentFrequency ;
        previousFrequency = currentFrequency;
        FreqDraw(currentFrequency, 0);
        delay(300);
      }
    } else {
      if (RETRObut) scrollRetro = 1;
      if ((currentMode != LSB) && (currentMode != USB) && (currentMode != CW) && (ThirdLayer || FirstLayer))   {
        SEEK = true;
        SEEKdispl(0);
        if (ThirdLayer) drawButton(L_THIRD, B_SEEKUP, B_SELECT);
        if (currentMode != FM) {
          if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
            si4735.setSeekAmSpacing(1);
            si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
          } else {
            bandIdx = 29; // all sw
            si4735.setSeekAmSpacing(1);
            si4735.setSeekAmLimits(band[bandIdx].minimumFreq, band[bandIdx].maximumFreq);
          }
        }
        si4735.seekStationProgress(SeekFreq, checkStopSeeking,  SEEK_UP);
        delay(300);
        SEEK = false;
        if (ThirdLayer) drawButton(L_THIRD, B_SEEKUP, B_NORMAL);
        currentFrequency = si4735.getFrequency();
        band[bandIdx].currentFreq = currentFrequency ;
        previousFrequency = currentFrequency;
        FreqDraw(currentFrequency, 0);
        delay(300);
      }
    }
  } else {
   bool mainpurp = true;

   if (scrollRetro) {
      scrollRetro = 0;
      mainpurp = false;
   }

   if (cityRETRObut) {
      cityRETRObut = false;
      RETRObut = true;
      if (prevPresetId != PresetId) {
        prevPresetId = PresetId;
        currentPRES = 0;
        presetLoad();
        presetSort();
        presetSetPos();
      }
      drawRETRO();
      currentRetroFreq = 0;
      mainpurp = false;
   }

   if (PRESbut) { // FM preset selection
      PRESbut = false;
      drawButton(L_THIRD, B_FM, B_NORMAL);
      DrawDispl();
      mainpurp = false;
   }

   if (bright) {
      bright = false;
      drawButton(L_THIRD, B_LIGHT, B_NORMAL);
      mainpurp = false;
   }

   if (AGCgainbut) {
      AGCgainbut = false;
      FreqDispl();
      drawButton(L_THIRD, B_ATT, B_NORMAL);
      mainpurp = false;
   }

   if (SETUPbut) mainpurp = false;

   if (MEMObut) {
      if (MEMOadd) {
        posMemoName++;
        if (posMemoName < 20) charMemoName = addMemoName[posMemoName];
      } else {
        if (presetBank) {
          int tmpBand = bandFreq(preset[currentMemo].presetIdx);
          if (tmpBand == 29 && (preset[currentMemo].presetIdx < 153 || preset[currentMemo].presetIdx > 30000)) {
            ErrorBeep();
          } else {
            Beep(1, 0);
            if (bandIdx != tmpBand || currentMode != bandMode[tmpBand]) {
              bandIdx = tmpBand;
              currentMode = bandMode[tmpBand];
              BandSet();
            }
            if (tmpBand) commSetFreq(preset[currentMemo].presetIdx); else commSetFreq(preset[currentMemo].presetIdx * 100);
            band[bandIdx].currentFreq = si4735.getFrequency();
            FreqDispl();
            displMEMO();
          }
        } else {
          if (MemoBank[currentMemo].freq < 153 || MemoBank[currentMemo].freq > 30000) {
            ErrorBeep();
          } else {
            if (bandIdx != (MemoBank[currentMemo].band & 0x1F) || currentMode != trunc(MemoBank[currentMemo].band / 32)) {
              bandIdx = MemoBank[currentMemo].band & 0x1F;
              currentMode = trunc(MemoBank[currentMemo].band / 32);
              BandSet();
            }
            commSetFreq(MemoBank[currentMemo].freq);
            band[bandIdx].currentFreq = MemoBank[currentMemo].freq;
            FreqDispl();
          }
        }
        checkAGC();
      }
      mainpurp = false;
   }

  #ifdef IhaveSI5351
   if (FirstLayer) {
      if ((bfoOn) && (calibratSI5351 == false)) {
        calibratSI5351 = true;
        calibratvalSI5351 = 3276800 - FreqSI5351;
        FreqDispl();
        mainpurp = false;
      }
   }
  #endif
   if (mainpurp) {
    // ================== Toggle between the volume control and the squelch control ========== LWH
    if (encoderBtnState == 0) {
      SQUELCHbut = false;
      squelchDecay = 0;
      if (VOLbut) VOLbut = false; else VOLbut = true;
    }
    if (encoderBtnState == 1) {
      VOLbut = false;
      if (SQUELCHbut) SQUELCHbut = false; else SQUELCHbut = true;
    }
    if (encoderBtnState == 2) {
      VOLbut = false;
      SQUELCHbut = false;
      squelchDecay = 0;
    }
    encoderBtnState = encoderBtnState + 1;
    if (encoderBtnState == 3) encoderBtnState = 0;
    // ==========================================================================
    if (FirstLayer) {
      if (VOLbut) drawButton(L_FIRST, B_VOL, B_SELECT); else drawButton(L_FIRST, B_VOL, B_NORMAL);
    }
    if (RETRObut) {
      if (VOLbut) drawButton(L_RETRO, 2, B_SELECT); else drawButton(L_RETRO, 2, B_NORMAL);
    }
    if (bfoOn) {
      bfoOn = false;
      if (FirstLayer) drawButton(L_FIRST, B_BFO, B_NORMAL);
      if (RETRObut) drawRETROscale();
    }
    if (RETRObut) drawRetroVol(); else FreqDispl();
    if (SCANbut) {
      if (VOLbut) {
        pauseScanIfNeeded();
      } else {
        DrawSCANind();
        DisplaySCANsignal();
      }
    }
   }
  }
}

void encoderButtonCheck() {
  if (analogRead(ENCODER_SWITCH) < 500 &&
      (ThirdLayer || FirstLayer || SecondLayer)) {
      Beep(1, 0);
      long encTime = millis() + 1000;
      while (analogRead(ENCODER_SWITCH) < 500 && encTime > millis());
    if (analogRead(ENCODER_SWITCH) < 500)
      processEncoderLongPress();
    else
      processEncoderShortPress();

    while (analogRead(ENCODER_SWITCH) < 500);
  }
}
// ============================================================================

// ============================================================================
// AFFICHAGE / COMMANDES ECRAN
// ============================================================================
void setStep()  {
// ============================================================================
  if (bfoOn && isSSBLikeMode())
  {
    if (currentBFOStep == 1) currentBFOStep = 10;
    else if (currentBFOStep == 10) currentBFOStep = 25;
    else currentBFOStep = 1;
  }
  if (SCANbut == false) {
    useBand();
    checkAGC();
    DrawDispl();
  }
}

// ============================================================================
void Beep(int cnt, int tlb) {
// ============================================================================
  if (beeperOn) {
    int tla = 100;
    for (int i = 0; i < cnt; i++) {
      digitalWrite(BEEPER, 1);
      delay(tla);
      digitalWrite(BEEPER, 0);
      delay(tlb);
    }
  }
}

// ============================================================================
void drawMainOCButton() {
  const int16_t bx = 160;
  const int16_t by = 130;
  const int16_t bh = 30;

  spr.createSprite(But_Width, bh);
  spr.fillScreen(COLOR_BACKGROUND);

  spr.pushImage(0, -5, But_Width, But_Height, (uint16_t *)But_);
  spr.setTextSize(2);
  spr.setTextDatum(BC_DATUM);
  spr.setTextPadding(0);

  spr.setTextColor(TFT_CYAN);
  spr.drawString("OC", (But_Width / 2) + 3, (bh / 2) + 10);
  spr.setTextColor(TFT_BLACK);
  spr.drawString("OC", (But_Width / 2) + 2, (bh / 2) + 9);

  spr.pushSprite(bx, by);
  spr.deleteSprite();
}

// ============================================================================
bool jamMainOCButton() {
  const int16_t bx = 160;
  const int16_t by = 130;
  const int16_t bh = 30;
  if (x >= bx && x < bx + But_Width && y >= by && y < by + bh) {
    Beep(1, 0);
    x = y = 0;
    delay(200);
    return true;
  }
  return false;
}

// ============================================================================
void DrawFila()   { // Draw of first layer
// ============================================================================
  FirstLayer = true;
  SecondLayer = false;
  tft.fillScreen(TFT_BLACK);
  DrawButFila();
  DrawDispl();
  DrawSmeter();
  DrawVolumeIndicator();
  DrawBatteryIndicator();
  drawMainOCButton();
  elapsedBat = 0;
}

// ============================================================================
void DrawThla()  { // Draw of Third layer
// ============================================================================
  ThirdLayer = true;
  ForthLayer = false;
  tft.fillScreen(TFT_BLACK);
  DrawButThla();
  DrawDispl();
  DrawSmeter();
  DrawVolumeIndicator();
  DrawBatteryIndicator();
  elapsedBat = 0;
}

// ============================================================================
void DrawButFila() { // Buttons First layer
// ============================================================================
  drawButtons(L_FIRST);
  if (currentMode != LSB && currentMode != USB && currentMode != CW) drawButton(L_FIRST, B_BFO, B_BLOCK);
  if (bfoOn) drawButton(L_FIRST, B_BFO, B_SELECT);
  si4735.getAutomaticGainControl();
  if (si4735.isAgcEnabled()) drawButton(L_FIRST, B_AGC, B_JAM);
  if (Mutestat) drawButton(L_FIRST, B_MUTE, B_JAM);
  if (currentMode == FM) drawButton(L_FIRST, B_MODE, B_BLOCK);
  if (isSSBLikeMode()) drawButton(L_FIRST, B_STEP, B_BLOCK);
}

// ============================================================================
void DrawButThla() { // Buttons Third layer
// ============================================================================
  drawButtons(L_THIRD);
  if (isSSBLikeMode()) {
    drawButton(L_THIRD, B_SEEKUP, B_BLOCK);
    drawButton(L_THIRD, B_SEEKDN, B_BLOCK);
  }
  if (RDS) drawButton(L_THIRD, B_RDS, B_JAM);
  if (displayPower) drawButton(L_THIRD, B_LIGHT, B_BLOCK);
}

// ============================================================================
void DrawVolumeIndicator()  {
// ============================================================================
  tft.setTextSize(1);
  tft.fillRect(XVolInd + 2, YVolInd - 1, 157, 28, TFT_GREY);
  tft.setTextColor(TFT_WHITE, TFT_GREY);
  tft.setCursor(XVolInd +  11, YVolInd + 7);
  tft.print("0%");
  tft.setCursor(XVolInd + 126, YVolInd + 7);
  tft.print("100%");
}

// ============================================================================
void DrawBatteryIndicator()  {
// ============================================================================
  tft.fillRect(XVolInd + 161, YVolInd - 1, 77, 28, TFT_GREY);
  if (batShow) tft.fillRect(XVolInd + 176, YVolInd + 5, 46, 16, TFT_NAVY);
}

// ============================================================================
void DrawSmeter()  {
// ============================================================================
  String IStr;
  tft.setTextSize(1);
  tft.fillRect(Xsmtr + 2, Ysmtr + 6, 236, 46, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  for (int i = 0; i < 10; i++) {
    tft.fillRect(Xsmtr + 15 + (i * 12), Ysmtr + 24, 2, 8, TFT_WHITE);
    IStr = String(i);
    tft.setCursor((Xsmtr + 14 + (i * 12)), Ysmtr + 13);
    tft.print(i);
  }
  for (int i = 1; i < 7; i++) {
    tft.fillRect((Xsmtr + 123 + (i * 16)), Ysmtr + 24, 3, 8, TFT_RED);
    IStr = String(i * 10);
    tft.setCursor((Xsmtr + 117 + (i * 16)), Ysmtr + 13);
    if ((i == 2) || (i == 4) || (i == 6))  {
      tft.print("+");
      tft.print(i * 10);
    }
  }
  tft.fillRect(Xsmtr + 15, Ysmtr + 32 , 112, 3, TFT_WHITE);
  tft.fillRect(Xsmtr + 127, Ysmtr + 32 , 100, 3, TFT_RED);
}

// ============================================================================

// ============================================================================
// SEEK / LISTES / SOUS-MENUS
// ============================================================================
void SEEKdispl (int dir)  {
// ============================================================================
  tft.setTextSize(2);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextDatum(BL_DATUM);
  tft.drawString("SEEK", XFreqDispl, YFreqDispl + 40);
  if (dir) tft.drawString("<<<<", XFreqDispl, YFreqDispl + 60); else tft.drawString(">>>>", XFreqDispl, YFreqDispl + 60);
}

// ============================================================================
void drawList(uint8_t lay, String text) {
// ============================================================================
  tft.fillScreen(COLOR_BACKGROUND);
  FreqDispl();
  int d = 0;
  if (!screenV && !BroadBand && !FREQbut){
    d = 40;
    for (int n = 1; n <= 20; n++) {
      tft.fillRect(40 - (n * 2), 0, 2, 240, ((int(n / 2) * 4096) + (n * 32)));
      tft.fillRect((n * 2) + 278, 0, 2, 240, ((int(n / 2) * 4096) + (n * 32)));
    }
  }
  tft.fillRect(d, 0, 240, 20, TFT_GREY);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_GREY);
  tft.setTextDatum(BC_DATUM);
  tft.drawString(text, 120 + d, 20);
  if (lay == L_SETUP) {
    spr.createSprite(265, 120);
    spr.fillScreen(COLOR_BACKGROUND);
    spr.pushImage(0, 0, LOGO_WIDTH, LOGO_HEIGHT, (uint16_t *)logo);
    if (screenV) spr.pushSprite(-25, 80); else spr.pushSprite(27, 80);
    spr.deleteSprite();
  }
  drawListBut(lay);
}

// ============================================================================
void drawListBut(uint8_t lay) {
// ============================================================================
  drawButtons(lay);
  if (BroadBand || HamBand) {
    drawButton(lay, bandIdx, B_JAM);
  } else if (BandWidth) {
    if (currentMode == AM) drawButton(lay, bwIdxAM, B_JAM);
    else if (currentMode == FM) drawButton(lay, bwIdxFM, B_JAM);
    else drawButton(lay, bwIdxSSB, B_JAM);
  } else if (STEPbut) {
    if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) drawButton(lay, ssIdxMW, B_JAM);
    else if (currentMode == FM) drawButton(lay, ssIdxFM, B_JAM);
    else drawButton(lay, ssIdxAM, B_JAM);
  } else if (Modebut) drawButton(lay, currentMode, B_JAM);
}

// ============================================================================
void subrstatus() {
// ============================================================================
  tft.fillScreen(TFT_BLACK);

  spr.fillScreen(COLOR_BACKGROUND);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Software version V5.3b  17-09-2022 IU4ALH Mod", 5, 0);
  tft.drawString("Mode     : " + String(bandModeDesc[band[bandIdx].prefmod]), 5, 10);
  if ( currentMode != FM)  tft.drawString("Freq.    : " + String(currentFrequency, 0) + " KHz", 5, 20);
  else tft.drawString("Freq.    : " + String(currentFrequency / 100, 1) + " MHz", 5, 20);
  si4735.getCurrentReceivedSignalQuality();
  tft.drawString("RSSI     : " + String(si4735.getCurrentRSSI()) + "dBuV", 5, 30);
  tft.drawString("SNR      : " + String(si4735.getCurrentSNR()) + "uV", 5, 40);
  if (  currentMode == FM ) {
    sprintf(buffer, "%s", (si4735.getCurrentPilot()) ? "STEREO" : "MONO");
    tft.drawString("         : " + String(buffer), 5, 50);
  }
  si4735.getAutomaticGainControl();
  si4735.getCurrentReceivedSignalQuality();
  tft.drawString("LNA GAIN index: " + String(si4735.getAgcGainIndex()) + "/" + String(currentAGCgain), 5, 60);
  tft.drawString("Volume   : )" + String(si4735.getVolume()), 5, 70);
  sprintf(buffer, "%s", (si4735.isAgcEnabled()) ? "AGC ON " : "AGC OFF");
  tft.drawString(buffer, 5, 80);
  if (bfoOn) tft.drawString("BFO ON  ", 5, 90);
  else tft.drawString("BFO OFF ", 5, 90);
  tft.drawString("AVC max GAIN  : " + String(si4735.getCurrentAvcAmMaxGain()), 5, 100);
  tft.drawString("Ant. Cap = " + String(si4735.getAntennaTuningCapacitor()) , 5, 110);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Band          :  " + String(bandIdx) + "  " + String(band[bandIdx].bandName) , 5, 120);
  tft.drawString("Bandwidth SSB : " + String(bandwidthSSB[bwIdxSSB]) + " KHz", 5, 130);
  tft.drawString("Bandwidth AM  : " + String(bandwidthAM[bwIdxAM]) + " KHz", 5, 140);
  tft.drawString("Bandwidth FM  : " + String(bandwidthFM[bwIdxFM]) + " KHz", 5, 150);
  tft.drawString("Stepsize  MW  :   " + String(ssIdxMW) + " KHz", 5, 160);
  tft.drawString("Stepsize  AM  :   " + String(ssIdxAM) + " KHz", 5, 170);
  tft.drawString("Stepsize SSB  :   " "1 KHz fixed", 5, 180);
  tft.drawString("Stepsize  FM  : " + String(ssIdxFM * 10) + " KHz", 5, 190);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  float vsupply;
  if (batShow) vsupply = 3.724 * analogRead(BAT_INFO) / 2047; else vsupply = ((1.66 / 1850) * analogRead(ENCODER_SWITCH)) * 2;
  tft.drawString("Power Supply   : " + String(vsupply, 2) + " V", 5, 200);
  tft.drawString("EEPROM SIZE    : " + String(EEPROM_SIZE) + " byte | FREE: " + String(EEPROM_SIZE - offsetEEPROM - sizeof(MemoBank) - sizeof(storage)) + " byte", 5, 210);
  tft.drawString("EEPROM storage : " + String(sizeof(storage)) + " byte. Offset: " + String(offsetEEPROM), 5, 220);
  tft.drawString("EEPROM MemoBank: " + String(sizeof(MemoBank)) + " byte. Offset: " + String(offsetMemoEEPROM), 5, 230);

  while (x == 0 && (elapsedSaver + 120000) > millis()) {
    presStat = tft.getTouch(&x, &y);
  }
  x = y = 0;
  Beep(1, 0);
  delay(400);
  elapsedSaver = millis();
}

// ============================================================================
void showRDSStation() {
// ============================================================================
  if ((FirstLayer) || (ThirdLayer)) {
    tft.setCursor(XFreqDispl + 80, YFreqDispl + 58);
    tft.print(stationName);
  }
  delay(150);
}

// ============================================================================
void showrdsMsg() {
// ============================================================================
  if ((FirstLayer) || (ThirdLayer)) {
    tft.setCursor(0, YFreqDispl + 75); //  sulle coord di DSP radio...
    tft.print(rdsMsg);
  }
  delay(100);
}

// =====================================

// =====================================

// ============================================================================

// ============================================================================
// DECODAGE RDS
// ============================================================================
void showRDSTime() {
  if (!((FirstLayer) || (ThirdLayer)))
    return;

  if (rdsTime == NULL || strlen(rdsTime) < 5)
    return;

  calcRDSTime();

  // Visible colors: the old firmware used BLACK on BLACK.
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextPadding(0);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.fillRect(0, YFreqDispl + 57, 68, 14, TFT_BLACK);
  tft.drawString(String(rdsTime), 2, YFreqDispl + 58);
}

void calcRDSTime() {
  // =======================================
  if (strlen(rdsTime) > 10) {
    int gmtHour = 0;
    int gmtMinute = 0;
    int gmtHourOffset = 0;
    int gmtMinuteOffset = 0;
    gmtHour = gmtHour + ((rdsTime[0] - 48) * 10);
    gmtHour = gmtHour + (rdsTime[1] - 48);
    gmtMinute = gmtMinute + ((rdsTime[3] - 48) * 10);
    gmtMinute = gmtMinute + (rdsTime[4] - 48);
    gmtHourOffset = gmtHourOffset + ((rdsTime[7] - 48) * 10);
    gmtHourOffset = gmtHourOffset + (rdsTime[8] - 48);
    gmtMinuteOffset = gmtMinuteOffset + ((rdsTime[10] - 48) * 10);
    gmtMinuteOffset = gmtMinuteOffset + (rdsTime[11] - 48);
    if (rdsTime[6] == '-') {
      gmtHour = gmtHour + gmtHourOffset;
      if (gmtHour > 23) gmtHour = gmtHour - 24;
      gmtMinute = gmtMinute + gmtMinuteOffset;
    } else {
      gmtHour = gmtHour - gmtHourOffset;
      if (gmtHour < 0) gmtHour = gmtHour + 24;
      gmtMinute = gmtMinute - gmtMinuteOffset;
    }
    rdsTime[0] = (trunc(gmtHour / 10)) + 48;
    rdsTime[1] = gmtHour - (trunc(gmtHour / 10) * 10) + 48;
    rdsTime[3] = (trunc(gmtMinute / 10)) + 48;
    rdsTime[4] = gmtMinute - (trunc(gmtMinute / 10) * 10) + 48;
    rdsTime[5] = '\0';
  }
}

void checkRDS() {
  si4735.getRdsStatus();

  if (!si4735.getRdsReceived())
    return;

  if (!(si4735.getRdsSync() && si4735.getRdsSyncFound()))
    return;

  stationName = si4735.getRdsText0A();
  rdsMsg = si4735.getRdsText2A();
  rdsTime = si4735.getRdsTime();

  if (!((FirstLayer) || (ThirdLayer)))
    return;

  if (stationName != NULL) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(BC_DATUM);
    showRDSStation();
  }

  if (rdsMsg != NULL) {
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(BC_DATUM);
    showrdsMsg();
  }

  if (rdsTime != NULL)
    showRDSTime();
}

bool checkStopSeeking() {
  return (bool) encoderCount || tft.getTouch(&x, &y) || analogRead(ENCODER_SWITCH) < 500; // returns true if the user rotates the encoder or touches on screen
}

void Segment(String freq, String mask, int d) {
// ============================================================================
 if (Saver) {
  spr.createSprite(140, 38);
  spr.fillScreen(COLOR_BACKGROUND);
  spr.setTextSize(1);
  spr.setTextPadding(0);
  spr.setFreeFont(&DSEG7_Classic_Mini_Regular_34);
  spr.setTextDatum(BR_DATUM);
  spr.setTextColor(TFT_DARKCYAN);
  spr.drawString(freq, 140, 38);
  spr.pushSprite(saverX, saverY);
 } else {
  if (!digitLigth) mask = "";
  if (SEEK) {
    spr.createSprite(194, 38);
    d = 46;
  } else {
    if (RETRObut) {
      if (RETROband) spr.createSprite(150, 38); else spr.createSprite(110, 38);
    } else {
#ifdef IhaveSI5351
      spr.createSprite(240, 38);
#endif
#ifdef IhaveCrystal
      if (bfoOn) spr.createSprite(110, 38); else spr.createSprite(240, 38);
#endif
    }
  }
  spr.fillScreen(COLOR_BACKGROUND);
  spr.setTextSize(1);
  spr.setTextPadding(0);
  spr.setFreeFont(&DSEG7_Classic_Mini_Regular_34);
  spr.setTextDatum(BR_DATUM);
  int x = 222;
  if (bfoOn && !RETRObut) {
#ifdef IhaveCrystal
    x = 110;
#endif
    spr.setTextColor(TFT_BROWN);
    spr.drawString(mask, x, 38);
    spr.setTextColor(TFT_ORANGE);
    spr.drawString(freq, x, 38);
  } else {
    if ((currentMode == AM || currentMode == FM) && !RETRObut) x = 190;
    if (SEEK) x = 144;
    if (RETRObut) {
      if (RETROband) x = 150; else x = 110;
    }
    if (bfoOn) spr.setTextColor(TFT_BROWN); else spr.setTextColor(TFT_BLACK); // mjs - WAS: else spr.setTextColor(TFT_DARKCYAN);
    spr.drawString(mask, x, 38);
    if (PREtap) {
      spr.setTextColor(TFT_LIGTHYELLOW);
    } else {
      if (bfoOn) spr.setTextColor(TFT_ORANGE); else spr.setTextColor(COLOR_INDICATOR_FREQ);
    }
    spr.drawString(freq, x, 38);
  }
  if (RETRObut) {
    if (screenV) {
      if (RETROband) spr.pushSprite(15, 200); else spr.pushSprite(50, 200);
    } else spr.pushSprite(125, -3);
  } else spr.pushSprite(XFreqDispl + d, YFreqDispl + 20);
 }
 spr.setFreeFont(NULL);
 spr.deleteSprite();
}

// ============================================================================
void FreqDispl() {
// ============================================================================
  if (FirstLayer || ThirdLayer || SecondLayer) {
    int d = 0;
    if ((SCANbut || HamBand || Modebut || STEPbut || BandWidth || MEMObut) && !screenV) d = 40;
    currentFrequency = si4735.getFrequency();
    if (!FREQbut && !HamBand && !Modebut && !BandWidth && !BroadBand && !SCANbut && !MEMObut && !STEPbut) {
      AGCfreqdisp();
      BFOStepdisp();
    }
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextSize(4);
    tft.setTextDatum(BC_DATUM);
    if ((VOLbut) || (AGCgainbut) || (SQUELCHbut)) { // LWH - Added SQUELCHbut
      int y = 40;
      if ((currentMode == LSB || currentMode == USB  || currentMode == CW ) && (FirstLayer || ThirdLayer)) y = 48;
      if (volDisp) tft.fillRect( XFreqDispl + d + 40, YFreqDispl + 20 , 55, y, TFT_BLACK); else tft.fillRect( XFreqDispl + d, YFreqDispl + 20 , 240, y, TFT_BLACK);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextSize(3);
   //============ Squelch Setting ============ LWH
      if (SQUELCHbut) {
        tft.drawString(String(currentSquelch), XFreqDispl + 80 + d, YFreqDispl + 53); //60
        tft.setTextSize(2);
        tft.drawString(" SQUELCH dB", XFreqDispl + 160 + d, YFreqDispl + 53);
      }
    // ========================================
      if (VOLbut) {
        tft.drawString(String(currentVOL), XFreqDispl + 84 + d, YFreqDispl + 53);
        tft.setTextSize(2);
        tft.drawString("VOLUME", XFreqDispl + 160 + d, YFreqDispl + 53);
      }
      if (AGCgainbut) {
        tft.drawString(String(currentAGCgain), XFreqDispl + 60, YFreqDispl + 53);
        tft.setTextSize(2);
        tft.drawString("RF ATT", XFreqDispl + 160, YFreqDispl + 53);
      }
      volDisp = true;
    } else {
      volDisp = false;
      if (currentMode == LSB || currentMode == USB  || currentMode == CW ) {

       Displayfreq = (currentFrequency * 1000) - (band[bandIdx].lastBFO);
       if (CWShift) Displayfreq = Displayfreq + 700;
       int mhz = trunc(Displayfreq / 1000000);
       int khz = Displayfreq - (mhz * 1000000);
       khz = trunc(khz / 1000);
       int hz = Displayfreq - (mhz * 1000000) - (khz * 1000);
       char s[12] = {'\0'};
       if (mhz > 0) sprintf(s, "%i %03i.%02i", mhz, khz, hz / 10); else sprintf(s, "%i.%02i", khz, hz / 10);

       if (!bfoOn || bfoTr) {
        tft.setTextDatum(BR_DATUM);
        tft.setTextColor(COLOR_INDICATOR_FREQ, COLOR_BACKGROUND);
        if (bfoTr) {
          bfoTr = false;
          for (int i = 4; i > 1; i--) {
            if (bfoOn) tft.setTextSize(i); else tft.setTextSize(6 - i);
            tft.fillRect( XFreqDispl + d, YFreqDispl + 20 , 240, 48, TFT_BLACK);
            tft.drawString(String(s), XFreqDispl + 230 + d, YFreqDispl + 62);
            delay(100);
          }
        }
        if (!bfoOn) Segment(String(s), "88 888.88", d);
        tft.setTextSize(2);
        if (FREQbut || HamBand || Modebut || BandWidth || BroadBand || SCANbut || MEMObut || STEPbut) {
          tft.fillRect(XFreqDispl + d, YFreqDispl + 60, 240, 20, TFT_GREY);
          tft.setTextColor(TFT_YELLOW, TFT_GREY);
          tft.drawString("KHz", XFreqDispl + 234 + d, YFreqDispl + 78);
        } else {
          tft.setTextColor(TFT_YELLOW, TFT_BLACK);
          tft.drawString("KHz", XFreqDispl + 229 + d, YFreqDispl + 84);
        }
        if (bfoOn == false && (FREQbut || HamBand || Modebut || BandWidth || BroadBand || SCANbut || MEMObut) == false) {
          tft.fillRect(XFreqDispl + 141 + d, YFreqDispl + 60, 81, 5, TFT_BLACK);
          if (freqstepnr == 0)  tft.fillRect(XFreqDispl + 141 + d, YFreqDispl + 60, 21, 5, TFT_ORANGE);
          if (freqstepnr == 1)  tft.fillRect(XFreqDispl + 171 + d, YFreqDispl + 60, 21, 5, TFT_ORANGE);
          if (freqstepnr == 2)  tft.fillRect(XFreqDispl + 200 + d, YFreqDispl + 60, 21, 5, TFT_ORANGE);
        }
       }
        if (bfoOn) {
#ifdef IhaveCrystal
          Segment(String(currentBFOmanu), "-888", d);
          tft.setTextSize(2);
          tft.setTextDatum(BL_DATUM);
          tft.setTextColor(TFT_ORANGE, TFT_BLACK);
          tft.drawString("Hz", XFreqDispl + 120 + d, YFreqDispl + 40);
          tft.setTextColor(TFT_BLACK, TFT_ORANGE);
          tft.fillRect(XFreqDispl + 156 + d, YFreqDispl + 21, 42, 20, TFT_ORANGE);
          tft.drawString("BFO", XFreqDispl + 160 + d, YFreqDispl + 40);
          tft.setTextDatum(BR_DATUM);
          if (PREtap) tft.setTextColor(TFT_LIGTHYELLOW, COLOR_BACKGROUND); else tft.setTextColor(COLOR_INDICATOR_FREQ, COLOR_BACKGROUND);
          tft.drawString(String(s), XFreqDispl + 230 + d, YFreqDispl + 62);
#endif
#ifdef IhaveSI5351
          float temp = FreqSI5351 + calibratvalSI5351;
          Segment(String((temp / 100), 2), "88 888.88", d);
          if (stepsizesynth == 10)  tft.fillRect(XFreqDispl + 171 + d, YFreqDispl + 60, 21, 5, TFT_ORANGE);
          if (stepsizesynth ==  1)  tft.fillRect(XFreqDispl + 200 + d, YFreqDispl + 60, 21, 5, TFT_ORANGE);
#endif
        }
        tft.setTextDatum(BC_DATUM);
      } else {
        FreqDraw(currentFrequency, d);
        if (FREQbut || HamBand || Modebut || BandWidth || BroadBand || SCANbut || MEMObut || STEPbut) tft.fillRect(XFreqDispl + d, YFreqDispl + 60, 240, 20, TFT_GREY);
      }
    }
  }
}

// ============================================================================
void FreqDraw (float freq, int d)  {
// ============================================================================
  tft.fillRect( XFreqDispl + 46 + d, YFreqDispl + 20 , 194, 48, TFT_BLACK);
  if (currentMode == FM) {
    Displayfreq =  freq / 100;
    Segment(String(Displayfreq, 2), "188.88", d);
    tft.setTextDatum(BC_DATUM);
    tft.setTextSize(2);
    if (Saver) {
      tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
      tft.drawString("MHz", saverX + 165, saverY + 38);
    } else {
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString("MHz", XFreqDispl + 215 + d, YFreqDispl + 60); // 60 posizione scritta Mhz in FM
    }
  } else {
    if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
      Displayfreq =  freq;
      Segment(String(Displayfreq, 0), "1888", d);
      tft.setTextDatum(BC_DATUM);
      tft.setTextSize(2);
      if (Saver) {
        tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
        tft.drawString("MHz", saverX + 165, saverY + 38);
      } else {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("KHz", XFreqDispl + 215 + d, YFreqDispl + 60);
      }
    } else {
      Displayfreq =  freq / 1000;
      Segment(String(Displayfreq, 3), "88.888", d);
      tft.setTextDatum(BC_DATUM);
      tft.setTextSize(2);
      if (Saver) {
        tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
      tft.drawString("MHz", saverX + 165, saverY + 38);
      } else {
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("MHz", XFreqDispl + 215 + d, YFreqDispl + 60);
      }
    }
  }
}

// ============================================================================
// ============================================================================
void SeekFreq (uint16_t freq)  {
// ============================================================================
  FreqDraw(float(freq), 0);
}

// ============================================================================
void DrawDispl() {
// ============================================================================
  tft.fillRect(XFreqDispl, YFreqDispl, 240, 86, TFT_BLACK);

  tft.setTextSize(1);
  tft.setTextDatum(BC_DATUM);

  tft.setTextColor(2031, TFT_BLACK);
  tft.drawString(band[bandIdx].bandName, XFreqDispl + 180, YFreqDispl + 15);
  tft.drawRect(XFreqDispl + 160, YFreqDispl + 2, 39, 16, 2031);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  Modtext = bandModeDesc[currentMode];
  if ((Modtext == "USB") && (CWShift == true)) Modtext = "CW";
  tft.drawString(Modtext, XFreqDispl + 95, YFreqDispl + 15);
  tft.drawRect(XFreqDispl + 80, YFreqDispl + 2, 29, 16, TFT_YELLOW);

  if (currentMode == AM) BWtext = bandwidthAM[bwIdxAM];
  if (isSSBLikeMode()) BWtext = bandwidthSSB[bwIdxSSB];
  if (currentMode == FM) BWtext = bandwidthFM[bwIdxFM];
  tft.setTextColor(64799, TFT_BLACK);
  if (BWtext == "AUTO") {
    tft.drawString("F AUTO", XFreqDispl + 135, YFreqDispl + 15);
  } else tft.drawString("F" + BWtext + "KHz", XFreqDispl + 135, YFreqDispl + 15);
  tft.drawRect(XFreqDispl + 110, YFreqDispl + 2, 49, 16, 64799);

  tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
  if (currentMode == FM) {
    tft.drawString(String((band[bandIdx].currentStep) * 10) + "KHz", XFreqDispl + 220, YFreqDispl + 15);
  } else  tft.drawString(String(band[bandIdx].currentStep) + "KHz", XFreqDispl + 220, YFreqDispl + 15);
  tft.drawRect(XFreqDispl + 200, YFreqDispl + 2, 39, 16, TFT_SKYBLUE);

  FreqDispl();
}

// ============================================================================
void AGCfreqdisp() {
// ============================================================================
  uint16_t col = TFT_SILVER;
  if (AGCgain) col = 64528;
  tft.setTextSize(1);
  tft.setTextColor(col, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  if (AGCgain > 1) {
    if (currentAGCgain > 9) tft.drawString("ATT" + String(currentAGCgain), XFreqDispl + 60, YFreqDispl + 15); else tft.drawString("ATT " + String(currentAGCgain), XFreqDispl + 60, YFreqDispl + 15);
  } else tft.drawString(" AGC ", XFreqDispl + 60, YFreqDispl + 15);
  tft.drawRect(XFreqDispl + 40, YFreqDispl + 2, 39, 16, col);
}

// ============================================================================
void BFOStepdisp() {
// ============================================================================
  uint16_t col = TFT_SILVER;
  if (isSSBLikeMode() && currentBFOmanu) col = TFT_ORANGE;
  tft.setTextSize(1);
  tft.setTextColor(col, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  if (bfoOn) {
#ifdef IhaveCrystal
    tft.drawString(String(currentBFOStep) + " Hz", XFreqDispl + 20, YFreqDispl + 15);
#endif
  } else {
    tft.drawString(" BFO ", XFreqDispl + 20, YFreqDispl + 15);
  }
  tft.drawRect(XFreqDispl, YFreqDispl + 2, 39, 16, col);
}

// ============================================================================
void ErrorBeep()  {
// ============================================================================
  Beep(2, 100);
}

// ============================================================================
void MuteAudOn() {
// ============================================================================
  si4735.setHardwareAudioMute(1);
  AudioMut = true;
  elapsedAudMut = millis();
}

// ============================================================================
void MuteAud() {
// ============================================================================
  if (((millis() - elapsedAudMut) > MIN_ELAPSED_AudMut_TIME ) && (AudioMut = true))
  {
    AudioMut = false;
    si4735.setHardwareAudioMute(0);
  }
}

// ============================================================================

// ============================================================================
// INFORMATIONS FIRMWARE / MUTE
// ============================================================================

// ============================================================================

// ============================================================================
// SCAN NATIF ATS
// ============================================================================
void pauseSCAN() {
// ============================================================================
  int d = screenV * 40;
  if (SCANpause) {
    si4735.setAudioMute(audioMuteOff);
    if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) si4735.setFrequencyStep(ssIdxMW);
    else if (currentMode == FM) si4735.setFrequencyStep(ssIdxFM);
    else si4735.setFrequencyStep(ssIdxAM);
    setFreq(currentScanFreq + int((currentScanLine - 159 + d + deltaScanLine) * SCANstep));
    AGCgain = ScanAGC;
    checkAGC();
    drawButton(L_SCAN, 1, B_JAM);
  } else {
    si4735.setAudioMute(audioMuteOn);
    si4735.setFrequencyStep(1);
    posScanFreq = currentScanFreq + int((deltaScanLine - 159 + d + posScan) * SCANstep);
    setFreq(posScanFreq);
    if (isSSBLikeMode()) band[bandIdx].lastBFO = currentBFO = 0;
    AGCgain = 0;
    checkAGC();
    drawButton(L_SCAN, 1, B_NORMAL);
  }
}

// ============================================================================
void DisplaySCANsignal() {
// ============================================================================
  int d = screenV * 40;
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_GREY);
  tft.setTextDatum(BL_DATUM);
  if (SCANpause) tft.drawString("RSSI " + String(si4735.getCurrentRSSI()) + " dBuV ", 130 - d, 70); else tft.drawString("RSSI " + String((198 - ScanValueRSSI[int(currentScanLine)]) / signalScale, 0) + " dBuV ", 130 - d, 70);
  tft.setTextColor(TFT_ORANGE, TFT_GREY);
  if (SCANpause) tft.drawString(" SNR " + String(si4735.getCurrentSNR()) + " dB ", 130 - d, 80); else tft.drawString(" SNR " + String(ScanValueSNR[int(currentScanLine)]) + " dB ", 130 - d, 80);
}

// ============================================================================
void drawSCAN() {
// ============================================================================
  setFreq(currentScanFreq + int((currentScanLine - 159 + (screenV * 40) + deltaScanLine) * SCANstep));
  FreqDispl();
  drawList(L_SCAN, String(band[bandIdx].bandName) + " BAND SCANNER");
  if (isSSBLikeMode() && bfoOn == false) drawButton(L_SCAN, 2, B_BLOCK);
  DrawSCANind();
}

// ============================================================================
void drawSCANgraf(bool erase) {
// ============================================================================
  int d = screenV * 80;
  if (erase) {
    ScanEmpty = true;
    posScanFreq = currentScanFreq + int((deltaScanLine - 159 + d) * SCANstep);
    clearScanMarks();
  }
  ScanBeginBand = -1;
  ScanEndBand = 320 - d;
  for (int n = 0; n < (320 - d); n++) {
    if (erase) {
      ScanValueRSSI[n] = 198 + (d / 2);
      ScanValueSNR[n] = 0;
    }
    ScanScaleLine[n] = 0;
    drawSCANline(n);
  }
}

// ============================================================================
void drawSCANline(int n) {
// ============================================================================
  int d = screenV * 40;

  int frq = currentScanFreq + ((n - 159 + d + deltaScanLine) * SCANstep);
  bool tmpLine = false;

  if (n == int(currentScanLine)) {
    tft.drawLine(int(currentScanLine), 81, int(currentScanLine), 198 + d, TFT_RED);
    DisplaySCANsignal();
  } else {
    int16_t colf = TFT_NAVY;
    int16_t colb = TFT_BLACK;

    if (!ScanScaleLine[n]) {
      if (SCANstep > 4) {
        if ((frq - (int(frq / 1000) * 1000)) < SCANstep) ScanScaleLine[n] = 2;
      } else {
        if ((frq - (int(frq / 100) * 100)) < SCANstep) {
          if (!prevScaleLine) ScanScaleLine[n] = 2;
          if (SCANstep < 2) tmpLine = true;
        }
      }
    }
    if (ScanScaleLine[n] == 2) {
      colf = TFT_BLACK;
      colb = TFT_OLIVE;
    }
    if (ScanValueSNR[n] > 0) {
      colf = TFT_NAVY + 0x8000;
      if (ScanValueSNR[n] < 16) {
        colf += (ScanValueSNR[n] * 2048);
      } else {
        colf = 0xF810;
        if (ScanValueSNR[n] < 24) colf += ((ScanValueSNR[n] - 16) * 258); else colf = 0xFF1E;
      }
    }

    if ((currentScanFreq + ((n - 159 + d + deltaScanLine) * SCANstep)) > band[bandIdx].maximumFreq) {
      colf = TFT_GREY;
      if (ScanEndBand == (320 - (d * 2))) ScanEndBand = n;
    }
    if ((currentScanFreq + ((n - 159 + d + deltaScanLine) * SCANstep)) < band[bandIdx].minimumFreq) {
      colf = TFT_GREY;
      if (ScanBeginBand < n) ScanBeginBand = n;
    }

    int tmpValue = ScanValueRSSI[n];
    if (tmpValue < 82) tmpValue = 82;
    tft.drawLine(n, tmpValue + 1, n, 198 + d, colf);
    tft.drawLine(n, 81, n, tmpValue - 1, colb);

    if (!ScanScaleLine[n]) {
      if (SCANstep <= 2){
        if (SCANstep == 2) {
          if ((frq + 50 - (int((frq + 50) / 100) * 100)) < 2) ScanScaleLine[n] = 3;
        } else {
          if ((frq + 50 - (int((frq + 50) / 100) * 100)) == 0) {
            if (!prevScaleLine) ScanScaleLine[n] = 3;
            tmpLine = true;
          }
        }
      }
    }
    if (ScanScaleLine[n] == 3) tft.drawLine(n, 81, n, 95, TFT_OLIVE);

    if (!ScanScaleLine[n]) {
      if (SCANstep < 2){
        for (int i = 10; i < 50; i +=10) {
          if ((frq + i - (int((frq + i) / 100) * 100)) == 0) {
            if (!prevScaleLine) ScanScaleLine[n] = 4;
            tmpLine = true;
          }
          if ((frq + i + 50 - (int((frq + i + 50) / 100) * 100)) == 0) {
            if (!prevScaleLine) ScanScaleLine[n] = 4;
            tmpLine = true;
          }
        }
      }
    }
    if (ScanScaleLine[n] == 4) tft.drawLine(n, 81, n, 88, TFT_OLIVE);

    prevScaleLine = tmpLine;
    if (!ScanScaleLine[n]) ScanScaleLine[n] = 1;

    int tmpValuePrev = ScanValueRSSI[n - 1];
    if (tmpValuePrev < 82) tmpValuePrev = 82;
    if (n == 0 || n == int(currentScanLine) + 1) tft.drawPixel(n, tmpValue, TFT_SILVER); else tft.drawLine(n - 1, tmpValuePrev, n, tmpValue, TFT_SILVER);
  }

  if (ScanMark[n]) tftTransRect(n, 95, 1, 5, TFT_YELLOW);
}

// ============================================================================
void DisplaySCAN() {
// ============================================================================
  int d = screenV * 40;
  bool setf = false;
  posScan = (int(posScanFreq - currentScanFreq) / SCANstep) + 159 - d - deltaScanLine;
  if (posScan < 0) posScan = 0;
  if (posScan >= ScanEndBand && !ScanEmpty) {
    posScan = 0;
    setf = true;
  }
  if (posScan > (319 - (d * 2))) {
    posScan = 0;
    setf = true;
    ScanEmpty = false;
  }
  if (posScan < 0) {
    posScan = 0;
    setf = true;
  }
  if (posScan <= ScanBeginBand && !ScanEmpty) {
    posScan = ScanBeginBand +1;
    setf = true;
  }
  if (setf) {
    setFreq(currentScanFreq + int((deltaScanLine - 159 + d + posScan) * SCANstep));
  } else {
    const unsigned long settleMs =
      (currentMode == FM) ? SCAN_SETTLE_FM_MS : SCAN_SETTLE_HAM_MS;
    if ((millis() - scanTuneStarted) < settleMs) return;

    if (posScan == posScanLast) ScanValueRSSI[posScan] = (ScanValueRSSI[posScan] + getSignal(true)) / 2; else ScanValueRSSI[posScan] = getSignal(true);
    if (posScan == posScanLast) ScanValueSNR[posScan] = (ScanValueSNR[posScan] + getSignal(false)) / 2; else ScanValueSNR[posScan] = getSignal(false);
    if (ScanValueSNR[posScan] >= ScanMarkSNR && posScan > ScanBeginBand && posScan < ScanEndBand) ScanMark[posScan] = true;
    if (!Saver) {
      drawSCANline(posScan);
      DrawSCANtxt(false);
    }
    if (SCANstep < 1) {
      for (int i = 1; i < 1 / SCANstep; i++) {
        posScan++;
        if (posScan < (320 - (d * 2))) {
          if (isSSBLikeMode() && !ScanEmpty) {
            si4735.setSSBBfo(1000 * SCANstep * i);
            ScanValueRSSI[posScan] = getSignal(true);
            ScanValueSNR[posScan] = getSignal(false);
          } else {
            ScanValueRSSI[posScan] = ScanValueRSSI[posScan - 1];
            ScanValueSNR[posScan] = ScanValueSNR[posScan - 1];
          }
          if (ScanValueSNR[posScan] >= ScanMarkSNR) ScanMark[posScan] = true;
          if (!Saver) {
            drawSCANline(posScan);
            DrawSCANtxt(false);
          }
        }
      }
      if (isSSBLikeMode() && !ScanEmpty) si4735.setSSBBfo(0);
    }
    posScanLast = posScan;

    if (ScanEmpty) setFreq(currentScanFreq + int((deltaScanLine - 159 + d + posScan + 1) * SCANstep)); else freqUp();
  }
}

// ============================================================================
int getSignal(bool rssi) {
// ============================================================================
  int res = 0;
  for (int i = 0; i < countScanSignal; i++) {
    si4735.getCurrentReceivedSignalQuality();
    if (rssi) res += si4735.getCurrentRSSI(); else res += si4735.getCurrentSNR();
  }
  if (rssi) res = 198 + (screenV * 40) - ((res / countScanSignal) * signalScale); else res /= countScanSignal;
  return (int) res;
}

// ============================================================================
void commSetFreq(float f) {
// ============================================================================
  si4735.setFrequency(f);
  Serial.println("commSetFreq()"); // mjs
  commStuff();
}

// ============================================================================
void setFreq(float f) {
// ============================================================================
  posScanFreq = f;
  commSetFreq(f);
  if (SCANbut) scanTuneStarted = millis();
  if (isSSBLikeMode()) si4735.setAutomaticGainControl(1, 0); //AGC disabled
}

// ============================================================================
void commFreqUp() {
// ============================================================================
  si4735.frequencyUp();
  Serial.println("commFreqUp()"); // mjs
  commStuff(); // mjs
}

// ============================================================================
void commFreqDown() {
// ============================================================================
  si4735.frequencyDown();
  Serial.println("commFreqDown()"); // mjs
  commStuff(); // mjs
}

// ============================================================================
void freqUp() {
// ============================================================================
  posScanFreq++;
  commFreqUp();
  if (SCANbut) scanTuneStarted = millis();
  if (isSSBLikeMode()) si4735.setAutomaticGainControl(1, 0); //AGC disabled
}

// ============================================================================
void DrawSCANtxt(bool all) {
// ============================================================================
  int d = screenV * 80;
  tft.setTextSize(1);
  tft.setTextColor(TFT_SILVER, TFT_BLACK);
  if ((ScanEndBand < (315 - d)) && ((posScan > (ScanEndBand + 5)) && (posScan < (ScanEndBand + 45))) || all) {
    tft.fillRect(ScanEndBand + 3, 110, 40, 32, TFT_BLACK);
    tft.setTextDatum(BL_DATUM);
    tft.drawString("END OF", ScanEndBand + 5, 120);
    tft.drawString("BAND", ScanEndBand + 5, 130);
    tft.drawString(band[bandIdx].bandName, ScanEndBand + 5, 140);
  }
  if ((ScanBeginBand > 5) && ((posScan > (ScanBeginBand - 43)) && (posScan < (ScanBeginBand - 3))) || all) {
    tft.fillRect(ScanBeginBand - 43, 110, 40, 32, TFT_BLACK);
    tft.setTextDatum(BR_DATUM);
    tft.drawString("BEGIN", ScanBeginBand - 5, 120);
    tft.drawString("BAND", ScanBeginBand - 5, 130);
    tft.drawString(band[bandIdx].bandName, ScanBeginBand - 5, 140);
  }
  if (posScan < 60 || all) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(BL_DATUM);
    if (currentMode == FM) tft.drawString(String((currentScanFreq + (SCANstep * (deltaScanLine - 159 + (d / 2)))) / 100) + " MHz ", 0, 90); else tft.drawString(String(int(currentScanFreq + (SCANstep * (deltaScanLine - 159 + (d / 2))))) + " KHz ", 0, 90);
    tft.fillRect(0, 181 + (d / 2), 47, 17, TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    if (currentMode == FM && SCANstep > 4) tft.drawString("10 MHz", 0, 195 + (d / 2));
    if ((currentMode == FM && SCANstep == 4) || (currentMode != FM && SCANstep > 4)) tft.drawString("1 MHz", 0, 195 + (d / 2));
    if (currentMode == FM && SCANstep == 2) tft.drawString("500 KHz", 0, 195 + (d / 2));
    if ((currentMode == FM && SCANstep < 2) || (currentMode != FM && SCANstep == 4)) tft.drawString("100 KHz", 0, 195 + (d / 2));
    if (currentMode != FM && SCANstep == 2) tft.drawString("50 KHz", 0, 195 + (d / 2));
    if (currentMode != FM && SCANstep < 2) tft.drawString("10 KHz", 0, 195 + (d / 2));
  }
  if (posScan > (240 - d) || all) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(BR_DATUM);
    if (currentMode == FM) tft.drawString(" " + String((currentScanFreq + (SCANstep * (160 - (d / 2) + deltaScanLine))) / 100) + " MHz", 319 - d, 90); else tft.drawString(" " + String(int(currentScanFreq + (SCANstep * (160 - (d / 2) + deltaScanLine)))) + " KHz", 319 - d, 90);
    tft.fillRect(296 - d, 181 + (d / 2), 23, 17, TFT_BLACK);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    if (SCANstep >= 1) tft.drawString("1:" + String(int(SCANstep)), 319 - d, 195 + (d / 2)); else tft.drawString("x" + String(int(1 / SCANstep)), 319 - d, 195 + (d / 2));
  }
}

// ============================================================================
void DrawSCANind() {
// ============================================================================
  int d = !screenV * 40;
  tft.fillRect(d, 60, 80, 20, TFT_GREY);
  tft.setTextSize(1);
  tft.setTextDatum(BC_DATUM);
  tft.drawRect(d, 64, 49, 13, TFT_SKYBLUE);
  tft.setTextColor(TFT_SKYBLUE, TFT_GREY);
  if (currentMode == FM) tft.drawString(String((band[bandIdx].currentStep) * 10) + " KHz", d + 25, 75); else  tft.drawString(String(band[bandIdx].currentStep) + " KHz", d + 25, 75);
  tft.drawRect(d + 50, 64, 29, 13, TFT_YELLOW);
  tft.setTextColor(TFT_YELLOW, TFT_GREY);
  Modtext = bandModeDesc[currentMode];
  if ((Modtext == "USB") && (CWShift == true)) Modtext = "CW";
  tft.drawString(Modtext, d + 65, 75);
}

// ============================================================================

// ============================================================================
// ============================================================================
void initRetro() {
// ============================================================================
  drawRETRO();
  if (bandIdx != bandRetro[RETROband].band || currentMode != bandMode[bandIdx]) {
    bandIdx = bandRetro[RETROband].band;
    currentMode = bandMode[bandIdx];
    BandSet();
  }
  if (bandRetro[RETROband].currentFreq < bandRetro[RETROband].minimumFreq || bandRetro[RETROband].currentFreq > bandRetro[RETROband].maximumFreq) bandRetro[RETROband].currentFreq = bandRetro[RETROband].minimumFreq;
  float freq = bandRetro[RETROband].currentFreq;
  if (bandIdx == 0) freq *= 100;
  commSetFreq(freq);
  si4735.setFrequencyStep(bandRetro[RETROband].hardStep);
  band[bandIdx].currentStep = bandRetro[RETROband].hardStep;
  band[bandIdx].currentFreq = si4735.getFrequency();
}

// ============================================================================
void drawRETRO() {
// ============================================================================
  tft.fillScreen(COLOR_BACKGROUND);
  int d = screenV * 80;
  if (screenV) {
    for (int n = 1; n <= 20; n++) {
      tft.fillRect(40 - (n * 2), 200, 2, 40, ((int(n / 2) * 4096) + (n * 32)));
      tft.fillRect((n * 2) + 198, 200, 2, 40, ((int(n / 2) * 4096) + (n * 32)));
    }
  }
  tft.fillRect(0, 36, 320 - d, 4, TFT_LIGTHYELLOW);
  tftRusSetFont(T1516_T);
  tftRusSetSize(1);
  tftRusSetColor(TFT_OLIVE, TFT_TRANS);
  tftRusSetDatum(BL_T);
  tftRusSetStyle(NCB_T);
  tftRusSetCut(0, 10 + (d / 10));
  tftRusPrint(String(presetNameLoad()), 0, 18);
  tftRusSetCut(0, 0);
  if (tftRusLength(String(presetNameLoad())) > (10 + (d / 10))) {
    textScroll = 0;
    elapsedScroll = millis() + 3000;
  }
  tftRusSetColor(TFT_RED, TFT_TRANS);
  tftRusSetStyle(NBL_T);
  if (RETROband || screenV) {
    x = 284 - (d * 1.375);
    y = 16 + (d * 2.525);
  } else {
    x = 293;
    y = 18;
  }
  if (langRetroEN) tftRusPrint(String(bandRetro[RETROband].BandName), x, y); else tftRusPrint(String(bandRetro[RETROband].BandNameRU), x, y);
  tft.setTextSize(1);
  tft.setTextDatum(BL_DATUM);
  tft.setTextColor(TFT_LIGTHYELLOW, TFT_BLACK);
  if (!RETROband || screenV) {
    tft.drawString("STEREO", 250 - d, 32);
    tft.drawRect(295 - d, 22, 20, 11, TFT_LIGTHYELLOW);
  }
  tft.drawString("VOLUME", 0, 32);
  tft.drawLine(46, 21, 127, 21, TFT_DARKCYAN);
  drawRetroVol();
  tft.setTextSize(2);
  if (RETROband || screenV) {
    x = 284 - (d * 1.375);
    y = 35 + (d * 2.5);
  } else {
    x = 250;
    y = 20;
  }
  if (RETROband > 1) tft.drawString("KHz", x, y); else tft.drawString("MHz", x, y);
  for (int i = 0; i < 6; i++) {
    tft.fillRect(0, i * 12 + 51, 320 - d, 1, TFT_LIGTHYELLOW);
    tft.fillRect(0, i * 12 + 139, 320 - d, 1, TFT_LIGTHYELLOW);
  }
  drawButtons(L_RETRO);
}

// ============================================================================
void drawRetroVol() {
// ============================================================================
  int vol = map(si4735.getVolume(), MinVOL, MaxVOL, 0, 19);
  for (int i = 0; i < 20; i++) {
    int color = (31 - (abs(10 - i) * 2)) * 2113;
    if (i == vol) color = TFT_RED;
    tft.drawRect((i * 4) + 47, 24, 2, 7, color);
  }
}

// ============================================================================
void drawRETROscale() {
// ============================================================================
  int d = screenV * 80;
  Displayfreq = si4735.getFrequency();
  if (bandHamRetro) Displayfreq -= (currentBFO / 1000);
  if (bandIdx == 0) Displayfreq /= 100;
  if (RETROband == 0) Segment(String(Displayfreq, 1), "188.8", 0);
  else
  if (RETROband == 1) Segment(String(Displayfreq, 2), "88.88", 0);
  else
  if (RETROband == 2) Segment(String(Displayfreq, 0), "888", 0);
  else
  if (RETROband == 3) Segment(String(Displayfreq, 0), "1888", 0);
  else Segment(String(Displayfreq, 0), "88888", 0);
  int color = TFT_LIGTHYELLOW;
  if (band[bandIdx].currentStep == bandRetro[RETROband].softStep && bandRetro[RETROband].hardStep != bandRetro[RETROband].softStep) color = 0xFE10;
  if (band[bandIdx].currentStep != bandRetro[RETROband].softStep && band[bandIdx].currentStep != bandRetro[RETROband].hardStep) color = 0xFF14;
  tftRusSetFont(T1012_T);
  tftRusSetSize(1);
  tftRusSetColor(TFT_BLACK, TFT_TRANS);
  tftRusSetDatum(BC_T);
  tftRusSetStyle(NBL_T);

  float tmp = 160 - (d / 2);
  float tmpMark = currentRetroScale * bandRetro[RETROband].mark;
  while (tmp >= tmpMark) tmp -= tmpMark;
  float freq = (Displayfreq - (trunc(Displayfreq / bandRetro[RETROband].mark) * bandRetro[RETROband].mark)) * currentRetroScale;
  tft.fillRect(0, 112, 320 - d, 16, color);

  for (float i = tmp - freq - tmpMark; i < (320 - d + (tmpMark / 2)); i += tmpMark) {
    freq = Displayfreq + ((i - 160 + (d / 2)) / currentRetroScale);
    if (freq > bandRetro[RETROband].maximumFreq || freq < bandRetro[RETROband].minimumFreq) {
      for (int n = 0; n < (tmpMark / 2); n += 4) tft.fillRect(i + n, 112, 2, 15, TFT_BLACK);
    } else {
      tftRusPrint(String(freq, 0), i, 126);
    }
    freq = Displayfreq + ((i + (tmpMark / 2) - 160 + (d / 2)) / currentRetroScale);
    if (freq > bandRetro[RETROband].maximumFreq || (freq + (bandRetro[RETROband].mark / 2)) < bandRetro[RETROband].minimumFreq) {
      for (int n = 0; n < (tmpMark / 2); n += 4) tft.fillRect(i + n + (tmpMark / 2), 112, 2, 15, TFT_BLACK);
    } else {
      tft.fillRect(i + (tmpMark / 2), 115, 2, 10, TFT_BLACK);
    }
  }
  if (RETROband > 3) {
    int i = 1;
    tft.fillRect(0, RetroStationPos[5] - 2, 319 - d, 9, TFT_BLACK);
    while (i < 29) {
      if (bandMode[i] == LSB || bandMode[i] == USB) {
        if (band[i].currentFreq >= bandRetro[RETROband].minimumFreq && band[i].currentFreq <= bandRetro[RETROband].maximumFreq) {
          int minPos = ((band[i].minimumFreq - bandRetro[RETROband].currentFreq) * currentRetroScale) + 159 - (d / 2);
          int maxPos = ((band[i].maximumFreq - bandRetro[RETROband].currentFreq) * currentRetroScale) + 159 - (d / 2);
          if (maxPos > -(currentRetroScale * 10) && minPos < (319 - d + (currentRetroScale * 10))) {
            if (minPos < -11) minPos = -11;
            if (maxPos > 330) maxPos = 330 - d;
            tft.fillRect(minPos, RetroStationPos[5] - 2, maxPos - minPos + 1, 9, TFT_BROWN);
          }
        }
      }
      i++;
    }
  }
  tftRusSetFont(T1012_T);
  tftRusSetSize(1);
  tftRusSetDatum(BR_T);
  tftRusSetStyle(NBL_T);
  tftRusBottomCut = true;
  for (int i = 0; i <= lastPreset; i++) {
    if (preset[i].presetIdx >= bandRetro[RETROband].minimumFreq && preset[i].presetIdx <= bandRetro[RETROband].maximumFreq) {
      freq = (((preset[i].presetIdx) - Displayfreq) * currentRetroScale) + 160 - (d / 2);
      int tmplen = tftRusLength(preset[i].PresetName) * 8;
      if (freq > -20 && freq < tmplen + 350 - d) {
        tft.fillRect(freq - tmplen - 40, preset[i].presetPos - 3, tmplen + 60, 11, TFT_BLACK);
        if ((preset[i].presetIdx) == Displayfreq) {
          tftRusSetColor(TFT_WHITE, TFT_TRANS);
          tft.fillRect(freq - 10, preset[i].presetPos, 20, 5, TFT_LIGTHYELLOW);
        } else {
          tftRusSetColor(TFT_SILVER, TFT_TRANS);
          tft.fillRect(freq - 10, preset[i].presetPos, 20, 5, TFT_OLIVE);
        }
        tftRusPrint(String(preset[i].PresetName), freq - 20, preset[i].presetPos + 9);
      }
    }
  }
  tftRusBottomCut = false;
  for (int i = 0; i < 6; i++) {
    tft.fillRect(158 - (d / 2), (i * 12) + 40, 4, 11, TFT_RED);
    tft.fillRect(158 - (d / 2), (i * 12) + 128, 4, 11, TFT_RED);
  }
}

// ============================================================================
void drawRetroCity() {
// ============================================================================
  int d = !screenV * 40;
  if (!cityRetroRotary) {
    tft.fillScreen(COLOR_BACKGROUND);
    tft.fillRect(d, 0, 240, 20, TFT_GREY);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_GREY);
    tft.setTextDatum(BC_DATUM);
    tft.drawString("CITY", 120 + d, 20);
    if (!screenV) {
      for (int n = 1; n <= 20; n++) {
        tft.fillRect(40 - (n * 2), 0, 2, 240, ((int(n / 2) * 4096) + (n * 32)));
        tft.fillRect((n * 2) + 278, 0, 2, 240, ((int(n / 2) * 4096) + (n * 32)));
      }
    }
  }

  int id;
  for (int i = 0; i <= lastGroup; i++) if (group[i].groupIdx == PresetId) id = i;

  id += cityRetroRotary;
  if (id < 0 || id > lastGroup) {
    id -= cityRetroRotary;
  } else {
    PresetId = group[id].groupIdx;

    uint16_t col;
    tftRusSetFont(T1012_T);
    tftRusSetSize(2);
    tftRusSetDatum(BC_T);
    tftRusSetStyle(NRG_T);

    for (int i = 0; i < 7; i++) {
      if (!screenV) {
        for (int n = 1; n <= 20; n++) {
          tft.fillRect(40 - (n * 2), (i * 30) + 20, 2, 30, ((int(n / 2) * 4096) + (n * 32)));
          tft.fillRect((n * 2) + 278, (i * 30) + 20, 2, 30, ((int(n / 2) * 4096) + (n * 32)));
        }
      }
      tft.fillRect(d, (i * 30) + 20, 240, 30, TFT_BLACK);
      if (i == 3) {
        tft.fillRect(d, (i * 30) + 20, 240, 1, TFT_CYAN);
        tft.fillRect(d, (i * 30) + 49, 240, 1, TFT_CYAN);
      }
      if ((id + i - 3) >= 0 && (id + i - 3) <= lastGroup) {
        switch (i) {
          case 0:
          case 6:
            col = 520;
            break;
          case 1:
          case 5:
            col = 780;
            break;
          case 2:
          case 4:
            col = TFT_DARKCYAN;
            break;
          default:
            col = TFT_WHITE;
            break;
        }
        tftRusSetColor(col, TFT_TRANS);
        tftRusPrint(String(group[id + i - 3].PresetName), 120 + d, (i * 30) + 45);
      }
    }
  }
  cityRetroRotary = 0;
}

// ============================================================================
void drawRetroBand() {
// ============================================================================
  int d = !screenV * 40;
  if (!screenV) {
    for (int n = 1; n <= 20; n++) {
      tft.fillRect(40 - (n * 2), 0, 2, 40, ((int(n / 2) * 4096) + (n * 32)));
      tft.fillRect((n * 2) + 278, 0, 2, 40, ((int(n / 2) * 4096) + (n * 32)));
    }
  }
  tft.fillScreen(COLOR_BACKGROUND);
  tft.fillRect(d, 0, 240, 20, TFT_GREY);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_GREY);
  tft.setTextDatum(BC_DATUM);
  tft.drawString("BAND", 120 + d, 20);
  drawRetroBandBut();
}

// ============================================================================
void drawRetroBandBut() {
// ============================================================================
  spr.createSprite(But_Width, But_Height);
  tftRusSetSize(1);
  for (int n = 0 ; n <= lastBandRetro; n++) {
    spr.fillScreen(COLOR_BACKGROUND);
    if (n == RETROband) {
      spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_fix);
    } else {
      if (VHFon || n != 1) spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_retro); else spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_block);
    }
    if (screenV) spr.pushSprite(bandRetro[n].xPosV, bandRetro[n].yPosV); else  spr.pushSprite(bandRetro[n].xPosH, bandRetro[n].yPosH);
    tftRusSetFont(T1516_T);
    tftRusSetStyle(NBL_T);
    tftRusSetDatum(BC_T);
    if (n == RETROband) {
      tftRusSetColor(TFT_LIGTHYELLOW, TFT_TRANS);
    } else {
      tftRusSetColor(TFT_WHITE, TFT_TRANS);
      if (langRetroEN) {
        if (screenV) tftRusPrint(String(bandRetro[n].BandName), bandRetro[n].xPosV + (But_Width / 2) + 3, bandRetro[n].yPosV + (But_Height / 2) + 9); else tftRusPrint(String(bandRetro[n].BandName), bandRetro[n].xPosH + (But_Width / 2) + 3, bandRetro[n].yPosH + (But_Height / 2) + 9);
      } else {
        if (screenV) tftRusPrint(String(bandRetro[n].BandNameRU), bandRetro[n].xPosV + (But_Width / 2) + 3, bandRetro[n].yPosV + (But_Height / 2) + 9); else tftRusPrint(String(bandRetro[n].BandNameRU), bandRetro[n].xPosH + (But_Width / 2) + 3, bandRetro[n].yPosH + (But_Height / 2) + 9);
      }
      tftRusSetColor(TFT_OLIVE, TFT_TRANS);
    }
    if (langRetroEN) {
      if (screenV) tftRusPrint(String(bandRetro[n].BandName), bandRetro[n].xPosV + (But_Width / 2) + 2, bandRetro[n].yPosV + (But_Height / 2) + 8); else tftRusPrint(String(bandRetro[n].BandName), bandRetro[n].xPosH + (But_Width / 2) + 2, bandRetro[n].yPosH + (But_Height / 2) + 8);
    } else {
      if (screenV) tftRusPrint(String(bandRetro[n].BandNameRU), bandRetro[n].xPosV + (But_Width / 2) + 2, bandRetro[n].yPosV + (But_Height / 2) + 8); else tftRusPrint(String(bandRetro[n].BandNameRU), bandRetro[n].xPosH + (But_Width / 2) + 2, bandRetro[n].yPosH + (But_Height / 2) + 8);
    }
    if ((langRetroEN && bandRetro[n].RetroBandTime != "") || (!langRetroEN && bandRetro[n].RetroBandTimeRU != "")) {
      if (screenV) tft.drawRect(bandRetro[n].xPosV, bandRetro[n].yPosV, 220, 40, TFT_OLIVE); else tft.drawRect(bandRetro[n].xPosH, bandRetro[n].yPosH, 220, 40, TFT_OLIVE);
    }
    tftRusSetFont(T1012_T);
    tftRusSetStyle(NRG_T);
    tftRusSetDatum(BL_T);
    tftRusSetColor(TFT_LIGTHYELLOW, TFT_TRANS);
    if (langRetroEN) {
      if (screenV) tftRusPrint(String(bandRetro[n].RetroBandTime), bandRetro[n].xPosV + 90, bandRetro[n].yPosV + 27); else tftRusPrint(String(bandRetro[n].RetroBandTime), bandRetro[n].xPosH + 90, bandRetro[n].yPosH + 27);
    } else {
      if (screenV) tftRusPrint(String(bandRetro[n].RetroBandTimeRU), bandRetro[n].xPosV + 90, bandRetro[n].yPosV + 27); else tftRusPrint(String(bandRetro[n].RetroBandTimeRU), bandRetro[n].xPosH + 90, bandRetro[n].yPosH + 27);
    }
  }
  spr.deleteSprite();
}

// ============================================================================

// ============================================================================
// MEMOIRES / SETUP
// ============================================================================
void displMEMO() {
// ============================================================================
  int d = !screenV * 40;
  tft.setTextSize(2);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_GREY);
  if (presetBank) tft.drawString("PRESETS", XFreqDispl + d + 120, YFreqDispl + 78); else tft.drawString("       ", XFreqDispl + d + 120, YFreqDispl + 78);
  tft.setTextSize(1);
  tftRusSetSize(1);
  tftRusSetStyle(NBL_T);
  for (int i = -1; i < (2 + screenV); i++) {
    if (MEMOdel) {
      tftTransRect(d, 120, 240, 40, TFT_RED);
    } else {
      if (i) {
        if (presetBank) tft.fillRect(d, (i * 40) + 120, 240, 40, TFT_NAVY); else tft.fillRect(d, (i * 40) + 120, 240, 40, TFT_BLACK);
      } else tft.fillRect(d, 120, 240, 40, TFT_DARKCYAN);
    }
    if (presetBank) {
      if ((currentMemo + i) >= 0 && (currentMemo + i) <= lastPreset ) {
        tftRusSetDatum(BL_T);
        tftRusSetFont(T1516_T);
        tftRusSetColor(TFT_YELLOW, TFT_TRANS);
        tftRusPrint(String(currentMemo + i + 1), d, (i * 40) + 141);
        tftRusSetColor(TFT_WHITE, TFT_TRANS);
        tftRusSetCut(0, 20);
        tftRusWidth = 12;
        tftRusPrint(String(preset[currentMemo + i].PresetName), d, (i * 40) + 157);
        tftRusSetCut(0, 0);
        tftRusSetDatum(BR_T);
        tftRusSetColor(TFT_CYAN, TFT_TRANS);
        if (preset[currentMemo + i].presetIdx < 109) tftRusPrint(String(preset[currentMemo + i].presetIdx, 2) + " MHz", d + 155, (i * 40) + 141); else tftRusPrint(String(preset[currentMemo + i].presetIdx, 0) + " KHz", d + 155, (i * 40) + 141);
        if (i) tft.setTextColor(2031, TFT_NAVY); else tft.setTextColor(2031, TFT_DARKCYAN);
        tft.drawString(band[bandFreq(preset[currentMemo + i].presetIdx)].bandName, d + 175, (i * 40) + 137);
        tft.drawRect(d + 155, (i * 40) + 124, 39, 16, 2031);
        if (i) tft.setTextColor(TFT_YELLOW, TFT_NAVY); else tft.setTextColor(TFT_YELLOW, TFT_DARKCYAN);
        tft.drawString(bandModeDesc[bandMode[bandFreq(preset[currentMemo + i].presetIdx)]], d + 215, (i * 40) + 137);
        tft.drawRect(d + 195, (i * 40) + 124, 39, 16, TFT_YELLOW);
      }
    } else {
      if ((currentMemo + i) >= 0 && (currentMemo + i) <= lastMemoBank ) {
        tftRusSetDatum(BL_T);
        tftRusSetFont(T1516_T);
        tftRusSetColor(TFT_YELLOW, TFT_TRANS);
        tftRusPrint(String(currentMemo + i + 1), d, (i * 40) + 141);
        if ((MemoBank[currentMemo + i].freq < 153 || MemoBank[currentMemo + i].freq > 30000) && !(!i && MEMOadd))  {
          if (!i && MEMOdel) {
            MEMOdel = false;
            ErrorBeep();
          }
          tftRusSetColor(TFT_WHITE, TFT_TRANS);
          tftRusPrint("empty", d, (i * 40) + 157);
        } else {
          tftRusSetColor(TFT_WHITE, TFT_TRANS);
          tftRusSetCut(0, 20);
          tftRusWidth = 12;
          if (MEMOadd && !i) tftRusPrint(String(addMemoName), d, 157); else tftRusPrint(String(MemoBank[currentMemo + i].Name), d, (i * 40) + 157);
          tftRusSetCut(0, 0);
          tftRusSetDatum(BR_T);
          tftRusSetColor(TFT_CYAN, TFT_TRANS);
          if (MEMOadd && !i) {
            if (addMemoBand == FM) tftRusPrint(String(float(addMemoFreq) / 100, 2) + " MHz", d + 155, 141); else tftRusPrint(String(addMemoFreq) + " KHz", d + 155, 141);
          } else {
            if (MemoBank[currentMemo + i].band == FM) tftRusPrint(String(float(MemoBank[currentMemo + i].freq) / 100, 2) + " MHz", d + 155, (i * 40) + 141); else tftRusPrint(String(MemoBank[currentMemo + i].freq) + " KHz", d + 155, (i * 40) + 141);
          }
          if (i) tft.setTextColor(2031, TFT_BLACK); else tft.setTextColor(2031, TFT_DARKCYAN);
          if (MEMOadd && !i) tft.drawString(band[addMemoBand].bandName, d + 175, 137); else tft.drawString(band[(MemoBank[currentMemo + i].band & 0x1F)].bandName, d + 175, (i * 40) + 137);
          tft.drawRect(d + 155, (i * 40) + 124, 39, 16, 2031);
          if (i) tft.setTextColor(TFT_YELLOW, TFT_BLACK); else tft.setTextColor(TFT_YELLOW, TFT_DARKCYAN);
          if (MEMOadd && !i) tft.drawString(bandModeDesc[addMemoMode], d + 215, 137); else tft.drawString(bandModeDesc[int(MemoBank[currentMemo + i].band / 32)], d + 215, (i * 40) + 137);
          tft.drawRect(d + 195, (i * 40) + 124, 39, 16, TFT_YELLOW);
        }
      }
    }
  }
}

// ============================================================================
int bandFreq(float freq) {
// ============================================================================
  int n = 0;
  if (freq < 64 || freq > 108) {
    n = 1;
    bool flag = false;
    while (n < 29 && !flag) if (freq >= band[n].minimumFreq && freq <= band[n].maximumFreq) flag = true; else n++;
  }
  return n;
}

// ============================================================================
void displSETUP() {
// ============================================================================
  int d = !screenV * 40;
  if (!screenV) {
    for (int n = 1; n <= 20; n++) {
      tft.fillRect(40 - (n * 2), 40, 2, 160, ((int(n / 2) * 4096) + (n * 32)));
      tft.fillRect((n * 2) + 278, 40, 2, 160, ((int(n / 2) * 4096) + (n * 32)));
    }
  }
  tft.fillRect(d, 20, 240, 100, TFT_BLACK);
  spr.fillScreen(COLOR_BACKGROUND);

  tft.setTextSize(2);
  tft.setTextDatum(BR_DATUM);
  tft.setTextColor(TFT_LIGTHYELLOW, TFT_BLACK);

  switch (pageSetup) {
    case 0:
      tft.drawString("SI473X", 240 + d, 40);
      displSETUPitem     ("FM start 64 MHz  ", 80,  prevVHFon, (VHFon != prevVHFon));
      displSETUPitem     ("Seek AM 1 KHz    ", 120,  prevseekAccuracy, (seekAccuracy != prevseekAccuracy));
      break;
    case 1:
      tft.drawString("USAGE", 240 + d, 40);
      displSETUPitem     ("RDS only FMbutton", 40,  !prevRDSalways, (RDSalways != prevRDSalways));
      displSETUPitem     ("Digit backlight  ", 80,  prevdigitLigth, (digitLigth != prevdigitLigth));
      displSETUPitem     ("Memo in preset   ", 120, prevmemoPreset, (memoPreset != prevmemoPreset));
      displSETUPitem     ("Retro language RU", 160, !prevlangRetroEN, (langRetroEN != prevlangRetroEN));
      break;
    case 2:
      tft.drawString("DISPLAY", 240 + d, 40);
      displSETUPitem     ("Screen saver     ", 40,  prevsaverOn, (saverOn != prevsaverOn));
      displSETUPitem     ("Display light off", 80,  prevdisplayOff, (displayOff != prevdisplayOff));
      displSETUPitemValue("Saver time in min", 120, String(prevsaverTime), (saverTime != prevsaverTime));
      displSETUPitem     ("Vertical screen  ", 160, prevscreenV, (screenV != prevscreenV));
      break;
    case 3:
      tft.drawString("SCAN", 240 + d, 40);
      displSETUPitemValue("Minimum scale    ", 40,  String("x" + String(int(1 / prevminSCANstep))), (minSCANstep != prevminSCANstep));
      displSETUPitemValue("Maximum scale    ", 80,  String("1:" + String(int(prevmaxSCANstep))), (maxSCANstep != prevmaxSCANstep));
      displSETUPitem     ("Auto scale       ", 120, prevautoSCANstep, (autoSCANstep != prevautoSCANstep));
      displSETUPitem     ("Scan accuracy    ", 160, prevSCANaccuracy, (SCANaccuracy != prevSCANaccuracy));
      break;
    case 4:
      tft.drawString("HARDWARE", 240 + d, 40);
      displSETUPitem     ("Battery show     ", 80,  prevbatShow, (batShow != prevbatShow));
      displSETUPitem     ("Beeper On        ", 120, prevbeeperOn, (beeperOn != prevbeeperOn));
      displSETUPitem     ("Bright disp power", 160, prevdisplayPower, (displayPower != prevdisplayPower));
      break;
    case 5:
      tft.drawString("DEFAULT", 240 + d, 40);
      displSETUPitem     ("Load default Memo", 80,  prevloadMemory, (loadMemory != prevloadMemory));
      displSETUPitem     ("Reset to factory ", 120, prevloadDefault, (loadDefault != prevloadDefault));
      break;
  }
}

// ============================================================================
void displSETUPitem(String itemName, int pos, bool state, bool changed) {
// ============================================================================
  int d = !screenV * 40;
  if (changed) tftTransRect(d, pos + 2, 240, 36, 0xC000);
  tft.drawRect(d, pos + 5, 30, 30, TFT_WHITE);
  if (state) tft.fillRect(d + 5, pos + 10, 20, 20, TFT_WHITE);
  tftRusSetSize(1);
  tftRusSetStyle(NBL_T);
  tftRusSetDatum(BL_T);
  tftRusSetFont(T1516_T);
  tftRusSetColor(TFT_WHITE, TFT_TRANS);
  tftRusWidth = 12;
  tftRusPrint(itemName, d + 40, pos + 30);
}

// ============================================================================
void displSETUPitemValue(String itemName, int pos, String state, bool changed) {
// ============================================================================
  int d = !screenV * 40;
  if (changed) tftTransRect(d, pos + 2, 280, 36, 0xC000);
  tft.drawRect(d, pos + 5, 30, 30, TFT_WHITE);
  tft.setTextSize(1);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(state, d + 15, pos + 25);
  tftRusSetSize(1);
  tftRusSetStyle(NBL_T);
  tftRusSetDatum(BL_T);
  tftRusSetFont(T1516_T);
  tftRusSetColor(TFT_WHITE, TFT_TRANS);
  tftRusWidth = 12;
  tftRusPrint(itemName, d + 40, pos + 30);
}

// ============================================================================
void defaultSETUP() {
// ============================================================================
  if (confirm("LOAD DEFAULT?") == 1) {
    prevVHFon = true;
    prevseekAccuracy = false;

    prevRDSalways = false;
    prevdigitLigth = true;
    prevmemoPreset = false;
    prevlangRetroEN = true;

    prevsaverOn = true;
    prevdisplayOff = false;
    prevsaverTime = 10;
    prevscreenV = false;

    prevminSCANstep = 0.125;
    prevmaxSCANstep = 8;
    prevautoSCANstep = true;
    prevSCANaccuracy = true;

    prevbatShow = false;
    prevbeeperOn = true;
    prevdisplayPower = false;

    prevloadMemory = false;
    prevloadDefault = false;
  }
  drawList(L_SETUP,"SETUP");
  if (!pageSetup) drawButton(L_SETUP, 0, B_BLOCK);
  if (pageSetup == maxPageSetup) drawButton(L_SETUP, 1, B_BLOCK);
}

// ============================================================================
void changeSETUP(int pos) {
// ============================================================================
  switch (pageSetup) {
    case 0:
      switch (pos) {
        case 1:
          prevVHFon = !prevVHFon;
          break;
        case 2:
          prevseekAccuracy = !prevseekAccuracy;
          break;
      }
      break;
    case 1:
      switch (pos) {
        case 0:
          prevRDSalways = !prevRDSalways;
          break;
        case 1:
          prevdigitLigth = !prevdigitLigth;
          break;
        case 2:
          prevmemoPreset = !prevmemoPreset;
          break;
        case 3:
          prevlangRetroEN = !prevlangRetroEN;
          break;
      }
      break;
    case 2:
      switch (pos) {
        case 0:
          prevsaverOn = !prevsaverOn;
          break;
        case 1:
          prevdisplayOff = !prevdisplayOff;
          break;
        case 2:
          if (prevsaverTime == 2) prevsaverTime = 5;
          else
          if (prevsaverTime == 5) prevsaverTime = 10;
          else
          if (prevsaverTime == 10) prevsaverTime = 30;
          else
          if (prevsaverTime == 30) prevsaverTime = 2;
          break;
        case 3:
          prevscreenV = !prevscreenV;
          break;
      }
      break;
    case 3:
      switch (pos) {
        case 0:
          prevminSCANstep *= 2;
          if (prevminSCANstep == 1) prevminSCANstep = 0.125;
          break;
        case 1:
          prevmaxSCANstep *= 2;
          if (prevmaxSCANstep == 16) prevmaxSCANstep = 1;
          break;
        case 2:
          prevautoSCANstep = !prevautoSCANstep;
          break;
        case 3:
          prevSCANaccuracy = !prevSCANaccuracy;
          break;
      }
      break;
    case 4:
      switch (pos) {
        case 1:
          prevbatShow = !prevbatShow;
          break;
        case 2:
          prevbeeperOn = !prevbeeperOn;
          break;
        case 3:
          prevdisplayPower = !prevdisplayPower;
          break;
      }
      break;
    case 5:
      switch (pos) {
        case 1:
          prevloadMemory = !prevloadMemory;
          if (prevloadMemory) prevloadDefault = false;
          break;
        case 2:
          prevloadDefault = !prevloadDefault;
          if (prevloadDefault) prevloadMemory = false;
          break;
      }
      break;
  }
}

// ============================================================================
void saveSETUP() {
// ============================================================================
  if (VHFon != prevVHFon || langRetroEN != prevlangRetroEN || beeperOn != prevbeeperOn || digitLigth != prevdigitLigth || loadMemory != prevloadMemory ||
      batShow != prevbatShow || memoPreset != prevmemoPreset || loadDefault != prevloadDefault || saverOn != prevsaverOn || saverTime != prevsaverTime ||
      screenV != prevscreenV || displayOff != prevdisplayOff || minSCANstep != prevminSCANstep || maxSCANstep != prevmaxSCANstep ||
      autoSCANstep != prevautoSCANstep || SCANaccuracy != prevSCANaccuracy || displayPower != prevdisplayPower || RDSalways != prevRDSalways ||
      seekAccuracy != prevseekAccuracy) {
    int n = confirm("SAVE CHANGES?");
    if (n == 1) {
      if (VHFon != prevVHFon) {
        VHFon = prevVHFon;
        if (VHFon) band[0].minimumFreq = 6400; else band[0].minimumFreq = 8750;
        si4735.setSeekFmLimits(band[0].minimumFreq, band[0].maximumFreq);
        if (!bandIdx) si4735.setFM(band[0].minimumFreq, band[0].maximumFreq, band[0].currentFreq, band[0].currentStep);
      }
      if (memoPreset != prevmemoPreset) {
        memoPreset = prevmemoPreset;
        presetLoad();
        presetSort();
      }
      if (loadDefault != prevloadDefault) {
        loadDefault = prevloadDefault;
        if (loadDefault) storage.chkDigit = 32; else storage.chkDigit = 64;
      }
      if (screenV != prevscreenV) {
        screenV = prevscreenV;
        screenRotate();
      }
      langRetroEN = prevlangRetroEN;
      beeperOn = prevbeeperOn;
      digitLigth = prevdigitLigth;
      loadMemory = prevloadMemory;
      batShow = prevbatShow;
      saverOn = prevsaverOn;
      saverTime = prevsaverTime;
      displayOff = prevdisplayOff;
      minSCANstep = prevminSCANstep;
      maxSCANstep = prevmaxSCANstep;
      autoSCANstep = prevautoSCANstep;
      SCANaccuracy = prevSCANaccuracy;
      displayPower = prevdisplayPower;
      RDSalways = prevRDSalways;
      seekAccuracy = prevseekAccuracy;
      if (SCANaccuracy) countScanSignal = 3; else countScanSignal = 1;

      if (loadMemory || loadDefault) {
        if (confirm("REEBOOT NOW?") == 1) {
          tft.fillRect(!screenV * 40, 40, 240, 120, TFT_BLACK);
          tft.setTextSize(2);
          tft.setTextDatum(BC_DATUM);
          tft.setTextColor(TFT_WHITE, TFT_BLACK);
          tft.drawString("REBOOTING...", 160 - (screenV * 40), 100);
          delay(5000);
          ESP.restart();
        }
      }
    } else if (n == -1) {
      drawList(L_SETUP,"SETUP");
      if (!pageSetup) drawButton(L_SETUP, 0, B_BLOCK);
      if (pageSetup == maxPageSetup) drawButton(L_SETUP, 1, B_BLOCK);
      SETUPbut = true;
    }
  }
}

// ============================================================================
int confirm(String text) {
// ============================================================================
  int d = !screenV * 40;
  if (!screenV) tftTransRect(0, 0, 320, 240, TFT_MAROON); else tftTransRect(0, 0, 240, 320, TFT_MAROON);
  tft.fillRect(d, 40, 240, 120, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(String(text), 120 + d, 80);
  drawButtons(L_CONFIRM);
  int n = -1;
  while (n < 0 && (elapsedSaver + 120000) > millis()) {
    presStat = tft.getTouch(&x, &y);
    n = jamButton(L_CONFIRM);
  }
  if (n >= 0) {
    elapsedSaver = millis();
    x = y = 0;
  }
  return n;
}

// ============================================================================
void tftTransRect(int x, int y, int w, int h, uint16_t c) {
// ============================================================================
  bool z = 0;
  for (int i = x; i < x + w; i++) {
    for (int j = y; j < y + h; j += 2) {
      tft.drawPixel(i, j + z, c);
    }
    z = !z;
  }
}

// ============================================================================
String presetNameLoad() { //loading preset name ---------------------------------
// ============================================================================
  String ret = "";
  for (int i = 0; i <= lastGroup; i++) if (group[i].groupIdx == PresetId) ret = String(group[i].PresetName);
  return (String) ret;
}

// ============================================================================
void presetLoad() { //loading preset by city name -----------------------------
// ============================================================================
  int count = 0;
  for (int i = 0; i <= lastMemory; i++) {
    if (memory[i].memoryGroup == "" && presetBank) {
        preset[count].presetIdx = memory[i].memoryIdx;
        preset[count].PresetName = memory[i].MemoryName;
        count++;
    } else {
      int n = strlen(memory[i].memoryGroup) + 1;
      char p[n];
      char *pp = memory[i].memoryGroup;
      for (int j = 0; j < n; j++) p[j] = pp[j];
      char *str = strtok(p, " ");
      while (str != NULL) {
        if (String(str) == String(PresetId) || String(str) == "ALL") {
          preset[count].presetIdx = memory[i].memoryIdx;
          preset[count].PresetName = memory[i].MemoryName;
          preset[count].presetPos = 0;
          count++;
        }
        str = strtok(NULL, " ");
      }
    }
  }

  if (memoPreset && !presetBank) {
    for (int i = 0; i <= lastMemoBank; i++) {
      if (MemoBank[i].freq >= 153 && MemoBank[i].freq <= 30000) {
        if (MemoBank[i].band) preset[count].presetIdx = MemoBank[i].freq; else preset[count].presetIdx = float(MemoBank[i].freq) / 100;
        preset[count].PresetName = MemoBank[i].Name;
        int j = 19;
        while (j > 0 && (MemoBank[i].Name[j] == char(32) || MemoBank[i].Name[j] == char(0))) j--;
        preset[count].PresetName[j + 1] = NULL;
        preset[count].presetPos = 0;
        count++;
      }
    }
  }

  lastPreset = count - 1;
}

// ============================================================================
void presetSort() { //sorting preset by frequency ----------------------------------------
// ============================================================================
  for (int i = 0; i < lastPreset; i++) {
    for (int j = i + 1; j <= lastPreset; j++) {
      if (preset[j].presetIdx < preset[i].presetIdx) {
        float f = preset[i].presetIdx;
        char *n = preset[i].PresetName;
        preset[i].presetIdx = preset[j].presetIdx;
        preset[i].PresetName = preset[j].PresetName;
        preset[j].presetIdx = f;
        preset[j].PresetName = n;
      }
    }
  }
}

// ============================================================================
void presetSetPos() { //set position fm station on retro scale ---------------------------
// ============================================================================
  currentRetroScale = bandRetro[RETROband].scale;
  int y = random(3);
  float tmpPos[12];
  for (int i = 0; i <= lastPreset; i++) {
    if (preset[i].presetIdx >= bandRetro[RETROband].minimumFreq && preset[i].presetIdx <= bandRetro[RETROband].maximumFreq) {
      int j = 0;
      while (j < 12) {
        if (tmpPos[j] <= (((preset[i].presetIdx) * currentRetroScale) - ((tftRusLength(preset[i].PresetName) * 8) + 60))) j = 12;
        j++;
      }
      if (j == 12) {
        for (int n = 0; n < 12; n++) tmpPos[n] = 0;
        currentRetroScale++;
        i = -1;
      } else {
        while (tmpPos[y] > (((preset[i].presetIdx) * currentRetroScale) - ((tftRusLength(preset[i].PresetName) * 8) + 60))) {
          y += random(1, 3);
          if (y > 11) y = random(3);
        }
        tmpPos[y] = (preset[i].presetIdx) * currentRetroScale;
        preset[i].presetPos = RetroStationPos[y];
      }
    }
  }
}

/* Cyrillic output library for TFT_eSPI
 *  23.01.2022 Bernard Binns
 */

// ============================================================================
void tftRusSetSize(float siz) {
// ============================================================================
  tftRusSize = siz;
}

// ============================================================================
void tftRusSetColor(uint16_t color, int32_t back) {
// ============================================================================
  tftRusColor = color;
  tftRusBack = back;
}

// ============================================================================
void tftRusSetDatum(int datum) {
// ============================================================================
  tftRusDatum = datum;
}

// ============================================================================
void tftRusSetStyle(int style) {
// ============================================================================
  tftRusStyle = style;
  tftRusWidth = 8 + (tftRusFont * 4) + (2 * !(tftRusStyle & 0x02)) + tftRusFont;
}

// ============================================================================
void tftRusSetFont(int font) {
// ============================================================================
  tftRusFont = font;
  tftRusWidth = 8 + (tftRusFont * 4) + (2 * !(tftRusStyle & 0x02)) + tftRusFont;
}

// ============================================================================
void tftRusSetCut(int beg, int con) {
// ============================================================================
  tftRusBeginChar = beg;
  tftRusContChar = con;
}

// ============================================================================
void tftRusPrint(String text, int x, int y) {
// ============================================================================
  int curcount;
  int cd = 0;
  int tmpLen = tftRusLength(text);
  if (tmpLen > tftRusContChar && tftRusContChar) tmpLen = tftRusContChar;
  uint8_t ascii[tmpLen];
  int textLength = 0;

  for (int i = 0; i < text.length(); i++) {
    if (char(text[i]) >= 208) i++;
    if (textLength >= tftRusBeginChar && ((textLength - tftRusBeginChar) < tftRusContChar || !tftRusContChar)) ascii[textLength - tftRusBeginChar] = char(text[i]) - 32;
    textLength++;
  }
  textLength -= tftRusBeginChar;
  if (textLength > tftRusContChar && tftRusContChar) textLength = tftRusContChar;

  if (textLength && (tftRusColor >= 0 || tftRusBack >= 0)) {

    y -= (tftRusSize * (12 + (tftRusFont * 4)));
    if (tftRusDatum > 0) x -= tftRusSize * tftRusWidth * textLength;
    if (tftRusDatum == 0) x -= tftRusSize * tftRusWidth * textLength / 2;

    for (int i = 0; i < textLength; i++) {
      for (int xx = 0; xx < tftRusWidth; xx++) {
        if (tftRusStyle > 3) {
          cd = (2 + tftRusFont) * tftRusSize;
          curcount = tftRusCursiveLevel - 1;
        }
        for (int yb = 0; yb < 2; yb++ ) {

          unsigned short by;
          unsigned short byp;
          if (tftRusFont) {
            if (xx > 10) {
              by = 0x00;
            } else {
              by = Tahoma15x16[(ascii[i] * 22) + (xx * 2) + yb];
            }
            if ((tftRusStyle & 0x01) && xx > 0 && xx < 12) byp = Tahoma15x16[(ascii[i] * 22) + ((xx - 1) * 2) + yb];
          } else {
            if (xx > 6) {
              by = 0x00;
            } else {
              by = Tahoma10x12[(ascii[i] * 14) + (xx * 2) + yb];
            }
            if ((tftRusStyle & 0x01) && xx > 0 && xx < 8) byp = Tahoma10x12[(ascii[i] * 14) + ((xx - 1) * 2) + yb];
          }

          for (int yy = 0; yy < (8 - (yb * 4 * (1 - tftRusFont)) - (tftRusBottomCut * (yb * (2 + tftRusFont)))); yy++ ) {
            bool bi = (by >> yy) & 0x01;
            if ((tftRusStyle & 0x01) && xx > 0 && xx < (8 + (4 * tftRusFont))) bi = bi | ((byp >> yy) & 0x01);
            for (int sy = 0; sy < tftRusSize; sy++) {
              for (int sx = 0; sx < tftRusSize; sx++) {

                if (bi) {
                  if (tftRusColor >= 0) tft.drawPixel(x + (i * tftRusSize * tftRusWidth) + (xx * tftRusSize) + sx + cd, y + (yb * tftRusSize * 8) + (yy * tftRusSize) + sy, tftRusColor);
                } else {
                  if (tftRusBack >= 0) tft.drawPixel(x + (i * tftRusSize * tftRusWidth) + (xx * tftRusSize) + sx + cd, y + (yb * tftRusSize * 8) + (yy * tftRusSize) + sy, tftRusBack);
                }

              }
              if (tftRusStyle > 3) {
                if (curcount) {
                  curcount--;
                } else {
                  curcount = tftRusCursiveLevel - 1;
                  cd--;
                }
              }
            }
          }
        }
      }
    }
  }
}

// ============================================================================
int tftRusLength(String text) {
// ============================================================================
  int textsize = 0;
  for (int i = 0; i < text.length(); i++) if (char(text[i]) < 208) textsize++;
  return (int) textsize;
}

// ============================================================================
int tftRusTextWidth(String text) {
// ============================================================================
  return (int) (tftRusLength(text) * tftRusWidth * tftRusSize) + (trunc(tftRusStyle / 4) * tftRusCursiveLevel * tftRusSize);
}

// ============================================================================
const uint8_t broadcastOCBands[] = {
  BAND_120M, BAND_90M, BAND_75M, BAND_49M, BAND_41M,
  BAND_31M, BAND_25M, BAND_22M, BAND_19M, BAND_17M,
  BAND_15M, BAND_13M, BAND_11M
};

const char *broadcastOCNames[] = {
  "OC1", "OC2", "OC3", "OC4", "OC5",
  "OC6", "OC7", "OC8", "OC9", "OC10",
  "OC11", "OC12", "OC13"
};

const uint8_t broadcastOCCount = sizeof(broadcastOCBands) / sizeof(broadcastOCBands[0]);

// ============================================================================
void drawBroadcastOCButton(int16_t bx, int16_t by, const char *label, bool backButton) {
  uint16_t fill = backButton ? TFT_DARKGREEN : TFT_NAVY;
  uint16_t border = backButton ? TFT_GREEN : TFT_CYAN;
  tft.fillRoundRect(bx + 2, by + 2, 76, 36, 5, fill);
  tft.drawRoundRect(bx + 2, by + 2, 76, 36, 5, border);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, fill);
  tft.drawString(label, bx + 40, by + 20, 2);
}

// ============================================================================
void drawBroadcastOCMenu() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("BANDES RADIODIFFUSION OC", screenV ? 120 : 160, 28, 2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Choisir une bande", screenV ? 120 : 160, 52, 2);

  uint8_t columns = screenV ? 3 : 4;
  uint8_t total = broadcastOCCount + 1; // + BACK
  for (uint8_t i = 0; i < total; i++) {
    uint8_t col = i % columns;
    uint8_t row = i / columns;
    int16_t bx = col * 80;
    int16_t by = 80 + row * 40;
    if (i < broadcastOCCount) drawBroadcastOCButton(bx, by, broadcastOCNames[i], false);
    else drawBroadcastOCButton(bx, by, "BACK", true);
  }
  x = y = 0;
}

// ============================================================================
int jamBroadcastOCMenu() {
  if (x > 20 && x < (screenV ? 220 : 300) && y > 10 && y < 65) {
    x = y = 0;
    Beep(1, 0);
    return -2;
  }

  if (y < 80) return -1;
  uint8_t columns = screenV ? 3 : 4;
  uint8_t col = x / 80;
  uint8_t row = (y - 80) / 40;
  if (col >= columns) return -1;
  int16_t item = row * columns + col;
  uint8_t total = broadcastOCCount + 1;
  if (item < 0 || item >= total) return -1;

  int16_t bx = col * 80;
  int16_t by = 80 + row * 40;
  if (x < bx || x >= bx + 80 || y < by || y >= by + 40) return -1;

  Beep(1, 0);
  x = y = 0;
  if (item == broadcastOCCount) return -2;
  return broadcastOCBands[item];
}

// ============================================================================
void drawButtons(uint8_t lay) {
// ============================================================================
  for (int n = 0 ; n <= lastBut; n++) if (but[n].layout == lay) drawBut(n, B_NORMAL, "");
}

// ============================================================================
void drawButton(uint8_t lay, uint8_t num, uint8_t state) {
// ============================================================================
  int n = 0;
  bool flag = false;
  while (n <= lastBut && !flag) {
    if (but[n].layout == lay && but[n].num == num) {
      flag = true;
      drawBut(n, state, "");
    }
    n++;
  }
}

// ============================================================================
void drawButton(uint8_t lay, uint8_t num, uint8_t state, String altText) {
// ============================================================================
  int n = 0;
  bool flag = false;
  while (n <= lastBut && !flag) {
    if (but[n].layout == lay && but[n].num == num) {
      flag = true;
      drawBut(n, state, altText);
    }
    n++;
  }
}

// ============================================================================
int jamButton(uint8_t lay) {
// ============================================================================
  int res = -1;
  int n = 0;
  bool flag = false;
  while (n <= lastBut && !flag) {
    if (but[n].layout == lay) {
      if (screenV) {
        if (x >= (but[n].xPosV) && x < (but[n].xPosV + But_Width) && y >= (but[n].yPosV) && y < (but[n].yPosV + But_Height)) flag = true;
      } else {
        if (x >= (but[n].xPosH) && x < (but[n].xPosH + But_Width) && y >= (but[n].yPosH) && y < (but[n].yPosH + But_Height)) flag = true;
      }
      if (flag && but[n].Name != "" && but[n].Name != " ") {
        res = but[n].num;
        if (!butBlock[n]) {
          drawBut(n, B_JAM, "");
          delay(200);
          Beep(1, 0);
          x = y = 0;
        }
      }
    }
    n++;
  }
  return res;
}

// ============================================================================
void drawBut(uint8_t id, uint8_t state, String alt) {
// ============================================================================
  spr.createSprite(But_Width, But_Height);
  spr.fillScreen(COLOR_BACKGROUND);
  spr.setTextSize(2);
  spr.setTextDatum(BC_DATUM);
  spr.setTextPadding(0);
  if (alt == "") alt = but[id].Name;
  int type = but[id].type;
  if (alt == "") state = B_BLOCK;
  if (state == B_BLOCK) butBlock[id] = true; else butBlock[id] = false;
  if (state == B_BLOCK) spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_block);
  else
  if (state == B_NORMAL) {
    if (type == B_BLUE)  spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_);
    if (type == B_GREEN) spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_page);
    if (type == B_GOLD)  spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_retro);
  } else
  if (type == B_GOLD) spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_fix);
  else
  if (state == B_JAM) spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_jam);
  else spr.pushImage(0, 0, But_Width, But_Height, (uint16_t *)But_select);
  if (state == B_BLOCK) spr.setTextColor(TFT_SILVER);
  else
  if (state == B_JAM) spr.setTextColor(TFT_WHITE);
  else
  if (state == B_SELECT) spr.setTextColor(TFT_YELLOW);
  else
  if (type == B_GOLD) spr.setTextColor(TFT_WHITE);
  else spr.setTextColor(TFT_CYAN);
  spr.drawString(alt, (But_Width / 2) + 3, (But_Height / 2) + 10);
  if (state != B_JAM && state != B_SELECT) {
    if (state == B_BLOCK || type != B_GOLD) spr.setTextColor(TFT_BLACK); else spr.setTextColor(TFT_OLIVE);
    spr.drawString(alt, (But_Width / 2) + 2, (But_Height / 2) + 9);
  }
  if (screenV) spr.pushSprite(but[id].xPosV, but[id].yPosV); else  spr.pushSprite(but[id].xPosH, but[id].yPosH);
  spr.deleteSprite();
}

// ============================================================================
void screenRotate() {
// ============================================================================
  if (screenV) {
    tft.setRotation(0);
    tft.setTouch(calDataV);
  } else {
    tft.setRotation(1);
    tft.setTouch(calDataH);
  }
}

// ============================================================================
