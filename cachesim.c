# include <stdio.h>
# include <stdlib.h>
# include <string.h>

// Functions and struct for blocks
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int log2(int n) {
    int r=0;
    while (n>>=1) r++;
        return r; }

int ones(int n){
    return ((1 << n) - 1);
}

struct block{
    int valid;
    int tag;
    int way;
};

int simMem[16777216];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]){

    // Load file and all arguments

    FILE *f;
    f = fopen(argv[1], "r");

    int cacheSize;
    sscanf(argv[2], "%d", &cacheSize);

    int assoc;
    sscanf(argv[3], "%d", &assoc);

    int blockSize;
    sscanf(argv[4], "%d", &blockSize);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Calculate the parameters of the cache

    int offset_bits = log2(blockSize);
    int numSets = (cacheSize * 1024 / blockSize) / assoc;
    int index_bits = log2(numSets);
    int tag_bits = 24 - offset_bits - index_bits;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Initialize cache
    struct block cache[numSets][assoc];

    for(int i = 0; i < numSets; i++){
        for(int k = 0; k < assoc; k++){
            struct block newBlock;
            cache[i][k] = newBlock;
            cache[i][k].valid = 0;
            cache[i][k].tag = -1;
        }
    }

    // Initialize 16MB of memory

    for(int i = 0; i < 16777216; i++){
        simMem[i] = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Declare variables that will be used within the while loop
    char instType[64];
    char store[] = "store";
    char load[] = "load";
    char hit[] = "hit";
    char miss[] = "miss";
    char beginAddr;
    int addr;
    int byteSize;
    int dataValue;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Begin loop
    int wayHit;
    while(fscanf(f, "%s", instType) != EOF){

        fscanf(f, "%2hhx", &beginAddr); // scan in the 0x from address --> we dont need this
        fscanf(f, "%x", &addr); // read in the rest of the address


        // Break down the address into components
        int block_offset = addr & ones(offset_bits);
        int index = (addr >> offset_bits) & ones(index_bits);
        int tag = addr >> (offset_bits + index_bits);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // If store....
        if(strcmp(instType, store) == 0){
            fscanf(f, "%d", &byteSize); // read in the byte size of data
            int status = 0;
            //int setHit;
            wayHit = 0;

            // Check if data is in cache
            for(int k = 0; k < assoc; k++){
                if(cache[index][k].valid == 1 && cache[index][k].tag == tag){
                    // If we reach this point, we have a cache hit!!
                    status ++; // increment status by 1
                    wayHit = k;
                }
            }
            

            // Commit change to memory regardless of whether it's a cache hit or miss
            for(int i = 0; i < byteSize; i++){
                    char newByte;
                    fscanf(f, "%2hhx", &newByte);
                    simMem[addr + i] = newByte;
            }


            // Update cache 
            if(status == 1){
                // If status == 1, we have a cache hit!! So we want to update the cache block for LRU purposes
                struct block blockHit = cache[index][wayHit];
                if(assoc == 1){
                    cache[index][0] = blockHit;
                }
                else if(wayHit == 0){
                    // do nothing
                }
                else{
                    for(int j = wayHit - 1; j >= 0; j--){
                    cache[index][j + 1] = cache[index][j];
                    }
                    cache[index][0] = blockHit; 
                }
                // Print the necessary output
                printf("%s ", instType);
                printf("0x%x ", addr);
                printf("%s\n", hit);
            }
            else{
               // Print the necessary output
                printf("%s ", instType);
                printf("0x%x ", addr);
                printf("%s\n", miss);
            }
        }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // If load....
        else{
            fscanf(f, "%d", &byteSize); // read in the byte size of data

            // Check if this block is in the cache
            wayHit = 0;
            int status = 0;

            for(int j = 0; j < assoc; j++){
                if(cache[index][j].valid == 1 && cache[index][j].tag == tag){
                    // If we reach this point, we have a cache hit!!
                    status ++;
                    wayHit = j; // keep track of which way the block was stored in
                }
            }

            if(status == 1){
                // If we have a cache hit, update LRU and return data
                struct block blockHit = cache[index][wayHit]; // this is the block that hit!

                // Update the ordering within the set for LRU
                if(assoc == 1){
                    cache[index][0] = blockHit;
                }
                else if(wayHit == 0){
                     // Do nothing
                }
                else{
                    for(int j = wayHit - 1; j >= 0; j--){
                    cache[index][j + 1] = cache[index][j];
                    }
                cache[index][0] = blockHit; // store the block that we just loaded as the most recently used
                }
                
                // Print the proper output
                printf("%s ", instType);
                printf("0x%x ", addr);
                printf("%s ", hit); 
                for(int h = 0; h < byteSize; h++){
                    int byteFromMem = simMem[addr + h];
                    printf("%02hhx", byteFromMem);
                }
                printf("\n");
            }

            else{
                // If we do NOT have a cache hit, locate a cache block in main memory to use
                struct block tempBlock; // create temporary block that will be put into cache
                tempBlock.valid = 1; // initialize the block's instance variables
                tempBlock.tag = tag;


                // ensure LRU is satisfied
                if(assoc == 1){
                    cache[index][0] = tempBlock;
                }
                else{
                    for(int i = assoc - 2; i >= 0; i--){
                    cache[index][i + 1] = cache[index][i];
                    }
                cache[index][0] = tempBlock;
                }

                // Print out the proper output
                printf("%s ", instType);
                printf("0x%x ", addr);
                printf("%s ", miss); 
                for(int h = 0; h < byteSize; h++){
                    int byteFromMem = simMem[addr + h];
                    printf("%02hhx", byteFromMem);
                }
                printf("\n");
            }
        }
    }
}
