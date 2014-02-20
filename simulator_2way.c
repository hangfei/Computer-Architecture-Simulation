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

void simulate(FILE* inputFile, FILE* outputFile, char *para)
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
  //int64_t totalConditionalBranches = 0;

	//**** HW4_variable ****
	int i = 0;
	int j = 0;
	char cacheDirtyBit = 'c';
	int printTrace = 0;
	uint64_t indexAddr = 0;
	uint64_t tagAddr = 0;
	int blockBits = 6;
	int blockSize = pow(2, blockBits);
	int wayBits = 1;
	int waySize = pow(2, wayBits);
	int cacheBits = 8;
	if(para != NULL) {
		if(cacheBits <= blockBits) printf("Error: cacheBits < block");
		else cacheBits = atoi(para);
	}
	int64_t cacheSize = pow(2, cacheBits);
	int setBits = cacheBits - blockBits - wayBits;
	int setSize = pow(2, setBits);
	int tempLRU = 0;
	float missPercentage = 0;
	uint64_t blockAddress;
  int64_t totalMissRate = 0;
	int64_t totalLoadStore = 0;
	//struct for the trace log
	typedef struct _wayLog
	{	
		uint64_t wayTag;
		char dirtyBit;
	} wayLog;

	struct cacheSet
	{
		wayLog *wayArrayPt;
		int LRU;
	};
	//allocate space
	struct cacheSet *cacheEntry;
	cacheEntry = malloc(setSize * sizeof(struct cacheSet));
	for(i = 0; i < setSize; i++) {
		cacheEntry[i].LRU = 0;
		cacheEntry[i].wayArrayPt = malloc(waySize * sizeof(wayLog));
	}
	//init LRU = 0;
	for(i = 0; i < setSize; i++) {
		cacheEntry[i].LRU = 0;
		for(j = 0; j < waySize; j++) {
			cacheEntry[i].wayArrayPt[j].wayTag = 0;
			cacheEntry[i].wayArrayPt[j].dirtyBit = 'C';
		}
	}
	char missOrHit = 'm';
	int wayIndex = 0;
	for(i = 0; i < setSize; i++) {
		//free(cacheEntry[i].wayArrayPt);
	}
	//free(cacheEntry);

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
	
		//******* HW4_start ********

    if (totalLoadOperation == 100) {
      //break;
    }
		

    if(loadStore == 'L' || loadStore == 'S') {
			indexAddr = (addressForMemoryOp / blockSize) % setSize;
			tagAddr = addressForMemoryOp / blockSize/ setSize;
			tempLRU = cacheEntry[indexAddr].LRU;
			//get status: miss or hit; dirtyBit
			for(i = 0 ; i < waySize; ++i) {
				//if hit
  			if(cacheEntry[indexAddr].wayArrayPt[i].wayTag == tagAddr) {
					missOrHit = 'h';
					wayIndex = i;
				}
			}
			if(missOrHit != 'h') {
				totalMissRate++;
				cacheDirtyBit = cacheEntry[indexAddr].wayArrayPt[tempLRU].dirtyBit;
			}
			//print cache
			blockAddress = addressForMemoryOp / blockSize * blockSize;	//block offset bits set to zero
			if(printTrace) {
				for(i = 0; i < setSize; i++) {
					fprintf(outputFile, "[Set %d:", i);
					for(j = 0; j < waySize; j++) {
						fprintf(outputFile, " {Way %d: ", 		j);
						fprintf(outputFile, "   %"PRIx64 "  ,", cacheEntry[i].wayArrayPt[j].wayTag);		
						fprintf(outputFile, " %c} ", cacheEntry[i].wayArrayPt[j].dirtyBit);
					}
					fprintf(outputFile, " LRU:%d ]    ", 		cacheEntry[i].LRU);
				}
				fprintf(outputFile, "   %"PRIx64 "  ", blockAddress);		
				fprintf(outputFile, "   %c  %c  %c \n", loadStore, missOrHit, cacheDirtyBit);			
			}
			//update cache
			if(missOrHit == 'h') {
						cacheEntry[indexAddr].LRU = 1 - wayIndex;
					if(loadStore == 'L' ) {
						//do nothing
					}
					else 	cacheEntry[indexAddr].wayArrayPt[wayIndex].dirtyBit = 'D';
			}
				//if not hit
			if(missOrHit != 'h') {
				//on miss, update cache
				cacheEntry[indexAddr].wayArrayPt[tempLRU].wayTag = tagAddr;
				if(loadStore == 'L' ) {
					cacheEntry[indexAddr].wayArrayPt[tempLRU].dirtyBit = 'C';
				}
				else 	cacheEntry[indexAddr].wayArrayPt[tempLRU].dirtyBit = 'D';
				//update LRU
				cacheEntry[indexAddr].LRU = 1 - tempLRU;
			}

			//reset 
			missOrHit = 'm';
    }
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
 a fprintf(outputFile, "Total Conditional Branch Operation: %" PRIi64 "\n", totalConditionalBranches);
	fprintf(outputFile, "Total Branch Taken: %" PRIi64 "\n", totalBranchTaken);
	fprintf(outputFile, "Total Branch Not Taken: %" PRIi64 "\n", totalBranchNotTaken);
	**/
  //fprintf(outputFile, "%d		%" PRIi64 "\n", atoi(para), totalMisPredictions);
  //******HW3_pipeline simulation _end****** 
  //fprintf(outputFile, "	%d %" PRIi64 "\n", setBits, totalMissRate);
	totalLoadStore = totalStoreOperation + totalLoadOperation;
	missPercentage = (float)totalMissRate/(totalLoadStore) * 100;
  fprintf(outputFile, "	 %" PRIi64 "	", cacheSize);
  fprintf(outputFile, "	 %" PRIi64 "  ",totalMissRate);
  fprintf(outputFile, "	 %" PRIi64 "  ",totalLoadStore );
	fprintf(outputFile, "  %f \n", missPercentage);
  fprintf(stdout, "	%d %" PRIi64 "\n", setBits, totalMissRate);
  fprintf(stdout, "	%d %" PRIi64 "\n", cacheBits, totalStoreOperation + totalLoadOperation);
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
	if(argc >= 2) {
		simulate(inputFile, outputFile, argv[2]);
	}
	else simulate(inputFile, outputFile, NULL);
  return 0;
}
