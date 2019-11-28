.data
fin:   .asciz "data.txt"
buffer: .align 2

.text
.globl file_read

file_read:
  ###############################################################
  # Open the file for reading
  li   a7, 1024     # system call for open file
  la   a0, fin    # output file name
  li   a1, 0        # Open for reading (flags are 0: read, 1: write)
  ecall             # open a file (file descriptor returned in a0)
  mv   s6, a0       # save the file descriptor
  ###############################################################
  # Write to file just opened
  li   a7, 63       # system call to read from the file
  mv   a0, s6       # file descriptor
  la   a1, buffer   # address of buffer to which we write
  li   a2, 10000     # hardcoded buffer length
  ecall             # write to file
  ###############################################################
  # Close the file
  li   a7, 57       # system call for close file
  mv   a0, s6       # file descriptor to close
  ecall             # close file
  ###############################################################

# data from the file is read as a ASCII chatater, this routine converts it into intergers
la s0, buffer
mv s3, s0
add s1, x0, x0
li s4, 1023
add s2, x0, s4
addi s5, x0, 57 # digit 9
# for loop which iters 1024 times
loop:
	addi t0, x0, 10 
	add  t1, x0, x0 
	addi t2, x0, 1  
	add  t3, x0, x0 # to hold number of digits in a number
	addi t4, x0, 10 # separator: '\n'
	
conv:	lbu t5, 0(s0)
	beq t5, t4, sep
	bgt t5, s5, garbage # anything greater than digit 9 is considered as garbage
	addi t6, x0, 48
	sub t5, t5, t6
	blt t5, x0, done
	mul t1, t1, t0
	add t1, t1, t5
	addi s0, s0, 1
	addi t3, t3, 1
	jal x0, conv

garbage:
	addi s0, s0, 1
	jal x0, conv		

sep:
	sw t1, 0(s3) # store the integer number
	addi s3, s3, 4 # next word address for store
	addi s0, s0, 1 # next byte
	addi s1, s1, 1 # loop increment
	ble s1, s2, loop
		
done:
	la a3, buffer
	mv a4, s3
	ret
