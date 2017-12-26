PROJECT 4 - Implementing Virtual Memory


Coded by: Ethan Schutzman && Steven McAteer



GOAL OF THIS ASSIGNMENT: 
To implement a representation of memory and a page table to keep 
track of certain pages. These pages can be swapped in and out to disk to make room. We started by 
making a struct for a page table entry. We then made an array of these entries called pageTable[4]. 
We also had another array called memory that is 64 bytes long. The pageTable is then put into memory[0], 
and takes up 16 bytes. There are functions for map, store, load and swap, as well as some helper functions.
The swap function is called when there isn't an open page. Swap uses a round-robin technique to 
cycle through the pages and puts the next page is gets to onto disk memory (a text file).




TO RUN:


1. Run "make clean", and then "make".

2. Run "./mem" to run the program.

3. A menu will print asking for an instruction.

4. Type a command, (example: 1,map,1,1), hit enter. The program will print out the result of what it did.

5. To stop the program, press ^C.

 

This program also supports piping files through the command prompt.  

USAGE:

./mem < test.txt
