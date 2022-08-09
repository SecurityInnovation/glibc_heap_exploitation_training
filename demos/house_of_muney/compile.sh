#!/bin/bash 

# Deals with the elf patching and versioning for us

version=( 2.31 )
file_name=munmap_rewrite

echo "Compile: munmap_rewrite"

# Compile the code. Explicitly REMOVE 'now' linker flag to force lazy symbol resolution
$(gcc $file_name.c -o $file_name -ggdb -g -Wl,-z,norelro -z lazy -no-pie) 

loader=$(realpath $(pwd)/$version/ld-$version.so)

echo "Set loader to version $version at $loader" 
patchelf --set-interpreter $loader ./$file_name
