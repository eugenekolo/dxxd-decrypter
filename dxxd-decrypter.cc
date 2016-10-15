#include <stdlib.h>
#include <stdio.h>

#define __ROL__(x, y) _rotl(x, y)
inline unsigned int __ROL4__(unsigned int value, int count) { return __ROL__((unsigned int)value, count); }
inline unsigned int __ROR4__(unsigned int value, int count) { return __ROL__((unsigned int)value, -count); }

static unsigned int KEY = 0xa7d46c76;
static unsigned int TEST = 0;

unsigned int* gen_keystream(unsigned int size, unsigned int* keystream) {
  unsigned int subkey = KEY;

  while (size) {
    *keystream = subkey;
    subkey = __ROL4__(subkey, 5);
    ++keystream;
    --size;
  }

  return keystream;
}

unsigned int encrypt(unsigned int *data, unsigned int size_in_dwords)
{
  int subkey;
  unsigned int encrypted_dword;

  // In place encryption
  subkey = KEY;
  while (size_in_dwords) {
    encrypted_dword = __ROR4__(subkey ^ _byteswap_ulong(*data), 3);
    subkey = __ROL4__(subkey, 5);
    *data = encrypted_dword;
    ++data;
    --size_in_dwords;
  }

  return encrypted_dword;
}

unsigned int decrypt(unsigned int *data, unsigned int size_in_dwords)
{
  unsigned int* keystream;
  unsigned int decrypted_dword;
  
  keystream = (unsigned int*)malloc(size_in_dwords*sizeof(unsigned int));
  gen_keystream(size_in_dwords, keystream);

  // In place decryption
  while (size_in_dwords) {
    decrypted_dword = _byteswap_ulong(*keystream ^ __ROL4__(*data, 3));
    *data = decrypted_dword;
    ++data;
    ++keystream;
    --size_in_dwords;
  }

  return decrypted_dword;
}

int test() {
  unsigned int* data;
  long file_size_in_dwords;
  char* file_name;
  FILE* file;

  // Encrypt a clear file using dxxd's algorithm
  file_name = "test/Desert.jpg";
  file = fopen(file_name, "rb");
  fseek(file , 0 , SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  file_size_in_dwords = file_size >> 2;
  data = (unsigned int*)malloc(file_size_in_dwords*sizeof(unsigned int));
  fread(data, 4, file_size_in_dwords, file);
  fclose(file);

  encrypt(data+1, file_size_in_dwords);

  char* outfile = "test/Desert.test.jpgdxxd";
  file = fopen(outfile, "wb");
  fwrite(data, 4, file_size_in_dwords, file);
  fclose(file);

  // Compare that the encrypted test file is the same as a file encrypted by a dxxd sample
  // TODO

  // Decrypt the encrypted file
  decrypt(data+1, file_size_in_dwords);
  file_name = "test/Desert.test.jpg";
  file = fopen(file_name, "wb");
  fwrite(data, 4, file_size_in_dwords, file);
  printf("decrypted!\n");
  
  // Compare that the decrypted file is the same as the original clear file
  // TODO

  return 0;
}

int main(int argc, char** argv) {
  FILE* file;
  char* infile_name;
  char* outfile_name;
  long file_size;
  long file_size_in_dwords;
  unsigned int* data;

  if (TEST) {
    test();
    return 0;
  }

  if(argc < 3) {
    printf("Usage: dxxd-decrypter.exe <encrypted_file_name> <desired_decrypted_file_name>\n");
    return -1;
  }

  infile_name = argv[1];
  outfile_name = argv[2];

  // Get file size
  file = fopen(infile_name, "rb");
  fseek(file , 0 , SEEK_END);
  file_size = ftell(file);
  rewind(file);

  // Read the file as dwords
  file_size_in_dwords = file_size >> 2;
  data = (unsigned int*)malloc(file_size_in_dwords*sizeof(unsigned int));
  fread(data, 4, file_size_in_dwords, file);
  fclose(file);

  // Decrypt it
  decrypt(data+1, file_size_in_dwords);

  // Write out the decrypted file
  file = fopen(outfile_name, "wb");
  fwrite(data, 4, file_size_in_dwords, file);
  printf("Decrypted!\n");
  
  return 0;
}
