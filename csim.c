#include "cachelab.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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

char* removeComma(char* address) {
	int stringLength = strlen(address);
	char* new = malloc(stringLength * sizeof(char));

	for (int i = 0; i < stringLength; i++) {
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
void checkBlockMatch(Cache cache, int index, int tagBits, const char *inst, int* match, int* lineIndex) {

	for (int i = 0; i < cache.E; i++) {
		if ( cache.cacheBlock[(index * cache.E)].isValid == 1 &&
			cache.cacheBlock[(index * cache.E)].tag == tagBits) {

			hits++;
			cache.cacheBlock[(index * cache.E)].data = *lineIndex;
			*match = 1;

			if (!(strcmp(inst, "M"))) {
				hits++;
			}
			break; //don't need to continue checking
	    }
	}
}

void checkEmptyLines(Cache cache, int index, _Bool* isActive, int* evictionIndex) {
	/*
	checks for any empty lines 
	a.) basically iterates through the lines and checking to see if any have isValid set to 1
	b.) if none are set to 1, set the block to not being in use
	c.) store the index value so that it can be used to determine the index at which there will be eviction
	*/
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
				exit(1);
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

	char operation; 
	char address; 
	int size; // specifies the number of bytes accessed by the operation
	int lineIndex = 1; //used as a counter used to check for evictions

	char* ptr; // needed in order to convert address to hexidecimal
	while(fscanf(trace, "%s %s %d", &operation, &address, &size) != EOF) {

		char* addressNC = removeComma(&address); // a version of address but without the comma
		int hexVersion = strtol(addressNC, &ptr, 16); // converts address to hexidecimal address

		if (strcmp(&operation, "I")) {

			// calculations to get the tag bits within each block
			int tagBit = hexVersion >> (cache.b + cache.s);

			// this will hold the set index within the address of the hexidecimal address
			int setIndex = ( (hexVersion ^ (tagBit << (cache.b + cache.s))) >> cache.b);

			int match = 0; // this will indicate if there is a matching block within the cache 


			checkBlockMatch(cache, setIndex, tagBit, &operation, &match, &lineIndex);

			// if there is no match, then overwrite into the cache
			if (!match) {
				misses++;
				_Bool active = true;
				evictionIndex = 0; 
				checkEmptyLines(cache, setIndex, &active, &evictionIndex);

				// Eviction Part
			}

		}
	}


    printSummary(hits, misses, evictions);
    return 0;
}



