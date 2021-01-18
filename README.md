# Malloc Implementation
Malloc implemented in C using sbrk in size of 8kbyte blocks, the programm can be used to show how a basic implementation of malloc can store and handle different sizes of requested memory.
## Further Details
Memory "Free blocks" are stored in linked list when free() is called on memory address. Adjacent memory blocks are coalesced to formed one bof twice the size.
Memory is returned in set block sizes to avoid External fragmentation, they are decided using best-fit (*Smallest available size*). 

**Memory sizes**
- 32 Bytes
- 64 Bytes
- 128 Bytes
- 256 Bytes
- 512 Bytes
-Larger are given to exact size, up to 8Kbytes
## Running
The program is optimised for page sizes of 4KBytes and can be compiled in command line:
```
gcc -Wall mallocTester.c newMalloc.c -o mallocTester
```
It can also be run on Linux system or WSL Ubuntu 20.04 using the *.exe* in target file.
