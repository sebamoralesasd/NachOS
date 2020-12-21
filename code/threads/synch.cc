/// Routines for synchronizing threads.
///
/// Three kinds of synchronization routines are defined here: semaphores,
/// locks and condition variables (the implementation of the last two are
/// left to the reader).
///
/// Any implementation of a synchronization routine needs some primitive
/// atomic operation.  We assume Nachos is running on a uniprocessor, and
/// thus atomicity can be provided by turning off interrupts.  While
/// interrupts are disabled, no context switch can occur, and thus the
/// current thread is guaranteed to hold the CPU throughout, until interrupts
/// are reenabled.
///
/// Because some of these routines might be called with interrupts already
/// disabled (`Semaphore::V` for one), instead of turning on interrupts at
/// the end of the atomic operation, we always simply re-set the interrupt
/// state back to its original value (whether that be disabled or enabled).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2020 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "synch.hh"
#include "system.hh"
#include <stdio.h>


/// Initialize a semaphore, so that it can be used for synchronization.
///
/// * `debugName` is an arbitrary name, useful for debugging.
/// * `initialValue` is the initial value of the semaphore.
Semaphore::Semaphore(const char *debugName, int initialValue)
{
    name  = debugName;
    value = initialValue;
    queue = new List<Thread *>;
}

/// De-allocate semaphore, when no longer needed.
///
/// Assume no one is still waiting on the semaphore!
Semaphore::~Semaphore()
{
    delete queue;
}

const char *
Semaphore::GetName() const
{
    return name;
}

/// Wait until semaphore `value > 0`, then decrement.
///
/// Checking the value and decrementing must be done atomically, so we need
/// to disable interrupts before checking the value.
///
/// Note that `Thread::Sleep` assumes that interrupts are disabled when it is
/// called.
void
Semaphore::P()
{
    #ifdef SEMAPHORE_TEST
    DEBUG('s', "Wait from %s\n", this->name);
    #endif
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);
      // Disable interrupts.

    while (value == 0) {  // Semaphore not available.
        queue->Append(currentThread);  // So go to sleep.
        currentThread->Sleep();
    }
    value--;  // Semaphore available, consume its value.

    interrupt->SetLevel(oldLevel);  // Re-enable interrupts.
}

/// Increment semaphore value, waking up a waiter if necessary.
///
/// As with `P`, this operation must be atomic, so we need to disable
/// interrupts.  `Scheduler::ReadyToRun` assumes that threads are disabled
/// when it is called.
void
Semaphore::V()
{
    #ifdef SEMAPHORE_TEST
    DEBUG('s', "Signal from %s\n", this->name);
    #endif
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);

    Thread *thread = queue->Pop();
    if (thread != nullptr)
        // Make thread ready, consuming the `V` immediately.
        scheduler->ReadyToRun(thread);
    value++;

    interrupt->SetLevel(oldLevel);
}

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

Lock::Lock(const char *debugName)
{
  name = debugName;
  state = new Semaphore(debugName, 1);
  holder = nullptr;
}

Lock::~Lock()
{
  name = nullptr;
  delete state;
  holder = nullptr;
}

const char *
Lock::GetName() const
{
    return name;
}

void
Lock::Acquire()
{
    ASSERT(!IsHeldByCurrentThread());

#ifdef INVPRIO
    if (holder && holder->GetPriority() < currentThread->GetPriority()) {
      // TODO: Agregar mensaje de debug
      holder->SetPriority(currentThread->GetPriority());
      scheduler->UpdatePriority(holder);
    }
#endif

    state->P();
    holder = currentThread;
}

void
Lock::Release()
{
    ASSERT(IsHeldByCurrentThread());

#ifdef INVPRIO
    if (currentThread->GetOldPriority() != currentThread->GetPriority()) {
      currentThread->SetPriority(currentThread->GetOldPriority());
      scheduler->UpdatePriority(currentThread);
    }
#endif

    state->V();
    holder = nullptr;
}

bool
Lock::IsHeldByCurrentThread() const
{
    return currentThread == holder;
}

// MARK: Condition variables
Condition::Condition(const char *debugName, Lock *conditionLock)
{
  lock = conditionLock;
  threadsSleeping = 0;
  name = debugName;
  sleepQueue = new List<Semaphore *>;
}

Condition::~Condition()
{
    lock = nullptr;
    delete sleepQueue;
}

const char *
Condition::GetName() const
{
    return name;
}

void
Condition::Wait()
{
    ASSERT(lock->IsHeldByCurrentThread());

    // Create a Semaphore to sleep the current thread
    char *semaphoreName = new char [64];
    snprintf(semaphoreName, 64, "Condition variable %s Semaphore of Thread %s",
             GetName(), currentThread->GetName());
    Semaphore *newSemaphore = new Semaphore(semaphoreName, 0);
    delete semaphoreName;

    threadsSleeping++;
    sleepQueue->Append(newSemaphore);

    lock->Release();
    newSemaphore->P();

    // Acquire lock when the thread had woke up
    lock->Acquire();

    delete newSemaphore;
}

void
Condition::Signal()
{
    ASSERT(lock->IsHeldByCurrentThread());

    if (threadsSleeping > 0) {
      threadsSleeping--;
      Semaphore *waker = sleepQueue->Pop();
      waker->V();
    }
}

void
Condition::Broadcast()
{
    ASSERT(lock->IsHeldByCurrentThread());

    while(threadsSleeping > 0) {
      Signal();
    }
}

// MARK: Channel

Channel::Channel(const char *debugName)
{
  lock = new Lock(debugName);

  char *conditionName = new char [64];
  snprintf(conditionName, 64, "Senders condition %s", debugName);
  senders = new Condition(conditionName, lock);

  snprintf(conditionName, 64, "Receivers condition %s", debugName);
  receivers = new Condition(conditionName, lock);

  buffer = new List<int>;

  delete conditionName;
}


Channel::~Channel() {
  delete lock;
  delete senders;
  delete receivers;
  delete buffer;
}

const char *
Channel::GetName() const {
  return name;
}

void
Channel::Send(int message) {
  lock->Acquire();
  buffer->Append(message);
  receivers->Signal();

  int head = buffer->Head();

  // Esperar hasta que el mensaje se haya copiado en el buffer.
  while (!buffer->IsEmpty() && head == buffer->Head()) {
    senders->Wait();
  }

  lock->Release();
}

void
Channel::Receive(int *message) {
  lock->Acquire();

  // Esperar hasta que haya un mensaje en el buffer.
  while (buffer->IsEmpty()) {
    receivers->Wait();
  }

  *message = buffer->Pop();
  senders->Signal();

  lock->Release();
}
