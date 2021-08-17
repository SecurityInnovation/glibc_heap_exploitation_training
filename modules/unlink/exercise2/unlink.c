#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

/*
Compile: 
 - gcc unlink.c -g -o unlink

Afterwards use the proper libc and patchelf the interpreter for the loader :)

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

int offset = 0;
// Making heap and stdin/stdout easier to deal with 
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

	// Chunks are of size 0x90. NOTE: Go into unsorted bin, if freed.
	char* shellcode = malloc(0x80); 	
	char* unlink_chunk = malloc(0x80);
	char* free_victim = malloc(0x80);

	// Want to make the heap executable, to make life more fun :) 
	mprotect(shellcode - offset - 0x10, 0x21000, PROT_READ | PROT_WRITE | PROT_EXEC);

	printf("Put Shellcode at %p\n", shellcode);
	printf("Unlink Chunk: %p\n", unlink_chunk);
	printf("Free Victim: %p\n", free_victim);

	// Yes, you can write NULLbytes to these locations
	puts("Enter Shellcode:");
	fgets(shellcode, 0x80, stdin);  // Shellcode here

	puts("Enter unlink payload:");
	fgets(unlink_chunk, 0x90, stdin); // <-- Unlink attack payload, first breakpoint here!

	// Start the unlinking, if you dare :) 
	free(free_victim); // second breakpoint here!

	puts("dynamite failed?\n");
}
