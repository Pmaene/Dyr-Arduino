//
// Dyr
// Pieter Maene <pieter@maene.eu>
//

#include <Bridge.h>
#include <Process.h>
#include <YunClient.h>
#include <YunServer.h>

#include "Speck.h"

// Constants
unsigned long key[4] = {0x03020100, 0x0b0a0908, 0x13121110, 0x1b1a1918};

// Ports
int relay = 12;
int led = 13;

// Main
unsigned long challenge[2];
YunServer server;

void setup() {
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);

  digitalWrite(relay, LOW);

  digitalWrite(led, LOW);
  Bridge.begin();
  digitalWrite(led, HIGH);

  server.noListenOnLocalhost();
  server.begin();
}

void loop() {
  YunClient client = server.accept();
  if (client) {
    client.setTimeout(5);
    process(client);
    String data = client.readStringUntil('\n');
    client.stop();
  }

  delay(5);
}

void process(YunClient client) {
  String command = client.readStringUntil('/');
  command.trim();

  if (command == "switch") {
    switchCommand(client);
  }

  if (command == "hello") {
    helloCommand(client);
  }
}

void helloCommand(YunClient client) {
  challenge[0] = arand();
  challenge[1] = arand();

  client.print("challenge/" + String(challenge[1], HEX) + String(challenge[0], HEX));
}

void switchCommand(YunClient client) {
  String nonce;
  String response;
  
  if (client.readStringUntil('/') == "nonce") {
    nonce = client.readStringUntil('/');
    
    if (client.readStringUntil('/') == "response") {
      response =  client.readStringUntil('\r');   
    }
  }
  
  if (nonce != "" && response != "") {
    Speck speck(key);

    unsigned long plaintext[2] = {
      stringToLong(nonce.substring(8, 16)),
      stringToLong(nonce.substring(0, 8))
    };
    unsigned long ciphertext[2];
    
    speck.enc(ciphertext, plaintext);
    
    String check[2] = {
      String(challenge[0] ^ ciphertext[0], HEX),
      String(challenge[1] ^ ciphertext[1], HEX)
    };
    
    if (response == (check[1] + check[0])) {
      driveRelay();
      client.print("status/success");
    } else {
      client.print("status/error");
    }
  }
}

unsigned long arand() {
  unsigned long seed;

  unsigned char i;
  for (i = 0; i < 4; i++) {
    seed += analogRead(0);
    seed += (seed << 10);
    seed ^= (seed >> 6);
  }

  seed += (seed << 3);
  seed ^= (seed >> 11);
  seed += (seed << 15);

  return seed;
}

void driveRelay() {
  digitalWrite(relay, HIGH);
  delay(100);
  digitalWrite(relay, LOW);
}

unsigned long stringToLong(String s) {
   char buf[9];
   s.toCharArray(buf, sizeof(buf));
   return strtol(buf, NULL, 16);
}

