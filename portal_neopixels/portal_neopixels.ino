#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#define NUM_PIXELS 60

ESP8266WebServer ws;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS,  // num leds on strip
																						D6, // data pin
																						NEO_GRB + NEO_KHZ800);

char* ssid = "SKY54B3D";
char* passwd = "xxxxx";

void handleRoot() {
	ws.send(200, "text/plain", "Surrealist Dreams!");
}

void colorChase(uint32_t c) {
	for (uint16_t i=0; i<strip.numPixels() + 4; i++) {
		strip.setPixelColor(i, c);
		strip.setPixelColor(i - 4, 0);
		strip.show();
		delay(25);
	}
}

void handleTurnStripOff() {
	setAllColorTo(0);
	ws.send(200, "text/plain", "Lights out!");
}

void setAllColorTo(uint32_t c) {
	for (uint16_t i=0; i<strip.numPixels(); i++) {
		strip.setPixelColor(i, c);
		strip.show();
		delay(20);
	}
	strip.show();
}

void handleSetFaction() {
	String fac = ws.arg("fac");
	String facName;
	if (fac == "") {
		setAllColorTo(0);
		facName = "neutral";
	} else if (fac == "enl") {
		setAllColorTo(strip.Color(0, 240, 0));
		facName = "enl";
	} else {
	  setAllColorTo(strip.Color(0, 0, 240));
		facName = "res";
	}
	strip.show();
	ws.send(200, "text/plain", facName + " portal");
}

void performSafeColorChase() {
	colorChase(strip.Color(255, 0, 0));
	colorChase(strip.Color(0, 255, 0));
	colorChase(strip.Color(0, 0, 255));
	strip.show();
}

void setup() {
	WiFi.begin(ssid, passwd);
	Serial.begin(115200);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(100);
	}
	Serial.println("");
	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());
	strip.begin();
	strip.show();

	ws.on("/", handleRoot);
	ws.on("/faction", handleSetFaction);
	ws.on("/strip/chase", performSafeColorChase);
	ws.on("/strip/off", handleTurnStripOff);
	ws.begin();
}

void loop () {
	ws.handleClient();
}
