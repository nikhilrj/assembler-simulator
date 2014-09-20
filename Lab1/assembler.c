/*
Name 1: Nikhil Joglekar
Name 2: Kevin Pham
UTEID 1: nrj328
UTEID 2: ktp364
*/

#include <stdio.h> /* standard input/output library */
#include <stdlib.h> /* Standard C Library */
#include <string.h> /* String operations library */
#include <ctype.h> /* Library for useful character operations */
#include <limits.h> /* Library for definitions of common variable type characteristics */

/* strcmp returns -1 if it is before alphabetically, 0 if it is equal, 1 if it is after*/


#define MAX_LINE_LENGTH 255

const char* opCodeList[] = { "add", "and", "brn", "brp", "brnp", "br", "brz", "brnz", "brzp", "brnzp", "halt", "jmp", "jsr", "jsrr", "ldb", "ldw", "lea", "nop", "not", "ret", "lshf", "rshfl", "rshfa", "rti", "stb", "stw", "trap", "xor" };
char* SymbolList[MAX_LINE_LENGTH];
int SymbolAddresses[MAX_LINE_LENGTH];
int symbolCounter = 0;

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
	char *lRet, *lPtr;
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

	if (isRegister(lArg1) != 1 || isRegister(lArg2) != 1)
		exit(4); /*Invalid register input*/

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

	if (isRegister(lArg1) != 1 || isRegister(lArg2) != 1)
		exit(4); /*Invalid register input*/

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
int branch(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4, int cc)
{
	int i;
	if (strcmp(lArg2, "") != 0)
		exit(4); /*Wrong number of operands*/
	
	/*CASE 1: IT IS A LABEL*/
	for (i = 0; i < symbolCounter; i++)
	{
		if (strcmp(SymbolList[i], lArg1) == 0)
		{
			int dist = (SymbolAddresses[i] - (pc + 1));
			if (dist > 255 || dist < -256)
				exit(4); /*Label is too far away!*/
			return (cc << 9) + (dist & 0x1FF);
		}
	}
	exit(1);
	if (lArg1[0] != '#' || lArg1[0] != 'x')
		exit(1); /*Label not found and is not a number, must be invalid label*/

	/*CASE 2: IT IS A NUMBER*/
	int pcOffset = toNum(lArg1);
	if (pcOffset > 255 || pcOffset < -256)
		exit(3); /*Invalid Constant*/
	return (cc << 9) + (pcOffset & 0x1FF);

}
int brn(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return branch(pc, lArg1, lArg2, lArg3, lArg4, 4);
}
int brp(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return branch(pc, lArg1, lArg2, lArg3, lArg4, 1);
}
int brnp(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return branch(pc, lArg1, lArg2, lArg3, lArg4, 5);
}
int br(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return branch(pc, lArg1, lArg2, lArg3, lArg4, 7);
}
int brnzp(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return branch(pc, lArg1, lArg2, lArg3, lArg4, 7);
}
int brz(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return branch(pc, lArg1, lArg2, lArg3, lArg4, 2);
}
int brnz(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return branch(pc, lArg1, lArg2, lArg3, lArg4, 6);
}
int brzp(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return branch(pc, lArg1, lArg2, lArg3, lArg4, 3);
}
int jmp(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{ 
	if (strcmp(lArg2, "") != 0)
		exit(4); /*Wrong number of operands*/
	
	if (isRegister(lArg1) == 1)
		return (12 << 12) + (decodeRegister(lArg1) << 6);
	else
		exit(4);
}
int jsr(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg2, "") != 0)
		exit(4); /*Wrong number of operands*/

	/*CASE 1: IT IS A LABEL*/
	int i;
	for (i = 0; i < symbolCounter; i++)
	{
		if (strcmp(SymbolList[i], lArg1) == 0)
		{
			int dist = (SymbolAddresses[i] - (pc + 1));
			if (dist > 1023 || dist < -1024)
				exit(4); /*Label is too far away!*/
			return (9 << 11) + (dist & 0x7FF);
		}
	}
	exit(1);
	if ((lArg1[0] == '#' || lArg1[0] == 'x'))
		exit(1); /*Label not found and is not a number, must be invalid label*/

	/*CASE 2: IT IS A NUMBER*/
	int pcOffset = toNum(lArg1);
	if (pcOffset > 1023 || pcOffset < -1024)
		exit(3); /*Invalid Constant*/
	return (9 << 11) + (pcOffset & 0x7FF);
}
int jsrr(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg2, "") != 0)
		exit(4); /*Wrong number of operands*/

	if (isRegister(lArg1) != 1)
		exit(4); /*Invalid register*/

	return (4 << 12) + (decodeRegister(lArg1) << 6);
}
int ldb(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*wrong number of operands*/
	if (isRegister(lArg1) == -1 || isRegister(lArg2) == -1)
		exit(4); /*invalid register or unexpected operand*/
	int boffset6 = toNum(lArg3); /* exits with 4 inside if lArg3 isn't already dec or hex number */
	if (boffset6 > ((1 << 5) - 1) || boffset6 < ((1 << 5) * -1))
		exit(3); /*too large for 4 bits*/
	return (2 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (boffset6 & 0x3f);
}
int ldw(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*wrong number of operands*/
	if (isRegister(lArg1) == -1 || isRegister(lArg2) == -1)
		exit(4); /*invalid register or unexpected operand*/
	int boffset6 = toNum(lArg3); /* exits with 4 inside if lArg3 isn't already dec or hex number */
	if (boffset6 > ((1 << 5) - 1) || boffset6 < ((1 << 5) * -1))
		exit(3); /*too large for 4 bits*/
	return (6 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (boffset6 & 0x3f);
}
int lea(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg2, "") == 0 || strcmp(lArg3, "") != 0)
		exit(4); /*Wrong number of operands*/

	if (isRegister(lArg1) != 1)
		exit(4); /*Invalid register*/

	int reg = decodeRegister(lArg1);

	/*CASE 1: IT IS A LABEL*/
	int i;
	for (i = 0; i < symbolCounter; i++)
	{
		if (strcmp(SymbolList[i], lArg2) == 0)
		{
			int dist = (SymbolAddresses[i] - (pc + 1));
			if (dist > 255 || dist < -256)
				exit(4); /*Label is too far away!*/
			return (14 << 12) + (reg << 9) + (dist & 0x1FF);
		}
	}
	exit(1); /*Label not found*/

	if (lArg2[0] != '#' || lArg2[0] != 'x')
		exit(1); /*Label not found and is not a number, must be invalid label*/

	/*CASE 2: IT IS A NUMBER*/
	int pcOffset = toNum(lArg2);
	if (pcOffset > 255 || pcOffset < -256)
		exit(3); /*Invalid Constant*/
	return (14 << 12) + (reg << 9) + (pcOffset & 0x1FF);
}
int nop(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return 0; /*This is correct :)*/
}
int not(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg2, "") == 0 || strcmp(lArg3, "") != 0 || strcmp(lArg4, "") != 0)
		exit(4); /*wrong number of operands*/
	if (isRegister(lArg1) == -1 || isRegister(lArg2) == -1)
		exit(4); /*invalid register or unexpected operand*/
	return (9 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (0x3f);
}
int ret(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg1, "") != 0)
		exit(4); /*Wrong number of operands*/

	return (12 << 12) + (7 << 6);

}
int lshf(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*wrong number of operands*/
	if (isRegister(lArg1) == -1 || isRegister(lArg2) == -1)
		exit(4); /*invalid register or unexpected operand*/
	int amount4 = toNum(lArg3); /* exits with 4 inside if lArg3 isn't already dec or hex number */
	if (amount4 > 15 || amount4 < 0)
		exit(3); /*too large for 4 bits or negative*/
	return (13 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (amount4);
}
int rshfl(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*wrong number of operands*/
	if (isRegister(lArg1) == -1 || isRegister(lArg2) == -1)
		exit(4); /*invalid register or unexpected operand*/
	int amount4 = toNum(lArg3); /* exits with 4 inside if lArg3 isn't already dec or hex number */
	if (amount4 > 15 || amount4 < 0)
		exit(3); /*too large for 4 bits or negative*/
	return (13 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (1 << 4) + (amount4);
}
int rshfa(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*wrong number of operands*/
	if (isRegister(lArg1) == -1 || isRegister(lArg2) == -1)
		exit(4); /*invalid register or unexpected operand*/
	int amount4 = toNum(lArg3); /* exits with 4 inside if lArg3 isn't already dec or hex number */
	if (amount4 > 15 || amount4 < 0)
		exit(3); /*too large for 4 bits or negative*/
	return (13 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (1 << 5) + (1 << 4) + (amount4);
}
int rti(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg1, "") != 0)
		exit(4); /*wrong number of operands*/
	return (8 << 12);
}
int stb(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*wrong number of operands*/
	if (isRegister(lArg1) == -1 || isRegister(lArg2) == -1)
		exit(4); /*invalid register or unexpected operand*/
	int boffset6 = toNum(lArg3); /* exits with 4 inside if lArg3 isn't already dec or hex number */
	if (boffset6 > ((1 << 5) - 1) || boffset6 < ((1 << 5) * -1))
		exit(3); /*too large for 4 bits*/
	return (3 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (boffset6&0x3f);
}
int stw(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*wrong number of operands*/
	if (isRegister(lArg1) == -1 || isRegister(lArg2) == -1)
		exit(4); /*invalid register or unexpected operand*/
	int offset6 = toNum(lArg3); /* exits with 4 inside if lArg3 isn't already dec or hex number */
	if (offset6 > ((1 << 5) - 1) || offset6 < ((1 << 5) * -1))
		exit(3); /*too large for 4 bits*/
	return (7 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (offset6 & 0x3f);
}
int trap(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	int num = toNum(lArg1);

	if (lArg1[0] == '#')
		exit(4);

	if (num < 0 || num > 127)
		exit(3); /*Invalid constants */
	return (0xF << 12) + (num & 0xFF);
}
int halt(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	return trap(pc, "x25", lArg2, lArg3, lArg4);
}
int xor(int pc, char* lArg1, char* lArg2, char* lArg3, char* lArg4)
{
	if (strcmp(lArg3, "") == 0 || strcmp(lArg4, "") != 0)
		exit(4); /*Incorrect number of instructions!*/

	if (isRegister(lArg1) != 1 || isRegister(lArg2) != 1)
		exit(4); /*Invalid register input*/

	if (isRegister(lArg3) == 1)
		return (9 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + decodeRegister(lArg3);
	else
	{
		int imm5 = toNum(lArg3);
		if (imm5 > 15 || imm5 < -16)
			exit(3); /*Invalid immediate */
		else
			return (9 << 12) + (decodeRegister(lArg1) << 9) + (decodeRegister(lArg2) << 6) + (1 << 5) + (imm5 & 0x1f);

	}
}

int main(int argc, char* argv[]) {
	char *prgName = NULL;
	char *iFileName = NULL;
	char *oFileName = NULL;
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
		if (isOpcode(lLabel) == 1)
			exit(4);
		if (lRet != DONE && lRet != EMPTY_LINE)
		{
			if (strcmp(lOpcode, ".end") == 0) break; 

			if (strcmp(".orig", lOpcode) == 0)
			{
				orig = toNum(lArg1);
				if (orig > 65535 || orig < -32768)
					exit(3); /*Invalid constant*/

				orig = orig & 0xffff;
			}
			if (lLabel != NULL && strcmp(lLabel, "") != 0)
			{
				if (strcmp(lLabel, "getc") == 0 || strcmp(lLabel, "in") == 0 || strcmp(lLabel, "out") == 0 || strcmp(lLabel, "puts") == 0 || lLabel[0] == 'x')
					exit(4); /*exit because the label is not allowed to be getc,in,out, or puts*/

				if (lLabel[0] >= '0' && lLabel[0] <= '9')
					exit(4);
				int j;
				for (j = 0; j<strlen(lLabel); j++)
				{
					if (isalnum(lLabel[j]) == 0)
						exit(4); /*exit because of non-alphanumeric char*/
				}
				int n;
				int labelAlreadyExists = -1;
				for (n = 0; n < symbolCounter; n++)
				{
					if (strcmp(lLabel, SymbolList[n]) == 0)
						labelAlreadyExists = 1;
				}

				if (labelAlreadyExists == -1)
				{
					if (strlen(lLabel)>20) exit(4); /*Line too long*/
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

	fclose(lInfile);



	/*PASS TWO: ASSEMBLE*/
	lineCounter = -1; /*when lineCounter == 0, it is the first instruction*/
	lInfile = fopen(iFileName, "r");	/* open the input file */
	FILE* pOutfile;
	pOutfile = fopen(oFileName, "w");
	if (!pOutfile) exit(5);

	do
	{
		int result = 0;
		lRet = readAndParse(lInfile, lLine, &lLabel, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
		if (lRet != DONE && lRet != EMPTY_LINE)
		{
			if (lOpcode[0] == '.')
			{
				if (strcmp(lOpcode, ".orig") == 0)
				{
					if (orig % 2 != 0)
						exit(3); /* address is not word-aligned */
					result = orig;
				}
				if (strcmp(lOpcode, ".end") == 0) break;
				if (strcmp(lOpcode, ".fill") == 0)
				{
					if (strcmp(lArg1, "") == 0 || strcmp(lArg2, "") != 0)
						exit(4); /*exit if there is 0 arguments or if there is >2 arguments*/
					
					result = toNum(lArg1);

					if (result > 65535 || result < -32768)
						exit(3); /*Invalid constant*/

					result = result & 0xffff;
				}
			}
			else
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
										if (strcmp(lOpcode, "brnzp") == 0) { result = br(orig + lineCounter, lArg1, lArg2, lArg3, lArg4); }
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
																															else { exit(2); /*congrats! you made it all the way here*/ }

			printf("%s %s %s %s %s \t\t %#04x \t0x%0.4X\n", lOpcode, lArg1, lArg2, lArg3, lArg4, lineCounter + orig, result);
			fprintf(pOutfile, "0x%0.4X\n", result);
			lineCounter++;
		}
	} while (lRet != DONE);
	printf(".end %s %s %s %s \t\t \t \n", lArg1, lArg2, lArg3, lArg4);
	int j;
	for (j = 0; j < symbolCounter; j++)
	{
		free(SymbolList[j]);
	}

	fclose(lInfile);
	fclose(pOutfile);
	exit(0);
}
