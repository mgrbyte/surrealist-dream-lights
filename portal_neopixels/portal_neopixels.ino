#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define TECTHULU_POLL_INTERVAL_MILLIS 5000
#define TECTHULU_HOSTNAME "some.host.name"
#define TECTHULU_PORT 8080
#define TECTHULU_STATUS_ENDPOINT "/some/endpoint/that/you/use"

#define HANDLE_RESOS 0

#define STRIP_LEFT 0
#define STRIP_RIGHT 1

// might want to do something different depending which side of the
// clock house we're running on
#define STRIP_SIDE STRIP_LEFT

// constants that are useful in the portal status JSON response
#define NEUTRAL "Neutral"
#define ENL "Enlightened"
#define RES "Resistance"

#define DATA_PIN D6

// Pixels
#define NUM_PIXELS 60 // 30 x 2 when we've connected 2 strips
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, DATA_PIN, NEO_GRBW + NEO_KHZ800);

/* RGB color defs from NIA css */
/* uint32_t enl_color = strip.Color(40, 244, 40, 200); */
/* uint32_t res_color = strip.Color(0, 194, 255, 200); */
/* uint32_t neu_color = strip.Color(249, 249, 249, 200); */

// WIFI
const char* ssid = "put-your-access-point-name-here";
const char* passwd = "...etc"; 

// Tecthulhu
HTTPClient http;
const size_t portal_status_capacity = JSON_ARRAY_SIZE(4) + JSON_ARRAY_SIZE(8) + 13*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(10) + 2200;

void setup_wifi() {
	WiFi.begin(ssid, passwd);
	Serial.print("Connecting to WiFi ....");
	int i = 0;
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(1000);
		i++;
	}
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(WiFi.SSID());
	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());
}


JsonObject& fetch_portal_status() {
	int response_code;
	String uri = TECTHULU_STATUS_ENDPOINT;
	http.begin(TECTHULU_HOSTNAME, TECTHULU_PORT, uri);
	response_code = http.GET();
	String response_text = http.getString();
	if (response_code == 200) {
		DynamicJsonBuffer jsonBuffer(portal_status_capacity);
		JsonObject& root = jsonBuffer.parseObject(response_text);
		http.end();
		if (root.success()) {
			return root["result"];
		}
		Serial.println("Parsing JSON object failed for " + uri);
	}
	Serial.println("Did not get 200 response from " + uri);
	Serial.print("Response code was: ");
	Serial.println(response_code);
	Serial.println("response text was:");
	Serial.println(response_text);
	return JsonObject::invalid();
}

uint16_t portal_level_to_brightness(int level) {
	int b_level = level == 0 ? 0 : 31 * level;
	return (uint16_t)b_level;
}

uint16_t portal_health_to_brighness(int health) {
	return (uint16_t)(((255 / 100) * health) / 2);
}

void handle_portal_title(String title) {
	Serial.println("Portal title is:" + title);
}

void set_portal_color(uint32_t c) {
	for (uint16_t i=0; i<strip.numPixels(); i++) {
		strip.setPixelColor(i, c);
	}
	strip.show();
}

// TODO: adjust brightness by health, but keeping faction color (not too white)
//       adjust vividness of color by portal level
void handle_portal_ownership(String faction, int level, int health) {
	Serial.print("Current portal level is: ");
	Serial.println(level);
	// uint16_t strength = portal_level_to_brightness(level);
	uint16_t white = portal_health_to_brighness(health);
	uint32_t color;
	if (faction == ENL) {
		color = strip.Color(40, 244, 40, white);
		Serial.print("Portal is controlled by the ");
		Serial.println(ENL);
	} else if (faction == RES) {
		color = strip.Color(0, 194, 255, white);
		Serial.print("Portal is controlled by the ");
		Serial.println(RES);
	} else {
		color = strip.Color(249, 249, 249, white);
		Serial.print("Portal is " );
		Serial.println(NEUTRAL);
	}
	set_portal_color(color);
}

void handle_resos(JsonArray& resos) {
	Serial.println("Handling resonators individually not implemented");
}

void handle_portal_status(JsonObject& status) {
	int level = status["level"].as<int>();
	int health = status["health"].as<int>();
	const char* faction = status["controllingFaction"].as<char*>();
	const char* title = status["title"].as<char*>();
	handle_portal_ownership(faction, level, health);
	handle_portal_title(title);
	if (HANDLE_RESOS != 0) {
		handle_resos(status["resonators"]);
	}
}

void setup_neopixels() {
	strip.begin();
	strip.show();
}

void setup_serial() {
	Serial.begin(115200);
	delay(10);
}

void setup() {
	setup_serial();
	setup_wifi();
	setup_neopixels();
}

void loop () {
	JsonObject& portal_status = fetch_portal_status();
	handle_portal_status(portal_status);
	delay(TECTHULU_POLL_INTERVAL_MILLIS);
}
