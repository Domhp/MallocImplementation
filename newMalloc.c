#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define ALIGNMENT 8              //Alignment used for blocks that arent specific size; eg greater than 512.
#define HEADER sizeof(freeBlock) //get rid of
#define HEADER_OPT (sizeof(freeBlock) - ALIGNMENT) // Used for the size of the memory once optimized
#define MICRO 32
#define SMALL 64
#define MEDIUM 128
#define LARGE 256
#define MASSIVE 512

//Linked list for each free block on any free list.
typedef struct freeBlock{
    size_t size;
    struct freeBlock *next;     //will be removed once allocated to optimise space
}freeBlock;


void getSpace();
void new_free(void* ptr);
void addToFreeList(freeBlock* ptrLocationOfMem);
void* new_malloc(size_t size);
freeBlock* getFreeBlock(freeBlock** head, size_t size);
freeBlock* findBlock(size_t size);
freeBlock* makeNewFreeBlock(size_t size);

//first items in each corrisponding free list
static freeBlock *micro_FList;
static freeBlock *small_FList;
static freeBlock *medium_FList;
static freeBlock *large_FList;
static freeBlock *massive_FList;
static freeBlock *all_free_mem = NULL; //store new sbrk() or free of mem larger than MASSIVE.


//Make free block or if not enough size make space and call self.
freeBlock* makeNewFreeBlock(size_t size){
    freeBlock ** block = &all_free_mem;
    freeBlock * block2 = NULL;
    while(*block){
        if(((*block) -> size) >= (size+HEADER_OPT)){
            freeBlock *b = *block;
            size_t sizeAllBlock= (*block) -> size; //store temp
            freeBlock* nextAllBlock = (*block) -> next; //store temp
            *block += (size + HEADER_OPT)/ HEADER;

            (*block) -> size = sizeAllBlock - (size + HEADER_OPT);
            (*block) -> next =  nextAllBlock;
            b -> next = NULL;
            b -> size = size;
            return b;
        }
        if(block2){
            block2-> next = *block;
        }
        block2 = *block;
        *block = (*block) ->next;
    }
    printf("Get space mate\n");
    getSpace();
    return makeNewFreeBlock(size);
}
//Checks for coalesce and coalesces and places in next largest free list
int checkForCoalesce(freeBlock* newItem, freeBlock* head){
    freeBlock* temp = NULL;
    if(head == NULL){return 0;}
    if(newItem == (void*) head - (newItem -> size + HEADER_OPT)){
        findBlock(newItem -> size);
        newItem ->size = ((newItem -> size + HEADER_OPT) *2) - HEADER_OPT;  
        addToFreeList(newItem);
        return 1;
    }
    if(newItem == (void*) head + (newItem -> size + HEADER_OPT)){
        temp = findBlock(newItem -> size);
        temp ->size = ((temp -> size + HEADER_OPT) *2) - HEADER_OPT;
        addToFreeList(temp);
        return 1;
    }
    return 0;
}
//Adds a block to a free list dependent on size
void addToFreeList(freeBlock* ptrLocationOfMem){
    if(ptrLocationOfMem -> size + HEADER_OPT > MASSIVE){ // add too all_free_mem
        ptrLocationOfMem -> next = all_free_mem;
        all_free_mem = ptrLocationOfMem;
    }
    switch (ptrLocationOfMem -> size + HEADER_OPT)
        {
        case MICRO:
            if(!checkForCoalesce(ptrLocationOfMem, micro_FList)){
                ptrLocationOfMem -> next = micro_FList;
                micro_FList = ptrLocationOfMem;
            }break;
        case SMALL:
            if(!checkForCoalesce(ptrLocationOfMem, small_FList)){
                ptrLocationOfMem -> next = small_FList;
                small_FList = ptrLocationOfMem;
            }break;
        case MEDIUM:
            if(!checkForCoalesce(ptrLocationOfMem, medium_FList)){
                ptrLocationOfMem -> next = medium_FList;
                medium_FList = ptrLocationOfMem;
            }break;
        case LARGE:
            if(!checkForCoalesce(ptrLocationOfMem, large_FList)){
                ptrLocationOfMem -> next = large_FList;
                large_FList = ptrLocationOfMem;
            }break;
        case MASSIVE:
            if(!checkForCoalesce(ptrLocationOfMem, massive_FList)){
                ptrLocationOfMem -> next = massive_FList;
                massive_FList = ptrLocationOfMem;
            }break;                          
        default:
            break;
        }
}
//Used to display usefull information about all the free lists and memory aviable from sbrk.
void displayFreeBlocks(){
    size_t sum = 0;
    size_t sumToAllocate = 0;
    //calc all size for each free list
    freeBlock *tempMicro = micro_FList;
    freeBlock *tempSmall = small_FList;
    freeBlock *tempMedium = medium_FList;
    freeBlock *tempLarge = large_FList;
    freeBlock *tempMassive = massive_FList;
    //Will Increase size sum for each freeblock depending on size
    if(tempMicro){
        printf("Micro Free List Blocks are at:");
        while(tempMicro){printf("| %p ",tempMicro); 
            sum+= tempMicro->size;
            tempMicro = tempMicro -> next;
        } 
        printf("\n");
    }
    if(tempSmall){
        printf("Small Free List Blocks are at:");
        while(tempSmall){printf("| %p ",tempSmall);
            sum+= tempSmall->size;
            tempSmall = tempSmall -> next;

        } 
        printf("\n");
    }
    if(tempMedium){
        printf("Medium Free List Blocks are at:");
        while(tempMedium){printf("| %p ",tempMedium);
            sum+= tempMedium->size;
            tempMedium = tempMedium ->next;
        } 
        printf("\n");
    }
    if(tempLarge){
        printf("Large Free List Blocks are at:");
        while(tempLarge){printf("| %p ",tempLarge);
            sum+= tempLarge->size;
            tempLarge = tempLarge ->next;
        }
        printf("\n");
    }
    if(tempMassive){
        printf("Massive Free List Blocks are at:");
        while(tempMassive){printf("| %p ",tempMassive);
            sum+= tempMassive->size;
            tempMassive = tempMassive -> next;
        }
        printf("\n");
    }
    printf("Total memory in FreeBlocks:%lu\n", sum);
    freeBlock* tempIterator = all_free_mem;
    while(tempIterator){
        sumToAllocate += tempIterator -> size;
        tempIterator  = tempIterator -> next;
    }  
    printf("The total memory still avaible from sbrk:%lu\n", sumToAllocate);
}
//Get space in 8kb blocks
void getSpace(){
    if(!all_free_mem){
        all_free_mem = sbrk(0); //check this adds correctly
        printf("THE start of sbrk is %p\n", all_free_mem);
        sbrk(8192);
        all_free_mem -> size = 8192 - HEADER;
        all_free_mem -> next = NULL;

    }
    else{   //sbrk not called inbetween succesive sbrk in new_malloc so added to all_free_list
        if(sbrk(0) == (void*) all_free_mem + 8192){
            all_free_mem -> size += 8192;
        }
        else{
            freeBlock *nextBlock  = sbrk(0); 
            sbrk(8192);
            nextBlock -> next = all_free_mem;
            all_free_mem = nextBlock;
            nextBlock -> size = 8192 - HEADER;
            nextBlock -> next = NULL;
        }
    }
}
//gets the first free block from head of a free list
freeBlock* getFreeBlock(freeBlock** head, size_t size){
    freeBlock *temp = *head;
    if(temp){
        if(temp -> next){
            *head = temp -> next;
        }
        else{
            *head = NULL;
            }
        return temp;
    }
    else{
        temp = makeNewFreeBlock(size);
    }
    return temp;
}
//find a block in the freeLists or make a new one. calls getFreeBlock()
freeBlock* findBlock(size_t size){

    freeBlock *temp ;
    size_t trueSize = size + HEADER_OPT;
    if(trueSize > MASSIVE){
        return makeNewFreeBlock((size + ALIGNMENT -1) & ~(ALIGNMENT-1)); //makes a larger block. to 8 bytes
    }
    if(size >= 0){
        if(trueSize <= MICRO){
            temp = getFreeBlock(&micro_FList, MICRO - HEADER_OPT);
            return temp;
        }
        if(trueSize <= SMALL){
            temp = getFreeBlock(&small_FList, SMALL - HEADER_OPT);
            return temp;
        }
        if(trueSize <= MEDIUM){
            temp = getFreeBlock(&medium_FList, MEDIUM - HEADER_OPT);
            return temp;
        }
        if(trueSize <= LARGE){
            temp = getFreeBlock(&large_FList, LARGE - HEADER_OPT);
            return temp;
        }
        if(trueSize <= MASSIVE){
            temp = getFreeBlock(&massive_FList, MASSIVE - HEADER_OPT);
            return temp;
        }
    }
    return NULL;
}
void* new_malloc(size_t size)
{
    freeBlock* block;
    if(size <= 0 || size >= 8192 - HEADER){return NULL;}
    if(!all_free_mem){  //first run for efficiency so it doesn't have to check each list before allocation.
        getSpace();
    }
    block = findBlock(size);
    if(!block){
        printf("Block was Null error\n");
        return NULL;
    }

    return ((void*) block) + ALIGNMENT;     //have to cast than add 8 bytes, "+1" would increase by 16(size of freeBlock)
}
void new_free(void* ptr){
    freeBlock *checker = ptr - HEADER_OPT;
    if(checker -> size > 0){//check valid pointer
        freeBlock *ptrToBlock = ptr - HEADER_OPT;
        addToFreeList(ptrToBlock);
    }
}