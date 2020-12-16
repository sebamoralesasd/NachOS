#include "synchConsole.hh"

static void
ReadAvailWrapper(void *arg) {
  ((SynchConsole *) arg)->ReadAvail();
}

static void
WriteDoneWrapper(void *arg) {
  ((SynchConsole *) arg)->WriteDone();
}

SynchConsole::SynchConsole() {
  readAvail = new Semaphore("Read Avail", 0);
  writeDone = new Semaphore("Write Done", 0);
  reading = new Lock("Reading character from console");
  writing = new Lock("Writing character to console");
  console = new Console(nullptr, nullptr, ReadAvailWrapper, WriteDoneWrapper, this);
}

SynchConsole::~SynchConsole() {
  delete readAvail;
  delete writeDone;
  delete reading;
  delete writing;
  delete console; 
}

void
SynchConsole::ReadAvail() {
  readAvail->V();
}

void
SynchConsole::WriteDone() {
  writeDone->V();
}

char
SynchConsole::GetChar() {
  reading->Acquire();
  readAvail->P();
  char c = console->GetChar();

  reading->Release();

  return c;
}

void
SynchConsole::PutChar(char ch) {
  writing->Acquire();
  console->PutChar(ch);
  writeDone->P();

  writing->Release();
}