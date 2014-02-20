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
#include <ctype.h>

void simulate(FILE* inputFile, FILE* outputFile, char* para)
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
  //fprintf(outputFile, "Processing trace...\n");
  //HW3_branch variables
	int64_t totalBranchTaken = 0;
	int64_t totalBranchNotTaken = 0;
	char predictionMade = 'N';
	int64_t totalMisPredictions = 0;
	int counterBitsBimodal = atoi(para) - 2;
	int counterBitsGshare = atoi(para) - 1;
	int chooserBits = atoi(para) - 2;
	int64_t counterEntryBimodal = pow(2, counterBitsBimodal);
	int64_t counterEntryGshare = pow(2, counterBitsGshare);
	int64_t chooserEntry = pow(2, chooserBits);
	int64_t predictionSelectorBimodal = 0;
	int64_t predictionSelectorGshare = 0;
	int64_t predictionSelectorChooser = 0;
	char counterTableBimodal[counterEntryBimodal];
	char counterTableGshare[counterEntryGshare];
	char chooserTable[chooserEntry];
	int64_t historyRegister = 0b00000000000000000000;
	int historyRegLength = atoi(para) - 1;
	int64_t i = 0;
	int printTrace = 0;	
	//int predictionResult = 0;
	//*** init
	for(i = 0; i < counterEntryBimodal; ++i) {
		counterTableBimodal[i] = 'N';
	}
	for(i = 0; i < counterEntryGshare; ++i) {
		counterTableGshare[i] = 'N';
	}
	for(i = 0; i < chooserEntry; ++i) {
		chooserTable[i] = 'B';
	}
	//*** start reading the trace file
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
	    total4byteMacroops = total4byteMacroops + 1;
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
    //******* HW1_end **********
		
		//********** begin: HW3 **********
		// init counter table
		//predictionMade = 'T';
    if(targetAddressTakenBranch != 0 && conditionRegister == 'R') {
        totalConditionalBranches++;
				//if(totalConditionalBranches > 200) break;
				
				//***made predictions
				predictionSelectorChooser = instructionAddress % chooserEntry;
				predictionSelectorBimodal = instructionAddress % counterEntryBimodal;
				predictionSelectorGshare = (instructionAddress % counterEntryGshare) ^ historyRegister;
				if(toupper(chooserTable[predictionSelectorChooser]) == 'B') {
					predictionMade = toupper(counterTableBimodal[predictionSelectorBimodal]);		
				}
				else {
					predictionMade = toupper(counterTableGshare[predictionSelectorGshare]);				
				}
				//***branch result
				if(TNnotBranch == 'T') {
					totalBranchTaken++;
				}
				if(TNnotBranch == 'N') {
					totalBranchNotTaken++;
				}
				//** print
				//*** print
				if(printTrace) {
					for(i = 0; i < chooserEntry; ++i) {
						fprintf(outputFile, "%c", chooserTable[i]);
					}
					fprintf(outputFile, "     ");
					for(i = 0; i < counterEntryBimodal; ++i) {
						fprintf(outputFile, "%c", counterTableBimodal[i]);
					}
					fprintf(outputFile, "     ");
					for(i = 0; i < counterEntryGshare; ++i) {
						fprintf(outputFile, "%c", counterTableGshare[i]);
					}
					fprintf(outputFile, "     ");
					fprintf(outputFile, "  %d%d%d%d    ", (historyRegister/2/2/2)%2, (historyRegister/2/2)%2,
						historyRegister/2%2, historyRegister%2);
					fprintf(outputFile, "     %" PRIx64 "", instructionAddress);
					fprintf(outputFile, " %c ", TNnotBranch);
					fprintf(outputFile, "| %c ", predictionMade);
				}
				
				if(TNnotBranch == predictionMade) {
					//predictionResult = 1;
					if(printTrace) 	fprintf(outputFile, "Correct");
				}
				else {
					//predictionResult = 0;
					totalMisPredictions++;	
					if(printTrace)	fprintf(outputFile, "Incorrect");
				}
				if(printTrace) 	fprintf(outputFile, "		%" PRIi64 "\n", totalMisPredictions);
				//fprintf(outputFile, "   %" PRIi64 "\n", totalMisPredictions);
				//***update the table: bimodal, gshare, chooser
				// four situations
				if(toupper(counterTableBimodal[predictionSelectorBimodal]) == TNnotBranch &&
						toupper(counterTableGshare[predictionSelectorGshare]) != TNnotBranch) {
					if(chooserTable[predictionSelectorChooser]== 'b') {
						chooserTable[predictionSelectorChooser] = 'B';
					}
					else if(chooserTable[predictionSelectorChooser]== 'B') {
						chooserTable[predictionSelectorChooser] = 'B';
					}
					else if(chooserTable[predictionSelectorChooser]== 'g') {
						chooserTable[predictionSelectorChooser] = 'b';
					}
					else if(chooserTable[predictionSelectorChooser]== 'G') {
						chooserTable[predictionSelectorChooser] = 'g';
					}
				}
				else if (toupper(counterTableGshare[predictionSelectorGshare]) == TNnotBranch &&
						toupper(counterTableBimodal[predictionSelectorBimodal]) != TNnotBranch) {
					if(chooserTable[predictionSelectorChooser]== 'b') {
						chooserTable[predictionSelectorChooser] = 'g';
					}
					else if(chooserTable[predictionSelectorChooser]== 'B') {
						chooserTable[predictionSelectorChooser] = 'b';
					}
					else if(chooserTable[predictionSelectorChooser]== 'g') {
						chooserTable[predictionSelectorChooser] = 'G';
					}
					else if(chooserTable[predictionSelectorChooser]== 'G') {
						chooserTable[predictionSelectorChooser] = 'G';
					}
				}

				historyRegister = historyRegister << 1;
				historyRegister &= ~(1 << historyRegLength);;
				
				if(TNnotBranch == 'T') {
					historyRegister |= 1 << 0;
					
					//update bimodal
					if(counterTableBimodal[predictionSelectorBimodal] == 'n') 
						counterTableBimodal[predictionSelectorBimodal] = 't';
					else if(counterTableBimodal[predictionSelectorBimodal] == 'N') 
						counterTableBimodal[predictionSelectorBimodal] = 'n';
					else if(counterTableBimodal[predictionSelectorBimodal] == 't') 
						counterTableBimodal[predictionSelectorBimodal] = 'T';
					else if(counterTableBimodal[predictionSelectorBimodal] == 'T') 
						counterTableBimodal[predictionSelectorBimodal] = 'T';

					//update gshare
					if(counterTableGshare[predictionSelectorGshare] == 'n') 
						counterTableGshare[predictionSelectorGshare] = 't';
					else if(counterTableGshare[predictionSelectorGshare] == 'N') 
						counterTableGshare[predictionSelectorGshare] = 'n';
					else if(counterTableGshare[predictionSelectorGshare] == 't') 
						counterTableGshare[predictionSelectorGshare] = 'T';
					else if(counterTableGshare[predictionSelectorGshare] == 'T') 
						counterTableGshare[predictionSelectorGshare] = 'T';

				}
				else {
					historyRegister |= 0 << 0;
					if(counterTableBimodal[predictionSelectorBimodal] == 'n') 
						counterTableBimodal[predictionSelectorBimodal] = 'N';
					else if(counterTableBimodal[predictionSelectorBimodal] == 'N') 
						counterTableBimodal[predictionSelectorBimodal] = 'N';
					else if(counterTableBimodal[predictionSelectorBimodal] == 't') 
						counterTableBimodal[predictionSelectorBimodal] = 'n';
					else if(counterTableBimodal[predictionSelectorBimodal] == 'T') 
						counterTableBimodal[predictionSelectorBimodal] = 't';

					if(counterTableGshare[predictionSelectorGshare] == 'n') 
						counterTableGshare[predictionSelectorGshare] = 'N';
					else if(counterTableGshare[predictionSelectorGshare] == 'N') 
						counterTableGshare[predictionSelectorGshare] = 'N';
					else if(counterTableGshare[predictionSelectorGshare] == 't') 
						counterTableGshare[predictionSelectorGshare] = 'n';
					else if(counterTableGshare[predictionSelectorGshare] == 'T') 
						counterTableGshare[predictionSelectorGshare] = 't';
				}
    }

		

    //********** end: HW3 **********
  }
  //******* begin_HW1_output **********
	/**
  fprintf(outputFile, "total4byteMacroops %" PRIi64 "\n", total4byteMacroops);
  fprintf(outputFile, "Processed %" PRIi64 " trace records.\n", totalMicroops);
  fprintf(outputFile, "Micro-ops: %" PRIi64 "\n", totalMicroops);
  fprintf(outputFile, "Macro-ops: %" PRIi64 "\n", totalMacroops);
  fprintf(outputFile, "Total Unconditional Branch Operation: %" PRIi64 "\n", totalUnconditionalBranches);
  //fprintf(outputFile, "Total Conditional Branch Operation: %" PRIi64 "\n", totalConditionalBranches);
	**/
  //******* end_HW1_output **********

  //******begin_HW3_pipeline simulation******  
	/**
  fprintf(outputFile, "Total Conditional Branch Operation: %" PRIi64 "\n", totalConditionalBranches);
	fprintf(outputFile, "Total Branch Taken: %" PRIi64 "\n", totalBranchTaken);
	fprintf(outputFile, "Total Branch Not Taken: %" PRIi64 "\n", totalBranchNotTaken);
	**/
	fprintf(outputFile, "%d		%" PRIi64 "\n", atoi(para), totalMisPredictions);
  //******HW3_pipeline simulation _end****** 
}


int main(int argc, char *argv[]) 
{
  FILE *inputFile = stdin;
  FILE *outputFile = stdout;

	if (argc >= 2) {
		//printf("argc >=2 %s\n", argv[1]);
		outputFile = fopen(argv[1], "a+");
  }
	/**
  printf("%d\n\n\n", argc);
	printf("%s\n", argv[0]);
	printf("%s\n", stdin);
	printf("%s\n", argv[1]);

	if (argc >= 3) {
		printf("argc >=2 %s\n", argv[2]);
    inputFile = fopen(argv[1], "r");
    assert(inputFile != NULL);
  }
  if (argc >= 3) {
    outputFile = fopen(argv[2], "w");
    assert(outputFile != NULL);
  }
	**/
  simulate(inputFile, outputFile, argv[2]);
  return 0;
}
