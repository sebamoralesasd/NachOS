#ifndef NACHOS_FILESYS_SYNCHCONSOLE__HH
#define NACHOS_FILESYS_SYNCHCONSOLE__HH

#include "../machine/console.hh"
#include "../threads/synch.hh"

class SynchConsole {
public:
  SynchConsole();

  ~SynchConsole();

  void ReadAvail();

  void WriteDone();

  char GetChar();

  void PutChar(char ch);

private:
  Console *console;
  Semaphore *readAvail;
  Semaphore *writeDone;
  Lock *reading;
  Lock *writing;
};

#endif
