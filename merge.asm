.globl merge

.eqv LENGTH 1023
.eqv HEAP_ADDR 0x10040000

.eqv array s0
.eqv start s1
.eqv mid s2
.eqv end s3
.eqv i_ s4
.eqv j_ s5
.eqv k_ s6
.eqv temp s7

.eqv hp s11

# Inputs:
#   a0 -> out_addr
#   a1 -> in_addr
#   a2 -> temp_addr
merge:

    addi sp, sp, -4
    sw ra, 0(sp)

    # Move unsorted data to output address
    li t0, 4
    li t1, LENGTH
    mul t1, t1, t0 # t1 = LENGTH * 4
    add t0, a0, t1 # t0 = a0 + (LENGTH * 4)
    add t1, a1, t1 # t1 = a1 + (LENGTH * 4)

    merge_copy_unsorted:
        lw t3, 0(t1)
        sw t3, 0(t0)
        addi t0, t0, -4
        addi t1, t1, -4
    bge t1, a1, merge_copy_unsorted

    # Perform mergesort
    li a1, 0
    li a2, LENGTH
    li hp, HEAP_ADDR
    call mergeSort

    lw ra, 0(sp)
    addi sp, sp, 4
    ret


# Inputs:
# a0 -> array
# a1 -> start
# a2 -> end
mergeSort:
    addi sp, sp, -20
    sw ra, 0(sp)
    sw array, 4(sp)
    sw start, 8(sp)
    sw mid, 12(sp)
    sw end, 16(sp)
    
    mv array, a0
    mv start, a1
    mv end, a2

    bge start, end, mergeSort_ret

        add mid, start, end
        srli mid, mid, 1

        mv a0, array
        mv a1, start
        mv a2, mid
        call mergeSort

        mv a0, array
        addi a1, mid, 1
        mv a2, end
        call mergeSort

        mv a0, array
        mv a1, start
        mv a2, mid
        mv a3, end
        call doMerge

    mergeSort_ret:
    lw ra, 0(sp)
    lw array, 4(sp)
    lw start, 8(sp)
    lw mid, 12(sp)
    lw end, 16(sp)
    addi sp, sp, 20
    ret


# Inputs:
#   a0 -> array
#   a1 -> start
#   a2 -> mid
#   a3 -> end
doMerge:
    addi sp, sp, -36
    sw ra, 0(sp)
    sw array, 4(sp)
    sw start, 8(sp)
    sw mid, 12(sp)
    sw end, 16(sp)
    sw i_, 20(sp)
    sw j_, 24(sp)
    sw k_, 28(sp)
    sw temp, 32(sp)

    mv array, a0
    mv start, a1
    mv mid, a2
    mv end, a3

    # Allocate scratch space
    mv temp, hp
    sub t0, end, start
    addi t0, t0, 1
    li t1, 4
    mul t0, t0, t1
    add hp, hp, t0

    mv i_, start
    addi j_, mid, 1
    add k_, x0, x0

    while_mid_end:
    bgt i_, mid, while_mid_end_break
    bgt j_, end, while_mid_end_break

        addi t0, x0, 4
        mul t0, t0, i_
        add t0, t0, array
        lw t0, 0(t0) # t0 = array[i]

        addi t1, x0, 4
        mul t1, t1, j_
        add t1, t1, array
        lw t1, 0(t1) # t1 = array[j]

        mv t3, temp
        addi t4, x0, 4
        mul t4, t4, k_
        add t4, t4, t3 # t4 = *temp[k]

        ble t0, t1, while_mid_end_if_true

            sw t0, 0(t4)
            addi i_, i_, 1
            addi k_, k_, 1

        beq x0, x0, while_mid_end
        while_mid_end_if_true:

            sw t1, 0(t4)
            addi j_, j_, 1
            addi k_, k_, 1

        beq x0, x0, while_mid_end
    while_mid_end_break:

    while_mid:
    bgt i_, mid, while_mid_break

        mv t3, temp
        addi t4, x0, 4
        mul t4, t4, k_
        add t4, t4, t3 # t4 = *temp[k]

        addi t5, x0, 4
        mul t5, t5, i_
        add t5, t5, array
        lw t5, 0(t5)

        sw t5, 0(t4)
        addi i_, i_, 1
        addi k_, k_, 1

    beq x0, x0, while_mid
    while_mid_break:

    while_end:
    bgt j_, mid, while_end_break

        mv t3, temp
        addi t4, x0, 4
        mul t4, t4, k_
        add t4, t4, t3 # t4 = *temp[k]

        addi t5, x0, 4
        mul t5, t5, j_
        add t5, t5, array
        lw t5, 0(t5)

        sw t5, 0(t4)
        addi j_, j_, 1
        addi k_, k_, 1

    beq x0, x0, while_end
    while_end_break:

    mv i_, start
    for_copy:
    bgt i_, end, doMerge_ret

        mv t3, temp
        addi t4, x0, 4
        sub t5, i_, start
        mul t4, t4, t5
        add t4, t4, t3 # t4 = *temp[i - start]
        lw t4, 0(t4)

        addi t5, x0, 4
        mul t5, t5, i_
        add t5, t5, array
        sw t4, 0(t5)

    addi i_, i_, 1
    beq x0, x0, for_copy
    doMerge_ret:

    lw ra, 0(sp)
    lw array, 4(sp)
    lw start, 8(sp)
    lw mid, 12(sp)
    lw end, 16(sp)
    lw i_, 20(sp)
    lw j_, 24(sp)
    lw k_, 28(sp)
    lw temp, 32(sp)
    addi sp, sp, 36
    ret