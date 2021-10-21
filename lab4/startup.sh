#!/bin/sh
 
# this function is called when Ctrl-C is sent
function trap_ctrlc ()
{
    # perform cleanup here
    
    #echo "Ctrl-C caught...performing clean up"
    umount root/dev
    #echo "Doing cleanup"
 
    # exit shell script with error code 2
    # if omitted, shell script will continue execution
    exit 2
}
 
# initialise trap to call trap_ctrlc function
# when signal 2 (SIGINT) is received
trap "trap_ctrlc" 2

mount --bind /dev/ root/dev
./server
umount root/dev
