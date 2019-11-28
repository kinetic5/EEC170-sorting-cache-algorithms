.globl quick

.eqv LENGTH 1023 # n - 1

.eqv in_addr s0
.eqv low s1
.eqv high s2
.eqv p s3

.eqv array s4
.eqv pivot s5
.eqv i_ s6
.eqv j_ s7

.eqv tmp t0

# Inputs:
#   a0 -> out_addr
#   a1 -> in_addr
#   a2 -> temp_addr
quick:
    addi sp, sp, -4
    sw ra, 0(sp)

    # Move unsorted data to output address
    li t0, 4
    li t1, LENGTH
    mul t1, t1, t0 # t1 = LENGTH * 4
    add t0, a0, t1 # t0 = a0 + (LENGTH * 4)
    add t1, a1, t1 # t1 = a1 + (LENGTH * 4)

    quick_copy_unsorted:
        lw t3, 0(t1)
        sw t3, 0(t0)
        addi t0, t0, -4
        addi t1, t1, -4
    bge t1, a1, quick_copy_unsorted

    # Perform quicksort
    li a1, 0
    li a2, LENGTH
    call quicksort

    lw ra, 0(sp)
    addi sp, sp, 4
    ret

# Inputs:
#   a0 -> in_addr
#   a1 -> low
#   a2 -> high
quicksort:
    addi sp, sp, -20
    sw ra, 0(sp)
    sw in_addr, 4(sp)
    sw low, 8(sp)
    sw high, 12(sp)
    sw p, 16(sp)

    mv in_addr, a0
    mv low, a1
    mv high, a2

    blt low, high, quicksort_if_true # if low < high
    beq x0, x0, quicksort_ret

    quicksort_if_true:

    mv a0, in_addr
    mv a1, low
    mv a2, high
    call partition
    mv p, a0

    mv a0, in_addr
    mv a1, low
    mv a2, p
    call quicksort

    mv a0, in_addr
    addi a1, p, 1
    mv a2, high
    call quicksort

    quicksort_ret:
    lw ra, 0(sp)
    lw in_addr, 4(sp)
    lw low, 8(sp)
    lw high, 12(sp)
    lw p, 16(sp)
    addi sp, sp, 20
ret

# Inputs:
# a0 -> array
# a1 -> low
# a2 -> high
partition:
    addi sp, sp, -28
    sw ra, 0(sp)
    sw array, 4(sp)
    sw low, 8(sp)
    sw high, 12(sp)
    sw pivot, 16(sp)
    sw i_, 20(sp)
    sw j_, 24(sp)

    mv array, a0 # Move inputs into saved registers
    mv low, a1
    mv high, a2

    sub pivot, high, low # pivot = high - low
    srli pivot, pivot, 1 # pivot /= 2
    add  pivot, pivot, low # pivot += low
    addi tmp, x0, 4
    mul pivot, pivot, tmp # pivot *= 4
    add pivot, pivot, array # pivot += *array
    lw pivot, 0(pivot) # pivot = array[low + (high - low) / 2]

    addi i_, low, -1 # i = low - 1
    addi j_, high, 1 # j = high + 1

    partition_loop:

        partition_inc_i:
            addi i_, i_, 1 # i += 1
            addi tmp, x0, 4
            mul tmp, tmp, i_
            add tmp, tmp, array
            lw tmp, 0(tmp) # tmp = A[i]
            blt tmp, pivot, partition_inc_i # A[i] < pivot


        partition_inc_j:
            addi j_, j_, -1 # j -= 1
            addi tmp, x0, 4
            mul tmp, tmp, j_
            add tmp, tmp, array
            lw tmp, 0(tmp) # tmp = A[j]
            bgt tmp, pivot, partition_inc_j # A[j] > pivot
        
        bge i_, j_, partition_ret # i >= j

        #swap
        mv a0, array
        mv a2, array
        mv a1, i_
        mv a3, j_
        call swap

        beq x0, x0, partition_loop

    partition_ret:
    mv a0, j_ # Set the output

    lw ra, 0(sp)
    lw array, 4(sp)
    lw low, 8(sp)
    lw high, 12(sp)
    lw pivot, 16(sp)
    lw i_, 20(sp)
    lw j_, 24(sp)
    addi sp, sp, 28
    ret


# Inputs: a0 -> base1, a1 -> index1, a2 -> base2, a3 -> index2
# Outputs: none
swap:
    addi t3, x0, 4
    mul t3, t3, a1
    add t3, t3, a0
    lw t4, 0(t3)

    addi t5, x0, 4
    mul t5, t5, a3
    add t5, t5, a2
    lw t6, 0(t5)

    sw t4, 0(t5)
    sw t6, 0(t3)
    ret
