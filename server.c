// Seth Hilaski
// CIS 452 - Operating Systems
// Program 2 - Server Process Management System
// A basic server that is capable of spawning copies of itself(processes)

#include "procMan.h"

void init_svrs(svrinfo_t *svr);
pid_t spwn_proc(svrinfo_t *svr);
void add_proc(pid_t pid, svrinfo_t *svr);
void del_proc(svrinfo_t *svr);
void del_pid(pid_t pid, svrinfo_t *svr);
void process(void);
static void svr_sigs(int, siginfo_t*, void*);
void quit(svrinfo_t *svr);

//Global Flags//
int d_serv = -1;    //Delete server flag
int d_proc = -1;    //Delete process flag
int ab_exit = -1;   //Abnormal exit flag
int cr_proc = -1;   //Create process flag

int main(int argc, char *argv[])
{
    svrinfo_t svr;
    svrinfo_fact(&svr);     //Initialize server info
    //Declare and initialize sigaction struct for signal registering//
    struct sigaction act;
    act.sa_sigaction = &svr_sigs;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, NULL);  //Register SIGINT signal handler
    sigaction(SIGTERM, &act, NULL); //Register SIGTERM signal handler
    sigaction(SIGUSR1, &act, NULL); //Register SIGUSR1 signal handler
    sigaction(SIGUSR2, &act, NULL); //Register SIGUSR2 signal handler

    //Populate server info structure//
    svr.pid = getpid();
    strcpy(svr.name, argv[3]);

    int temp = atoi(argv[1]);       //Check if min procs in range
    svr.minp = (temp >= MINPROC && temp <= MAXPROC) ? temp : MINPROC;

    temp = atoi(argv[2]);           //Check if max procs in range
    svr.maxp = (temp >= MINPROC && temp <= MAXPROC) ? temp : MAXPROC;

    if(svr.maxp < svr.minp) {       //Fix case where min less than max
        svr.maxp = svr.minp;
    }

    init_svrs(&svr);            //Create initial server processes
    while(1) {
        if(d_serv > 0) {        //Check for delete server flag
            quit(&svr);
            exit(0);
        }
        if(d_proc > 0) {        //Check for delete process flag
        //    printf("This was reached 3\n");
            del_proc(&svr);
            d_proc = -1;        //Reset flag
        }
        if( ab_exit > 0) {      //Check for abnormal exit (kill used on process)
            printf("Process on server \"%s\" abnormal exit.\n", svr.name);
            pid_t pid = waitpid((pid_t)0, 0, 0);    //Wait for child to exit
            svr.active--;
            del_pid(pid, &svr);         //Delete pid from process list
            cr_proc = 1;                //Indicate new process needs creation
            ab_exit = -1;               //Reset flag
        }
        //Check for create process flag and active procs less than minp
        if( (cr_proc > 0) || (svr.active < svr.minp) ) {
            pid_t n_proc;
            if( (n_proc = spwn_proc(&svr)) > 0) {   //Create new process
                add_proc(n_proc, &svr);
            }
            cr_proc = -1;       //Reset flag
        }

    }
}

/* Initialize server by spawning processes equal to the value of minp.
 *     svr: data structure containing information about the server
 */
void init_svrs(svrinfo_t *svr)
{
    int i;
    pid_t n_proc = -1;
    for(i = 0; i < svr->minp; i++) {        //Spawn minp processes
        if( (n_proc = spwn_proc(svr)) > 0) {
            add_proc(n_proc, svr);          //Add to list
        }
    }
}

/* Spawn a process for the server.
 *     svr: data structure containing information about the server
 */
pid_t spwn_proc(svrinfo_t *svr)
{
    if(svr->active >= svr->maxp) {  //Check if maximum processes reached
        printf("ERROR: Process creation aborted. Max processes reached.\n");
        return -1;
    }

    pid_t pid;
    if((pid = fork()) < 0) {        //Check for fork error (create process)
        perror("The fork failed.\n");
        exit(1);
    } else if(pid == 0) {           //Child process code
        process();
    }
    return pid;                     //Return child pid to parent process
}

/* Adds a process pid the the servers list of process IDs
 *     pid: process ID of the newly created process
 *     svr: data structure containing information about the server
 */
void add_proc(pid_t pid, svrinfo_t *svr)
{
    int i;
    for(i = 0; i < MAXPROC; i++) {      //Find first empty element in array
        if(svr->procs[i] <= 0) {
            svr->procs[i] = pid;        //Add new process pid
            svr->active++;
            break;
        }
    }
    printf("\tAdded process %d to server \"%s\".\n", pid, svr->name);
}

/* Deletes the first valid process ID found from the server.
 *     svr: data structure containing information about the server
 */
void del_proc(svrinfo_t *svr)
{
    if(svr->active <= 0) {          //Check for processes to delete
       printf("Server \"%s\" has no processes to abort.\n", svr->name);
       return;
    }

    int i, found = -1;
    for (i = 0; i < MAXSERV; i++) { //Find the first process in procs array
        if(svr->procs[i] > 0) {
            found = 1;              //Set flag indicating process was found
            break;
        }
    }

    if(found) {                         //If process found, terminate
        kill(svr->procs[i], SIGINT);    //Send terminate signal
        waitpid(svr->procs[i], 0, 0);   //Wait for process to terminate
        svr->active--;
        svr->procs[i] = -1;             //Clear array element
    }
}

/* Deletes a specified pid from the servers list of processes.
 *     pid: The process ID to delete from the list
 *     svr: data structure containing information about the server
 */
void del_pid(pid_t pid, svrinfo_t *svr)
{
    int i;
    for(i = 0; i < MAXSERV; i++) {
        if(svr->procs[i] == pid) {      //Find specified pid in list
            svr->procs[i] = -1;         //Clear its value
            break;
        }
    }
}

/* Code for the server's child processes to run.
 */
void process(void)
{
    while(1) {
        if(d_serv > 0) {      //Check if terminate flag is true
            printf("\tQuitting process: %d\n", (int)getpid());
            exit(0);
        } else if(ab_exit > 0) {    //Check for abnormal exit (kill signal sent)
            kill(getppid(), SIGTERM);   //Tell parent svr about abnormal exit
            exit(0);
        }
    }
}

/* Signal handler for all signals send to the server.
 *     signum: the signal caught by the handler function
 *     siginfo: information about about signal
 *     context:  typically not used.
 */
static void svr_sigs(int signum, siginfo_t *siginfo, void *context)
{
    if(signum == SIGINT) {  //Set delete server flag if indicated
        d_serv = 1;
    } else if (signum == SIGUSR1) {     //Set create process flag if indicated
        cr_proc = 1;
    } else if (signum == SIGUSR2) {     //Set delete process flag if indicated
        d_proc = 1;
    } else if (signum == SIGTERM) {     //Set to indicate abnormal exit
        ab_exit = 1;
    }
}

/* Clean up all server resources before server termination.
 */
void quit(svrinfo_t *svr) {
    int i;
    for(i = 0; i < MAXPROC; i++) {  //Loop through processes and terminate
        if(svr->procs[i] > 0) {
            kill(svr->procs[i], SIGINT);
        }
    }
    while(waitpid((pid_t)-1, 0, 0) > 0);  //Wait for procs to terminate
    printf("\tShutting down server \"%s\"\n", svr->name);
}
