// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void Nachos_Halt() {					// System call 0
	DEBUG('v', "Halt()\n");

	interrupt->Halt();
}	// Nachos_Halt

void Nachos_Exit() {					// System call 1
	int status = machine->ReadRegister( 4 );

	DEBUG('v', "Exit(%d)\n", status);

	currentThread->Finish();
}	// Nachos_Exit

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2);
	static int pf = 0;
	int dir, page;
	//static int cnt = 3;

	switch ( which ) {
	case SyscallException:
		switch ( type ) {
		case SC_Halt:
			Nachos_Halt();				// System call # 0
			break;
		case SC_Exit:
			Nachos_Exit();				// System call # 1
			break;
		default:
			printf("Unexpected syscall exception %d\n", type );
			ASSERT(false);
			break;
		}
		break;
	case PageFaultException:
		/*
			Se atrapa la excepción del PageFault, se llama el metodo de decisión
			ubicado en addspace llamado load con la página que dio el error..
		*/
		dir = machine->ReadRegister( BadVAddrReg );
		page = dir / PageSize;
		DEBUG('w', "%d: PageFaultException at addr %d in log page %d\n", ++pf, dir, page );
		currentThread->space->load(page);
		//DEBUG('v', "Regresa de load\n");
		//if (!cnt--) exit(0);
		break;
	default:
		printf( "Unexpected exception %d\n", which );
		ASSERT(false);
		break;
	}
}
