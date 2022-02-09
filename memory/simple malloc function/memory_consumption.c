#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MB 1024*1024

// every block would be distributed 128 MB memo
typedef struct Block {
    char *unit;
    struct Block *next;
} Block;



// main process of consuming memo
int steal_memo (int size, int time)
{
    int seg = 128 * MB;
    int total = 0;

    Block *header = (Block*) malloc (sizeof(Block));
    memset(header, 0, sizeof(Block));
    Block *current = NULL;
    
    do
    {
        if (size >= 128)
        {
            size -= 128;
            seg = 128*MB;
            total += 128;
        }
        else
        {
            seg = size*MB;
            total += size;
            size = 0;
        }

        Block *block = (Block*) malloc (sizeof(Block));
        block->unit = (char*) malloc (seg);
        if(block->unit == NULL)       
        {            
            printf("Insufficient memory\n");            
            return 0;
        }
        memset(block->unit, 'x', seg);
        if (header->next == NULL)
            header->next = block;
        else
            current->next = block;
        current = block;
        printf("%d MB has been distributed...\n", total);
    } while (0 < size);
    
    printf("\n...\n");

    if (time != 0)
    {
        printf("done! The program will wait %d seconds before freeing\n", time);
        printf("You can also use ctrl+c to kill process...\n");
        sleep(time);
        getchar();
    }
    else
    {
        printf("done! Input any key to free memory...");
        getchar();
        getchar();

    }

    
    
    Block *previous = NULL;
    current = header->next;
    while (current != NULL){
        previous = current;
        current = current->next;
        free(previous->unit);
        free(previous);
    }
    free(header);
    header = NULL;

    printf("done! memory freed\n");
    printf("Input any key to kill the process...");
    getchar();

    return 1;
}

int main(int argc, char const *argv[])
{
    char* size = (char*) malloc (sizeof(char));
    char* time = (char*) malloc (sizeof(char));
    printf("How many Megabytes would you like to consume: ");
    scanf("%s", size);
    printf("How many seconds would you like to run\n(input 0 if you want infinite time): ");
    scanf("%s", time);
    printf("\n");
    if (steal_memo(atoi(size), atoi(time)) == 1 && atoi(size) > 0 && atoi(time) >= 0)
        printf("\nSuccessfully simulated memory consumption process!\n");
    else
        printf("\nMemory allocation fail, please restart!\n");
    free(size);
    free(time);
    size = time = NULL;


    return 0;
}
