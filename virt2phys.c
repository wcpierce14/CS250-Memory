# include <stdio.h>
# include <stdlib.h>
# include <string.h>

int log2(int n) {
    int r=0;
    while (n>>=1) r++;
        return r; }

int ones(int n){
    return ((1 << n) - 1);
}





int main(int argc, char* argv[]){
    // Open and read the .txt file. Also load virtual address
    FILE *f;
    f = fopen(argv[1], "r");
    int virtAddr;
    sscanf(argv[2], "%x", &virtAddr);

    // Scan the address bits
    int addBits;
    fscanf(f, "%d", &addBits);

    // Scan the page size
    int pageSize;
    fscanf(f, "%d", &pageSize);

    // Find the number of bits in the page offset
    int pageOffset = log2(pageSize);

    //Find number of page numbers
    int totalPages = (1 << (addBits - pageOffset));

    // Get page number
    int virtualPageNum = virtAddr >> pageOffset;
    // printf("%d\n", virtualPageNum);

    // Get low pageOffset # of bits
    int mask;
    int lastXbits;
    mask = ones(pageOffset);
    lastXbits = virtAddr & mask;

    // Find the physical page number
    int arr[totalPages];
    for(int i = 0; i <= totalPages; i++) {
        int physicalPageNum;
        fscanf(f, "%d", &physicalPageNum);
        arr[i] = physicalPageNum;
    }

    int physicalPageAddr;
    int physicalPageNum = arr[virtualPageNum];

    if(physicalPageNum == -1){
        printf("PAGEFAULT\n");
        return EXIT_SUCCESS;
    }

    // Combine physical page number with page offset
    physicalPageAddr = lastXbits + (physicalPageNum << pageOffset);

    printf("%x\n", physicalPageAddr);

    return EXIT_SUCCESS;
}





