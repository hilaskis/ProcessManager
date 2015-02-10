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

#define MAXSERV 3
#define MAXINPUT 512
#define MAXARGS 15

#define MAXPROC 10
#define MINPROC 2

typedef struct server_info {
    pid_t  pid;
    char name[MAXINPUT];
    int minp;
    int maxp;
    int active;
} svrinfo_t;

void svrinfo_fact(svrinfo_t* svr) {
    svr->pid = 0;
    svr->name[0] = '\0';
    svr->minp = MINPROC;
    svr->maxp = MAXPROC;
    svr->active = 0;
}

#endif
