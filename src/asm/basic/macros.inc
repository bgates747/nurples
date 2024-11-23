	; Title:	BBC Basic Interpreter - Z80 version
	;		Useful macros
	; Author:	Dean Belfield
	; Created:	12/05/2023
	; Last Updated:	11/06/2023
	;
	; Modinfo:
	; 11/06/2023:	Modified to run in ADL mode
	; 11/06/2024:   Make compatible with ez80asm by Brandon R. Gates

	MACRO EXREG	rp1, rp2
		PUSH	rp1 
		POP	rp2
	ENDMACRO 

	; MACRO ADD8U_DE	reg
		MACRO ADD8U_DE
		ADD	A, E 
		LD	E, A 
		ADC	A, D
		SUB	E
		LD	D, A 
	ENDMACRO 

	; MACRO ADD8U_HL	reg
	MACRO ADD8U_HL
		ADD	A, L 
		LD	L, A 
		ADC	A, H
		SUB	L
		LD	H, A 
	ENDMACRO 

	MACRO VDU	val
		LD	A, val
		CALL	OSWRCH
	ENDMACRO

	MACRO SET_GPIO	reg, val
		IN0	A, (reg)
		OR	val
		OUT0	(reg), A
	ENDMACRO

	MACRO RES_GPIO	reg, val
		PUSH	BC
		LD	A, val
		CPL
		LD	C, A
		IN0	A, (reg)
		AND	C
		OUT0	(reg), A
		POP	BC
	ENDMACRO
