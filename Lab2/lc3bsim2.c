/*
    Name 1: Nikhil Joglekar 
    Name 2: Kevin Pham
    UTEID 1: nrj328
    UTEID 2: ktp364
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Instruction Level Simulator                         */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3b machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
*/

#define WORDS_IN_MEM    0x08000 
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */


typedef struct System_Latches_Struct{

  int PC,		/* program counter */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P;		/* p condition bit */
  int REGS[LC_3b_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {                                                    
  printf("----------------LC-3b ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
  int i;

  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
  int address; /* this is a byte address */

  printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = (start >> 1); address <= (stop >> 1); address++)
    fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%0.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3b_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3b-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch(buffer[0]) {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
    else {
	    scanf("%d", &cycles);
	    run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {                                           
  int i;

  for (i=0; i < WORDS_IN_MEM; i++) {
    MEMORY[i][0] = 0;
    MEMORY[i][1] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word >> 1;
  else {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
	    exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii][0] = word & 0x00FF;
    MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */ 
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while(*program_filename++ != '\0');
  }
  CURRENT_LATCHES.Z = 1;  
  NEXT_LATCHES = CURRENT_LATCHES;
    
  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
  FILE * dumpsim_file;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3b Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");    exit(-1);

  }

  while (1)
    get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/


















int parseRegister(int instruction, int position){
	return (((0x07 << position) & instruction) >> position);
}

void setCC(int regnum) {
	int reg = NEXT_LATCHES.REGS[regnum];

	NEXT_LATCHES.Z = 0;
	NEXT_LATCHES.N = 0;
	NEXT_LATCHES.P = 0;

	if (reg == 0)
		NEXT_LATCHES.Z = 1;
	else if (reg > 0)
		NEXT_LATCHES.P = 1;
	else
		NEXT_LATCHES.N = 1;

	NEXT_LATCHES.REGS[regnum] &= 0x0000FFFF;
}

void incrementPC(){
	NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
}


void BR(int instruction){
	if (((((instruction & 0x0800) >> 11) & CURRENT_LATCHES.N) + (((instruction & 0x0400) >> 10) & CURRENT_LATCHES.Z) + (((instruction & 0x0200) >> 9) & CURRENT_LATCHES.P)) > 0)
		NEXT_LATCHES.PC = Low16bits((CURRENT_LATCHES.PC+2) + ((((instruction & 0x1FF) << 23) >> 23)<<1));
	else
		incrementPC();
}

void ADD(int instruction){
	if ((instruction & 0x0020) == 0) {
		NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] + CURRENT_LATCHES.REGS[parseRegister(instruction, 0)];
	}
	else{
		NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] + (((0x001F & instruction) << 27) >> 27);
	}

	setCC(parseRegister(instruction, 9));
	incrementPC();
}

void LDB(int instruction){
	int location = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] + (((0x003F & instruction) << 26) >> 26);
	NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = MEMORY[location >> 1][location%2];

	setCC(parseRegister(instruction, 9));
	incrementPC();
}

void STB(int instruction){
	int location = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] + (((0x003F & instruction) << 26) >> 26);
	MEMORY[location >> 1][location%2] = NEXT_LATCHES.REGS[parseRegister(instruction, 9)] & 0x00FF;

	setCC(parseRegister(instruction, 9));
	incrementPC();
}

void JSR(int instruction){
	int temp = Low16bits(CURRENT_LATCHES.PC + 2);
	if (instruction & 0x0800 == 0){
		NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[parseRegister(instruction, 6)]);
	}
	else {
		NEXT_LATCHES.PC = Low16bits((CURRENT_LATCHES.PC + 2) + ((((instruction & 0x7FF) << 21) >> 21) << 1));
	}
	NEXT_LATCHES.REGS[7] = temp;
}

void AND(int instruction){
	if ((instruction & 0x0020) == 0) {
		NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] & CURRENT_LATCHES.REGS[parseRegister(instruction, 0)];
	}
	else{
		NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] & (((0x001F & instruction) << 27) >> 27);
	}

	setCC(parseRegister(instruction, 9));
	incrementPC();
}

void LDW(int instruction){
	int location = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] + (((0x003F & instruction) << 26) >> 26);
	NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = MEMORY[location >> 1][0] + (MEMORY[location >> 1][1] << 8);


	setCC(parseRegister(instruction, 9));
	incrementPC();
}

void STW(int instruction){
	int location = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] + (((0x003F & instruction) << 26) >> 26);
	MEMORY[location >> 1][0] = NEXT_LATCHES.REGS[parseRegister(instruction, 9)] & 0x00FF;
	MEMORY[location >> 1][1] = ((NEXT_LATCHES.REGS[parseRegister(instruction, 9)] & 0x0000FF00) >> 8);

	setCC(parseRegister(instruction, 9));
	incrementPC();
}

void XOR(int instruction){
	if ((instruction & 0x0020) == 0) {
		NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = (CURRENT_LATCHES.REGS[parseRegister(instruction, 6)]) ^ (CURRENT_LATCHES.REGS[parseRegister(instruction, 0)]);
	}
	else{
		NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] ^ (((0x001F & instruction) << 27) >> 27);
	}

	setCC(parseRegister(instruction, 9));
	incrementPC();
}

void JMP(int instruction){
	NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] & 0x0000FFFF;
}

void SHF(int instruction){
	/*
	if ((instruction & 0x0020) == 0) {
		NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] << (((0x000F & instruction) << 28) >> 28);
	}*/
	/* values for SHF are non-negative */
	if ((instruction & 0x0030) >> 4 == 0)
	{
		NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] << (0x000F & instruction);
	}
	else if ((instruction & 0x0030) >> 4 == 1)
	{
		NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] >> (0x000F & instruction);
	}
	else if ((instruction & 0x0030) >> 4 == 3)
	{
		if ((0x000F) & instruction == 0)
		{
			NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)];
		}
		else
		{
			int number = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)];
			if ((number & 0x8000) >> 15 == 0)
			{
				NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] >> (0x000F & instruction);
				int mask = 0x7FFFFFFF;
				int k;
				for (k = 0; k < (0x000F & instruction) - 1; k++)
				{
					mask = mask >> 1;
				}
				NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = NEXT_LATCHES.REGS[parseRegister(instruction, 9)] & mask;
			}
			else
			{
				NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = CURRENT_LATCHES.REGS[parseRegister(instruction, 6)] >> (0x000F & instruction);
				int mask = 0xFFFF8000;
				int k;
				for (k = 0; k < (0x000F & instruction) - 1; k++)
				{
					mask = mask >> 1;
				}
				NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = NEXT_LATCHES.REGS[parseRegister(instruction, 9)] | mask;
			}
		}
	}

	setCC(parseRegister(instruction, 9));
	incrementPC();
}

void LEA(int instruction){
	NEXT_LATCHES.REGS[parseRegister(instruction, 9)] = Low16bits((CURRENT_LATCHES.PC + 2) + ((((instruction & 0x1FF) << 23) >> 23) << 1));
	incrementPC();
}

void TRAP(int instruction){
	NEXT_LATCHES.REGS[7] = Low16bits((CURRENT_LATCHES.PC + 2));
	int location = (instruction & 0x000000FF) << 1;
	NEXT_LATCHES.PC = Low16bits(((MEMORY[location][1]) << 8) + (MEMORY[location][0]));
}


void process_instruction(){
  /*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */     

	int current_instruction = (MEMORY[CURRENT_LATCHES.PC>>1][1]<<8) + (MEMORY[CURRENT_LATCHES.PC>>1][0]);
	printf("%d 0x%0.4X 0x%0.4X\n", CURRENT_LATCHES.PC, (current_instruction & 0x0000f000) >> 12, current_instruction);

	switch ((current_instruction & 0x0000f000) >> 12){

	case 0: BR(current_instruction); break;
	case 1: ADD(current_instruction); break;
	case 2: LDB(current_instruction); break;
	case 3: STB(current_instruction); break;
	case 4: JSR(current_instruction); break;
	case 5: AND(current_instruction); break;
	case 6: LDW(current_instruction); break;
	case 7: STW(current_instruction); break;
	case 8: break;
	case 9: XOR(current_instruction); break;
	case 10: break;
	case 11: break;
	case 12: JMP(current_instruction); break;
	case 13: SHF(current_instruction); break;
	case 14: LEA(current_instruction); break;
	case 15: TRAP(current_instruction); break;
	}
}
