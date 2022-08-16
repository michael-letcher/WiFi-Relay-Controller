#include <ESP8266WebServer.h>
#include <FS.h>

// Web server to serve our app
ESP8266WebServer server(80);

char ssid[] = "";            // SSID of your network
char pass[] = "";            // password of your WPA Network
int status = WL_IDLE_STATUS; // the Wifi radio's status

// another define bunch similar to the uno code
#define D(...) Serial.println(__VA_ARGS__);
// #define D(...)

const long serial_baud = 115200;

void setup()
{
  Serial.begin(serial_baud);
  D("Starting")
  SPIFFS.begin();

  setupWifi();

  server.on("/relay", sendRelay);
  server.onNotFound(searchFileSystem);
  server.begin();
}

void loop()
{
  server.handleClient();
}

void setupWifi()
{
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED)
  {
    Serial.print(".");
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println();
  // you're connected:
  Serial.println("You're connected to the network");
  // print your WiFi shield's IP address:

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.print(ip);

  printMacAddress();
}

void printMacAddress()
{
  byte mac[6];

  WiFi.macAddress(mac);

  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
}

void sendRelay()
{
  const String relayString = server.arg("relay");
  const String modeString = server.arg("mode");

  if (relayString == "")
  {
    server.send(404, "text/plain", "No relay arg");
    return;
  }

  if (modeString == "")
  {
    server.send(404, "text/plain", "No mode arg");
    return;
  }

  // we have a relay and a mode; let's work on them
  short relay = relayString.toInt();

  if (relay < 0 || relay > 3)
  {
    server.send(400, "text/plain", "Invalid relay");
    return;
  }

  char mode = modeString.equals("activate") ? 'a' : 'd';

  // send this to the UNO
  Serial.write('>');
  Serial.write('0' + relay);
  Serial.write(mode);

  // we send a response to the web-app
  server.send(200, "text/plain", "OK");
}

// ---------------------------------------------------------------------------
void searchFileSystem()
{
  // server.uri() is the param accepted; ie:
  //     http://10.20.30.40/somefile.txt - uri is /somefile.txt
  //  we will put it into a string for the string utility functions

  D("Calling filesystem with")
  D(server.uri())
  String filepath = server.uri();

  if (filepath.endsWith("/")) // is this a folder?
  {
    filepath += "index.html"; // index page of the folder being accessed
  }

  if (SPIFFS.exists(filepath))
  {
    String contentType = getFileContentType(filepath);

    File fp = SPIFFS.open(filepath, "r");
    server.client().setNoDelay(true);

    server.streamFile(fp, contentType);
  }
  else
  {
    Serial.printf("%s was not found, plaintext response", filepath.c_str());
    server.send(404, "text/plain", filepath + " File not found, have you uploaded data to the ESP correctly?");
  }
}

String getFileContentType(String &filepath)
{

#define FILE_MATCH(fp, ret)  \
  if (filepath.endsWith(fp)) \
    return ret;

  // got the following from:
  // https://stackoverflow.com/questions/23714383/what-are-all-the-possible-values-for-http-content-type-header
  FILE_MATCH(".html", "text/html")
  FILE_MATCH(".txt", "text/plain")
  FILE_MATCH(".css", "text/css")
  FILE_MATCH(".js", "application/javascript")

  // the following is provided as a "just in case" -
  // it doesn't hurt to include them, but we haven't used them in this project (yet)
  FILE_MATCH(".mp4", "audio/mpeg")
  FILE_MATCH(".jpg", "image/jpeg")
  FILE_MATCH(".png", "image/png")

  // at the very least just return something
  return "text/plain";
}

// -- Macro definitions -----------------

// --------------------------------------
