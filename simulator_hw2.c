#define __STDC_FORMAT_MACROS
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

void simulate(FILE* inputFile, FILE* outputFile)
{
  // See the documentation to understand what these variables mean.
  int32_t microOpCount;
  uint64_t instructionAddress;
  int32_t sourceRegister1;
  int32_t sourceRegister2;
  int32_t destinationRegister;
  char conditionRegister;
  char TNnotBranch;
  char loadStore;
  int64_t immediate;
  uint64_t addressForMemoryOp;
  uint64_t fallthroughPC;
  uint64_t targetAddressTakenBranch;
  char macroOperation[12];
  char microOperation[23];

  int64_t totalMicroops = 0;
  int64_t totalMacroops = 0;
  int64_t totalInstructionSize = 0;
  int64_t total4byteMacroops = 0;
  int64_t totalLoadOperation = 0;
  int64_t totalStoreOperation = 0;
  int64_t totalUnconditionalBranches = 0;
  int64_t totalConditionalBranches = 0;
  float fourByteMacroPercentage = 0; 
  int64_t branchEncodeWithinByte = 0;
  int64_t branchEncodeTotalNumber = 0;
  fprintf(outputFile, "Processing trace...\n");
  int lastDestFlagW = 0;
  int64_t totalCompareBranchPair = 0;

  //HW2_pipeline simulation variables
  int nextInstruction = 0;
  int registerBoard[50] = { 0 };
  int arrayIndex = 0;
  int64_t totalCycles = 0;
  while (true) {
    int result = fscanf(inputFile, 
                        "%" SCNi32
                        "%" SCNx64 
                        "%" SCNi32
                        "%" SCNi32
                        "%" SCNi32
                        " %c"
                        " %c"
                        " %c"
                        "%" SCNi64
                        "%" SCNx64
                        "%" SCNx64
                        "%" SCNx64
                        "%11s"
                        "%22s",
                        &microOpCount,
                        &instructionAddress,
                        &sourceRegister1,
                        &sourceRegister2,
                        &destinationRegister,
                        &conditionRegister,
                        &TNnotBranch,
                        &loadStore,
                        &immediate,
                        &addressForMemoryOp,
                        &fallthroughPC,
                        &targetAddressTakenBranch,
                        macroOperation,
                        microOperation);
                        
    if (result == EOF) {
      break;
    }

    if (result != 14) {
      fprintf(stderr, "Error parsing trace at line %"PRIi64 PRIi64 PRIi64 "\n", totalMicroops);
      abort();
    }

    //*******HW1**********
    // For each micro-op
    totalMicroops++;
        // For each macro-op:
    if (microOpCount == 1) {
      totalMacroops++;
	totalInstructionSize = totalInstructionSize + fallthroughPC - instructionAddress;

        if((fallthroughPC - instructionAddress) == 4) {
            //fprintf(outputFile, "%" SCNx64 "\n", instructionAddress);	
	    //fprintf(outputFile, "%" SCNx64 "\n", fallthroughPC);
	    //fprintf(outputFile, "%" PRIi64 "\n", fallthroughPC - instructionAddress);
	    total4byteMacroops = total4byteMacroops + 1;
	    //fprintf(outputFile, "%" PRIi64 "\n", total4byteMacroops);
        }
    }

 
    //instruction category
    if(loadStore == 'L') {
	totalLoadOperation++;
	//fprintf(outputFile, "123microOperation: % s \n", microOperation);

    }
    if(loadStore == 'S') {
	totalStoreOperation++;
	//fprintf(outputFile, "123microOperation: % s \n", microOperation);
    }
    if(targetAddressTakenBranch != 0 && conditionRegister == '-') {
        totalUnconditionalBranches++;
    }
    if(targetAddressTakenBranch != 0 && conditionRegister == 'R') {
        totalConditionalBranches++;
    }

    //Question 5
    //2 + floor(log2(abs(InstructionPC - TargetPC)))
    if(targetAddressTakenBranch != 0) {
	branchEncodeTotalNumber++;
	if( (2 + floor((log(abs(instructionAddress - targetAddressTakenBranch)) / log(2)))) <= 11) {
	    branchEncodeWithinByte++;
	}
    }

    //Question 8
    if(targetAddressTakenBranch != 0 && conditionRegister == 'R' && lastDestFlagW == 1) {
        totalCompareBranchPair++;
	lastDestFlagW = 0;
    }
    if(conditionRegister == 'W') {
	lastDestFlagW = 1;
    }
    else lastDestFlagW = 0;
    //*******HW1_end**********


    //******begin_HW2_pipeline simulation******  
  /*
  next_instruction = true
  while (true):
  if (next_instruction):
    read next micro-op from trace
    next_instruction = false

  if instruction is "ready" based on scoreboard entries for valid input and output registers:
    if the instruction writes a register:
      "execute" the instruction by recording when its register will be ready
    next_instruction = true

  advance one cycle by decrementing non-zero entries in the scoreboard
  increment total cycles counter
  */
    //loop over each microOPs
    while(!nextInstruction) {
	nextInstruction  = 1;
		//printf("22sdfsf323\n");
	if(sourceRegister1 != -1) {
	    if(registerBoard[sourceRegister1]) nextInstruction = 0;
	}
	if(sourceRegister2 != -1) {
	    if(registerBoard[sourceRegister2]) nextInstruction = 0;
	}
	if(destinationRegister != -1 && nextInstruction != 0) {
		if(!registerBoard[destinationRegister]) {
			//printf("22!!!!323\n");
			if(loadStore == 'L') {
			    registerBoard[destinationRegister] = 2;
			//printf("22323\n");
			}
			else registerBoard[destinationRegister] = 1;
		    //nextInstruction = 1;	//go out of the loop
		}
		else nextInstruction = 0;
	}
	//else nextInstruction = 1;	//go out of the loop
	//decrementing the registerBoard
	/*
	fprintf(outputFile, "%" PRIi64 "    ", totalCycles);
	fprintf(outputFile, "%" PRIi64 "    ", totalMicroops);
	fprintf(outputFile, "%" PRIi32 "    ", sourceRegister1);
	fprintf(outputFile, "%" PRIi32 "    ", sourceRegister2);
	fprintf(outputFile, "  ->  ");
	fprintf(outputFile, "%" PRIi32 "", destinationRegister);
	fprintf(outputFile, "  -  %c", loadStore);
	fprintf(outputFile, "    -    ");*/
        for(arrayIndex = 0; arrayIndex < 50; arrayIndex++) 
        {

	    //if(registerBoard[arrayIndex] == 0) fprintf(outputFile, "-");
            //else fprintf(outputFile, "%d", registerBoard[arrayIndex]);
	    if(registerBoard[arrayIndex]) {
		--registerBoard[arrayIndex];
    	    }
        }
	//fprintf(outputFile, "\n");
 	++totalCycles;	
	//fprintf(outputFile, "totalCycles: %"PRIi64"\n", totalCycles);
    }
    nextInstruction = 0;
    //******HW2_pipeline simulation _end****** 
    
  }
  //*******HW1**********
  fourByteMacroPercentage = (float)total4byteMacroops/(float)totalMacroops;
  fprintf(outputFile, "total4byteMacroops %" PRIi64 "\n", total4byteMacroops);
  fprintf(outputFile, "Processed %" PRIi64 " trace records.\n", totalMicroops);
  fprintf(outputFile, "Micro-ops: %" PRIi64 "\n", totalMicroops);
  fprintf(outputFile, "Macro-ops: %" PRIi64 "\n", totalMacroops);
  fprintf(outputFile, "Total instruction size: %" PRIi64 "\n", totalInstructionSize);
  fprintf(outputFile, "Percentage of all macro-ops are 4 bytes in size: % f \n", fourByteMacroPercentage);

  fprintf(outputFile, "Total LOAD Operation: %" PRIi64 "\n", totalLoadOperation);
  fprintf(outputFile, "Total STORE Operation: %" PRIi64 "\n", totalStoreOperation);
  fprintf(outputFile, "Total Unconditional Branch Operation: %" PRIi64 "\n", totalUnconditionalBranches);
  fprintf(outputFile, "Total Conditional Branch Operation: %" PRIi64 "\n", totalConditionalBranches);
  fprintf(outputFile, "Branch %" PRIi64 "\n", branchEncodeTotalNumber);
  fprintf(outputFile, "Branch %" PRIi64 "\n", branchEncodeWithinByte);
  fprintf(outputFile, "totalCompareBranchPair22 %" PRIi64 "\n", totalCompareBranchPair);
  //*******HW1**********

  //******begin_HW2_pipeline simulation******  
  fprintf(outputFile, "total cycles: %"PRIi64  "\n", totalCycles);
  //******HW2_pipeline simulation _end****** 


}


int main(int argc, char *argv[]) 
{
  FILE *inputFile = stdin;
  FILE *outputFile = stdout;
  
  if (argc >= 2) {
    inputFile = fopen(argv[1], "r");
    assert(inputFile != NULL);
  }
  if (argc >= 3) {
    outputFile = fopen(argv[2], "w");
    assert(outputFile != NULL);
  }
  
  simulate(inputFile, outputFile);
  return 0;
}
