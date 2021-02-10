/// Copyright (c) 2019-2020 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "../threads/system.hh"
#include "../lib/utility.hh"


void ReadBufferFromUser(int userAddress, char *outBuffer,
                        unsigned byteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(outBuffer != nullptr);

    while (byteCount > 0) {
      ASSERT(machine->ReadMem(userAddress++, 1, (int *) outBuffer));
      byteCount--;
      outBuffer++;
    }
}

bool ReadStringFromUser(int userAddress, char *outString,
                        unsigned maxByteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(outString != nullptr);
    ASSERT(maxByteCount != 0);

    unsigned count = 0;
    do {
        int temp;
        count++;
        ASSERT(machine->ReadMem(userAddress++, 1, &temp));
        *outString = (unsigned char) temp;
    } while (*outString++ != '\0' && count < maxByteCount);

    return *(outString - 1) == '\0';
}

void WriteBufferToUser(const char *buffer, int userAddress,
                       unsigned byteCount)
{
  ASSERT(userAddress != 0);
  ASSERT(buffer != nullptr);

  while (byteCount > 0) {
    ASSERT(machine->WriteMem(userAddress++, 1, *buffer));
    byteCount--;
    buffer++;
  }
}

void WriteStringToUser(const char *string, int userAddress)
{
    ASSERT(string != nullptr);

    while (*string) {
      ASSERT(machine->WriteMem(userAddress++, 1, *string));
      string++;
    }
}
