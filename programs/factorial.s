    .friday_asm

    in_f
    cf2i # read float and convert it to int in order to demonstrate it is working )))
    call factorial
    out
    end

factorial:
    pop r0 # Push ret address
    pop r1 # arg int n

    # Check if r0 == 1
    push r1
    push 1
    ja factorial_internal
    push 1
    push r0 # Push addr to return
    ret

factorial_internal:
    # save regs before call
    push r0
    push r1

    # call factorial(r1 - 1)
    push r1
    push 1
    sub
    call factorial

    # mul result and restore regs
    mul
    pop r1
    pop r0

    # ret
    push r1
    push r0
    ret
