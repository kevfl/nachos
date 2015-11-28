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
/*
	Se inicializan todas las páginas del pageTable inválidas cuando se usa 
	VirtualMemory(VM).
*/
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
/*
	Se elimina la parte de carga del archivo a disco, a fin de
	que no se cargue ninguna página a memoria cuando se usa el 
	VirtualMemory(VM).
*/
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
/*
	Se guarda en la TLB el estado de la página que esta en memoria. 
*/
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

void AddrSpace::RestoreState(){
/*
	Reinicia el estado de la TBL, que se ocupa que este inválido.
*/
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

/*
	Método setFilname, recibe como parámetros const chart* filename, int len.
	Nombra el archivo con el nombre que nos pasan.
	Se modifica *file de addrspace.
*/
void AddrSpace::setFilename(const char *filename, int len) {
	file = new char[len+1];
	strcpy(file, filename);
	DEBUG('v', "Ejecutable: %s\n", file );
}

/*
	Método updateTLB, recibe como parámetro un int page.
	Guarda en el TBL la página 'page' que esta en memoria.
	Modifica la TLB en la dirección dada por page.
*/
void AddrSpace::updateTLB(int page) {
	// Actualiza pageTable antes de desalojar página del TLB
	if ( machine->tlb[ pTLB ].valid ) {								//	Conflict misses
		pageTable[ machine->tlb[ pTLB ].virtualPage ].use = machine->tlb[ pTLB ].use;
		pageTable[ machine->tlb[ pTLB ].virtualPage ].dirty = machine->tlb[ pTLB ].dirty;
	}
	DEBUG('v', "En posición %d del TLB copio pageTable[%d]\n", pTLB, page );
	machine->tlb[ pTLB ].virtualPage = pageTable[ page ].virtualPage;
	machine->tlb[ pTLB ].physicalPage = pageTable[ page ].physicalPage;
	machine->tlb[ pTLB ].valid = true;
	machine->tlb[ pTLB ].dirty = pageTable[ page ].dirty;
	machine->tlb[ pTLB ].use = pageTable[ page ].use;
	machine->tlb[ pTLB ].dirty = pageTable[ page ].dirty;
	machine->tlb[ pTLB ].readonly = pageTable[ page ].readonly;
	//DEBUG('v', "\tvirtualPage = %d\n", pageTable[ page ].virtualPage );
	//DEBUG('v', "\tphysicalPage = %d\n", pageTable[ page ].physicalPage );
	//DEBUG('v', "\tdirty = %d\n", pageTable[ page ].dirty );
	pTLB = (pTLB + 1) % TLBSize;
}

/*
	Método getPage, recibe como parámetro int page.
	Busca si existe una posición de memoria libre. De no encontrar
	tal posición en memoria, manda al Swap la página *pSwap y aumenta dicho
	puntero.
	Devuelve una página libre.
*/
int AddrSpace::getPage() {
	int next = memMap->Find();
	if (next == -1) {
		// second chance
		//pageTable[tinv[pMem]].dirty = true;
		//if dirty toSwap(pMem);
		next = pMem;
		pMem = (pMem + 1) % NumPhysPages;
	}
	DEBUG('v', "Entregada la pág %d\n", next );
	// update TPI
	return next;
}

/*
	Método fromFile, recibe como parámetro una página.
	Si es un archivo válido, modifica el pageTable en la página 
	que se manda como parámetro, además carga dicha página memoria.
	Si es un archivo valido, modifica el pageTable y la memoria principal.
*/
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
	
	int physicalPage = getPage();
	DEBUG('v', "physicalPage %d\n", physicalPage );
	pageTable[ page ].physicalPage = physicalPage;
	
	int filAddr = noffH.code.inFileAddr + pageTable[page].virtualPage * PageSize;
	int memAddr = /*noffH.code.virtualAddr +*/ physicalPage * PageSize;
	DEBUG('v', "Escribe en RAM at %d. Inicio %d\n", memAddr, noffH.code.virtualAddr );
	if (noffH.code.size > 0) {
		executable->ReadAt(&(machine->mainMemory[memAddr]), PageSize, filAddr);
	}
}

/*
	Metodo getBlank, recibe como parámetro una página.
	Obtiene una página, la rellena de ceros. Sirve para pila y datos no
	inicializados.
	Modifica dicha página.
*/
void AddrSpace::getBlank(int page) {
	pageTable[page].valid = true;

	int physicalPage = getPage();
	DEBUG('v', "physicalPage %d\n", physicalPage );
	pageTable[ page ].physicalPage = physicalPage;

	int memAddr = /*noffH.code.virtualAddr +*/ physicalPage * PageSize;
	bzero(machine->mainMemory + memAddr, PageSize);
}

/*
	Método Load, recibe como parámetro una página.
	Implementa tabla de decisión para cargar la página en el TLB. Se encarga
	de controlar el pageTable con sus posibles casos: que la página 
	este sucia, vacía, válida o inválida.
	Modifica según el caso la pageTable, la página que esta en memoria,
	o la memoria.
*/
void AddrSpace::load(int page) {
	if (pageTable[page].valid) {			// La página está en memoria.
		DEBUG('v', "\tPág. %d está en la memoria\n", page );
		updateTLB(page);
	} else {								// La página no está en la memoria.
		DEBUG('v', "\tPág. %d no está en la memoria\n", page );
		if (pageTable[page].dirty) {		// La página está sucia.
			DEBUG('v', "\tPág. %d está sucia\n", page );				//	Capacity missis
			//fromSwap(page);
			updateTLB(page);
		} else {							// La página está limpia.
			DEBUG('v', "\tPág. %d está limpia\n", page );
			if ( inFile(page) ) {			// Proviene de un archivo.
				DEBUG('v', "\tPág. %d viene del archivo\n", page );		//	Compulsory misses
				fromFile(page);
				updateTLB(page);
			} else {						// Es una página en blanco.
				DEBUG('v', "\tPág. %d de ceros\n", page );				//	Compulsory misses
				getBlank(page);
				updateTLB(page);
			}
		}
	}
}

/*
	Método inFile, recibe como parámetro un int page.
	Función que indica si la página existe o no en el archivo.
	Devuelve un booleano con la respuesta.
*/
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
