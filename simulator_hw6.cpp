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

//C++
//when include c++ file, no .h sufix
#include <deque>
#include <iostream>
#include <string>
using namespace std;


//for C++, need to put strct before func to use deque
typedef struct _reorderBufferEntry
{
	int arch_reg_source_index1;
	int arch_reg_source_index2;
	int arch_reg_source_index3;
	int arch_reg_dest_index1;
	int arch_reg_dest_index2;
	int sourceRegister1;
	int sourceRegister2;
	int sourceRegister3;
	bool registerReady1;
	bool registerReady2;
	bool registerReady3;
	int registerDestination1;
	int registerDestination2;
	int phys_reg_to_free1;
	int phys_reg_to_free2;
	bool isLoad;	
	bool isStore;
	uint64_t addressForMemoryOp;
	int sequenceNumber;
	uint64_t microopNumber;
	bool isIssued;
	bool isMisPredictedBranch;
	int fetchCycle;
	int issueCycle;
	int doneCycle;
	int commitCycle;
	char macroOperationName[11];
	char microOperationName[22];
	//string infoString;		//bus error
} reorderBufferEntry;

bool checkReady(deque<reorderBufferEntry>*, reorderBufferEntry*, int*, int);






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
  char microOperation[22];
  int64_t totalMicroops = 0;
  int64_t totalMacroops = 0;
  int64_t totalInstructionSize = 0;
  int64_t total4byteMacroops = 0;
  int64_t totalLoadOperation = 0;
  int64_t totalStoreOperation = 0;
  int64_t totalUnconditionalBranches = 0;
	int result = 1;
  //int64_t totalConditionalBranches = 0;

  //*** HW5 variables **//
  int reg_size = 50;	//architectural reg: remain 50 all the time
	int wayNumber = 4;
	int i = 0;
	//int insn_way_count = 0;
	int64_t currentCycle = 0;
	int	count = 0;
	string displayString;
	reorderBufferEntry microop;
	reorderBufferEntry *microopPointer;

	//reorderBufferEntry
	reorderBufferEntry commitMicroop;
	int latency = 0;
	int j = 0;
	int reorderBufferBits = 6;
	int reorderBufferTableLimit = pow(2, reorderBufferBits);
	typedef struct _mapTableEntry
	{	
		int phys_reg_source_index;
		int arch_reg_source_index;
	} mapTableEntry;
	mapTableEntry *mapTable;
	mapTable = (mapTableEntry *) malloc(reg_size * sizeof(mapTableEntry));

	//init mapTable
	for(i = 0; i < reg_size; i++) {
		mapTable[i].phys_reg_source_index = i;
		mapTable[i].arch_reg_source_index = i;
	}

	int PHY_REG_SIZE = 512;
	int new_reg;
	int registerBoard[PHY_REG_SIZE];
	for (i=0; i < PHY_REG_SIZE; ++i) {
		registerBoard[i] = 0;
  }

	deque<int> freelist;  
	// populate freelist
	for (i=0; i < PHY_REG_SIZE - reg_size; ++i) {
		freelist.push_back(i+reg_size);
  }
	deque<reorderBufferEntry> reorderBufferTable;  
	//***** HW5_variables_end *****//


  //HW3_branch variables
	int64_t totalBranchTaken = 0;
	int64_t totalBranchNotTaken = 0;
	char predictionMade = 'N';
	int64_t totalMisPredictions = 0;
	int counterBits = 10;
	int64_t counterEntry = pow(2, counterBits);
	int64_t predictionSelector = 0;
	char counterTable[counterEntry];
	int historyRegister = 0b0000000000;
	int printTrace = 0;
	int restart_penalty = 3;
	int fetchReady = 0;	
	int historyBits = 10;
	//int predictionResult = 0;
	//*** init
	for(i = 0; i < counterEntry; ++i) {
		counterTable[i] = 'N';
	}

	// Cache variables
	int missPenalty = 7;
	//**** HW4_variable ****
	char cacheDirtyBit = 'c';
	uint64_t indexAddr = 0;
	uint64_t tagAddr = 0;
	int blockBits = 4;
	int blockSize = pow(2, blockBits);
	int wayBits = std::stoi(para);
	int waySize = pow(2, wayBits);
	int cacheBits = 15;
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
	cacheEntry = (struct cacheSet*)malloc(setSize * sizeof(struct cacheSet));
	for(i = 0; i < setSize; i++) {
		cacheEntry[i].LRU = 0;
		cacheEntry[i].wayArrayPt = (wayLog*)malloc(waySize * sizeof(wayLog));
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



	printf("\n PHY_REG_SIZE: %d", PHY_REG_SIZE);
	printf("\n ROB size: %d", reorderBufferTableLimit);
	printf("\n way number: %d", wayNumber);
	printf("\n conservative memory scheduling。");
	printf("\n GShare: %d entires %d history bits。", counterEntry, historyBits);
	printf("\n data cache: %d KB size, %d way set-associativity, %dB blocks.", 
		cacheSize/1024,waySize, blockSize);


	
  while (true) {
	  //printf(" %d %d \n", currentCycle, totalMicroops);

		//printf("******Step1:Commit\n");
		//if(currentCycle == 1000) break;
		//1. Commit
			for (i=0; i<wayNumber; i++)	{
				if(reorderBufferTable.empty()) break;
				if (reorderBufferTable.front().doneCycle != -1 &&
					reorderBufferTable.front().doneCycle <= currentCycle) {
          if (reorderBufferTable.front().phys_reg_to_free1 != -1){
            freelist.push_back(reorderBufferTable.front().phys_reg_to_free1);
          }
          if (reorderBufferTable.front().phys_reg_to_free2 != -1){
            freelist.push_back(reorderBufferTable.front().phys_reg_to_free2);
          }
					reorderBufferTable.front().commitCycle = currentCycle;

					displayString = to_string(reorderBufferTable.front().sequenceNumber) + ": " +
										 to_string(reorderBufferTable.front().fetchCycle) + " " +
										 to_string(reorderBufferTable.front().issueCycle) + " " + 
										 to_string(reorderBufferTable.front().doneCycle) + " " +
										 to_string(reorderBufferTable.front().commitCycle) + ","; 
										 //reorderBufferTable.front().infoString;
				  //cout << displayString << endl;
					reorderBufferTable.pop_front();
				}
			}
		
		//printf("*****Step2: Issue\n");
		//2. Issue
		count = 0;
		j = 0;
		//foreach microop in ROB (iterating from the oldest to the youngest)?????
		//use i?
		//printf("*****Step2.1: ROB size %d \n",  (int)reorderBufferTable.size());
		for (j = 0; j < (int)reorderBufferTable.size(); ++j) {
			//printf("\n---------------------\n");		
			if (!reorderBufferTable.at(j).isIssued  
				&& checkReady(&reorderBufferTable, &reorderBufferTable.at(j), registerBoard, currentCycle) ) {
				
				if (reorderBufferTable.at(j).isLoad) {
					// add cache simulation
					// cache 
					// update the address to ROB
					indexAddr = (reorderBufferTable.at(j).addressForMemoryOp / blockSize) 
						% setSize;

					tagAddr = reorderBufferTable.at(j).addressForMemoryOp / blockSize/ setSize;
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
					blockAddress = reorderBufferTable.at(j).addressForMemoryOp 
						/ blockSize * blockSize;	//block offset bits set to zero

					// Update cache
					// if hit
					if(missOrHit == 'h') {
							latency = 4;
							// update LRU
							cacheEntry[indexAddr].LRU = (cacheEntry[indexAddr].LRU + 1) % waySize;	
							if(loadStore == 'L' ) {
								//do nothing
							}
							else 	cacheEntry[indexAddr].wayArrayPt[wayIndex].dirtyBit = 'D';
					}
					//if not hit
					if(missOrHit != 'h') {
						//on miss, update cache
						latency = 4 + missPenalty;
						cacheEntry[indexAddr].wayArrayPt[tempLRU].wayTag = tagAddr;
						if(loadStore == 'L' ) {
							cacheEntry[indexAddr].wayArrayPt[tempLRU].dirtyBit = 'C';
						}
						else 	cacheEntry[indexAddr].wayArrayPt[tempLRU].dirtyBit = 'D';
						//update LRU
						cacheEntry[indexAddr].LRU =  (cacheEntry[indexAddr].LRU + 1) % waySize;;
					}
					//reset 
					missOrHit = 'm';
				}
				else	latency = 1;

				reorderBufferTable.at(j).isIssued = true;
				reorderBufferTable.at(j).issueCycle = currentCycle;
				reorderBufferTable.at(j).doneCycle = currentCycle + latency;

				if(reorderBufferTable.at(j).registerDestination1 != -1) {
					registerBoard[reorderBufferTable.at(j).registerDestination1] = latency;
				}
				if(reorderBufferTable.at(j).registerDestination2 != -1) {
					registerBoard[reorderBufferTable.at(j).registerDestination2] = latency;
				}
				if (reorderBufferTable.at(j).isMisPredictedBranch) {
					assert(fetchReady == -1);
					fetchReady = latency + restart_penalty;  // The mis-prediction restart penalty
				}
				count = count + 1;
				//N = wayNumber?
				//printf("issudeCycle: %d ", 	reorderBufferTable.at(j).issueCycle );

				if (count == wayNumber)	{
					break;            // stop looking for instructions to execute	
				}
			}
		}//end of for loop: reorderBufferTable
		

		
		//printf("Step3: \n");
		//3. Fetch & Rename
		//At fetch, instruction are renamed and inserted in the ROB:		
		for (i=0; i<wayNumber; i++) {
			if (fetchReady > 0) {
				fetchReady = fetchReady - 1;
			  break;   // Waiting for fetch to be redirected after a branch misprediction
			}
			if (fetchReady == -1) {  
				break;   // Waiting for a mispredicted branch to execute
			}
			if(fetchReady != 0) printf("Error!!!");	
			if((int)reorderBufferTable.size() == reorderBufferTableLimit)	break;    //ROB is full
			//1. Read the next micro-op from the trace
			result = fscanf(inputFile, 
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
			//printf("before abort\n");

			if (result != 14) {
				abort();
			}

			// For each micro-op
			totalMicroops++;
			// For each macro-op:
			if (microOpCount == 1) {
				totalMacroops++;
			}
			
			//create a new reorderBufferEntry
			reorderBufferEntry microop;
			// init: microop
			microop.isMisPredictedBranch = false;
			microop.phys_reg_to_free1 = -1;		//bug: forgot to init!!!
			microop.phys_reg_to_free2 = -1;		//bug: forgot to init!!! take 4hours to debug
			microop.fetchCycle = -1;
			microop.issueCycle = -1;
			microop.doneCycle = - 1;
			microop.commitCycle = -1;
			microop.sourceRegister1 = -1;
			microop.sourceRegister2 = -1;
			microop.sourceRegister3 = -1;
			microop.registerDestination1 = -1;
			microop.registerDestination2 = -1;
			microop.arch_reg_source_index1 = -1;
			microop.arch_reg_source_index2 = -1;
			microop.arch_reg_source_index3 = -1;
			microop.arch_reg_dest_index1 = -1;
			microop.arch_reg_dest_index2 = -1;

			
			//********2. Rename the micro-op (as per the above renaming algorithm)

			if(sourceRegister1 < -1 || sourceRegister1 > 100)	{
					printf("error\n");
					abort();
			}
			//update input register
			//why 3 source register?
			if(sourceRegister1 != -1) {
				//phys_reg_source1[insn_way_count] = mapTable[sourceRegister1].phys_reg_source_index;
				microop.arch_reg_source_index1 = sourceRegister1;
				microop.sourceRegister1 = mapTable[sourceRegister1].phys_reg_source_index;
			}
			if(sourceRegister2 != -1) {
				//phys_reg_source2[insn_way_count] = mapTable[sourceRegister2].phys_reg_source_index;
				microop.arch_reg_source_index2 = sourceRegister2;
				microop.sourceRegister2 = mapTable[sourceRegister2].phys_reg_source_index;
			}
			if(conditionRegister == 'R') {
				//phys_reg_source3[insn_way_count] = mapTable[49].phys_reg_source_index;
				microop.arch_reg_source_index3 = 49;
				microop.sourceRegister3 = mapTable[49].phys_reg_source_index;
			}
			//For each valid destination register:
			//remember old output phy_reg
			
			if(destinationRegister != -1 || conditionRegister == 'W') {
				if(destinationRegister != -1)	{
					// ? how to modify this?
					microop.arch_reg_dest_index1 = destinationRegister;
					microop.phys_reg_to_free1 = mapTable[destinationRegister].phys_reg_source_index;
					//dequeue the freelist
					//what if queue is empty?
					new_reg = freelist.front();

					if(!freelist.empty()) {
						freelist.pop_front();
					}
					//update map table: map output arch_reg to freed phy reg
					mapTable[destinationRegister].phys_reg_source_index = new_reg;
					//phys_reg_destination[insn_way_count] = new_reg;		
					microop.registerDestination1 = new_reg;
				
				}	
				//?
				
				if(conditionRegister == 'W') {
					// ? how to modify this?
					microop.arch_reg_dest_index2 = 49;
					microop.phys_reg_to_free2 = mapTable[49].phys_reg_source_index;
					
					new_reg = freelist.front();
					if(!freelist.empty()) {
						freelist.pop_front();
					}
					mapTable[49].phys_reg_source_index = new_reg;
					microop.registerDestination2 = new_reg;	
				}

					for(int arrayIndex = 0; arrayIndex < freelist.size(); arrayIndex++) {
						// >0 or != 0?
					}

				//rename output reg
			}//end of destinationRegister if
			
			
			//3.3. Enqueue the micro-op into the ROB, filling in the fields as appropriate
			//which fields are necessary?
			microop.sequenceNumber = totalMicroops;
			microop.microopNumber = totalMicroops;
			microop.fetchCycle = currentCycle;
			microop.addressForMemoryOp = addressForMemoryOp;
			//how to init other para???
			microop.isIssued = false;
			string s = "";
			string microString(microOperation);
            string strMarco(macroOperation);
            string strMicro(microOperation);
            s += strMarco + " " + strMicro;
			//microop.infoString += s;

			//microop.microOperationName = microOperation;
			if(loadStore == 'L')	{
				microop.isLoad = true;
				microop.isStore = false;
			}
			else if(loadStore == 'S') {
				microop.isLoad = false;
				microop.isStore = true;
			}
			else {
				microop.isLoad = false;
				microop.isStore = false;
			}
			//experiment2
			//make branch prediction
			// begin prediction
			if(targetAddressTakenBranch != 0 && conditionRegister == 'R') {
					//if(totalConditionalBranches > 200) break;
					
					//***made predictions
					predictionSelector = (instructionAddress % counterEntry) ^ historyRegister;
					predictionMade = toupper(counterTable[predictionSelector]);
					//***branch result					
					if(TNnotBranch == predictionMade) {
						//predictionResult = 1;
						//fprintf(outputFile, "Correct");
					}
					else {
						//predictionResult = 0;
						totalMisPredictions++;
						//fprintf(outputFile, "Incorrect");
					}
					//fprintf(outputFile, "   %" PRIi64 "\n", totalMisPredictions);
					//***update the table
					historyRegister = historyRegister << 1;
					historyRegister &= ~(1 << historyBits);;
					
					if(TNnotBranch == 'T') {
						historyRegister |= 1 << 0;
						if(counterTable[predictionSelector] == 'n') 
							counterTable[predictionSelector] = 't';
						else if(counterTable[predictionSelector] == 'N') 
							counterTable[predictionSelector] = 'n';
						else if(counterTable[predictionSelector] == 't') 
							counterTable[predictionSelector] = 'T';
						else if(counterTable[predictionSelector] == 'T') 
							counterTable[predictionSelector] = 'T';
					}
					else {
						historyRegister |= 0 << 0;
						if(counterTable[predictionSelector] == 'n') 
							counterTable[predictionSelector] = 'N';
						else if(counterTable[predictionSelector] == 'N') 
							counterTable[predictionSelector] = 'N';
						else if(counterTable[predictionSelector] == 't') 
							counterTable[predictionSelector] = 'n';
						else if(counterTable[predictionSelector] == 'T') 
							counterTable[predictionSelector] = 't';
					}

				if (predictionMade != TNnotBranch ) {
					fetchReady = -1;         // Suspend fetch
					microop.isMisPredictedBranch = true;
				}

			}//end of predictor


			//enqueue from back: yes
			
			//3.4. foreach valid destination physical register
				// set the register as "not ready" by setting the correspond scoreboard entry to -1.
			if(microop.registerDestination1 >= 0) {
				registerBoard[microop.registerDestination1] = -1;
			}	
			if(microop.registerDestination2 >= 0)	{
				registerBoard[microop.registerDestination2] = -1;
			}
			reorderBufferTable.push_back(microop);

			//3.5
			//if micro-op is a taken branch/call/return:
			if(TNnotBranch == 'T') {
				break;    // Encountered taken branch, stop fetching this cycle
			}
		}// end of the reading insn step
		
		
		//printf("Step 4: advance to the next cycle\n");
		// 4. Advance to the Next Cycle
		// At the end of each cycle, we need to advance to the next cycle:
		// currentCycle = totalCycles;
		currentCycle = currentCycle + 1;
		// update scoreboard
		//printf("\n registerBoard[arrayIndex]:\n");
		for(int arrayIndex = 0; arrayIndex < PHY_REG_SIZE; arrayIndex++) {
			// >0 or != 0?
			//printf(" %d_%d  ", arrayIndex, registerBoard[arrayIndex]);
			if(registerBoard[arrayIndex] > 0) {
				--registerBoard[arrayIndex];
			}
		}
		if (result == EOF && reorderBufferTable.empty()) {
				printf("\nresult: %d %d\n", totalMicroops, currentCycle);
				break;
		}

  } // end: first while loop for cycles
} // end: func_simulate

bool checkReady(deque<reorderBufferEntry>  *reorderBufferTable, reorderBufferEntry *microopPointer, int *registerBoardPointer, int currentCycle) {
		//set nextInstruction to 1, then modify it
		//use pointer or not?
		//printf("checkReady: ");
		int i = 0;
		bool isReady  = true;
		
		if((*microopPointer).sourceRegister1 != -1 ) {
			if(registerBoardPointer[(*microopPointer).sourceRegister1] != 0) isReady = false;
		}
		if((*microopPointer).sourceRegister2 != -1 ) {
			if(registerBoardPointer[(*microopPointer).sourceRegister2] != 0) isReady = false;
		}
		if((*microopPointer).sourceRegister3 != -1 ) {
			if(registerBoardPointer[(*microopPointer).sourceRegister3] != 0) isReady = false;
		}
		if ((*microopPointer).isLoad) {
			//foreach op2 in ROB:
			for(i = 0; i < (*reorderBufferTable).size(); ++i) {
				// op2.isNotDone(): op.issued is true and DoneCycle is <= Current Cycle
				// (op2.isStore) and (op2.SequenceNum < op.SequenceNum) and (op2.isNotDone())
				if ((*reorderBufferTable).at(i).isStore && 
					((*reorderBufferTable).at(i).sequenceNumber < (*microopPointer).sequenceNumber) &&
				  !((*reorderBufferTable).at(i).isIssued && ((*reorderBufferTable).at(i).doneCycle <= currentCycle))) {
					isReady = false;
				}
			}
		}
		return isReady;
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
