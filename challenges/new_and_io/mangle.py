'''
GLibC Memory Protection Functions. 

These are meant to handle the pointer mangling features that were introducted into GLibC 2.32 via the research done by Checkpoint.
'''

import fire 
import sys 

# Mangle the ptr
def encode_ptr(fd_ptr, storage_location, print_hex=False):
	if(print_hex):
		return hex((storage_location >> 12) ^ fd_ptr)	
	return (storage_location >> 12) ^ fd_ptr


# Demangle the ptr
def decode_ptr(mangled_ptr,storage_location, print_hex=False):
	if(print_hex):
		return hex((storage_location >> 12) ^ mangled_ptr)
	return (storage_location >> 12) ^ mangled_ptr


# Unmangle a pointer while only knowing the value of the mangled_ptr
'''
The ptr is stored in the following way: 
- (Bin Ptr) ^ (Storage Location >> 12)

Two features make this possible to predict: 
- Relative closeness to the pointers in question 
- The 12 bit shift

By using the following equation, we can determine the value 
and location of the ptr. 
The equation makes ONE assumption: The nibbles of pointers are the same. 

Equation
==================

1. First, we KNOW the top 12 bits of the program. Because of 
the shift, the bits of the Storage Location Pointer are given 
to us for free.

2. Next, we xor this value with the next (to the right) 12 bits. 
This outputs the value of the 12 bits of the Fd Pointer.

3. Repeat until we are at the end of the program. 
The 'known bits' are the bits outputted in step 2 for the
next iteration.


Parameters: 
==============
value: The pointer to be demangled 
loc_final_bits (optional) The final twelve bits of the location 
       pointer. These are needed because the least significant 
       twelve bits are lost when shifting.

Returns: 
=============
Ptr: The pointer than is being stored 
Location: The location where the pointer is being stored.
'''
def recover_ptrs(mangled_ptr, loc_final_bits=0x0, print_hex=False):

	count = 0x0 	
	tmp_value = mangled_ptr
	while(tmp_value & 0xFFFFFF000 != 0x0):
		tmp_value = tmp_value >> 4
		count +=1 

	# Get the top-most 12 bits to initialize the process
	initial = mangled_ptr & (0xFFF * (0x10 ** count))
	final_ptr = initial
	final_location = initial	
	known_bits = initial >>  (count * 4) 

	for iteration in range(1, (count/3) + 1):

		exp_amount = (count - (3 * iteration))	
		shift_amount = (count - (3 * iteration)) * 4

		# Get the 12 bits to the right of the top-most 12 bits of the value.
		tmp_value = mangled_ptr & (0xFFF * (0x10 ** exp_amount))

		# Shift the values over. Then, operate on them.

		ptr_shift = tmp_value >> shift_amount

		# Operate on the bits in order to get the ptr bits at the specific location.
		known_bits = ptr_shift ^ known_bits
		
		# Add the new known_bits to the total for the final_ptr
		new_bits = known_bits << shift_amount

		# The 'known_bits' are the location in which the ptr was stored at that we are getting.
		final_ptr = final_ptr + new_bits


	# The least significant twelve bits are unknown for the storage location. So, we remove them.
	final_location =  final_location & 0xFFFFFFFFFFFFF000
	
	# If the final bits of the location are given, we add them back in.
	if(loc_final_bits):
		final_location += (loc_final_bits & 0xFFF)

	if(print_hex):
		final_ptr = hex(final_ptr) 
		final_location = hex(final_location)
	return final_ptr, final_location

# Just an example
def showcase():
	print ""
	print "Demo"
	print "======================================"
	print ""
	ptr = 0x987654321
	loc = 0x987654987
	print "Starting Out"
	print "=================="
	print "Pointer: ", hex(ptr) 
	print "Storage Location: ", hex(loc)

	print "\n\nMangling the Pointers"
	print "======================"
	leaked_ptr = encode_ptr(ptr, loc)
	print "Mangled Pointer: ", hex(leaked_ptr)
	print "Demangle Pointer (manually): ", hex(decode_ptr(leaked_ptr,loc))

	print "\n\nUnmangle the Pointers with Magic Algorithm"
	print "=================================="
	fixed_ptr, fixed_loc = recover_ptrs(leaked_ptr, 0x0, loc & 0xFFF)
	print "Found Pointer: ", hex(ptr)
	print "Found Location: ", hex(loc)

if __name__ == "__main__":
	fire.Fire()
