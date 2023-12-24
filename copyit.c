#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//Full permissions to read/write or execute for all files
#define PERMS 0777 

extern int errno;

void display_message (int signum){
    //Prints a message after a 1 second alarm so the user knows the program is still running
    printf("copyit: still copying...\n");
    //Sets the alarm again
    alarm(1);
}

//Argc is number of arguments, argv[] is an array of the different arguments as characters.
int main(int argc, char *argv[]){

    //Variables for our source file, target file, read result, and write result respectively.
    int src, target, r, w;
    //Stores the number of bytes displayed at the end. Used a long long since an int breaks with larger files
    unsigned long long totalBytes;
    //Defining a character array of the size of the buffer to store data from the read so we can write it afterwards.
    char buf [BUFSIZ];

    //Setting the first alarm which will refer to display_message
    signal(SIGALRM, display_message);
    alarm(1);

    //Arg count needs to be 3, copyit, src, and target, otherwise we need to quit early ]
    //and tell the user what went wrong.
    if(argc  != 3){
        if(argc > 3){
            //More than 3 is too many
            printf("copyit: too many arguments!\n");
        }
        if(argc < 3){
            //Less than 3 is too few
            printf("copyit: too few arguments!\n");
        }
        //Give the user an example of proper usage, then exit with a failure
        printf("usage: copyit <sourcefile> <targetfile>\n");
        exit(1);
    }

    //Opens the source file with read only permissions and checks for an error
    //src will be used to reference that file in the reading process
    src = open(argv[1], O_RDONLY, 0);
    if(errno != 0){
        //If there is an error (errno != 0) print an error message and exit with a failure
        printf("copyit: Unable to open %s: %s\n", argv[1], strerror(errno));
        exit(1);
    }

    //Creates the target file with the permissions at the top of the file and checks for an error
    //target will be used to reference that file in the writing process
    target = creat(argv[2], PERMS);
    if(errno != 0){
        //If there is an error (errno != 0) print an error message and exit with a failure
        printf("copyit: Unable to create %s: %s\n", argv[2], strerror(errno));
        exit(1);
    }

    //Initializes a value for totalBytes, which is used to display the total number of bytes copied later.
    totalBytes = 0;

    //read returns the number of bytes read, 0 if it reaches end of file or -1 if it encounters an error
    //Whats read is stored in buf, with a max size of BUFSIZE (determined by the third argument)
    while ((r = read(src, buf, BUFSIZ)) != 0){
        //If we get an error, errno wont be 0
        while(errno != 0){
            //If its an interrupt error, we keep retrying until we finish reading properly.
            if(errno == EINTR){
                printf("Interrupted read\n");
                while (errno == EINTR && r == -1){
                    r = read(src, buf, BUFSIZ);
                }
                //Resetting the error number and then breaking from the "while errno != 0" loop
                errno = 0;
                break;
            } else{ 
                //If its any other type of error, we display an error message and exit with a failure.
                printf("copyit: Unable to read %s: %s\n", argv[1], strerror(errno));
                exit(1);
            }
        }

        //Writes to the target file from buf, with the number of bytes being the same as the read variable
        //This is because we are writing from what we read to copy it over to the new file.
        w = write(target, buf, r);
        //If we get an error, errno wont be 0
        while(errno != 0){
            //If its an interrupt error, we keep retrying until we finish writing properly.
            if(errno == EINTR) {
                printf("Interrupted write\n");
                while(errno == EINTR && w == -1){
                    w = write(target, buf, r);
                }
                //Resetting the error number and then breaking from the "while errno != 0" loop
                errno = 0;
                break;
            }else {
                //If its any other type of error, we display an error message and exit with a failure.
                printf("copyit: Unable to write to %s: %s\n", argv[2], strerror(errno));
                exit(1);
            }
        }
        
        //the variable w holds the number of bytes written, so its added to the total bytes counter 
        totalBytes += w;
    }

    close(src);
    close(target);
    //Tells the user how many bytes were copied and the source and target files.
    printf("copyit: Copied %lli bytes from file %s to %s. \n", totalBytes, argv[1], argv[2]);
    exit(0);
}
