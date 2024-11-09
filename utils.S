.section .text
.global memset

memset:
    cmp x2, #0 
    beq memset_end

set:
    strb w1, [x0], #1 //store w1 data at addr present in x0 and increment x0 by 1
    subs x2, x2, #1 //x2 = x2-1
    bne set

memset_end:
    ret