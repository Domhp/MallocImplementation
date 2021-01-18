#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "newMalloc.h"

int main(int argc, char const *argv[])
{
    char user_input[32];
    displayFreeBlocks();
    
    while(user_input[0] != 'c'){
        printf("----------------\n");

        printf("Please enter memory you want allocating in form of\nA<bytes> to free use F<addr>:\n");
        scanf("%[^\n]%*c", user_input);
        switch (user_input[0])
        {
            case 'F': ;
                void *free_addr = (void*) strtol(user_input + sizeof(char), NULL, 16);
                new_free(free_addr);
                displayFreeBlocks();
                break;
            case 'A': ;
                int byte_to_malloc = atoi(user_input + sizeof(char));
                void *ptr = new_malloc(byte_to_malloc);
                printf("Address of the avaible Memory is %p\n", ptr);
                displayFreeBlocks();
                break;
            default:
                printf("Failed to get correct input\n");
        }   
    }
    return 0;
}
