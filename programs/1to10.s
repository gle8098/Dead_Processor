	.friday_asm

	push 1
	pop r0

next:
	push r0
	push 10
	ja stop

	push r0
	push r0
	mul
	out
	push r0
	push 1
	add
	pop r0
	jmp next

stop:
	end
