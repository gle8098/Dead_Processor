    .friday_asm

    push 9.0
    pop r0

    push r0
    sqrt
    outf

    push r0
    sqrt
    sqrt
    outf

    push 0.015
    push r0
    mulf
    outf

    end
