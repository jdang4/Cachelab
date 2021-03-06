#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <math.h>

typedef struct Block {
	char isValid;       // sees if the cache block is valid
	unsigned long tag;  // tag bit
	int data;           // the data wihtin the cache block

} Block;

typedef struct cache {
	int s;
	int S;
	int b;
	int B;
	int E;

	int numOfBlocks;
	int Size;  // holds the size for the specific cache

	Block *cacheBlock;  // each cache holds a cache block structure 
} Cache;

int hits = 0;
int misses = 0;
int evictions = 0;

void message() {
	printf ("Format: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile> \n");
	printf ("-h : Optional help flag that prints usage info\n");
	printf ("-v : Optional verbose flag that displays trace info \n");
	printf ("-s <s> : Number of set index bits (the number of sets is 2s)\n");
	printf ("-E <E> : Associativity (number of lines per set)\n");
	printf ("-b <b>: Number of block bits (the block size is 2b)\n");
	printf ("-t <tracefile> : Name of the valgrind trace to replay\n");

	exit(0);
}

//gets the size from the second part of the tracefile
char* getSize(char* address) {
	int stringLength = strlen(address);
	char* Size = malloc(stringLength * sizeof(char));

	for(int i = 0; i <= stringLength; i++) {
		if(address[i] == ',') {
			Size[0] = address[i + 1];
			Size[1] = '\0';
			break;
		}
	}

	return Size;
}

// this removes the comma from the address
char* removeComma(char* address) {
	int stringLength = strlen(address);
	char* new = malloc(stringLength * sizeof(char));

	for (int i = 0; i <= stringLength; i++) {
		if (address[i] != ',') {
			new[i] = address[i];
		}

		else {
			new[i] = '\0';
			break;
		}
	}

	return new;
}

// this checks to see if there is a matching block within the row of the set index
void checkBlockMatch(Cache cache, int index, int tagBits, char *inst, int* match, int* lineIndex) {

	for (int i = 0; i < cache.E; i++) {
		if ( cache.cacheBlock[(index * cache.E) + i].isValid == 1 &&
			cache.cacheBlock[(index * cache.E) + i].tag == tagBits) {

			hits++;
			cache.cacheBlock[(index * cache.E) + i].data = *lineIndex;
			*match = 1;

			if (!(strcmp(inst, "M"))) {
				hits++;
				printf("Hit ");
			}
			printf("Hit\n");
			break;
	    }
	}
}

// this checks to see if there are any empty lines 
void checkEmptyLines(Cache cache, int index, int* isFull, int* evictionIndex) {

	  for (int i = 0; i < cache.E; i++) {
	  	if ( cache.cacheBlock[((index * cache.E)+i)].isValid != 1) {
	  		*evictionIndex = i;
	  		*isFull = 0;
	  		break;
	  	}
	  }
}

// this basically handles the event where there is an eviction
void cacheEviction(Cache cache, int* isFull, int index, int* temp, int* evictionIndex, int* evict) {
	if (*isFull == 1) {
		for (int i = 0; i < cache.E; i++) {
			// if the value of the data is lower than the current value of temp, then set index value
			// of the evicted block to the current i position
			// used to determine the index value of the evicted block
			if (cache.cacheBlock[(index * cache.E) + i].data < *temp) {
				*temp = cache.cacheBlock[(index * cache.E) + i].data;
				*evictionIndex = i;
			}
		}
		evictions++;
		*evict = 1; // there has been an eviction that occurred so set to 1 (aka true)
	}
}


int main(int argc, char *argv[]) {

	if (argc == 1) {
		message();
	}

	Cache cache;
	// this goes through each of the options that were specified within the format
	char option;
	int vFlag = 0; // initially the validility is set to 0 (not valid)
	char *file;

	// Putting in the arguments	
	while ((option = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
		switch (option) {
			case ('h') :
				message();
				exit(0);
			case ('v') : 
				vFlag = 1;  // right now this is giving error, try to fix this
				break;
			case ('s') :
				if (atoi(optarg) <= 0) {
					printf ("The 's' bits cannot be less than or equaled to 0\n");
					exit(0);
				}

				cache.s = atoi(optarg);
				cache.S = pow(2, cache.s); // the number of sets (cache.S) is 2^(cache.s)
				break;

			case ('E') :
				if (atoi(optarg) <= 0) {
					printf ("The 'E' bits cannot be less than or equaled to 0\n");
					exit(0);
				}

				cache.E = atoi(optarg);
				break;
			case ('b') :
				if (atoi(optarg) <= 0) {
					printf ("The 'b' bits cannot be less than or equaled to 0\n");
					exit(0);
				}

				cache.b = atoi(optarg);
				cache.B = pow(2, cache.b);
				break;

			case ('t') :
				file = optarg;  // right now this is giving error, try to fix this
				break;

			default :
				message();
		}
	}

	// tracing the file stored at the t bit (setting our access to read)
	FILE *trace = fopen(file, "r");

	if (!trace) {
		printf("File does not exist\n");
		exit(0);
	}

	// calculating the number of blocks within the cache
	cache.numOfBlocks = cache.S * cache.B;

	// calculating the total size of the cache
	cache.Size = cache.numOfBlocks * cache.B;

	// allocating space for the cacheBlock struct within the cache struct
	cache.cacheBlock = malloc(cache.Size * sizeof(Block));

	// first initialize the cache to 0
	for (int i = 0; i < cache.numOfBlocks; ++i) {
		cache.cacheBlock[i].isValid = 0;
		cache.cacheBlock[i].tag = 0;
		cache.cacheBlock[i].data = 0;
	}

	char operation[10][10]; // will store all the operations
	char address[10][10];  // will store all the addresses
	int lineIndex = 1; //used as a counter used, which would be used to check for evictions

	char* ptr; // needed in order to convert address to hexidecimal
	while(fscanf(trace, "%s %s ", *operation, *address) != EOF) {

		char* addressNC = removeComma(*address); // a version of address but without the comma
		char* Size = getSize(*address); // gets the size from the tracefile
		int hexVersion = strtol(addressNC, &ptr, 16); // converts address to hexidecimal address

		if (strcmp(*operation, "I")) {

			if (vFlag == 1) {
				printf(" %s %s, %s ", *operation, addressNC, Size);
			}

			// calculations to get the tag bits within each block
			int tagBit = hexVersion >> (cache.b + cache.s);

			// this will hold the set index within the address of the hexidecimal address
			int setIndex = ( (hexVersion ^ (tagBit << (cache.b + cache.s))) >> cache.b);

			int match = 0; // this will indicate if there is a matching block within the cache 
			int evict = 0; // this will indicate if there is an eviction that occured


			checkBlockMatch(cache, setIndex, tagBit, *operation, &match, &lineIndex);

			// if there is no match, then overwrite into the cache
			if (!match) {
				misses++; // not in cache, so miss
				printf("miss ");
				int isFull = 1; // checks to see if any space left
				int evictionIndex; // this will store the index of the evicted block 
				int temp = lineIndex; // temporarily holds the value of the line index

				checkEmptyLines(cache, setIndex, &isFull, &evictionIndex);

				cacheEviction(cache, &isFull, setIndex, &temp, &evictionIndex, &evict);

				cache.cacheBlock[(setIndex * cache.E) + evictionIndex].isValid = 1;
				cache.cacheBlock[(setIndex * cache.E) + evictionIndex].tag = tagBit;
				cache.cacheBlock[(setIndex * cache.E) + evictionIndex].data = lineIndex;

				if (evict) {
					printf("eviction ");
				}

				if (!(strcmp(*operation, "M"))) {
					hits++;
					printf("hit");
				}

				printf("\n");
			}

			lineIndex++;

		}
	}

	printf("Congratulations!!! You have proven that P = NP\n");

    printSummary(hits, misses, evictions);
    return 0;
}



