//
// SPECK
// Pieter Maene <pieter@maene.eu>
//

#ifndef SPECK_H
#define SPECK_H

class Speck {
  public:
    Speck(unsigned long *key);
    ~Speck();

    void enc(unsigned long *ciphertext, unsigned long *plaintext);

  private:
    unsigned long keys[26];
    unsigned long ls[26];

    void expandKey();
};

#endif

