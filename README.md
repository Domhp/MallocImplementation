# Malloc Implementation
Malloc implemented in C using sbrk in size of 8kbyte blocks, the programm can be used to show how a basic implementation of malloc can store and handle different sizes of requested memory.
## Furtherdetails
Memory "Free blocks" are stored in linked list when free() is called on memory address. Adjacent memory blocks are coalesced to formed one bof twice the size.
Memory is returned in set block sizes to avoid External fragmentation, they are decided using best-fit (*Smallest available size*).
## Running
