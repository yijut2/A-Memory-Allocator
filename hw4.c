/* ICS 53
 * Project: A Memory Allocator
 * Instructor: Ian Harris
 *
 * Student 1: Hongji Wu (ID: 32938089)
 * Student 2: Yi-Ju, Tsai (ID: 31208132)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// global variable
int arg_count;
unsigned char * heap;
unsigned char * start;
unsigned char * end;
int track = 0;

unsigned char *find_free_blk(int realsize);
void addblock(unsigned char* p, int len);
void keep_track(unsigned char * newblock);


void init_mem()
{
    heap = (unsigned char *)malloc(127); // The heap is 127 bytes long
    start = heap; // start ptr
    end = heap + 127; // end ptr
    heap[0] = 127*2;
}


void alloc_mem(int intsize)
{
    unsigned int wholesize = intsize + 2; // header + payload + footer
    // if(wholesize > 127) // check if entire block exceeds 127
    // {
    //     return;
    // }
    unsigned char * newblock = find_free_blk(wholesize);
    if(newblock == NULL) // all blocks are allocated or no suitable block is found
    {
        return;
    }
    addblock(newblock, wholesize);
    keep_track(newblock);
    if (start==newblock)
    {
        track = 1;
    }
    printf("%d\n", track);
}

// First-fit allocation
unsigned char *find_free_blk(int realsize)
{
    unsigned char * p = start; 
    while (p<end)
    {
        // printf("p[0]&1 = %d\n", p[0]&1);
        // printf("p[0] = %d\n", p[0]);
        // printf("realsize*2)|1 = %d\n", (realsize*2)|1);
        if (   ((p[0]&1)==0) && (p[0] >= ((realsize*2))))
        {
            return p;
        }
        p += (p[0]&-2)/2; // go to next block
    }
    return NULL;
}

// controller of track variable (for alloc_mem print)
void keep_track(unsigned char * newblock)
{
    track = 1;
    unsigned char * p = start; 
    while (p<end && p< newblock)
    {
        track += (p[0]&-2) /2;
        p += (p[0]&-2)/2; // go to next block
    }
}


void addblock(unsigned char* p, int len)
{
    int newsize = len; // 6
    int oldsize = (p[0]&-2)/2; // 10

    // printf("newsize: %d/ oldsize: %d\n", newsize, oldsize);
    // splitting
    if (newsize < oldsize)
    {
        // printf("split\n");
        (p + ((p[0]/2)-1))[0] = (oldsize - newsize)*2; // change the rightest of heap
        p[0] = (newsize*2) | 1; // set the header
        (p + newsize -1)[0] = p[0]; // set the footer
    
        p += (p[0]&-2)/2;
        p[0] = (oldsize - newsize)*2;
    }
    else // no splitting needed
    {
        // printf("NO split\n");
        p[0] = (newsize*2) | 1; // store size and mark allocation bit 1
        (p + newsize -1)[0] = p[0];
    } 
}


// unsigned char * find_block(int intindex)
// {
//     unsigned char * temp = start;
//     while(temp < end)
//     {
//         if(temp[0] == intindex)
//         {
//             return temp;
//         }
//         temp += (temp[0]&-2)/2;
//     }
//     return NULL; // assume the input is correct, won't return NULL
// }


void free_mem(int intindex)
{
    unsigned char * target = start + intindex - 1; // -1 to find the header of target block
    target[0] = target[0] & -2; // clear allocation bit (24--> realsize = 12) // 
    unsigned char * next = target + ((target[0]&-2)/2); // next_block = target + 12  

    unsigned char * set_zero = start + intindex;
    unsigned char zero_count = ((target[0] & -2)/2)-2; // number of space needed to be reser to 0 // 5
    int i;
    for (i=0; i<zero_count; i++)
    {
        set_zero[i] = 0;
    }


    
    // not the first block
    if (target!=start)
    {
        if ((next[0] & 1)==1) //  ~~~~~~~~~|| allocated
        {
            /* free || allocated || allocated (coalesing previous block) */
            if (((target-1)[0]&1) == 0)
            {
                unsigned char * prev = target - (( (target-1)[0] & -2 )/2); // get previous block ptr
                prev[0] = (target[0] + (    (target-1)[0] & -2)   ); // combine prev blk size and the free size & allocation bit 0
                ( target + (target[0]/2)-1 )[0] = prev[0]; // update the free blk footer to be n+m1
                target = prev; // let p point to the whole coalescing block
            }
            /* allocated || allocated || allocated (No coalesing) */
            else
            {
                target[0] = target[0] & -2; // clear header allocation flag 
                (target + (((target[0] & -2)/2)-1))[0] = target[0]; // clear footer allocation flag 
            }
        }
        else //  ~~~~~~~~~|| free
        {
            /* free || allocated || free (coalesing previous and next block) */
            if (((target-1)[0]&1) == 0)
            {
                target[0] = target[0] + next[0];
                (target + (target[0]/2)-1 )[0] = target[0] ; // set the footer of next block to be same as the entire current + next
                unsigned char * prev = target - (( (target-1)[0] & -2 )/2 );
                prev[0] = (target[0] + (    (target-1)[0] & -2)   ); // update the free blk footer to be n+m1
                ( target + (target[0]/2)-1 )[0] = prev[0];
                target = prev;
            }
            /* allocated || allocated || free (coalesing next block) */
            else
            {
                target[0] = target[0] + next[0];
                (target + (target[0]&-2)/2-1)[0] = target[0]; // update the footer to be n+m2
            }
        }
    }
    // if target == start (first block)
    else
    {
        /* allocated (start) || allocated ~~~~~~ */
        if ((next[0] & 1)==1 )
        {
            target[0] = target[0] & -2; // clear header allocation flag 
            (target + (((target[0] & -2)/2)-1))[0] = target[0]; // clear footer allocation flag
        }
        /* allocated (start) || free ~~~~~~ */
        else
        {
            target[0] = target[0] + next[0];
            (target + (target[0]&-2)/2-1)[0] = target[0];
        }
    }
}


void blocklist()
{
    unsigned char * temp = start;
    unsigned char st_of_payload = 0;
    unsigned char payload_size;
    char status[10];
    while(temp < end)
    {
        // free
        if ((temp[0] & 1) == 0)
        {
            payload_size = ((temp[0]&-2)/2)-2;
            strcpy(status, "free");
        }
        // allocated
        else
        {
            payload_size = ((temp[0]&-2)/2)-2;
            strcpy(status, "allocated");
        }
        printf("%d, %d, %s.\n", st_of_payload+1, payload_size, status);
        st_of_payload += ((temp[0]&-2)/2);
        temp += ((temp[0]&-2)/2);
    }
}

void write_mem(int index, unsigned char*str)
{
    unsigned char * target = start + index;
    int char_count = strlen(str);
    int i;
    for (i=0; i < char_count; i++)
    {
        target[i] = str[i];
    }
}

void print_mem(int idx, int num_to_print)
{
    unsigned char * target = start + idx;
    int i;
    for (i=0; i < num_to_print; i++)
    {
        printf("%02X ",target[i]);
    }
    printf("\n");
}

int eval_cmd(char* str)
{
    if (strcmp(str, "malloc")==0)
    {
        return 1;
    }
    if (strcmp(str, "free")==0)
    {
        return 2;
    }
    if (strcmp(str, "blocklist")==0)
    {
        return 3;
    }
    if (strcmp(str, "writemem")==0)
    {
        return 4;
    }
    if (strcmp(str, "printmem")==0)
    {
        return 5;
    }
    if (strcmp(str, "quit")==0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

void parse_cmd(char *cmd, char **argv)
{
    // the cmd will be parsed and store in argv list
    cmd[strlen(cmd)-1] = ' ';
    char *token = strtok(cmd, " ");
    while(token != NULL)
    {
        argv[arg_count++] = token;
        token = strtok(NULL," ");
    }
    return;
}

void run_mem_alloc()
{
    char cmdline[80];
    char *argv[80];
    arg_count = 0;
    int cmd_type = 0;
    init_mem();
    
    do
    {
        arg_count = 0;
        printf("> ");
        fgets(cmdline, 80, stdin);
        if (feof(stdin))
        {
            return;
        }
        parse_cmd(cmdline, argv);
        cmd_type = eval_cmd(argv[0]);
        switch (cmd_type)
        {
            // MALLOC
            case (1):
            {
                int intsize = atoi(argv[1]);
                alloc_mem(intsize);
                break;
            }
            // FREE
            case (2):
            {
                int intindex = atoi(argv[1]);
                free_mem(intindex);
                break;
            }
            // BLOCKLIST
            case (3):
            {
                blocklist();
                break;
            }
            // WRITE MEM
            case (4):
            {
                int index = atoi(argv[1]);
                write_mem(index, argv[2]);
                break;
            }
            // PRINT MEM
            case (5):
            {
                int idx = atoi(argv[1]);
                int num_to_print = atoi(argv[2]);
                print_mem(idx, num_to_print);
                break;
            }
        }
    }while (cmd_type != 0);
}


int main()
{
    run_mem_alloc();
    return 0;
}
