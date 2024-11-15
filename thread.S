.section .text

.global setup_thread

//setup the tls base register in tpidr_el1
setup_thread:
    mov	x1, xzr
    add	x1, x1, :tprel_hi12:current_thread
    add	x1, x1, :tprel_lo12_nc:current_thread
    sub	x1, x0, x1
    msr	TPIDR_EL1, x1
    ret
