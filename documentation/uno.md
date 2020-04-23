1- #TODO: Desarrollar

const unsigned NUM_PHYS_PAGES = 32;
const unsigned SECTOR_SIZE = 128;
const unsigned PAGE_SIZE = SECTOR_SIZE;
const unsigned MEMORY_SIZE = NUM_PHYS_PAGES * PAGE_SIZE;

2- Este valor es dependiente de NUM_PHYS_PAGES y SECTOR_SIZE, así que se puede modificar modificando dichas variables.

3-

static const unsigned MAGIC_NUMBER = 0x456789AB;
static const unsigned MAGIC_SIZE = sizeof (int);

const unsigned SECTORS_PER_TRACK = 32;  ///< Number of sectors per disk
const unsigned NUM_TRACKS = 32;         ///< Number of tracks per disk.
const unsigned NUM_SECTORS = SECTORS_PER_TRACK * NUM_TRACKS;

static const unsigned DISK_SIZE = MAGIC_SIZE + NUM_SECTORS * SECTOR_SIZE;

4- #TODO

5- #TODO

6-
Initialize -> system.cc
 - ASSERT
 - SystemDep::RandomInit
 - atoi
 - debug.SetFlags()
 - Statistics
 - Interrupt
 - Scheduler
 - Timer
 - Thread
 - currentThread->SetStatus()
 - interrupt->Enable()
 - SystemDep::CallOnUserAbort()
 - PreemptiveScheduler()
 - SetUp()
DEBUG -> utility.hh
 - Debug::Print()
strcmp -> string.h
PrintVersion -> main.cc
 - printf()
ThreadTest ->thread_test.cc
 - DEBUG
 - char
 - strncpy
 - Thread()
 - newThread->Fork
 - SimpleThread
currentThread->Finish() -> thread.cc
 - interrupt->SetLevel()
 - ASSERT
 - DEBUG
 - GetName()
 - Sleep()
