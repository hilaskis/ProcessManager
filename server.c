#include "procMan.h"

void init_svrs(svrinfo_t *server);
void spwn_proc(void);
static void signal_handler(int, siginfo_t*, void*);

int main(int argc, char *argv[])
{
    //Declare and initialize sigaction struct for signal registering
    struct sigaction act;
    act.sa_sigaction = &signal_handler;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGINT, &act, NULL);  //Register SIGINT signal handler
    sigaction(SIGTERM, &act, NULL); //Register SIGTERM signal handler

    //Populate srvinfo_t server info data structure//
    svrinfo_t server;
    svrinfo_fact(&server);
    int temp;
    server.pid = getpid();     //Store process pid
    strcpy(server.name, argv[3]);
    //server.name = argv[3];     //Store server name
    temp = atoi(argv[1]);       //Temp variable stores min processes
    //Check if argument in range//
    server.minp = (temp >= MINPROC && temp <= MAXPROC) ? temp : MINPROC;
    temp = atoi(argv[2]);   //Temp variable stores max processes
    server.maxp = (temp >= MINPROC && temp <= MAXPROC) ? temp : MAXPROC;
    //Fix case where maximum is less than minium//
    if(server.maxp < server.minp) {
        server.maxp = server.minp;
    }

    init_svrs(&server);

    exit(0);
}

void init_svrs(svrinfo_t *server)
{
    int i;
    for(i = 0; i < server->minp; i++) {
        spwn_proc();
        server->active++;
    }
}

void spwn_proc(void)
{

}

static void signal_handler(int signum, siginfo_t *siginfo, void *context)
{
    if(signum == SIGINT) {
        printf("Server is terminating...\n");
        exit(0);
    } else if (signum == SIGTERM) {
        //while(waitpid((pid_t)-1, 0, WNOHANG) > 0);
        exit(0);
    }
}
