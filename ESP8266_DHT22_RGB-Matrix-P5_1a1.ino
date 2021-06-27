// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - RGB matrix Panel Library: https://github.com/2dom/PxMatrix
// - Adafruit_GFX Library: https://github.com/adafruit/Adafruit-GFX-Library
// - esp8266 library (nodemcu) found at https://github.com/esp8266/Arduino
// - package_esp8266com_index.json found at http://arduino.esp8266.com/stable/package_esp8266com_index.json
// Find All "Great Projects" Videos : https://www.youtube.com/channel/UCCC8DuqicBtP3A_aC53HYDQ/videos


#include "DHT.h"
#define DHTPIN 0 
#define DHTTYPE DHT22   // DHT 22
DHT dht(DHTPIN, DHTTYPE);
#include <ESP8266WiFi.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <FreeMonoBold12pt7b.h>
#include <kongtext4pt7b.h>
#include <PxMatrix.h>
#include <Ticker.h>
Ticker display_ticker;
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_OE 2
// Pins for LED MATRIX
#define matrix_width 64
#define matrix_height 32
int timezone = 3;
int dst = 0;
  float Humi;
  float Temp;
 static uint32_t lastTime = 0; // millis() memory
static bool flasher = false;  // seconds passing flasher
uint8_t frameDelay = 10;  // default frame delay value

int h,m,s,d;
uint8_t dow;
int  day;
uint8_t month;
String  year;
String date;
String WeatherT;
String WeatherH;
String Weather;
String  text;
#define MAX_MESG  9
#define MAX_MES  20
#define MAX_ME  50
#define BUF_SIZE  612
//char curMessage[BUF_SIZE] = { "Hello! Enter new message?" };
char curMessage[BUF_SIZE] = { "niq_ro test P5 display" };
char newMessage[BUF_SIZE];
bool newMessageAvailable = false;
static bool Mode = true; 
uint8_t r=0, g=0, b=0;
unsigned int NewRTCh = 24;
unsigned int NewRTCm = 60;
unsigned int NewRTCs = 10;
char szTime[4];    // 00
char szMesg[10] = "";
char  szBuf[10];
  
const char* ssid     = "niq_ro";     // SSID of local network
const char* password = "cold3beers";   // Password on network
WiFiServer server(80);

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time=10; //10-50 is usually fine

//PxMATRIX display(matrix_width,matrix_height,P_LAT, P_OE,P_A,P_B,P_C);
PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
//PxMATRIX display(64,64,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);

byte fakeDHT = 1;  // 0 = real DHT

int cin = 30; //color intensity
int cinmin = 30;  // night
int cinmax = 255; // day
unsigned long tpschimbare = 0;
int cin0 = 51;

int vit = 140;
int vitmin = 100;
int vitmax = 150;

// Some standard colors
uint16_t myRED = display.color565(cin, 0, 0);
uint16_t myGREEN = display.color565(0, cin, 0);
uint16_t myBLUE = display.color565(0, 0, cin);
uint16_t myWHITE = display.color565(cin, cin, cin);
uint16_t myYELLOW = display.color565(cin, cin, 0);
uint16_t myCYAN = display.color565(0, cin, cin);
uint16_t myMAGENTA = display.color565(cin, 0, cin);
uint16_t myBLACK = display.color565(cin, cin, cin);

uint16_t myCOLORS[8]={myRED,myGREEN,myBLUE,myWHITE,myYELLOW,myCYAN,myMAGENTA,myBLACK};


#ifdef ESP8266
// ISR for display refresh
void display_updater()
{
  display.display(display_draw_time);
}
#endif



void display_update_enable(bool is_enable)
{

#ifdef ESP8266
  if (is_enable)
    display_ticker.attach(0.002, display_updater);
  else
    display_ticker.detach();
#endif

}

void(* resetFunc) (void) = 0;//declare reset function at address 0
char *mon2str(uint8_t mon, char *psz, uint8_t len)
{
  
  static const char str[][4] PROGMEM =
  {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  /*
  static const char str[][4] PROGMEM =
  {
    "Ian", "Feb", "Mar", "Apr", "Mai", "Iun",
    "Iul", "Aug", "Sep", "Oct", "Noi", "Dec"
  };
  */
  *psz = '\0';
  mon--;
  if (mon < 12)
  {
    strncpy_P(psz, str[mon], len);
    psz[len] = '\0';
  }

  return(psz);
}
char *dow2str(uint8_t code, char *psz, uint8_t len)
{
  
  static const char str[][10] PROGMEM =
  {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };
  /*
  static const char str[][10] PROGMEM =
  {
    "Duminica", "Luni", "Marti", "Miercuri",
    "Joi", "Vineri", "Sambata"
  };
  */
  
  *psz = '\0';
  code--;
  if (code < 7)
  {
    strncpy_P(psz, str[code], len);
    psz[len] = '\0';
  }

  return(psz);
}

void getDateWeb()
{
  date = dow2str(dow, szBuf, sizeof(szBuf)-1);
  date += " ";
  if (day < 10) { date += "0";  }
  date += day;
  date += mon2str(month, szBuf, sizeof(szBuf)-1);
  date += year;
  date += "   ";
  if (h < 10) { date += "0";  }
  date += h;
  date += ":";
  if (m < 10) { date += "0";  }
  date += m;
  date += ":";
  if (s < 10) { date += "0";  }
  date += s;
}

void getWeather()
{
  if (fakeDHT == 0)
  {
  dht.begin();
  Temp = dht.readTemperature();
  Humi = dht.readHumidity();
  if (isnan(Humi) || isnan(Temp)) {
      delay(100);
    return;
  }
  }
  else
  {
  Temp = random(-2000,4000)/100.;
  Humi = random(5000,7000)/100.;
  }
 // WeatherT = "Temperature: ";
  WeatherT = "";
  WeatherT += Temp;
  WeatherT += "$";  // [°C]
 // WeatherH = "Humidity: ";
  WeatherH = "";
  WeatherH += Humi;
  WeatherH += "%";

  //Weather = "Temperature: ";
  Weather = "";
  Weather += Temp;
  Weather += "$C";  // [°C]
  //Weather += "     Humidity: ";
  Weather += "        ";
  Weather += Humi;
  Weather += "%";
}

void getDate()
// Code for reading date
{    
  text = mon2str(month, szBuf, sizeof(szBuf)-1);
        display.setCursor(0,0);
  display.fillRect(0, 0, 64, 8, display.color565(0, 0, 0));
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myRED);
  if (day < 10) { display.print("0");  }
  display.print(day);
  display.setTextColor(myGREEN);
  display.print(text);
  display.setTextColor(myRED);
  display.print(year);
  display.setFont();
}

void getDowe()
// Code for reading date
{
  text = dow2str(dow, szBuf, sizeof(szBuf)-1);
  uint16_t text_length = text.length();
  int xpos = (matrix_width - text_length*7)/2;
  display.setCursor(xpos,0);
  display.fillRect(0, 0, 64, 8, display.color565(0, 0, 0));
  display.setFont(&kongtext4pt7b);
  uint8_t y = 0;
  for (y=0; y<10; y++) {
    display.setTextColor(Whel(y));
    display.print(text[y]);
    }
  display.setFont();
}  

void getRTCh(char *psz)
// Code for reading clock time
{
  sprintf(psz, "%02d", h);
  display.setCursor(0, 16); 
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(myMAGENTA);
  display.fillRect(0, 8, 24, 15, display.color565(0, 0, 0));
  display.print(szTime);
  display.setFont();
  NewRTCh=h;
}

void getRTCm(char *psz)
// Code for reading clock time
{
  sprintf(psz, "%02d", m);
  display.setCursor(26, 16); 
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(myMAGENTA);
  display.fillRect(26, 8, 25, 15, display.color565(0, 0, 0));
  display.print(szTime);
  display.setFont();
  NewRTCm=m;
}

void getTim(char *psz, bool f = true)
// Code for reading clock time
{
  
  if (NewRTCs != s/10) 
  {
  display.setCursor(20, 8); 
  display.setTextSize(2);
  display.setTextColor(myCOLORS[g]);
  display.fillRect(24, 12, 2, 6, display.color565(0, 0, 0));
    display.print(f ? ':' : ' ');
  display.setCursor(54, 10); 
  display.setTextSize(1);
  display.fillRect(54, 10, 10, 6, display.color565(0, 0, 0));
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myCOLORS[b]);
    display.print(f ? ' ' : '*');
  display.setFont();
  display.setCursor(51, 16); 
  display.setTextSize(1);
  display.setTextColor(myCOLORS[r]);
  sprintf(psz, "%02d", s);
  display.fillRect(51, 17, 13, 6, display.color565(0, 0, 0));
  display.setFont(&kongtext4pt7b);
    display.print(szTime);
  display.setFont();
      NewRTCs=s/10;
    }
    else
  {
  display.setCursor(20, 8); 
  display.setTextSize(2);
  display.setTextColor(myCOLORS[g]);
  display.fillRect(24, 12, 2, 6, display.color565(0, 0, 0));
    display.print(f ? ':' : ' ');
  display.setCursor(54, 10); 
  display.setTextSize(1);
  display.fillRect(54, 10, 10, 6, display.color565(0, 0, 0));
  display.setFont(&kongtext4pt7b);
  display.setTextColor(myCOLORS[b]);
    display.print(f ? ' ' : '*');
  display.setFont();
  display.setCursor(51, 16); 
  display.setTextColor(myCOLORS[r]);
  sprintf(psz, "%02d", s);
  display.fillRect(58, 17, 6, 6, display.color565(0, 0, 0));
  display.setFont(&kongtext4pt7b);
    display.print(szTime);
  display.setFont();
    }
}

void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text)
  {  
    handleWiFi();
  //  display.setFont(&kongtext4pt7b);
    uint16_t text_length = text.length();
    // Asuming 5 pixel average character width
    for (int xpos=matrix_width; xpos>-(matrix_width+text_length*5); xpos--)
    {
  handleWiFi();
      
      display.fillRect(0, 23, 64, 8, display.color565(0, 0, 0));
      display.setCursor(xpos,ypos);
      display.setFont(&kongtext4pt7b);
      display.print(text);
      display.setFont();
      //display.setFont(&kongtext4pt7b);
     // delay(scroll_delay);
    //  yield();
      
  if (millis() - lastTime >= 1000)
  {
    handleWiFi();
    lastTime = millis();
    updateTime();
    getTim(szTime, flasher);
    flasher = !flasher;
    if (NewRTCh != h) 
   {
        getTime();
        getRTCh(szTime);
   }
    if (NewRTCm != m) 
   {
      getRTCm(szTime);
      getWeather();
      Mode = true;
   }
    }
     delay(scroll_delay);
     yield();
   //  delay(scroll_delay);
}
      r++;
      if(r == 8) {
        r = 0;
        g++;
        if(g == 8) {
          g = 0;
          b++;
        if(b == 8) {
          b = 0;
        }
        }
      }

}

const char WebResponse[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";

char WebPage[] =
"<!DOCTYPE html>" \
"<html>" \
"<head>" \
"<meta http-equiv=\"refresh\" content=\"60\"; url=\"http://192.168.1.24\" />" \
"<title>ESP8266 Weather Station</title>" \
"<script>" \
"strLine = \"\";" \
"function DisplayCurrentTime()" \
"{" \
"  var dt = new Date();" \
"  var weekday = new Array(7);" \
"  weekday[0] = \"Sunday\";" \
"  weekday[1] = \"Monday\";" \
"  weekday[2] = \"Tuesday\";" \
"  weekday[3] = \"Wednesday\";" \
"  weekday[4] = \"Thursday\";" \
"  weekday[5] = \"Friday\";" \
"  weekday[6] = \"Saturday\";" \
"  var dow = weekday[dt.getDay()];" \
"  document.getElementById(\"datetime\").innerHTML = (dow) +\" \"+ (dt.toLocaleString());" \
"  setTimeout('DisplayCurrentTime()', 1000);" \
"}" \
"function SendData()" \
"{" \
"  nocache = \"/&nocache=\" + Math.random() * 1000000;" \
"  var request = new XMLHttpRequest();" \
"  strLine = \"&MSG=\" + document.getElementById(\"data_form\").Message.value;" \
"  strLine = strLine + \"/&SP=\" + document.getElementById(\"data_form\").Speed.value;" \
"  request.open(\"GET\", strLine + nocache, false);" \
"  request.send(null);" \
"}" \
"function restart()" \
"{" \
"  nocache = \"/&nocache=\" + Math.random() * 1000000;" \
"  var request = new XMLHttpRequest();" \
"  strLine = \"/&Rst=\" + document.getElementById(\"date_form\").rerset.value;" \
"  request.open(\"GET\", strLine + nocache, false);" \
"  request.send(null);" \
"}" \
"function resttime()" \
"{" \
"  nocache = \"/&nocache=\";" \
"  var request = new XMLHttpRequest();" \
"  strLine = \"/&RT=\" + document.getElementById(\"date_form\").restmie.value;" \
"  request.open(\"GET\", strLine + nocache, false);" \
"  request.send(null);" \
"}" \
      "document.addEventListener('DOMContentLoaded', function() {" \
        "DisplayCurrentTime();" \
      "}, false);" \
"</script>" \
      "<style>" \
         "body {" \
        "text-align: center;" \
             "margin: 5;" \
             "padding: 5;" \
             "background-color: rgba(72,72,72,0.4);" \
         "}" \
         "#wrapper { " \
     "margin: 0 auto;" \ 
     "width: 100%;" \
     "}" \
         "#form-div {" \
         "background-color: rgba(72,72,72,0.4);" \
         "padding-left: 10px;" \
         "padding-right: 10px;" \
         "padding-bottom: 80px;" \
         "padding-top: 5px;" \
         "-moz-border-radius: 7px;" \
         "-webkit-border-radius: 7px;" \
         "}" \
         ".data-input {" \
         "padding: 10px 5px 5px 5px;" \
         "background-color: #bbbbff;" \
         "font-size:26px;" \
         "color:red;" \
         "padding-bottom:46px;" \
         "border: 5px solid #444444;" \
         "-moz-border-radius: 7px;" \
         "-webkit-border-radius: 7px;" \
         "}" \
         ".button-blue {" \
         "padding: 5px 5px 5px 5px;" \
         "float:left;" \
         "width: 100%;" \
         "border: #fbfb00 solid 3px;" \
         "cursor:pointer;" \
         "background-color: #4444ff;" \
         "color:white;" \
         "font-size:20px;" \
         "padding-bottom:5px;" \
         "font-weight:700;" \
         "-moz-border-radius: 7px;" \
         "-webkit-border-radius: 7px;" \
         "}" \
         ".button-blue:hover {" \
         "background-color: #2222aa;" \
         "color: #ff93bd;" \
         "}" \
         ".text {" \
         "background-color: #ff0000;" \
         "font-size:76px;" \
         "color: #ffff99;" \
         "}" \
      "table {" \
        "border: 2px solid #ff00ff;" \
        "background-color: #ffffff;" \
        "width:100%;" \
         "color: #0000ff;" \
         "-moz-border-radius: 7px;" \
         "-webkit-border-radius: 7px;" \
      "}" \
      "h1 {" \
        "color: #ff0000;" \
         "background-color: #ffff00;" \
         "-moz-border-radius: 7px;" \
         "-webkit-border-radius: 7px;" \
      "}" \
      "tr {" \
        "border: 2px solid #ff0000;" \
         "background-color: #ffff00;" \
         "color: #ff0000;" \
         "-moz-border-radius: 7px;" \
         "-webkit-border-radius: 7px;" \
      "}" \
      "td {" \
        "border: 2px solid #ff0000;" \
        "padding: 16px;" \
         "-moz-border-radius: 7px;" \
         "-webkit-border-radius: 7px;" \
      "}" \
   "</style>" \
   "</head>" \
   "<body>" \
   "<div id=\"wrapper\">" \
   "<div id=\"form-div\">" \
   "<div class=\"submit\">" \
   "<h1>ESP8266 Weather Station</h1>" \
   "<h1>Local Date-Time: <span id=\"datetime\"></span></h1>" \
   "</div>" \
  "<form id=\"data_form\" name=\"frmText\">" \
  "<div class=\"data-input\">" \
   "Brighness <input type=\"text\" name=\"Message\" maxlength=\"655\" class=\"button-white\" style=\"width:90%;height:35px;font-size:30px\">" \
   "<input type=\"range\" name=\"Speed\"min=\"30\" max=\"255\" class=\"button-blue\" style=\"width:76%\">" \
   "<input type=\"submit\" value=\"Send\" style=\"width:20%;float:right\" onclick=\"SendData()\" class=\"button-blue\">" \
   "</div></form></br></br>" \
  "<form id=\"date_form\" name=\"frmText\">" \
   "<div class=\"data-input\">" \
   "<b class=\"button-blue\" style=\"width:8%\">GMT+</b>" \
   "<input type=\"number\" value=\"0\" name=\"restmie\" min=\"-12\" max=\"+12\" class=\"button-blue\" style=\"width:6%\">" \
   "<input type=\"submit\" value=\"Update Time\" onClick=\"resttime()\" class=\"button-blue\" style=\"width:80%\">" \
   "</div><div class=\"data-input\">" \
   "<input type=\"submit\" name = \"rerset\" value=\"Restart ESP8266\" onClick=\"restart()\" class=\"button-blue\">" \
   "</div></form>" \
   "<div class=\"data-input\">" \
    "<table>" \
   "<tr>" \
        "<td style=\"width:48%\"><b>Temp. Celsius</b></td>" \
        "<td style=\"width:48%\"><b>Humidity</b></td>" \
  "</tr>" \
  "<tr class=\"text\">";

uint8_t htoi(char c)
{
  c = toupper(c);
  if ((c >= '0') && (c <= '9')) return(c - '0');
  if ((c >= 'A') && (c <= 'F')) return(c - 'A' + 0xa);
  return(0);
}

void getData(char *szMesg, uint8_t len)
// Message may contain data for:
// New text (/&MSG=)
// Invert (/&I=)
// Speed (/&SP=)
{
  char *pStart, *pEnd;      // pointer to start and end of text

  // check text message
  pStart = strstr(szMesg, "/&MSG=");
  if (pStart != NULL)
  {
    char *psz = newMessage;

    pStart += 6;  // skip to start of data
    pEnd = strstr(pStart, "/&");

    if (pEnd != NULL)
    {
      while (pStart != pEnd)
      {
        if ((*pStart == '%') && isdigit(*(pStart + 1)))
        {
          // replace %xx hex code with the ASCII character
          char c = 0;
          pStart++;
          c += (htoi(*pStart++) << 4);
          c += htoi(*pStart++);
          *psz++ = c;
        }
        else
          *psz++ = *pStart++;
      }

      *psz = '\0'; // terminate the string
      newMessageAvailable = (strlen(newMessage) != 0);
    }
  }
  // check speed
  pStart = strstr(szMesg, "/&SP=");
  if (pStart != NULL)
  {
    pStart += 5;  // skip to start of data

    int16_t speed = atoi(pStart);
 // int16_t cin = atoi(pStart);
    cin = speed;
  //  Serial.println(cin);
   // frameDelay = speed;
  }
  // check reset time
  pStart = strstr(szMesg, "/&RT=");
  if (pStart != NULL)
  {
    pStart += 5;  // skip to start of data

    timezone = atoi(pStart);
    getTime();
    updateTime();
  }
  // check reset
  pStart = strstr(szMesg, "/&Rst=");
  if (pStart != NULL)
  {
    pStart += 6;  // skip to start of data
    if(*pStart != NULL)
  {
      resetFunc();
  }
  }
}

void handleWiFi(void)
{
  static enum { S_IDLE, S_WAIT_CONN, S_READ, S_EXTRACT, S_RESPONSE, S_DISCONN } state = S_IDLE;
  static char szBuf[1024];
  static uint16_t idxBuf = 0;
  static WiFiClient client;
  static uint32_t timeStart;

  switch (state)
  {
  case S_IDLE:   // initialise
    idxBuf = 0;
    state = S_WAIT_CONN;
    break;

  case S_WAIT_CONN:   // waiting for connection
  {
    client = server.available();
    if (!client) break;
    if (!client.connected()) break;
    timeStart = millis();
    state = S_READ;
  }
  break;

  case S_READ: // get the first line of data

    while (client.available())
    {
      char c = client.read();

      if ((c == '\r') || (c == '\n'))
      {
        szBuf[idxBuf] = '\0';
        client.flush();
        state = S_EXTRACT;
      }
      else
        szBuf[idxBuf++] = (char)c;
    }
    if (millis() - timeStart > 1000)
    {
      state = S_DISCONN;
    }
    break;

  case S_EXTRACT: // extract data
    // Extract the string from the message if there is one
    getData(szBuf, BUF_SIZE);
    state = S_RESPONSE;
    break;

  case S_RESPONSE: // send the response to the client
    // Return the response to the client (web page)
    getDateWeb();
    client.print(WebResponse);
    client.print(WebPage);
    sendXMLFile(client);
    state = S_DISCONN;
    break;

  case S_DISCONN: // disconnect client
    client.flush();
    client.stop();
    state = S_IDLE;
    break;

  default:  state = S_IDLE;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(" ");
  // Define your display layout here, e.g. 1/8 step
  display.begin(16);

  // Define your scan pattern here {LINE, ZIGZAG, ZAGGIZ, WZAGZIG, VZAG} (default is LINE)
  //display.setScanPattern(LINE);

  // Define multiplex implemention here {BINARY, STRAIGHT} (default is BINARY)
  //display.setMuxPattern(BINARY);

  display.setFastUpdate(true);
  display.setRotation(0); // we don't wrap text so it scrolls nicely
  display.clearDisplay();
  display.setTextColor(myCYAN);
  display.setCursor(2,0);
  display.println("Connecting");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.print(".");
  }
  display.setTextWrap(false); 
  display.clearDisplay();
  display.println(""); 
  display.print("WiFi OK"); 
  delay(2000);
  display.clearDisplay();
  IPAddress ip = WiFi.localIP();
  display.setCursor(17,0);
  display.print(ip);
  display.setCursor(-43,10);
  display.print(ip);
  Serial.print("IP Address: ");
  Serial.println(ip);
  display_update_enable(true);
  delay(5000);
  display.clearDisplay();
  // Start the server
  server.begin();
  newMessage[0] = '\0';
  handleWiFi();
  getDate();
  getWeather();
}

void loop() 
{
  handleWiFi();
    if (newMessageAvailable)
    {
      strcpy(curMessage, newMessage);
      newMessageAvailable = false;
    }
    if (Mode)
    {
      getDowe();
  scroll_text(23,vit,Weather);
        getDate();
      Mode = false;
    }
    
  scroll_text(23,vit,curMessage);
  handleWiFi();
  scroll_text(23,vit,Weather);
  
/*  
if (millis() - tpschimbare > 5000)
{
//Serial.println(cin);
tpschimbare = millis();
}
*/

if (cin != cin0)
{
Serial.print("cin = "); 
Serial.println(cin); 
NewRTCh = 24;
NewRTCm = 60;
NewRTCs = 10;
//getTime();
updateTime();
  display.clearDisplay();
  
  // Some standard colors
myRED = display.color565(cin, 0, 0);
myGREEN = display.color565(0, cin, 0);
myBLUE = display.color565(0, 0, cin);
myWHITE = display.color565(cin, cin, cin);
myYELLOW = display.color565(cin, cin, 0);
myCYAN = display.color565(0, cin, cin);
myMAGENTA = display.color565(cin, 0, cin);
myBLACK = display.color565(cin, cin, cin);

myCOLORS[0] = myRED;
myCOLORS[1] = myGREEN;
myCOLORS[2] = myBLUE;
myCOLORS[3] = myWHITE;
myCOLORS[4] = myYELLOW;
myCOLORS[5] = myCYAN;
myCOLORS[6] = myMAGENTA;
myCOLORS[7] = myBLACK;

     getDowe();
//  scroll_text(23,12,WeatherT);
//  scroll_text(23,12,WeatherH);
        getDate();
      Mode = false;

vit = map(cin, cinmin, cinmax, vitmax, vitmin);
Serial.print("vit = "); 
Serial.println(vit);
}
cin0 = cin;
}  // end main loop ?!

  
void getTime()
{

  configTime(timezone * 3600, dst, "pool.ntp.org","time.nist.gov");

  while(!time(nullptr)){
     display.print(".");
  }
}

void updateTime()
{
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
      dow = p_tm->tm_wday+1;
      day = p_tm->tm_mday;
      month = p_tm->tm_mon + 1;
      year = p_tm->tm_year + 1900;
      h = p_tm->tm_hour;
      m = p_tm->tm_min;
      s = p_tm->tm_sec;
}
void sendXMLFile(WiFiClient cl){
  if (fakeDHT == 0)
  {
  dht.begin();
  Temp = dht.readTemperature();
  Humi = dht.readHumidity();
  if (isnan(Humi) || isnan(Temp)) {
      delay(100);
    return;
  }
  }
  else
  {
  Temp = random(20,40);
  Humi = random(50,70);
  }
  // send rest of HTTP header
     cl.print("<td style=\"width:48%\">");
     cl.print(Temp);
     cl.print(" *C</td><td style=\"width:48%\">");
     cl.print(Humi);
     cl.print(" %</td></tr></table></div><br>");
     cl.print("<div class=\"submit\">");
     cl.print("<input type=\"button\" value=\"Actualiser\"");
     cl.print(" onClick=\"javascript:window.location.reload()\" class=\"button-blue\">");
     cl.print("</div><div class=\"submit\">");
     cl.print("<input type=\"button\" value=\"More Progets\" onClick=\"Javascript:window.open('https://www.youtube.com/channel/UCCC8DuqicBtP3A_aC53HYDQ/videos');\"");
     cl.print(" class=\"button-blue\"></div>");
     cl.print("<div class=\"submit\" style=\"width:100%\"><h1>ESP8266 Date-Time: ");
     cl.print(date);
     cl.print("</h1></div>");
     cl.print("</div></div></body></html>");
}
// Input a value 0 to 24 to get a color value.
// The colours are a transition r - g - b - back to r.
uint16_t Wheel(byte WheelPos) {
  if(WheelPos < 2) {
   return display.color565(cin, 0, 0);
  } else if(WheelPos < 5) {
   WheelPos -= 2;
   return display.color565(0, 0, cin);
  } else {
   WheelPos -= 5;
   return display.color565(cin, 0, 0);
  }
}

// Input a value 0 to 24 to get a color value.
// The colours are a transition r - g - b - back to r.
uint16_t Whel(byte WheelPos) {
  if(WheelPos < 1) {
   return display.color565(cin, 0, 0);
  } else if(WheelPos < 2) {
   WheelPos -= 1;
   return display.color565(0, cin, 0);
  } else if(WheelPos < 3) {
   WheelPos -= 2;
   return display.color565(cin, cin, 0);
  } else if(WheelPos < 4) {
   WheelPos -= 3;
   return display.color565(cin, 0, cin);
  } else if(WheelPos < 5) {
   WheelPos -= 4;
   return display.color565(0, 0, cin);
  } else if(WheelPos < 6) {
   WheelPos -= 5;
   return display.color565(0, cin, cin);
  } else if(WheelPos < 7) {
   WheelPos -= 6;
   return display.color565(cin, 0, cin);
  } else if(WheelPos < 8) {
   WheelPos -= 7;
   return display.color565(cin, cin, 0);
  } else if(WheelPos < 9) {
   WheelPos -= 8;
   return display.color565(cin, 0, 0);
  } else {
   WheelPos -= 9;
   return display.color565(cin, cin, cin);
  }
}
