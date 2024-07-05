#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include<time.h>

// Define cache parameters
int SIZE_OF_CACHE;
int BLOCK_SIZE;
int ASSOCIATIVITY;
char REPLACEMENT_POLICY[10];
char WRITEBACK_POLICY[10];
int numCacheLines;

int hexCharToDecimal(char hex) {
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    } else if (hex >= 'a' && hex <= 'f') {
        return hex - 'a' + 10;
    } else if (hex >= 'A' && hex <= 'F') {
        return hex - 'A' + 10;
    }
    return 0;  
}

char* hexToBinary(const char* hexString) {
    static const char* hexToBinaryTable[16] = {
        "0000", "0001", "0010", "0011",
        "0100", "0101", "0110", "0111",
        "1000", "1001", "1010", "1011",
        "1100", "1101", "1110", "1111"
    };

    char binaryString[33];
    for (int i = 0; i < 32; i++) {
        binaryString[i] = '0';
    }
    binaryString[32] = '\0';

    int hexStringLength = strlen(hexString);
    if (hexStringLength != 8) {
        
        return NULL;
    }

    for (int i = 0; i < 8; i++) {
        int decimalValue = hexCharToDecimal(hexString[i]);
        strncpy(binaryString + i * 4, hexToBinaryTable[decimalValue], 4);
    }
    return strdup(binaryString);
}

void extractAddress(char* address, int* setIndex, char* tag, char* offsetString, char* indexString, char* tagString) {
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    char slicedString[32];
    slicedString[8]='\0';
    strncpy(slicedString, address+2, 8);
    address=hexToBinary(slicedString);

    offsetString[offsetBits] = '\0';
    if(setIndexBits==0){
        indexString[0]='0';
        indexString[1] = '\0';
    }
    else{
        indexString[setIndexBits] = '\0';
        strncpy(indexString, address + tagBits, setIndexBits);
    }
    
    tagString[tagBits] = '\0';
    strncpy(offsetString, address + (32 - offsetBits), offsetBits);
    strncpy(tagString, address, tagBits);
}

char* binaryToHex(char* binaryString) {
    int length = strlen(binaryString);
    int extra = (4 - (length % 4)) % 4;
    char* paddedBinaryString = (char*)malloc(length + extra + 1);
    strcpy(paddedBinaryString, "");
    for (int i = 0; i < extra; i++) {
        strcat(paddedBinaryString, "0");
    }
    strcat(paddedBinaryString, binaryString);
    int hexLength = (strlen(paddedBinaryString) + 3) / 4; 
    char* hexString = (char*)malloc(hexLength + 1);
    for (int i = 0; i < hexLength; i++) {
        char arra[5];
        strncpy(arra, paddedBinaryString + i * 4, 4);
        arra[4] = '\0';
        int decimalValue = strtol(arra, NULL, 2);
        sprintf(hexString + i, "%X", decimalValue);
    }

    free(paddedBinaryString);

    return hexString;
}


int binaryToDecimal(const char* binaryString) {
    int result = 0;
    int multiplier = 1;
    int length = strlen(binaryString);

    for (int i = length - 1; i >= 0; i--) {
        if (binaryString[i] == '1') {
            result += multiplier;
        } else if (binaryString[i] == '0') {
        }  
        multiplier *= 2;
    }

    return result;
}

void simulateReadAccessAssociative(char mode, char* address, int Indexbit, char* arr2[Indexbit][ASSOCIATIVITY],int valid[Indexbit][ASSOCIATIVITY],int dirty[Indexbit][ASSOCIATIVITY]) {
    int setIndex;
    char tag[32];
    char offsetString[32];
    char indexString[32];
    char tagString[32];
    extractAddress(address, &setIndex, tag,offsetString, indexString, tagString);
    setIndex=binaryToDecimal(indexString);

    int cacheIndex = setIndex; 
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    int flag=0;
    
    char* hexIndex = binaryToHex(indexString);
    char* hexTag = binaryToHex(tagString);

    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (arr2[setIndex][i] != NULL && strcmp(tagString, arr2[setIndex][i]) == 0) {
            printf("Address: %s, Set:0x%s, Hit, Tag: 0x%s\n", address, hexIndex, hexTag);
            flag = 1;
            break; 
        }
    }
    if (flag==0){
        printf("Address: %s, Set:0x%s, Miss, Tag:0x%s\n", address, hexIndex, hexTag); 

        int randomNum = rand() % ASSOCIATIVITY;
        arr2[setIndex][randomNum] = strdup(tagString);
        valid[setIndex][randomNum]=1;
        dirty[setIndex][randomNum]=0;
    }

}

void simulateWriteAccessWB(char mode, char* address, int Indexbit, char* arr2[Indexbit][ASSOCIATIVITY],int valid[Indexbit][ASSOCIATIVITY],int dirty[Indexbit][ASSOCIATIVITY]) {
    int setIndex;
    char tag[32];
    char offsetString[32];
    char indexString[32];
    char tagString[32];
    extractAddress(address, &setIndex, tag,offsetString, indexString, tagString);

    setIndex=binaryToDecimal(indexString);

    int cacheIndex = setIndex; 
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    int flag=0;
    char* hexIndex = binaryToHex(indexString);
    char* hexTag = binaryToHex(tagString);
    
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (arr2[setIndex][i] != NULL && strcmp(tagString, arr2[setIndex][i]) == 0) {
            printf("Address: %s, Set:0x%s, Hit, Tag: 0x%s\n", address, hexIndex, hexTag);
            flag = 1;
            break; 
        }
    }
    if (flag==0){
        printf("Address: %s, Set:0x%s, Miss, Tag:0x%s\n", address, hexIndex, hexTag); 
        int randomNum = rand() % ASSOCIATIVITY;
        arr2[setIndex][randomNum] = strdup(tagString);
        valid[setIndex][randomNum]=1;
        dirty[setIndex][randomNum]=1;
    }
}

void simulateWriteAccessWT(char mode, char* address, int Indexbit, char* arr2[Indexbit][ASSOCIATIVITY],int valid[Indexbit][ASSOCIATIVITY],int dirty[Indexbit][ASSOCIATIVITY]) {
    int setIndex;
    char tag[32]; 
    char offsetString[32];
    char indexString[32];
    char tagString[32];
    extractAddress(address, &setIndex, tag,offsetString, indexString, tagString);

    setIndex=binaryToDecimal(indexString);

    int cacheIndex = setIndex; 
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    int flag=0;
    char* hexIndex = binaryToHex(indexString);
    char* hexTag = binaryToHex(tagString);
    
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (arr2[setIndex][i] != NULL && strcmp(tagString, arr2[setIndex][i]) == 0) {
            printf("Address: %s, Set:0x%s, Hit, Tag: 0x%s\n", address, hexIndex, hexTag);
            flag = 1;
            break; 
        }
    }
    if (flag==0){
        printf("Address: %s, Set:0x%s, Miss, Tag:0x%s\n", address, hexIndex, hexTag); 
    }

}

void simulateReadAccessAssociativeFIFO(char mode, char* address, int Indexbit, char* arr2[Indexbit][ASSOCIATIVITY], int FIFO_Index[Indexbit],int valid[Indexbit][ASSOCIATIVITY],int dirty[Indexbit][ASSOCIATIVITY]) {
    int setIndex;
    char tag[32]; 
    char offsetString[32];
    char indexString[32];
    char tagString[32];
    extractAddress(address, &setIndex, tag,offsetString, indexString, tagString);
    setIndex=binaryToDecimal(indexString);

    int cacheIndex = setIndex; 
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    int flag=0;
    char* hexIndex = binaryToHex(indexString);
    char* hexTag = binaryToHex(tagString);
    
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (arr2[setIndex][i] != NULL && strcmp(tagString, arr2[setIndex][i]) == 0) {
            //printf(" ** ");
            printf("Address: %s, Set:0x%s, Hit, Tag: 0x%s\n", address, hexIndex, hexTag);
            flag = 1;
            break; 
        }
    }
    if (flag==0){
        printf("Address: %s, Set:0x%s, Miss, Tag:0x%s\n", address, hexIndex, hexTag); 
        arr2[setIndex][FIFO_Index[setIndex]] = strdup(tagString);
        dirty[setIndex][FIFO_Index[setIndex]]=0;
        valid[setIndex][FIFO_Index[setIndex]]=1;
        FIFO_Index[setIndex]=(FIFO_Index[setIndex]+1) % ASSOCIATIVITY;
    }

}

void simulateReadAccessAssociativeLRU(char mode, char* address, int Indexbit, char* arr2[Indexbit][ASSOCIATIVITY], int LRU_Index[Indexbit][ASSOCIATIVITY],int valid[Indexbit][ASSOCIATIVITY],int dirty[Indexbit][ASSOCIATIVITY]) {
    int setIndex;
    char tag[32]; 
    char offsetString[32];
    char indexString[32];
    char tagString[32];
    extractAddress(address, &setIndex, tag,offsetString, indexString, tagString);

    setIndex=binaryToDecimal(indexString);

    int cacheIndex = setIndex; 
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    int flag=0;
    char* hexIndex = binaryToHex(indexString);
    char* hexTag = binaryToHex(tagString);
    
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (arr2[setIndex][i] != NULL && strcmp(tagString, arr2[setIndex][i]) == 0) {
            printf("Address: %s, Set:0x%s, Hit, Tag: 0x%s\n", address, hexIndex, hexTag);
            LRU_Index[setIndex][i]--;
            flag = 1;
            break;
        }
    }
    if (flag==0){
        printf("Address: %s, Set:0x%s, Miss, Tag:0x%s\n", address, hexIndex, hexTag); 
        int max=0;
        for(int i=0;i<ASSOCIATIVITY;i++){
            if(LRU_Index[setIndex][i]>LRU_Index[setIndex][max]){
                max=i;
            }
        }
        
        arr2[setIndex][max] = strdup(tagString);
        valid[setIndex][max]=1;
        dirty[setIndex][max]=0;
        LRU_Index[setIndex][max]--;
    }
}

void simulateWriteAccessLRUWB(char mode, char* address, int Indexbit, char* arr2[Indexbit][ASSOCIATIVITY], int LRU_Index[Indexbit][ASSOCIATIVITY],int valid[Indexbit][ASSOCIATIVITY],int dirty[Indexbit][ASSOCIATIVITY]) {
    int setIndex;
    char tag[32];
    char offsetString[32];
    char indexString[32];
    char tagString[32];
    extractAddress(address, &setIndex, tag,offsetString, indexString, tagString);

    setIndex=binaryToDecimal(indexString);

    int cacheIndex = setIndex; 
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    int flag=0;
    char* hexIndex = binaryToHex(indexString);
    char* hexTag = binaryToHex(tagString);
    
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (arr2[setIndex][i] != NULL && strcmp(tagString, arr2[setIndex][i]) == 0) {
            printf("Address: %s, Set:0x%s, Hit, Tag: 0x%s\n", address, hexIndex, hexTag);
            LRU_Index[setIndex][i]--;
            flag = 1;
            break;
        }
    }
    if (flag==0){
        printf("Address: %s, Set:0x%s, Miss, Tag:0x%s\n", address, hexIndex, hexTag); 
        int max=0;
        for(int i=0;i<ASSOCIATIVITY;i++){
            if(LRU_Index[setIndex][i]>LRU_Index[setIndex][max]){
                max=i;
            }
        }
        arr2[setIndex][max] = strdup(tagString);
        dirty[setIndex][max]=1;
        valid[setIndex][max]=1;
        LRU_Index[setIndex][max]--;
    }

}

void simulateWriteAccessLRUWT(char mode, char* address, int Indexbit, char* arr2[Indexbit][ASSOCIATIVITY], int LRU_Index[Indexbit][ASSOCIATIVITY],int valid[Indexbit][ASSOCIATIVITY],int dirty[Indexbit][ASSOCIATIVITY]) {
    int setIndex;
    char tag[32]; 
    char offsetString[32];
    char indexString[32];
    char tagString[32];
    extractAddress(address, &setIndex, tag,offsetString, indexString, tagString);

    setIndex=binaryToDecimal(indexString);

    int cacheIndex = setIndex; 
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    int flag=0;

    char* hexIndex = binaryToHex(indexString);
    char* hexTag = binaryToHex(tagString);

    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (arr2[setIndex][i] != NULL && strcmp(tagString, arr2[setIndex][i]) == 0) {
            printf("Address: %s, Set:0x%s, Hit, Tag: 0x%s\n", address, hexIndex, hexTag);
            LRU_Index[setIndex][i]--;
            flag = 1;
            break;
        }
    }
    if (flag==0){
        printf("Address: %s, Set:0x%s, Miss, Tag:0x%s\n", address, hexIndex, hexTag); 
    }

}

void simulateWriteAccessFIFOWB(char mode, char* address, int Indexbit, char* arr2[Indexbit][ASSOCIATIVITY], int FIFO_Index[Indexbit],int valid[Indexbit][ASSOCIATIVITY],int dirty[Indexbit][ASSOCIATIVITY]) {
    int setIndex;
    char tag[32]; 
    char offsetString[32];
    char indexString[32];
    char tagString[32];
    extractAddress(address, &setIndex, tag,offsetString, indexString, tagString);
    setIndex=binaryToDecimal(indexString);

    int cacheIndex = setIndex; 
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    int flag=0;
    char* hexIndex = binaryToHex(indexString);
    char* hexTag = binaryToHex(tagString);

    
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (arr2[setIndex][i] != NULL && strcmp(tagString, arr2[setIndex][i]) == 0) {
           printf("Address: %s, Set:0x%s, Hit, Tag: 0x%s\n", address, hexIndex, hexTag);
            flag = 1;
            break; 
        }
    }
    if (flag==0){
        printf("Address: %s, Set:0x%s, Miss, Tag:0x%s\n", address, hexIndex, hexTag); 
        arr2[setIndex][FIFO_Index[setIndex]] = strdup(tagString);
        dirty[setIndex][FIFO_Index[setIndex]]=1;
        valid[setIndex][FIFO_Index[setIndex]]=1;
        FIFO_Index[setIndex]=(FIFO_Index[setIndex]+1) % ASSOCIATIVITY;
        
    }

}

void simulateWriteAccessFIFOWT(char mode, char* address, int Indexbit, char* arr2[Indexbit][ASSOCIATIVITY], int FIFO_Index[Indexbit],int valid[Indexbit][ASSOCIATIVITY],int dirty[Indexbit][ASSOCIATIVITY]) {
    int setIndex;
    char tag[32]; 
    char offsetString[32];
    char indexString[32];
    char tagString[32];
    extractAddress(address, &setIndex, tag,offsetString, indexString, tagString);

    setIndex=binaryToDecimal(indexString);

    int cacheIndex = setIndex; 
    int offsetBits = log2(BLOCK_SIZE);
    int setIndexBits = log2(numCacheLines / ASSOCIATIVITY);
    int tagBits = 32 - offsetBits - setIndexBits;
    int flag=0;
    char* hexIndex = binaryToHex(indexString);
    char* hexTag = binaryToHex(tagString);
    
    for (int i = 0; i < ASSOCIATIVITY; i++) {
        if (arr2[setIndex][i] != NULL && strcmp(tagString, arr2[setIndex][i]) == 0) {
            printf("Address: %s, Set:0x%s, Hit, Tag: 0x%s\n", address, hexIndex, hexTag);
            flag = 1;
            break;
        }
    }
    if (flag==0){
        printf("Address: %s, Set:0x%s, Miss, Tag:0x%s\n", address, hexIndex, hexTag);     
    }
}
int main() {
    srand(time(NULL));
    FILE *configFile = fopen("cache.config", "r");
    if (!configFile) {
        perror("Error opening configuration file");
        return 1;
    }

    fscanf(configFile, "%d", &SIZE_OF_CACHE);
    fscanf(configFile, "%d", &BLOCK_SIZE);
    fscanf(configFile, "%d", &ASSOCIATIVITY);
    fscanf(configFile, "%s", REPLACEMENT_POLICY);
    fscanf(configFile, "%s", WRITEBACK_POLICY);
    fclose(configFile);
    numCacheLines = SIZE_OF_CACHE / BLOCK_SIZE;
    FILE *accessFile = fopen("cache.access", "r");
    if (!accessFile) {
        perror("Error opening access file");
        return 1;
    }
    int Indexbit;
    char mode;
    char address[12]; 
    if(ASSOCIATIVITY==0){
        Indexbit=1;
        ASSOCIATIVITY=numCacheLines;
    }
    else{
        Indexbit=numCacheLines / ASSOCIATIVITY;
    }
    char* arr2[Indexbit][ASSOCIATIVITY];
    for (int i = 0; i < Indexbit; ++i) {
        for (int j = 0; j < ASSOCIATIVITY; ++j) {
            arr2[i][j] = NULL;
        }
    }
    int dirty[Indexbit][ASSOCIATIVITY];
    int valid[Indexbit][ASSOCIATIVITY];
    for (int i = 0; i < Indexbit; ++i) {
        for (int j = 0; j < ASSOCIATIVITY; ++j) {
            dirty[i][j]=0;
            valid[i][j]=0;
        }
    }
    int FIFO_Index[Indexbit];
    int LRU_Index[Indexbit][ASSOCIATIVITY];
    for(int i=0;i<Indexbit;i++){
        FIFO_Index[i]=0;
    }
    for(int i=0;i<Indexbit;i++){
        for(int j=0;j<ASSOCIATIVITY;j++){
            LRU_Index[i][j]=100;
        }
    }
    while (fscanf(accessFile, " %c: %s", &mode, address) == 2) {
        if(strcmp(REPLACEMENT_POLICY,"FIFO")==0 && strcmp(WRITEBACK_POLICY,"WB")==0){
            if(mode=='R'){
                
            simulateReadAccessAssociativeFIFO(mode, address,Indexbit,arr2,FIFO_Index,valid,dirty);
            }
            else{
            simulateWriteAccessFIFOWB(mode, address,Indexbit,arr2,FIFO_Index,valid,dirty);
            }
        }
        else if(strcmp(REPLACEMENT_POLICY,"LRU")==0 && strcmp(WRITEBACK_POLICY,"WB")==0){
            if(mode=='R'){
            simulateReadAccessAssociativeLRU(mode, address,Indexbit,arr2,LRU_Index,valid,dirty);
            }
            else{
            simulateWriteAccessLRUWB(mode, address,Indexbit,arr2,LRU_Index,valid,dirty);
            }
        }
        else if(strcmp(REPLACEMENT_POLICY,"RANDOM")==0 && strcmp(WRITEBACK_POLICY,"WB")==0){
            if(mode=='R'){
            simulateReadAccessAssociative(mode, address,Indexbit,arr2,valid,dirty);
            }
            else{
            simulateWriteAccessWB(mode, address,Indexbit,arr2,valid,dirty);  
            }
        }
        else if(strcmp(REPLACEMENT_POLICY,"FIFO")==0 && strcmp(WRITEBACK_POLICY,"WT")==0){

            if(mode=='R'){
                
            simulateReadAccessAssociativeFIFO(mode, address,Indexbit,arr2,FIFO_Index,valid,dirty);
            }
            else{
            simulateWriteAccessFIFOWT(mode, address,Indexbit,arr2,FIFO_Index,valid,dirty);
            }
        }
        else if(strcmp(REPLACEMENT_POLICY,"LRU")==0 && strcmp(WRITEBACK_POLICY,"WT")==0){
            if(mode=='R'){
            simulateReadAccessAssociativeLRU(mode, address,Indexbit,arr2,LRU_Index,valid,dirty);
            }
            else{
            simulateWriteAccessLRUWT(mode, address,Indexbit,arr2,LRU_Index,valid,dirty);
            }
        }
        else if(strcmp(REPLACEMENT_POLICY,"RANDOM")==0 && strcmp(WRITEBACK_POLICY,"WT")==0){
            if(mode=='R'){
            simulateReadAccessAssociative(mode, address,Indexbit,arr2,valid,dirty);
            }
            else{
            simulateWriteAccessWT(mode, address,Indexbit,arr2,valid,dirty);  
            }
        }
        
}
    return 0;
}
