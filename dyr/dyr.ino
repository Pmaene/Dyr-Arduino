//
// Dyr
// Pieter Maene <pieter@maene.eu>
//

// The identifier is the Arduino's IP, where the integer that remains after
// all dots have been removed is converted to hex

#include <Bridge.h>
#include <BridgeClient.h>
#include <BridgeServer.h>
#include <HttpClient.h>
#include <Process.h>

#include "speck.h"

// Constants
unsigned long key[4] = {0x75b7a326, 0x38aed491, 0x735e4aa9, 0x2e83e923};
String name = "dyr";
String host = "0.0.0.0:0" ;
unsigned long identifier = 0;

// Ports
int relay = 12;
int led = 13;

// Main
unsigned long nonce;
BridgeServer server;

void setup() {
  Serial.begin(9600);
  
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);

  digitalWrite(relay, LOW);

  digitalWrite(led, LOW);
  Bridge.begin();
  digitalWrite(led, HIGH);

  initNonce();

  server.noListenOnLocalhost();
  server.begin();
}

void loop() {
  BridgeClient client = server.accept();
  if (client) {
    client.setTimeout(5);
    process(client);
    String data = client.readStringUntil('\n');
    client.stop();
  }

  delay(5);
}

void process(BridgeClient client) {
  String command = client.readStringUntil('/');
  command.trim();

  if (command == "switch") {
    switchCommand(client);
  }
}

void switchCommand(BridgeClient client) {
  String clientNonce;
  String clientToken;
  
  if (client.readStringUntil('/') == "nonce") {
    clientNonce = client.readStringUntil('/');
    
    if (client.readStringUntil('/') == "token") {
      clientToken = client.readStringUntil('\r');   
    }
  }
  
  if (clientNonce != "" && clientToken != "") {
    if (stringToLong(clientNonce) > nonce) {
      Speck speck(key);
      
      unsigned long plaintext[2] = {
        stringToLong(clientNonce),
        identifier
      };
      unsigned long ciphertext[2];
      
      speck.enc(ciphertext, plaintext);

      unsigned long token[2] = {
        stringToLong(clientToken.substring(8, 16)),
        stringToLong(clientToken.substring(0, 8))
      };

      if (token[1] == ciphertext[1] && token[0] == ciphertext[0]) {
        nonce = stringToLong(clientNonce);
        driveRelay();
        client.print("status/success");
      } else {
        client.print("status/error");
      }
    } else {
      client.print("status/error");
    }
  }
}

void driveRelay() {
  digitalWrite(relay, HIGH);
  delay(100);
  digitalWrite(relay, LOW);
}

void initNonce() {
  HttpClient httpClient;
  httpClient.get("http://" + String(host) + "/api/v1/accessories/doors/nonce/" + String(name));

  char response[20];

  unsigned char i = 0;
  while (httpClient.available()) {
    response[i] = httpClient.read();
    i++;
  }

  nonce = stringToLong(String(response).substring(13, 21));
}

unsigned long stringToLong(String s) {
  char buf[9];
  s.toCharArray(buf, sizeof(buf));
  return strtoul(buf, NULL, 16);
}

