// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
	NoffHeader noffH;
	unsigned int i, size;

	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) &&
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
	numPages = divRoundUp(size, PageSize);
	size = numPages * PageSize;

#ifndef VM
	ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory
#endif

	DEBUG('a', "Initializing address space, num pages %d, size %d\n",
					numPages, size);
// first, set up the translation
	pageTable = new TranslationEntry[numPages];
	for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].physicalPage = i;
#ifdef VM
	pageTable[i].valid = false;
#else
	pageTable[i].valid = true;
#endif
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = false;  // if the code segment was entirely on
					// a separate page, we could set its
					// pages to be read-only
	}

#ifndef VM
// zero out the entire address space, to zero the unitialized data segment
// and the stack segment
	bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory
	if (noffH.code.size > 0) {
		DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
			noffH.code.virtualAddr, noffH.code.size);
		executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
	}
	if (noffH.initData.size > 0) {
		DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
			noffH.initData.virtualAddr, noffH.initData.size);
		executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
	}
#endif
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
	delete file;
   	delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
	int i;

	for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

	// Initial program counter -- must be location of "Start"
	machine->WriteRegister(PCReg, 0);

	// Need to also tell MIPS where next instruction is, because
	// of branch delay possibility
	machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
	machine->WriteRegister(StackReg, numPages * PageSize - 16);
	DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
#ifdef VM
	DEBUG('v', "Saving state de la TLB\n" );
	for (int i = 0; i < TLBSize; i++) {
		pageTable[machine->tlb[i].virtualPage].use = machine->tlb[i].use;
		pageTable[machine->tlb[i].virtualPage].dirty = machine->tlb[i].dirty;
	}
#endif
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
#ifdef VM
	DEBUG('v', "Restore invalidando la TLB\n" );
	for (int i = 0; i < TLBSize; i++) {
		machine->tlb[i].valid = false;
	}
#else
	machine->pageTable = pageTable;
	machine->pageTableSize = numPages;
#endif
}

void AddrSpace::setFilename(const char *filename, int len) {
	file = new char[len+1];
	strcpy(file, filename);
	DEBUG('v', "Ejecutable: %s\n", file );
}

void AddrSpace::updateTLB(int page) {
	DEBUG('v', "En posición %d del TLB copio pageTable[%d]\n", pTLB, page );
	machine->tlb[ pTLB ].virtualPage = pageTable[ page ].virtualPage;
	machine->tlb[ pTLB ].physicalPage = pageTable[ page ].physicalPage;
	machine->tlb[ pTLB ].dirty = pageTable[ page ].dirty;
	DEBUG('v', "\tvirtualPage = %d\n", pageTable[ page ].virtualPage );
	DEBUG('v', "\tphysicalPage = %d\n", pageTable[ page ].physicalPage );
	DEBUG('v', "\tdirty = %d\n", pageTable[ page ].dirty );
	machine->tlb[ pTLB ].use = true;
	machine->tlb[ pTLB ].valid = true;
	pTLB = (pTLB + 1) % TLBSize;
}

int AddrSpace::getPage() {
	int next = memMap->Find();
	if (next == -1) {
		//toSwap(pSwap);
		next = pSwap;
		pSwap = (pSwap + 1) % NumPhysPages;
	}
	DEBUG('v', "A seguir pág %d\n", next );
	return next;
}

void AddrSpace::fromFile(int page) {
	pageTable[page].valid = true;

	OpenFile *executable = fileSystem->Open(file);
	if (executable == NULL) {
		printf("Unable to open file %s\n", file);
		return;
	}

	NoffHeader noffH;
	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) &&
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

	int filAddr = noffH.code.inFileAddr + pageTable[page].virtualPage * PageSize;
	int physicalPage = getPage();
	int memAddr = /*noffH.code.virtualAddr +*/ physicalPage * PageSize;
	DEBUG('v', "Escribe en RAM at %d. Inicio %d\n", memAddr, noffH.code.virtualAddr );
	if (noffH.code.size > 0) {
		executable->ReadAt(&(machine->mainMemory[memAddr]), PageSize, filAddr);
	}
}

void AddrSpace::load(int page) {
	//int memAddr = pageTable[page].physicalPage * PageSize;
	if (pageTable[page].valid) {	// está en la memoria
		DEBUG('v', "Pág. %d está en la memoria\n", page );
		updateTLB(page);
	} else {						// no está en la memoria
		DEBUG('v', "Pág. %d no está en la memoria\n", page );
		if (pageTable[page].dirty) {
			DEBUG('v', "Pág. %d está sucia\n", page );
			//fromSwap(page);
			updateTLB(page);
		} else {					// está limpia
			DEBUG('v', "Pág. %d está limpia\n", page );
			if ( inFile(page) ) {	// está en el archivo
				DEBUG('v', "Pág. %d viene del archivo\n", page );
				fromFile(page);
				updateTLB(page);
			} else {
				DEBUG('v', "Pág. %d de ceros\n", page );
				//bzero(&(machine->mainMemory[memAddr], PageSize);
				//valid
				updateTLB(page);
			}
		}
	}
}

bool AddrSpace::inFile(int page) {
	OpenFile *executable = fileSystem->Open(file);
	if (executable == NULL) {
		printf("Unable to open file %s\n", file);
		return -1;
	}

	NoffHeader noffH;
	executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) &&
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
		SwapHeader(&noffH);
	ASSERT(noffH.noffMagic == NOFFMAGIC);

	unsigned int size = noffH.code.size + noffH.initData.size;
	return page < divRoundUp(size, PageSize);
}
