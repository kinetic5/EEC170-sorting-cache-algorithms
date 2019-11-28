.text

.macro exit ()
.text
li a7,10
ecall
.end_macro

.macro print_str (%str)
.data
str_label: .string %str
.text
li a7, 4
la x10, str_label
ecall
.end_macro

# to avoid unwanted values creeping in
.macro reg_reset()
.text
andi a3, a3, 0
andi a4, a4, 0
andi a5, a5, 0
andi a6, a6, 0
andi a7, a7, 0
andi t0, t0, 0
andi t1, t1, 0
andi t2, t2, 0
andi t3, t3, 0
andi t4, t4, 0
andi t5, t5, 0
andi t6, t6, 0
andi s0, s0, 0
andi s1, s1, 0
andi s2, s2, 0
andi s3, s3, 0
andi s4, s4, 0
andi s5, s5, 0
andi s6, s6, 0
andi s7, s7, 0
andi s8, s8, 0
andi s9, s9, 0
andi s10, s10, 0
andi s11, s11, 0
.end_macro

# macro for function call
# store the necessary registers before 
# function call and clear everthing else
.macro func_call(%func)
addi sp, sp, -24
sw t0, 0(sp)
sw t1, 4(sp)
sw t2, 8(sp)
sw t3, 12(sp)
sw t4, 16(sp)
sw t5, 20(sp)
reg_reset()
call %func
lw t0, 0(sp)
lw t1, 4(sp)
lw t2, 8(sp)
lw t3, 12(sp)
lw t4, 16(sp)
lw t5, 20(sp)
addi sp, sp, 24
.end_macro

.globl main
main:
	print_str("Reading inputs.......\n")
	call file_read
	mv t0, a3 # starting address of unsorted data
	mv t5, a4 # end address of unsorted data
	print_str("Finished reading.....")
	
	li t1, 0x10011800 # output space for bubble sort
	li t2, 0x10012900 # output space for merge sort
	li t3, 0x10014000 # output space for quick sort
	li t4, 0x10015100 # scratch space
	
	# to call buble sort
	mv a0, t1 
	mv a1, t0
	mv a2, t4
	func_call(bubble)
	
	# to call merge sort
	mv a0, t2
	mv a1, t0
	mv a2, t4
	func_call(merge)
	
	# to call quick sort
	mv a0, t3
	mv a1, t0
	mv a2, t4
	func_call(quick) 
	
	exit()
