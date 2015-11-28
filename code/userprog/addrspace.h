// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch

	void setFilename(const char *filename, int len);	//  Nombra el archivo con el nombre que nos pasan.
	void load(int page);		//  Implementa tabla de decisión para cargar la página en el TLB.

  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual
					// address space
	char *file;									// Nombre del archivo.

	bool inFile(int page);	//	Indica si la página existe o no en el archivo.
	int getPage(int page);							//  Busca si existe una posición de memoria libre.
															//  Devuelve una página libre.
	void toSwap(int page);			//	Carga en el Swap, la página que se le indica.
	void fromSwap(int page);		//	Desde el swap, carga la página que se indica.

	void fromFile(int page);		//  Si es un archivo válido, modifica el pageTable en la página 
	void getBlank(int page);
															//  que se manda como parámetro, además carga dicha página memoria.
	void updateTLB(int page);		//  Guarda en el TBL la página 'page' que esta en memoria.
};

#endif // ADDRSPACE_H
