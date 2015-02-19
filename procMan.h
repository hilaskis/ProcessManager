// Seth Hilaski
// CIS 452 - Operating Systems
// Program 2 - Server Process Management System

#ifndef PROC_MAN_H
#define PROC_MAN_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAXSERV 3       //Maximum number of servers allowed
#define MAXINPUT 512    //Maximum user input size supported
#define MAXARGS 15      //Maximum user command arguments supported

#define MAXPROC 10      //Maximum number of processes a server can have
#define MINPROC 2       //Minimum number of process a servers starts with

typedef struct server_info {
    pid_t  pid;             //Server process id
    char name[MAXINPUT];    //Server name
    int minp;               //Minimum processes
    int maxp;               //Maximum processes
    int active;             //Number of active processes
    pid_t procs[MAXPROC];   //List of active process PIDs
} svrinfo_t;

/* Factory funtion initializes server_info data structure.
 *     svr: pointer to server_info data structure to initialize
 */
void svrinfo_fact(svrinfo_t* svr) {
    svr->pid = 0;
    svr->name[0] = '\0';
    svr->minp = MINPROC;
    svr->maxp = MAXPROC;
    svr->active = 0;
    int i;
    for(i = 0; i < MAXPROC; i++) {
        svr->procs[i] = -1;
    }
}

#endif
