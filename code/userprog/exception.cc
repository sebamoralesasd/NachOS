/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2020 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "syscall.h"
#include "../filesys/directory_entry.hh"
#include "../threads/system.hh"

#include <stdio.h>


static void
IncrementPC()
{
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// NOTE: this function is meant specifically for unexpected exceptions.  If
/// you implement a new behavior for some exception, do not extend this
/// function: assign a new handler instead.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
static void
DefaultHandler(ExceptionType et)
{
    int exceptionArg = machine->ReadRegister(2);

    fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
            ExceptionTypeToString(et), exceptionArg);
    ASSERT(false);
}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)
static void
SyscallHandler(ExceptionType _et)
{
    int scid = machine->ReadRegister(2);

    switch (scid) {

        case SC_HALT:
            DEBUG('e', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;

        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);

            if (!filenameAddr) {
              DEBUG('e', "Error: address to filename string is null.\n");
              break;
            }

            DEBUG('e', "Filename address: %d\n", filenameAddr);
            char filename[FILE_NAME_MAX_LEN + 1];
            if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)) {
              DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
                FILE_NAME_MAX_LEN);
              break;
            }

            DEBUG('e', "`Create` requested for file `%s`.\n", filename);
            ASSERT(fileSystem->Create(filename, 0));
            DEBUG('e', "File `%s` created.\n", filename);
            break;
        }

        case SC_READ: {
          int bufferAddr = machine->ReadRegister(4);
          int bufferSize = machine->ReadRegister(5);
          OpenFileId fd = machine->ReadRegister(6);

          int i;

          if(!bufferAddr)
            DEBUG('e', "Error: address to buffer is null.\n");
          else if (bufferSize < 0)
            DEBUG('e',"Error: invalid size.\n");
          else if (fd == CONSOLE_OUTPUT || fd < 0)
            DEBUG('e', "Error: invalid file descriptor.\n");
          else if (!currentThread->GetFileTable()->HasKey(fd))
            DEBUG('e', "Could not open file descriptor");
          else {
            char read[bufferSize+1];

            if (fd == CONSOLE_OUTPUT) {
              for(i=0; i < bufferSize; i++)
                read[i] = synchConsole->GetChar();
              read[i--] = '\0';
            } else {
              i = currentThread->GetFileTable()->Get(fd)->Read(read, bufferSize);
            }

            DEBUG('e', "String read: %s. Size: %d\n", read, i);
          }

          machine->WriteRegister(2, i);
          break;
        }

        case SC_WRITE: {
          DEBUG('e', "`Write`, initiated by user program.\n");

          int stringAddress = machine->ReadRegister(4);
          int stringLength = machine->ReadRegister(5);
          OpenFileId fileID = machine->ReadRegister(6);

          DEBUG('e', "`Write` request: \n");
          DEBUG('e',  "StrAddr: %d, length: %d, fileID: %d\n",
                stringAddress, stringLength, fileID);

          if (!stringAddress)
            DEBUG('e', "Error: address to filename is null.\n");
          else if (stringLength < 0)
            DEBUG('e', "Invalid size.\n");
          else if (fileID < CONSOLE_OUTPUT) {
            DEBUG('e', "Error: invalid file descriptor.\n");
          } else if (!currentThread->GetFileTable()->HasKey(fileID))
              DEBUG('e', "Error: could not open file descriptor.\n"); 
          else {
            char stringToWrite[stringLength];

          ReadBufferFromUser(stringAddress, stringToWrite, stringLength);
          DEBUG('e', "Writing %s", stringToWrite);

          if (fileID == CONSOLE_OUTPUT)
            for (int i=0; i < stringLength; i++)
              synchConsole->PutChar(stringToWrite[i]);
          else
            currentThread->GetFileTable()->Get(fileID)->Write(stringToWrite, stringLength);

          break;
          }

        }

        case SC_OPEN: {
          DEBUG('e', "Open, initiated by user program.\n");

          int filenameAddr = machine->ReadRegister(4);
          DEBUG('e', "Filename address: %d\n", filenameAddr);

          if (!filenameAddr) {
            DEBUG('e', "Error: address to filename string is null.\n");
            break;
          }

          char filename[FILE_NAME_MAX_LEN + 1];

          if (!ReadStringFromUser(filenameAddr, filename, sizeof filename)) {
            DEBUG('e', "Error: filename string too long (maximum is %u bytes).\n",
              FILE_NAME_MAX_LEN);
            break;
          }

          DEBUG('e', "`Open` requested for file `%s`.\n", filename);

          OpenFile *file = fileSystem->Open(filename);

          if (!file) {
            DEBUG('e', "Error: file `%s` not found.\n", filename);
            machine->WriteRegister(2, -1);
            break;
          } else {
            int fd = currentThread->GetFileTable()->Add(file);
            DEBUG('e', "File %s opened. Fd assigned: %d.\n", file, fd);

            machine->WriteRegister(2, fd);
          }

        break;

        }

        case SC_CLOSE: {
            int fid = machine->ReadRegister(4);
            DEBUG('e', "`Close` requested for id %u.\n", fid);
            
            if(fid < 0)
              DEBUG('e',"Error: invalid file descriptor.\n");
            else if(!currentThread->GetFileTable()->HasKey(fid))
              DEBUG('e', "Error: could not open file descriptor.\n");
            else {
              currentThread->GetFileTable()->Remove(fid);
              DEBUG('e',"File closed.\n");
            }

            break;
        }

        case SC_EXIT: {
            int status = machine->ReadRegister(4);
            DEBUG('e', "Exiting with status: %d.\n", status);
            currentThread->Finish(status);
            break;
        }

        case SC_JOIN: {
          int threadID = machine->ReadRegister(4);
          DEBUG('e', "`Join` requested. Thread ID: %d.\n", threadID);

          if(threadID < 0 || !threadTable->HasKey(threadID))
            DEBUG('e', "Error. Invalid Thread ID.\n");
          else
            machine->WriteRegister(2, threadTable->Get(threadID)->Join());

          break;
        }

        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);

    }

    IncrementPC();
}


/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void
SetExceptionHandlers()
{
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &DefaultHandler);
    machine->SetHandler(READ_ONLY_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}
