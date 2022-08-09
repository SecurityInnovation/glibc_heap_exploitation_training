'''
Launch script for the POC. Can be used for the following: 
- Loading the proper version of LibC and Loader with the POC 
- Easier debugging (in particular, setting breakpoints) 


Mmap LEAKLESS Chunk Overwrites 
;tldr: Rewrite symbol table of LibC to get code execution

Notes: 
- This POC should work on all versions of libc. But, the LibC 
  specific version for this is 2.31. 
- This technique only works if no or partial RELRO is used 
  with a dynamicly linked binary. 
- Full RELRO does not do LAZY dynamic symbol resolution so the technique
  does not work with full relro enabled. 

Steps in the code (POC in munmap_rewrite.c): 
- Buffer overflow (vulnerability) 
- Munmap LibC (.gnu.hash and .dynsym) 
- Allocate over LibC with mmap
- Rewrite string hashing and symbol table
- Pop shell :) 
'''

from pwn import * 
import os

mode = 'DEBUG' # Turn on gdb for this
libc_name = './2.31/libc-2.31.so' # Set LibC vesion
env = {}

# Binary setup
elf_name = './munmap_rewrite' 

elf = ELF(elf_name)
if libc_name != '': 
	libc = ELF(libc_name)
	env = {"LD_PRELOAD": libc.path}

	
p = gdb.debug([ elf.path],env=env)


p.interactive()


