/*
	Ethan Schutzman && Steven McAteer
	ehschutzman     ::       smcateer
*/
#include <unistd.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define NUM_PAGES 5
/*
  A char has 16 bits, 
  pid - [0,3]  
  instruction type - [store/load/map]
  virtual_addr - [0,63]
  value-[0,63]
  Need 4 page table entries
*/

//present bit - if in mem or not (1 in mem, 0 on disk)
//valid bit
//physical frame number
//may or may not need virtual page number -> can use array index to tell what page you are on
//last 4 bits are byte offset for that page
//first two bits tell what page table entry it is
//one page for the page table
//lseek to write bytes in kernelspace


typedef struct {
    u_int8_t presentBit:1; //Is the info is 1=> in memory, 0=> on disk
    u_int8_t validBit:1; //If it exists or is valid 
    u_int8_t value:1; //0 if read only =>1 if can be written
    u_int8_t page:3; //Page number [0,3]
    u_int8_t processId:4; //Process ID [0,3] also known as VPN
    u_int8_t vpn:4; //Location in memory
    u_int8_t instruction:2;

} entryInfo;

//Declare the memory array with 64 bytes
char memory[64];
char tmp1[16];
char tmp2[16];
int isWrite = 0;
int pageToEvict = 0;

//Create the pageTable
entryInfo pageTable[NUM_PAGES];

//function prototypes
int checkPageAvail(char *mem);

int isWritable(entryInfo entry);

void map(int procID);

int store(int procID, char *mem, int value);

int load(int procID, char *mem, int virtual_addr);

int swap(int procIDToEvict, int vpnWanted, char *mem);

void printPageTable();

int main(int argc, char **argv) {
    int bytes_read;
    int nbytes = 100;
    char *input = NULL;
    input = (char *) malloc(nbytes + 1);
    char *p;




    //Set all default values
    for (int i = 0; i < NUM_PAGES; i++) {
        pageTable[i].validBit = 0;
        pageTable[i].presentBit = 0;
        pageTable[i].value = 0;
        pageTable[i].page = -1;
        pageTable[i].vpn = i;
        pageTable[i].processId = i;

    }
    //make 
    pageTable[0].validBit = 1;
    pageTable[0].page = 0;
    pageTable[0].value = 0;
    pageTable[0].processId = 0;
    pageTable[0].presentBit = 1;
    //put the page table in first slot
    memcpy(memory, pageTable, 16);


    int process_id;
    char *instruction;
    int instructionInt;
    int virtual_address;
    int value;
    int count = 0;

    while (!feof(stdin)) {
        printf("Instruction: ");
        bytes_read = getline(&input, &nbytes, stdin);
        //  printf("line is %s\n",input);
        p = strtok(input, ",");

        count = 0;
        while (p != NULL) {

            if (count == 0) {
                process_id = atoi(p);
            }
            if (count == 1) {
                instruction = p;
            }
            if (count == 2) {
                virtual_address = atoi(p);
            }
            if (count == 3) {
                value = atoi(p);
            }
            count++;
            p = strtok(NULL, ",");

        }
        count = 0;
        if (instruction[0] == 'm') {
            instructionInt = 0;
            //printf("Got into map call\n");
            //call map function
            //printf("virtual address is %d\n", virtual_address);

            //printf("process_id is %d\n", process_id);
            map(process_id);
        } else if (instruction[0] == 's') {
            instructionInt = 1;

            store(process_id, memory, virtual_address);
        } else if (instruction[0] == 'l') {
            instructionInt = 2;
            //call load function
            load(process_id, memory, virtual_address);
        } else if (instruction[0] ==
                   'p') {//If 'print' is passed as the second argument of the 4-tuple, print the page table
            printPageTable();

        }

    }

}

int checkPageAvail(char *mem) {
    int full = 0;
    for (int i = 0; i < 4; i++) {
        if (mem[i * 16] == 0) {
            return i;
        }

    }
    return -1;
}

int isWritable(entryInfo entry) {
    if (entry.value = 1) {
        isWrite = 1;
    } else if (entry.value == 0) {
        isWrite = 0;
    }
    return isWrite;
}

void map(int procID) {
    int whereIsFree;
    if (pageTable[procID].presentBit == 1) {
        //already allocated
        printf("Memory already allocated for process %d on page %d\n", procID, pageTable[procID].page);
        return;
    } else {

        //allocate memory
        whereIsFree = checkPageAvail(memory);
        if (whereIsFree != -1) {
            pageTable[procID].validBit = 1;
            pageTable[procID].presentBit = 1;
            pageTable[procID].page = whereIsFree;
            memory[whereIsFree * 16] = (char) pageTable[procID].vpn;

        } else {
            printf("No pages are free.\n");
            pageToEvict = ((pageToEvict + 1) % NUM_PAGES) + 1;
            swap(pageToEvict, procID, memory);
            pageTable[procID].validBit = 1;
            pageTable[procID].presentBit = 1;
            pageTable[procID].page = pageToEvict;
            memory[pageToEvict * 16] = pageTable[procID].vpn;
            whereIsFree = pageToEvict;


        }
    }
    printf("Mapped process %d (virtual page %d) into physical frame %d\n",
           pageTable[procID].processId, pageTable[procID].vpn, whereIsFree);

}

int store(int procID, char *mem, int value) {
    //If its present, no problem
    //If it isnt, swap it in
    if (pageTable[procID].validBit != 1) {
        printf("Illegal instruction, process has not been mapped into memory yet\n");
        return 1;
    }
    if (pageTable[procID].presentBit == 1) {
        int memLoc = pageTable[procID].page * 16 + 1 + value;
        mem[memLoc] = (char) value;
        printf("Stored value %d at virtual address %d (physical address %d)\n",
               value, value, memLoc); //again, might be out of order 
    } else {
        pageToEvict = ((pageToEvict + 1) % NUM_PAGES) + 1;
        swap(pageToEvict, procID, memory);
        store(procID, mem, value);
    }

    return 0;
}

int load(int procID, char *mem, int virtual_addr) {
    //If its present, no problem
    //If it isnt, swap it in
    if (pageTable[procID].validBit != 1) {
        printf("Illegal instruction, process has not been mapped into memory yet\n");
        return 1;
    }
    if (pageTable[procID].presentBit == 1) {
        int memLoc = pageTable[procID].page * 16 + 1 + virtual_addr;
        int value = -1;
        if (pageTable[procID].presentBit == 1) {
            value = (int) mem[memLoc];
            printf("Value is %d stored at virual address %d, (Physical address %d)\n", value, virtual_addr,
                   pageTable[procID].page * 16 + 1 + virtual_addr);
            return value;
        }
    } else {
        pageToEvict = ((pageToEvict + 1) % NUM_PAGES) + 1;
        swap(pageToEvict, procID, memory);
        load(procID, mem, virtual_addr);
    }

}

void printPageTable() {
    for (int i = 0; i < NUM_PAGES; i++) {
        printf("num pages is %d\n", NUM_PAGES);
        printf("I is %d\n", i);
        printf("Data for process %d\n", pageTable[i].processId);
        printf("The process valid status is: %d\n", pageTable[i].validBit);
        printf("The data is currently in memory: %d \n", pageTable[i].presentBit);
        printf("The data is currently on page %d\n", pageTable[i].page);
        printf("The data is currently writable: %d\n", pageTable[i].value);

    }

}

int swap(int procIDToEvict, int vpnWanted, char *mem) {
    FILE *swapFile;
    swapFile = fopen("swap.swippityswappity", "a+");
    // printf("File opened\n");
    int pageStartLoc = pageTable[procIDToEvict].page * 16;
    memcpy(tmp1, mem + pageStartLoc, 16 * sizeof(char));
    //  printf("mem copied out\n");
    char *line = NULL;
    size_t len = 16;
    ssize_t read;
    pageTable[procIDToEvict].presentBit = 0;
    if (swap == NULL) {
        exit(EXIT_FAILURE);
    }
    //if pte for vpnwanted is valid, s earch file
    if (pageTable[vpnWanted].validBit == 1) {
        while ((read = fread(tmp2, 1, sizeof(tmp2), swapFile)) == 16) {
            if ((int) tmp2[0] == vpnWanted) {
                memcpy(mem + pageStartLoc, tmp2, 16);
                pageTable[vpnWanted].presentBit = 1;

            }
        }

    } else {
        fwrite(tmp1, 1, sizeof(char), swapFile); //Write tmp to file
        pageTable[procIDToEvict].presentBit = 0;
    }


}
