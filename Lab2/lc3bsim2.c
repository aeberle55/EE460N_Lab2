/*
    Name 1: George Netscher 
    Name 2: Austin Eberle
    UTEID 1: GMN255
    UTEID 2: AJE542 
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
  /*initialize("test2.txt",1);*/	

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
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

#define BIT4 0x0010
#define BIT5 0x0020
#define BIT7 0x0080
#define BIT8 0x0100
#define BIT9 0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT15 0x8000


int getWordValue(int MAR);
int getByteValue(int MAR);
void defaultNextState();
void evaluateConditional(int c);
int signExtend(int x,int numBits);

/*  function: process_instruction
   *  
   *    Process one instruction at a time  
   *       -Fetch one instruction
   *       -Decode 
   *       -Execute
   *       -Update NEXT_LATCHES
   */
void process_instruction(){
	int instruction, opcode, num1, num2, DR, SR1, SR2;
	defaultNextState();				/*Sets Next State to be default, including incrementing PC*/
	num1 = 0;
	num2 = 0;
	instruction = getWordValue(CURRENT_LATCHES.PC);
	/*DR and SR valid for appropriate functions*/
	DR = instruction & 0x0E00;
	DR = DR>>9;
	SR1 = instruction & 0x01C0;
	SR1 = SR1>>6;					
	SR2 = instruction & 0x0007;
	opcode=instruction & 0xF000;
	opcode=opcode>>12;
	switch(opcode)
	{
		case 0:		/*Branch*/
			num2 = instruction & 0x01FF; 
			if( ( (instruction & BIT11) && (CURRENT_LATCHES.N) ) 
			  || ( (instruction & BIT10) && (CURRENT_LATCHES.Z) )
			  || ( (instruction & BIT9) && (CURRENT_LATCHES.P) ) ) {
				if(instruction & BIT8) 
					num2 = signExtend(num2,9);
				NEXT_LATCHES.PC = CURRENT_LATCHES.PC + (num2<<1) + 2;	/*Fixed*/
			}			  
			break;
		case 1:		/*Add*/
			num1 = CURRENT_LATCHES.REGS[SR1];
			if(num1 & BIT15)
				num1=signExtend(num1,16);
			if(instruction & BIT5)	/*Immediate*/
			{
				num2 = instruction & 0x001F;
				if(instruction & BIT4)			/*Bit 4 represents sign*/
					num2=signExtend(num2,5);
			}
			else
			{
				num2 = CURRENT_LATCHES.REGS[SR2];
				if(num2 & BIT15)
					num2=signExtend(num2,16);
			}
			num1 = num1+num2;
			NEXT_LATCHES.REGS[DR]=Low16bits(num1);
			evaluateConditional(num1);
			break;
		case 2:		/*LDB*/
			num1 = CURRENT_LATCHES.REGS[SR1];
			num2 = instruction & 0x003F;
			if(instruction & BIT5)
				num2=signExtend(num2,6);
			num1+=num2;
			num1 = getByteValue(num1);
			NEXT_LATCHES.REGS[DR]=Low16bits(num1);
			if(num1 & BIT7)
				num1=signExtend(num1,8); /*Set condition code*/
			evaluateConditional(num1);
			break;
		case 3:		/*STB*/
			num1 = CURRENT_LATCHES.REGS[SR1];
			num2 = instruction & 0x003F;
			if(instruction & BIT5)
				num2=signExtend(num2,6);
			num1+=num2;
			MEMORY[num1>>1][num1&1] = CURRENT_LATCHES.REGS[DR] & 0x00FF;
			break;
		case 4:		/*JSR*/
			num2 = NEXT_LATCHES.PC; /* store incremented PC in temporary variable */
			/* PC incremented in defaultNextState() at process start */
			if(instruction & BIT11) {
				/* PCoffset */
				num1 = instruction & 0x07FF;
				if(num1 & BIT10) 
					num1 = signExtend(num1,11);		
				NEXT_LATCHES.PC = Low16bits(NEXT_LATCHES.PC + (num1<<1) + 2); 
			} else {
				NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[SR1] + 2);
			}
			NEXT_LATCHES.REGS[7] = Low16bits(num2);
 			break;
		case 5:		/*And*/
			num1 = CURRENT_LATCHES.REGS[SR1];
			if(num1 & BIT15)
				num1=signExtend(num1,16);
			if(instruction & BIT5)	/*Immediate*/
			{
				num2 = instruction & 0x001F;
				if(instruction & BIT4)			/*Bit 4 represents sign*/
					num2=signExtend(num2,5);
			}
			else
			{
				num2 = CURRENT_LATCHES.REGS[SR2];
				if(num2 & BIT15)
					num2=signExtend(num2,16);
			}
			num1 = num1&num2;
			NEXT_LATCHES.REGS[DR]=Low16bits(num1);
			evaluateConditional(num1);
			break;
		case 6:		/*LDW*/
			num1 = CURRENT_LATCHES.REGS[SR1];
			num2 = instruction & 0x003F;
			if(instruction & BIT5)
				num2=signExtend(num2,6);
			num2 = (num2<<1);
			num1+=num2;
			NEXT_LATCHES.REGS[DR] = (MEMORY[num1>>1][1]<<8) + MEMORY[num1>>1][0];
			if(num1 & BIT15) 
				num1=signExtend(num1,16);
			evaluateConditional(num1);
			break;
		case 7:		/*STW*/
			num1 = CURRENT_LATCHES.REGS[SR1];
			num2 = instruction & 0x003F;
			if(instruction & BIT5)
				num2=signExtend(num2,6);
			num2 = (num2<<1);
			num1+=num2;
			MEMORY[num1>>1][1] = Low16bits((CURRENT_LATCHES.REGS[DR] & 0xFF00)>>8);
			MEMORY[num1>>1][0] = Low16bits(CURRENT_LATCHES.REGS[DR] & 0x00FF);
			break;
		case 8: 	/*RTI (1000) is not implemented*/
			break;

		case 9:		/*XOR and NOT*/
			num1 = CURRENT_LATCHES.REGS[SR1];
			if(num1 & BIT15)
				num1=signExtend(num1,16);
			if(instruction & BIT5)	/*Immediate*/
			{
				num2 = instruction & 0x001F;
				if(instruction & BIT4)			/*Bit 4 represents sign*/
					num2=signExtend(num2,5);
			}
			else
			{
				num2 = CURRENT_LATCHES.REGS[SR2];
				if(num2 & BIT15)
					num2=signExtend(num2,16);
			}
			num1 = num1^num2;
			NEXT_LATCHES.REGS[DR]=Low16bits(num1);
			evaluateConditional(num1);
			break;

			/*1010 and 1011 are unused*/

		case 12:	/*JMP*/
			NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.REGS[SR1] + 2);
			break;
		case 13:	/*SHF*/
			num1 = CURRENT_LATCHES.REGS[SR1];
			num2 = instruction & 0x000F;
			if(instruction & BIT5)
				if(num1 & BIT5)
					num1=signExtend(num1,16);	/*Only Sign Extended when bit 5 is set*/
			if(instruction & BIT4) { /*Right Shift*/
				if(instruction & BIT5){
					/* logical shift */
					num1=(int)((unsigned int)num1>>num2);
				} else {
					/* arithmetic shift */
					num1=(num1>>num2);
				}
			}
			else					/*Left Shift*/
				num1=(num1<<num2);
			NEXT_LATCHES.REGS[DR]=Low16bits(num1);
			if(num1 & BIT15)		/*Possible to become a negitive according to lc3, but not c*/
				num2=signExtend(num1,16);
			evaluateConditional(num1);
			break;
		case 14:	/*LEA*/
			num1 = (instruction & 0x01FF);
			if(num1 & BIT8) 
				num1 = signExtend(num1,9);
			NEXT_LATCHES.REGS[DR] = Low16bits(NEXT_LATCHES.PC + (num1<<1));
			/* do not set condition codes as per lab manual */	
			break;
		case 15:	/*TRAP*/
			num1=instruction & 0x00FF;
			NEXT_LATCHES.REGS[7]=CURRENT_LATCHES.PC+2;	/*Store incremented PC in R7*/
			num1 = num1<<1;
			NEXT_LATCHES.PC= getWordValue(num1);	/*Load PC with correct value from TRAP vector table*/
			break;
	}
}

/*Returns the Word value at a given memory location*/
int getWordValue(int MAR)
{
	int v1,v2;
	v1=MEMORY[MAR>>1][0];				/*v1 is Least Significant 8 bits*/
	v2=MEMORY[MAR>>1][1];
	return Low16bits((v2<<8)+v1);
}

/*Returns the Byte value at a given memory location*/
int getByteValue(int MAR)
{
	return Low16bits(MEMORY[MAR>>1][MAR&1]);
}

/*Sets the NEXT_LATCHES structure to be as it would be following a NOP*/
void defaultNextState()
{
	int k;
	NEXT_LATCHES.N=CURRENT_LATCHES.N;
	NEXT_LATCHES.Z=CURRENT_LATCHES.Z;
	NEXT_LATCHES.P=CURRENT_LATCHES.P;
	NEXT_LATCHES.PC=CURRENT_LATCHES.PC + 2;
	for(k=0; k<LC_3b_REGS;k++)
		NEXT_LATCHES.REGS[k]=CURRENT_LATCHES.REGS[k];
}

/*Sets the condition codes based on the integer c*/
void evaluateConditional(int c)
{
	NEXT_LATCHES.N=(c<0);
	NEXT_LATCHES.Z=(c==0);
	NEXT_LATCHES.P=(c>0);
}
/*Negative sign extends a number x with a size of numBits*/
int signExtend(int x,int numBits)
{
	int mask = 0xFFFF0000;
	mask = mask >> (16-numBits);	/*Right Shift is Arithmetic in C*/
	return x+mask;
}
