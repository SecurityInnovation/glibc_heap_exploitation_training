#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

/*
The actual env has the following: 
- ASLR turned off 
- LibC with a removed security check on 'unlink' macro 

NOTE: 
- This file uses mprotect to make the heap executable.
*/

// Easy function to jump to for win :) 
void winner()
{
	system("/bin/bash");
}

// Making heap and stdin/stdout easier to deal with 
int offset = 0;

void* setup(){
	malloc(0x10); // Creates a 0x20 sized chunk
	offset +=  0x20; // For the metadata below and the data section
	setvbuf(stdout,0,2,0);
	setvbuf(stdin,0,2,0);
	clearenv();
}

int main(int argc, char **argv)
{
	setup();

	// Chunks are of size 0x90. Go into the unsorted bin
	char* shellcode = malloc(0x90 - 0x10); 	
	char* unlink_chunk = malloc(0x90 - 0x10);
	char* free_victim = malloc(0x90 - 0x10);

	// Want to make the heap executable, to make life more fun :) 
	mprotect(shellcode - offset - 0x10, 0x21000, PROT_READ | PROT_WRITE | PROT_EXEC);

	// Get the shellcode from the user (cannot contain \n)
	printf("Put Shellcode at %p\n", shellcode); 
	printf("Unlink Chunk: %p\n", unlink_chunk);
	printf("Free Victim: %p\n", free_victim);
	fgets(shellcode, 0x80, stdin); 

	// Free the middle chunk
	free(unlink_chunk); // First breakpoint here!

	// UAF on unsorted bin chunk (unlink_chunk) 	
	printf("Use after free write on 'unlink_chunk': ");
	fgets(unlink_chunk, 0x40, stdin); 

	// Trigger the consolidation with chunk 'unlink_chunk'
	free(free_victim); // Second breakpoint here!
	puts("dynamite failed?\n");
}
