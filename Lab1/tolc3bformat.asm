		.ORIG x3000
A		.FILL x4000
B		.FILL x4002
C		.FILL x0000
		LEA R1, A
		LDW R1, R1, #0 ; so R1 now contains x4000, and mem of x4000 has starting address 
		LDW R1, R1, #0 ; so R1 now contains the starting address for the input string
		LEA R2, B
		LDW R2, R2, #0 ; so R2 now contains x4002
		LDW R2, R2, #0 ; so R2 now contains starting output address
	
MORE	LDB R3, R1, #0 ; loads the byte from input string into R3
		BRz DONE
		STB R3, R2, #0 ; stores that byte into the memory of the address in R2
		ADD R1, R1, #2 ; shift the pointer for word-separated data by 2
		ADD R2, R2, #1 ; shift the pointer for byte-separated, new data by 1
		BR MORE
DONE	STB R3, R2, #0 ; add the final byte termination for output string. note that i'm not sure about what the correct behavior should be if the input string is simply 0/