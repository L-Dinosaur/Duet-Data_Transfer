# duet-telemetry
## What it does
Duet telemetry is a program that collects block level events from cache 
(block entry, block modification, block removal, etc.) and transfer them 
via network to a server, where data is stored into raw files and databases.

The data collected are (as of now):
* Getpath code
* Event code
* Full path of the file
* Offset of the block within the file
* Size of the file
* ...

## What are each scripts
The duet telemetry program consists of a list of files:
1. main.c
   The script that contains modified dummy.c code. The main program

2. duet_collect.c
   The script that contains all the helper functions that does a range of 
   different things from getting the time stamp to opening up a thread and 
   send data through a socket.

3. duet_collect.h
   The header file.

4. run.sh
   The bash script that executes the program with pre-specified parameters
   (exact parameters that dummy task used)

