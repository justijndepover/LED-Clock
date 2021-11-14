#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <time.h>
#include "DHT.h"

// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

Max72xxPanel matrix = Max72xxPanel(D4, 12, 1); // pinCS, # horizontal displays, #vertical displays
ESP8266WebServer server(80);

int wait = 70; // In milliseconds
int spacer = 1;
int width = 5 + spacer; // The font width is 5 pixels
char time_value[20];
DHT dht(D3, DHT22);

void setup() {
    // set wifi
    WiFi.begin("SSID","PASSWORD");

    matrix.setIntensity(7); // Use a value between 0 and 15 for brightness
    matrix.setRotation(0, 1); // turn screen
    matrix.setRotation(1, 1);
    matrix.setRotation(2, 1);
    matrix.setRotation(3, 1);
    matrix.setRotation(4, 1);
    matrix.setRotation(5, 1);
    matrix.setRotation(6, 1);
    matrix.setRotation(7, 1);
    matrix.setRotation(8, 1);
    matrix.setRotation(9, 1);
    matrix.setRotation(10, 1);
    matrix.setRotation(11, 1);
    matrix.fillScreen(LOW);
    matrix.write();

    while ( WiFi.status() != WL_CONNECTED ) {
      matrix.drawChar(34,0, 'W', HIGH,LOW,1); // H
      matrix.drawChar(40,0, 'I', HIGH,LOW,1); // HH  
      matrix.drawChar(46,0,'-', HIGH,LOW,1); // HH:
      matrix.drawChar(52,0,'F', HIGH,LOW,1); // HH:M
      matrix.drawChar(58,0,'I', HIGH,LOW,1); // HH:MM
      matrix.write(); // Send bitmap to display
      delay(250);
      matrix.fillScreen(LOW);
      matrix.write();
      delay(250);
    }

    // set time
    configTime(0 * 3600, 0, "pool.ntp.org");
    setenv("TZ", "GMT-1BST",1);

    dht.begin();

    server.on("/message", handleMessage);
    server.onNotFound(handleNotFound);
    server.begin();

    String ip = WiFi.localIP().toString();
    showText(ip);
    delay(5000);
}

void loop() {
  server.handleClient();
  showClockAndTemperature();
}

void showClockAndTemperature() {
    time_t now = time(nullptr);
    String time = String(ctime(&now));
    time.trim();
    time = time.substring(11,16);

    String temp = (String) (int) dht.readTemperature();
    String humidity = (String) (int) dht.readHumidity();

    String display = time + " - " + temp + "C " + humidity + "%";
    display.toCharArray(time_value, 20);
    showText(display);    
}

void handleMessage() {
    if (! server.hasArg("message") || ! server.hasArg("duration")) {
        server.send(420, "application/json", "{error:\"message and duration are required\"}");
        return;
    }

    server.send(200, "application/json", "{message:\"success\"}");

    if (server.hasArg("type") && server.arg("type") == "scroll") {
        scrollText(server.arg("message"));
    } else {
        showText(server.arg("message"));
        delay(server.arg("duration").toInt());
    }
}

void handleNotFound() {
    server.send(404, "application/json", "{error:\"Not found\"}");
}

void showText(String message) {
    int offset = 2;
    matrix.fillScreen(LOW);
    for (int i = 0; i < message.length(); i++) {
        matrix.drawChar(offset, 0, message[i], HIGH, LOW, 1);
        offset = offset + 6;
    }
    matrix.write();
}

void scrollText(String message){
    for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
        matrix.fillScreen(LOW);
        int letter = i / width;
        int x = (matrix.width() - 1) - i % width;
        int y = (matrix.height() - 8) / 2; // center the text vertically
        while ( x + width - spacer >= 0 && letter >= 0 ) {
            if ( letter < message.length() ) {
                matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
            }
            letter--;
            x -= width;
        }
        matrix.write(); // Send bitmap to display
        delay(wait/2);
    }
}
