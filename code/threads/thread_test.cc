/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2020 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "system.hh"

#include <stdio.h>
#include <string.h>

#ifdef SEMAPHORE_TEST
Semaphore Semaforo("Semaforo ThreadTest", 3);
#endif

#ifdef CONDITION_TEST
//lock = new Lock("Condition lock");
//condition = new Condition("Condition", lock);
#include "synch_list.hh"
SynchList<int *> *synchList = new SynchList<int *>();
#endif


/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.
void
SimpleThread(void *name_)
{
    // Reinterpret arg `name` as a string.
    char *name = (char *) name_;

    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.

    #ifdef SEMAPHORE_TEST
    Semaforo.P();
    #endif

    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
        currentThread->Yield();
    }
    printf("!!! Thread `%s` has finished\n", name);

    #ifdef SEMAPHORE_TEST
    Semaforo.V();
    #endif
}

void
Add(void *item_) {
  int *item = (int *) item_;
  DEBUG('t', "Adding number %d\n", item);
  synchList->Append(item);
}

void
Delete(void *ptr) {
  int* item = synchList->Pop();
  DEBUG('t', "Number %d deleted\n", item);
}

/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`, and finally
/// calling `SimpleThread` ourselves.
void
ThreadTest()
{
    DEBUG('t', "Entering thread test\n");

    char *nameList[5] = {"1st","2nd", "3rd", "4th", "5th"};
    int i;
    for(i=0; i<5; i++){
      Thread *newThread = new Thread(nameList[i], 0);
      if(i%2)
        newThread->Fork(Add, (void *) i);
      else
        newThread->Fork(Delete, (void *) nullptr);
    }
    Add((void *) 0);
}
