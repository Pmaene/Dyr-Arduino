//
// SPECK
// Pieter Maene <pieter@maene.eu>
//

#include "speck.h"

#define ROTL(x, n) ((x << n) | (x >> (32-n)))
#define ROTR(x, n) ((x >> n) | (x << (32-n)))

Speck::Speck(unsigned long *key) {
  keys[0] = key[0];

  ls[0] = key[1];
  ls[1] = key[2];
  ls[2] = key[3];

  expandKey();
}

Speck::~Speck() {}

void Speck::enc(unsigned long *ciphertext, unsigned long *plaintext) {
  ciphertext[1] = plaintext[1];
  ciphertext[0] = plaintext[0];

  unsigned char i;
  for (i = 0; i < 27; i++) {
    ciphertext[1] = (ROTR(ciphertext[1], 8) + ciphertext[0]) ^ keys[i];
    ciphertext[0] = ROTL(ciphertext[0], 3) ^ ciphertext[1];
  }
}

void Speck::expandKey() {
  unsigned char i;
  for (i = 0; i < 26; i++) {
    ls[i+3] = (keys[i] + ROTR(ls[i], 8)) ^ i;
    keys[i+1] = ROTL(keys[i], 3) ^ ls[i+3];
  }
}

