#include <iostream>
#include <vector>
#include <limits>

uint32_t Ch(const uint32_t &x, const uint32_t &y, const uint32_t &z){
  return (x & y) ^ ((~x) & z);
}

uint32_t Parity(const uint32_t &x, const uint32_t &y, const uint32_t &z){
  return x ^ y ^ z;
}

uint32_t Maj(const uint32_t &x, const uint32_t &y, const uint32_t &z){
  return (x & y) ^ (x & z) ^ (y & z);
}

uint32_t ROTR(const uint32_t &x, const uint32_t &n){
  return (x >> n) | ( x << (32-n));
}

uint32_t ft(const uint32_t &x, const uint32_t &y, const uint32_t &z, size_t t){

  if(60 <= t && t <= 79){
    return Parity(x, y, z);
  } else if(40 <= t && t <= 59){
    return Maj(x, y, z);
  } else if(20 <= t && t <= 39){
    return Parity(x, y, z);
  } else if(0 <= t && t <= 19){
    return Ch(x, y, z);
  }

  throw std::runtime_error("No existe la función para t");

}

uint32_t ROTL(const uint32_t &x, const size_t &n){
  return (x << n) | ( x >> (32-n));
}

// circular bit shift left of integer 'number' by 'n' bit positions
template<typename T>
T circular_shift_left(T number, std::size_t n) {
  static_assert(std::is_integral<T>::value, "an integral type is required");

  // the corresponding unsigned type if T is a signed integer, T itself otherwise
  using unsigned_type = std::make_unsigned_t<T>;

  // number of bits in the integral type
  constexpr std::size_t NBITS = std::numeric_limits<unsigned_type>::digits;

  n %= NBITS; // bring the number of bit positions to shift by to within a valid range
  const unsigned_type un = number; // the number interpreted as an unsigned value

  // circular bit shift left of an unsigned NBITS-bit integer by n bit positions
  return (un << n) | (un >> (NBITS - n));
}

int main() {

  // Arreglos de prueba
//  std::vector<uint8_t> Message {'a','b','c'};
//  std::vector<uint8_t> Message {'a','b','c','d','b','c','d','e','c','d','e','f','d','e','f','g','e','f','g','h','f','g',
//                                'h','i','g','h','i','j','h','i','j','k','i','j','k','l','j','k','l','m','k','l','m','n',
//                                'l','m','n','o','m','n','o','p','n','o','p','q'};
  std::vector<uint8_t> Message(1000000,'a');

  /* ------------------------------- Padding ------------------------------- */
  // Número de 64 elementos completos en el arreglo Message
  size_t incomplete8BitBlockSize = Message.size() % 64;

  // Número de bits en el mensaje
  unsigned long long numberOfBits = Message.size() * 8;

  // Convertir número de bits en dos de 32: numberOfBitsUpper || numberOfBitsLow
  uint32_t numberOfBitsUpper = numberOfBits >> 32;
  uint32_t numberOfBitsLow = (numberOfBits << 32) >> 32;
  uint8_t numberOfBitsPart1 = numberOfBitsUpper >> 24;
  uint8_t numberOfBitsPart2 = (numberOfBitsUpper << 8) >> 24;
  uint8_t numberOfBitsPart3 = (numberOfBitsUpper << 16) >> 24;
  uint8_t numberOfBitsPart4 = (numberOfBitsUpper << 24) >> 24;
  uint8_t numberOfBitsPart5 = numberOfBitsLow >> 24;
  uint8_t numberOfBitsPart6 = (numberOfBitsLow << 8) >> 24;
  uint8_t numberOfBitsPart7 = (numberOfBitsLow << 16) >> 24;
  uint8_t numberOfBitsPart8 = (numberOfBitsLow << 24) >> 24;

  //  448 bits o 56 elementos de 8 bits >=
  if(incomplete8BitBlockSize >= 56) {
    // Add 1000...000 (8 bits)
    Message.push_back(128);

    // Add 000.. (8 bits) until 0 bits remaining to 512
    for(size_t i = incomplete8BitBlockSize + 1; i < 64; ++i){
      Message.push_back(0);
    }

    // Add 000.. (8 bits) until 64 bits remaining to 512
    for(size_t i = 0; i < 56; ++i){
      Message.push_back(0);
    }

    // Add length
    Message.push_back(numberOfBitsPart1);
    Message.push_back(numberOfBitsPart2);
    Message.push_back(numberOfBitsPart3);
    Message.push_back(numberOfBitsPart4);
    Message.push_back(numberOfBitsPart5);
    Message.push_back(numberOfBitsPart6);
    Message.push_back(numberOfBitsPart7);
    Message.push_back(numberOfBitsPart8);

  } else if ( 0 <= incomplete8BitBlockSize && incomplete8BitBlockSize < 56) {
    // Add 1000...000 (8 bits)
    Message.push_back(128);

    // Add 000.. (8 bits) until 64 bits remaining
    for(size_t i = incomplete8BitBlockSize + 1; i < 56; ++i){
      Message.push_back(0);
    }

    // Add length
    Message.push_back(numberOfBitsPart1);
    Message.push_back(numberOfBitsPart2);
    Message.push_back(numberOfBitsPart3);
    Message.push_back(numberOfBitsPart4);
    Message.push_back(numberOfBitsPart5);
    Message.push_back(numberOfBitsPart6);
    Message.push_back(numberOfBitsPart7);
    Message.push_back(numberOfBitsPart8);

  }

  /* ------------------------------- Constants and Initial hash value ------------------------------- */

  std::vector<uint32_t> K0(20, 0x5a827999);
  std::vector<uint32_t> K1(20, 0x6ed9eba1);
  std::vector<uint32_t> K2(20, 0x8f1bbcdc);
  std::vector<uint32_t> K3(20, 0xca62c1d6);

  std::vector<uint32_t> K;
  K.insert(K.end(), K0.begin(), K0.end());
  K.insert(K.end(), K1.begin(), K1.end());
  K.insert(K.end(), K2.begin(), K2.end());
  K.insert(K.end(), K3.begin(), K3.end());

  std::vector<uint32_t> H {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0};

  // Parse to 512 bit messsage block
  for(size_t indexMessage512Bits = 0; indexMessage512Bits < Message.size(); indexMessage512Bits += 64){
    uint32_t M1, M2, M3, M4, M5, M6, M7, M8, M9, M10, M11, M12, M13, M14, M15, M16;

    /* Initialize M's*/
    M1 = Message[indexMessage512Bits];
    M1 = (M1 << 8) + Message[indexMessage512Bits + 1];
    M1 = (M1 << 8) + Message[indexMessage512Bits + 2];
    M1 = (M1 << 8) + Message[indexMessage512Bits + 3];

    M2 = Message[indexMessage512Bits + 4];
    M2 = (M2 << 8) + Message[indexMessage512Bits + 5];
    M2 = (M2 << 8) + Message[indexMessage512Bits + 6];
    M2 = (M2 << 8) + Message[indexMessage512Bits + 7];

    M3 = Message[indexMessage512Bits + 8];
    M3 = (M3 << 8) + Message[indexMessage512Bits + 9];
    M3 = (M3 << 8) + Message[indexMessage512Bits + 10];
    M3 = (M3 << 8) + Message[indexMessage512Bits + 11];

    M4 = Message[indexMessage512Bits + 12];
    M4 = (M4 << 8) + Message[indexMessage512Bits + 13];
    M4 = (M4 << 8) + Message[indexMessage512Bits + 14];
    M4 = (M4 << 8) + Message[indexMessage512Bits + 15];

    M5 = Message[indexMessage512Bits + 16];
    M5 = (M5 << 8) + Message[indexMessage512Bits + 17];
    M5 = (M5 << 8) + Message[indexMessage512Bits + 18];
    M5 = (M5 << 8) + Message[indexMessage512Bits + 19];

    M6 = Message[indexMessage512Bits + 20];
    M6 = (M6 << 8) + Message[indexMessage512Bits + 21];
    M6 = (M6 << 8) + Message[indexMessage512Bits + 22];
    M6 = (M6 << 8) + Message[indexMessage512Bits + 23];

    M7 = Message[indexMessage512Bits + 24];
    M7 = (M7 << 8) + Message[indexMessage512Bits + 25];
    M7 = (M7 << 8) + Message[indexMessage512Bits + 26];
    M7 = (M7 << 8) + Message[indexMessage512Bits + 27];

    M8 = Message[indexMessage512Bits + 28];
    M8 = (M8 << 8) + Message[indexMessage512Bits + 29];
    M8 = (M8 << 8) + Message[indexMessage512Bits + 30];
    M8 = (M8 << 8) + Message[indexMessage512Bits + 31];

    M9 = Message[indexMessage512Bits + 32];
    M9 = (M9 << 8) + Message[indexMessage512Bits + 33];
    M9 = (M9 << 8) + Message[indexMessage512Bits + 34];
    M9 = (M9 << 8) + Message[indexMessage512Bits + 35];

    M10 = Message[indexMessage512Bits + 36];
    M10 = (M10 << 8) + Message[indexMessage512Bits + 37];
    M10 = (M10 << 8) + Message[indexMessage512Bits + 38];
    M10 = (M10 << 8) + Message[indexMessage512Bits + 39];

    M11 = Message[indexMessage512Bits + 40];
    M11 = (M11 << 8) + Message[indexMessage512Bits + 41];
    M11 = (M11 << 8) + Message[indexMessage512Bits + 42];
    M11 = (M11 << 8) + Message[indexMessage512Bits + 43];

    M12 = Message[indexMessage512Bits + 44];
    M12 = (M12 << 8) + Message[indexMessage512Bits + 45];
    M12 = (M12 << 8) + Message[indexMessage512Bits + 46];
    M12 = (M12 << 8) + Message[indexMessage512Bits + 47];

    M13 = Message[indexMessage512Bits + 48];
    M13 = (M13 << 8) + Message[indexMessage512Bits + 49];
    M13 = (M13 << 8) + Message[indexMessage512Bits + 50];
    M13 = (M13 << 8) + Message[indexMessage512Bits + 51];

    M14 = Message[indexMessage512Bits + 52];
    M14 = (M14 << 8) + Message[indexMessage512Bits + 53];
    M14 = (M14 << 8) + Message[indexMessage512Bits + 54];
    M14 = (M14 << 8) + Message[indexMessage512Bits + 55];

    M15 = Message[indexMessage512Bits + 56];
    M15 = (M15 << 8) + Message[indexMessage512Bits + 57];
    M15 = (M15 << 8) + Message[indexMessage512Bits + 58];
    M15 = (M15 << 8) + Message[indexMessage512Bits + 59];

    M16 = Message[indexMessage512Bits + 60];
    M16 = (M16 << 8) + Message[indexMessage512Bits + 61];
    M16 = (M16 << 8) + Message[indexMessage512Bits + 62];
    M16 = (M16 << 8) + Message[indexMessage512Bits + 63];

    /* Prepare the message schedule */
    std::vector<uint32_t> W(80);
    W[0] = M1;
    W[1] = M2;
    W[2] = M3;
    W[3] = M4;
    W[4] = M5;
    W[5] = M6;
    W[6] = M7;
    W[7] = M8;
    W[8] = M9;
    W[9] = M10;
    W[10] = M11;
    W[11] = M12;
    W[12] = M13;
    W[13] = M14;
    W[14] = M15;
    W[15] = M16;

    for(size_t t = 16; t < 80; ++t){
      uint32_t tmp = W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16];
      W[t] = circular_shift_left(tmp, 1);
    }

    std::vector<uint32_t> tmp(H);

    for(size_t t = 0; t < 80; ++t){
      uint64_t T = circular_shift_left(tmp[0], 5) + ft(tmp[1], tmp[2], tmp[3], t) + tmp[4] + K[t] + W[t];
      tmp[4] = tmp[3];
      tmp[3] = tmp[2];
      tmp[2] = circular_shift_left(tmp[1], 30);
      tmp[1] = tmp[0];
      tmp[0] = T;
    }

    H[0] = tmp[0] + H[0];
    H[1] = tmp[1] + H[1];
    H[2] = tmp[2] + H[2];
    H[3] = tmp[3] + H[3];
    H[4] = tmp[4] + H[4];

  }

  for(size_t i = 0; i < H.size(); ++i){
    std::cout << std::hex << H[i] << " \n"[i == (H.size() - 1)];
  }

  return 0;
}
