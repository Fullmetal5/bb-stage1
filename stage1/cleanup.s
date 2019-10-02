
# We need to
#   1. Disable interrupts
#   2. Move the loader to it's proper location
#   3. Sync caches
#   4. Jump to it


.set loader_start, 0x90000020
.set loader_size, 0x80000 # 512KB
.set stub_size, (_end - _start)


	.globl _start
_start:
	
	# 1. Disable interrupts and enable FP
	mfmsr 3 ; rlwinm 3,3,0,17,15 ; ori 3,3,0x2000 ; mtmsr 3 ; isync
	
	bl _getpc
_getpc:
	mflr 3
	subi 31, 3, (_getpc - _start)
	
	# 2. Move the loader
	lis 0, ((loader_size - stub_size) / 4)@h ; ori 0, 0, ((loader_size - stub_size) / 4)@l
	addi 3, 31, (stub_size - 4)
	lis 4, (loader_start - 4)@h ; ori 4, 4, (loader_start - 4)@l
	
	mtctr 0
0:	lwzu 0, 4(3) ; stwu 0, 4(4) ; bdnz 0b
	
	# 3. Sync caches
	lis 3, loader_start@h ; ori 3, 3, loader_start@l
	lis 4, (loader_size - stub_size)@h ; ori 4, 4, (loader_size - stub_size)@l
	# 3 = location, 4 = size to flush in bytes
	li 5, 31
	rlwinm 3, 3, 0, 0, 26
	add 4, 4, 5
	srwi 4, 4, 5
	mtctr 4
1:	dcbst 0, 3
	sync
	icbi 0, 3
	addi  3, 3, 32
	bdnz 1b
	sync
	isync
	
	# 4. Jump
	lis 3, loader_start@h ; ori 3, 3, loader_start@l
	mtctr 3
	bctr
	
_end:
