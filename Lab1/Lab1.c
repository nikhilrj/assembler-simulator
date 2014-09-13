#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

#define MAX_LINE_LENGTH 255

const char* opCodeList[] = { "add", "and", "brn", "brp", "brnp", "br", "brz", "brnz", "brzp", "halt", "jmp", "jsr", "jsrr", "ldb", "ldw", "lea", "nop", "not", "ret", "lshf", "rshfl", "rshfa", "rti", "stb", "stw", "trap", "xor" };
char* SymbolList[MAX_LINE_LENGTH];
int SymbolAddresses[MAX_LINE_LENGTH];

enum
{
	DONE, OK, EMPTY_LINE
};

int isOpcode(char* opCode)
{
	/*TODO: finish function, it's messing up on BRZ / BRNZP etc */
	int i;
	for (i = 0; i < sizeof(opCodeList) / sizeof(char*); i++)
	{
		if (strcmp(opCodeList[i], opCode) == 0)
			return 1;
	}

	return -1;
}

int readAndParse(FILE * pInfile, char * pLine, char ** pLabel, char ** pOpcode, char ** pArg1, char ** pArg2, char ** pArg3, char ** pArg4)
{
	char * lRet, *lPtr;
	int i;
	if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
		return(DONE);
	for (i = 0; i < strlen(pLine); i++)
		pLine[i] = tolower(pLine[i]);

	/* convert entire line to lowercase */
	*pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);
	/* ignore the comments */
	lPtr = pLine;
	while (*lPtr != ';' && *lPtr != '\0' &&
		*lPtr != '\n')
		lPtr++;
	*lPtr = '\0';
	if (!(lPtr = strtok(pLine, "\t\n ,")))
		return(EMPTY_LINE);
	if (isOpcode(lPtr) == -1 && lPtr[0] != '.') /* found a label */
	{
		*pLabel = lPtr;
		if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	}

	*pOpcode = lPtr;
	if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);

	*pArg1 = lPtr;

	if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	*pArg2 = lPtr;
	if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	*pArg3 = lPtr;
	if (!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	*pArg4 = lPtr;
	return(OK);
}

int toNum(char * pStr)
{
	char * t_ptr;
	char * orig_pStr;
	int t_length, k;
	int lNum, lNeg = 0;
	long int lNumLong;

	orig_pStr = pStr;
	if (*pStr == '#')				/* decimal */
	{
		pStr++;
		if (*pStr == '-')				/* dec is negative */
		{
			lNeg = 1;
			pStr++;
		}
		t_ptr = pStr;
		t_length = strlen(t_ptr);
		for (k = 0; k < t_length; k++)
		{
			if (!isdigit(*t_ptr))
			{
				printf("Error: invalid decimal operand, %s\n", orig_pStr);
				exit(4);
			}
			t_ptr++;
		}
		lNum = atoi(pStr);
		if (lNeg)
			lNum = -lNum;

		return lNum;
	}
	else if (*pStr == 'x')	/* hex     */
	{
		pStr++;
		if (*pStr == '-')				/* hex is negative */
		{
			lNeg = 1;
			pStr++;
		}
		t_ptr = pStr;
		t_length = strlen(t_ptr);
		for (k = 0; k < t_length; k++)
		{
			if (!isxdigit(*t_ptr))
			{
				printf("Error: invalid hex operand, %s\n", orig_pStr);
				exit(4);
			}
			t_ptr++;
		}
		lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
		lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;
		if (lNeg)
			lNum = -lNum;
		return lNum;
	}
	else
	{
		printf("Error: invalid operand, %s\n", orig_pStr);
		exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
	}
}

int isRegister(char* reg)
{
	if (reg[0] == 'r' && (reg[1] >= '0' && reg[1] <= '7') && strlen(reg) == 2)
		return 1;
	return -1;
}

int decodeRegister(char* reg)
{
	/*decodes R7 -> int(7), etc*/
	return reg[1] - '0';

}

int add(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*Incorrect number of instructions!*/

	if (isRegister(lArg3) == 1)
		return (1 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + decodeRegister(lArg3);
	else
	{
		int imm5 = toNum(lArg3);
		if (imm5 > 15 || imm5 < -16)
			exit(3); /*Invalid immediate */
		else
			return (1 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (1 << 5) + (imm5&0x1f);

	}
}
int and(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*Incorrect number of instructions!*/

	if (isRegister(lArg3) == 1)
		return (5 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + decodeRegister(lArg3);
	else
	{
		int imm5 = toNum(lArg3);
		if (imm5 > 15 || imm5 < -16)
			exit(3); /*Invalid immediate */
		else
			return (5 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (1 << 5) + (imm5 & 0x1f);

	}
}
int brn(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int brp(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int brnp(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int br(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int brz(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int brnz(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int brzp(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int halt(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int jmp(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int jsr(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int jsrr(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int ldb(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int ldw(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int lea(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int nop(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int not(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int ret(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int lshf(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int rshfl(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int rshfa(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int rti(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int stb(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int stw(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int trap(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}
int xor(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0;
}

int main(int argc, char* argv[]) {
	char *prgName = NULL;
	char *iFileName = NULL;
	char *oFileName = NULL;
	int symbolCounter = 0;
	int orig;
	int lineCounter = -1;

	prgName = argv[0];
	iFileName = argv[1];
	oFileName = argv[2];

	char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
	/* lLabel[i] */
	/* *(lLabel+i)*/
	int lRet;
	FILE * lInfile;

	/*PASS ONE: GENERATE SYMBOL TABLE*/
	lInfile = fopen(iFileName, "r");	/* open the input file */

	do
	{
		lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
		if (lRet != DONE && lRet != EMPTY_LINE)
		{
			if (strcmp(lOpcode, ".end") == 0) break; 

			if (strcmp(".orig", lOpcode) == 0)
			{
				orig = toNum(lArg1);
			}
			if (lLabel != NULL && strcmp(lLabel, "") != 0)
			{
				if (strcmp(lLabel, "getc") == 0 || strcmp(lLabel, "in") == 0 || strcmp(lLabel, "out") == 0 || strcmp(lLabel, "puts") == 0 || lLabel[0] == 'x')
					exit(4); /*exit because the label is not allowed to be getc,in,out, or puts*/

				if (lLabel[0] >= '0' && lLabel[0] <= '9')
					exit(4);

				for (int j = 0; j<strlen(lLabel); j++)
				{
					if (isalnum(lLabel[j]) == 0)
						exit(4); /*exit because of non-alphanumeric char*/
				}

				int labelAlreadyExists = -1;
				for (int n = 0; n < symbolCounter; n++)
				{
					if (strcmp(lLabel, SymbolList[n]) == 0)
						labelAlreadyExists = 1;
				}

				if (labelAlreadyExists == -1)
				{
					SymbolList[symbolCounter] = (char*)malloc(MAX_LINE_LENGTH);/*lLabel*/
					strcpy(SymbolList[symbolCounter], lLabel);
					SymbolAddresses[symbolCounter] = orig + lineCounter;
					symbolCounter++;
				}
				else
				{
					exit(4); /*exit because the label has appeared twice in the assembly program*/
				}
			}

			lineCounter++;
		}
	} while (lRet != DONE);

	fclose(iFileName);


	/*PASS TWO: ASSEMBLE*/
	lineCounter = -1; /*when lineCounter == 0, it is the first instruction*/
	lInfile = fopen(iFileName, "r");	/* open the input file */
	do
	{
		int result = 0;
		lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
		if (lRet != DONE && lRet != EMPTY_LINE)
		{
			if (strcmp(lOpcode, ".orig") == 0) result = orig;
			if (strcmp(lOpcode, ".end") == 0) break; 

			if (strcmp(lOpcode, ".fill") == 0) { if (strcmp(lArg2, "") != 0) exit(4); result = toNum(lArg1); } else
			if (strcmp(lOpcode, "add") == 0) { result = add(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
			else
				if (strcmp(lOpcode, "and") == 0) { result = and(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
				else
					if (strcmp(lOpcode, "brn") == 0) { result = brn(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
					else
						if (strcmp(lOpcode, "brp") == 0) { result = brp(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
						else
							if (strcmp(lOpcode, "brnp") == 0) { result = brnp(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
							else
								if (strcmp(lOpcode, "br") == 0) { result = br(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
								else
									if (strcmp(lOpcode, "brz") == 0) { result = brz(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
									else
										if (strcmp(lOpcode, "brnz") == 0) { result = brnz(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
										else
											if (strcmp(lOpcode, "brzp") == 0) { result = brzp(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
											else
												if (strcmp(lOpcode, "halt") == 0) { result = halt(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
												else
													if (strcmp(lOpcode, "jmp") == 0) { result = jmp(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
													else
														if (strcmp(lOpcode, "jsr") == 0) { result = jsr(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
														else
															if (strcmp(lOpcode, "jsrr") == 0) { result = jsrr(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
															else
																if (strcmp(lOpcode, "ldb") == 0) { result = ldb(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																else
																	if (strcmp(lOpcode, "ldw") == 0) { result = ldw(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																	else
																		if (strcmp(lOpcode, "lea") == 0) { result = lea(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																		else
																			if (strcmp(lOpcode, "nop") == 0) { result = nop(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																			else
																				if (strcmp(lOpcode, "not") == 0) { result = not(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																				else
																					if (strcmp(lOpcode, "ret") == 0) { result = ret(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																					else
																						if (strcmp(lOpcode, "lshf") == 0) { result = lshf(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																						else
																							if (strcmp(lOpcode, "rshfl") == 0) { result = rshfl(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																							else
																								if (strcmp(lOpcode, "rshfa") == 0) { result = rshfa(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																								else
																									if (strcmp(lOpcode, "rti") == 0) { result = rti(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																									else
																										if (strcmp(lOpcode, "stb") == 0) { result = stb(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																										else
																											if (strcmp(lOpcode, "stw") == 0) { result = stw(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																											else
																												if (strcmp(lOpcode, "trap") == 0) { result = trap(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
																												else
																													if (strcmp(lOpcode, "xor") == 0) { result = xor(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }

			printf("%s %s %s %s %s \t\t %#04x \t %#04x\n", lOpcode, lArg1, lArg2, lArg3, lArg4, lineCounter + orig, result);
			lineCounter++;
		}
	} while (lRet != DONE);
	
	getchar();
}
