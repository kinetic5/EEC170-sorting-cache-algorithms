.globl bubble

.eqv LENGTH 1023

.eqv n_ t0
.eqv newn t1
.eqv i_ t2
.eqv tmp1 t3
.eqv tmp2 t4
.eqv tmp3 t5
.eqv tmp4 t6

# Inputs:
#   a0 -> out_addr
#   a1 -> in_addr
#   a2 -> temp_addr
bubble:

    # Move unsorted data to output address
    li t0, 4
    li t1, LENGTH
    mul t1, t1, t0 # t1 = LENGTH * 4
    add t0, a0, t1 # t0 = a0 + (LENGTH * 4)
    add t1, a1, t1 # t1 = a1 + (LENGTH * 4)

    bubble_copy_unsorted:
        lw t3, 0(t1)
        sw t3, 0(t0)
        addi t0, t0, -4
        addi t1, t1, -4
    bge t1, a1, bubble_copy_unsorted

    li n_, LENGTH
    bubble_repeat:
        li newn, 0 # newn = 0
        li i_, 1 # i_ = 1
        bubble_for:
            addi tmp1, x0, 4
            mul tmp1, tmp1, i_
            add tmp1, tmp1, a0 #tmp1 = *A[i]
            lw tmp2, 0(tmp1) # tmp2 = A[i]
            lw tmp3, -4(tmp1) # tmp3 = A[i - 1]

            ble tmp3, tmp2, bubble_for_contine

            sw tmp3, 0(tmp1) # Swap
            sw tmp2, -4(tmp1)
            mv newn, i_ # newn = i

        bubble_for_contine:
        addi i_, i_, 1
        ble i_, n_, bubble_for # if i_ <= n_ then for

        mv n_, newn # n = newn
        li tmp1, 1
        ble n_, tmp1, bubble_ret # if n_ <= 1 then bubble_ret
        beq x0, x0, bubble_repeat
        bubble_ret:
        ret
