#include "procMan.h"

//Parses user input into command -> argument array form
int prsipt(char *input, char** argv, int argv_size);
//Checks for valid user commands and executes them
int input_mux(char** argv, svrinfo_t *servers);
//Spawn a new server process using passed arguments
int fork_svr(char** argv);
//Add a servers to the servers list
void add_svr(int pid, char *svr_name, svrinfo_t *servers);
//Delete a server from the servers list
void del_svr(char *svr_name, svrinfo_t *servers);
//Verify input for the create server command
int verify_cs(char **argv, svrinfo_t *servers);
//Verify server name passed is unique in the list
int verify_svrn(char *svr_name, svrinfo_t *servers);
//Create a server process
void cr_proc(char *svr_name, svrinfo_t *servers);
//Delete a server process
void del_proc(char *svr_name, svrinfo_t *servers);
//Display diagnostics
void disp_diag(svrinfo_t *servers);
//Quit the process manager
void quit(svrinfo_t *servers);

static void nop_handler(int, siginfo_t*, void*);

//svrinfo_t servers[MAXSERV];
int num_servers = 0;

int main(void)
{
    char input[MAXINPUT];       //Buffer for user input
    char *argv[MAXARGS];        //Parsed user commands
    int status = 0;
    svrinfo_t servers[MAXSERV];

    //Register handler for these signals (handler does nothing)
    //Needed because SIGUSRx signals were causing pcm termination
    struct sigaction act;
    act.sa_sigaction = &nop_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);

    // Input loop
    while(1) {
        // Get input from the user
        if( fgets(input, MAXINPUT, stdin) == NULL) continue;
        // Parse the input and return a status
        status = prsipt(input, argv, MAXARGS);

        if(status > 0) {            // Quit program if quit command entered
            quit((svrinfo_t*)&servers);
            return 0;
        } else {                    //Check for and act on valid commands
            input_mux(argv, (svrinfo_t*)&servers);
        }
    }

    return 0;
}

// Parse the user input into commands and arguments
int prsipt(char *input, char** argv, int argv_size)
{
  // Get first argument from input
  char* t_arg = strtok(input, " \n");

  // Return negative number if there was no input
  if(t_arg == NULL) return -1;
  // Return positive number (true) if quit command was issued;
  if(!strcmp(t_arg, "quit")) return 1;

  // Tokenize each parameter into char* array argv
  int i;
  for(i = 0; i < argv_size; i++)
  {
    // Fill argv with arguments until none are left or one element position left
    if(t_arg != NULL && i != argv_size - 1)
    {
      argv[i] = t_arg;
      t_arg = strtok(NULL, " \n");
    }
    else    // Null terminate the array and exit the loop
    {
      argv[i] = '\0';
    }
  }

  return 0;
}

int input_mux(char** argv, svrinfo_t *servers)
{
    if(!strcmp(argv[0], "cs")) {            //Create Server Command
        if(!verify_cs(argv, servers)) return 0;      //Verify command input

        int s_pid = fork_svr(argv);           //Create server
        if(s_pid > 0) {
            add_svr(s_pid, argv[3], servers);     //Add server to list
        }
        return 1;

    } else if ( !strcmp(argv[0], "ds") ) {  //Abort Server Command
        del_svr(argv[1], servers);
        return 1;
    } else if ( !strcmp(argv[0], "cp") ) {  //Create Process Command
        cr_proc(argv[1], servers);
        return 1;
    } else if ( !strcmp(argv[0], "dp") ) {  //Abort Process Command
        del_proc(argv[1], servers);
        return 1;
    } else if( !strcmp(argv[0], "diag") ) {   //Display Status Command
        disp_diag(servers);
        return 1;
    }

    printf("Command \"%s\" not recognized\n", argv[0]);
    return 0;
}

// Verify input for the cs command
int verify_cs(char **argv, svrinfo_t *servers) {
    if(num_servers == MAXSERV) {
        printf("ERROR: Server creation aborted. Max servers reached.\n");
        return 0;
    }
    //Verify correct number of arguments and server name
    int vf = verify_svrn(argv[3], servers);
    if( vf < 0) {               //Server name not unique
        printf("ERROR: Invalid server name.\n");
        return 0;
    } else if (vf == 0) {       //Not enough arguments for cs command
        printf("ERROR: Invalid number of arguments for \"cs\" command.\n");
        return 0;
    }

    int i;
    for(i = 1; i < 3; i++) {    //Verify first two arguments are numeric.
        if(!isdigit(argv[i][0])) {
            printf("ERROR: Argument %d of \"cs\" command must be an integer.", i);
            return 0;
        }
    }

    return 1;
}

// Verify the uniqueness of a passed in name and that correct number of arguments
// passed for the cs command.
int verify_svrn(char *svr_name, svrinfo_t *servers) {
    int i;
    if(svr_name == NULL || svr_name[0] == '\0') return 0;      //Return 0 if svr_name null
    for( i = 0; i < MAXSERV; i++) {
        if(!strcmp(svr_name, servers[i].name)) {
            return -1;      //Return false if server name found in list
        }
    }

    return 1;           //Return true if passed server name unique in list
}

//Create a new server process
int fork_svr(char **argv)
{
    pid_t pid;

    // Check if error occured during fork
    if((pid = fork()) < 0)
    {
      perror("The fork failed.\n");
      exit(1);
    }
    else if(pid == 0)       //Child (server) process code.
    {
        argv[0] = "./svr";      //File name for server code
        // Create a new server using the passed in commands
        if(execvp(*argv, argv) < 0)
        {
            printf("Error executing command: %s\n", strerror(errno));
            // printf("%s %s %s\n", argv[0], argv[1], argv[2]);
            exit(1);
        }
    }
    return pid;
}

void add_svr(int pid, char *svr_name, svrinfo_t *servers)
{
    // Make the new server data entry
    svrinfo_t server;
    svrinfo_fact(&server);
    server.pid = pid;
    memcpy(server.name, svr_name, strlen(svr_name) + 1);

    int i;
    for (i = 0; i < MAXSERV; i++) {     //Find the first empty array element
        if(servers[i].pid <= 0) {
            servers[i] = server;        //Store server data in empty element
            num_servers++;
            break;
        }
    }
    printf("\tServer \"%s\" created.\n", svr_name);
}

void del_svr(char *svr_name, svrinfo_t *servers)
{
    if(svr_name == NULL || svr_name[0] == '\0') {
        printf("ERROR: Invalid number of arguments for \"ds\" command.\n");
        return;
    }
    int i;
    pid_t d_pid = -1;
    for(i = 0; i < MAXSERV; i++) {  //Find specified server in list
        if(!strcmp(svr_name, servers[i].name)) {
            d_pid = servers[i].pid;     //Copy server PID
            //"Delete" server record
            servers[i].pid = -1;
            servers[i].name[0] = '\0';
        }
    }
    // If a match was found send termination signal
    if(d_pid > 0) {
        kill(d_pid, SIGINT); // send terminate signal
        waitpid(d_pid, 0, 0);
        num_servers--;
    } else {    //otherwise match was not found
        printf("Delete server command failed. Server %s not found.\n", svr_name);
    }
}

//Create Process
void cr_proc(char *svr_name, svrinfo_t *servers)
{
    if(svr_name == NULL || svr_name[0] == '\0') {
        printf("ERROR: Invalid number of arguments for \"cp\" command.\n");
        return;
    }
    int i;
    for(i = 0; i < MAXSERV; i++) {
        if(!strcmp(servers[i].name, svr_name)) {    //Send add proc signal
            kill(servers[i].pid, SIGUSR1);
            return;
        }
    }

    printf("Server name \"%s\" not found\n", svr_name);
}

//Delete Process
void del_proc(char *svr_name, svrinfo_t *servers)
{
    if(svr_name == NULL || svr_name[0] == '\0') {
        printf("ERROR: Invalid number of arguments for \"dp\" command.\n");
        return;
    }

    int i;
    for( i = 0; i < MAXSERV; i++) {
        if(strcmp(servers[i].name, svr_name)) {
            kill(servers[i].pid, SIGUSR2);
            return;
        }
    }

    printf("Server name \"%s\" not found\n", svr_name);
}

//Routine that cleans up process manager resources before quitting.
void quit(svrinfo_t *servers) {
    int i;
    for(i = 0; i < MAXSERV; i++) {
        if(servers[i].pid > 0) {
            kill(servers[i].pid, SIGINT);
        }
    }
    // Wait for all child processes to terminate
    while(waitpid((pid_t)-1, 0, WNOHANG) > 0);
    printf("\tShutting down process manager...\n");
}

void disp_diag(svrinfo_t *servers)
{
    // printf("\tProcess Manager\n");
    // int i;
    // for(i = 0; i < MAXSERV; i++) {
    //     if()
    // }
}


static void nop_handler(int sigint, siginfo_t* siginfo, void* context)
{

}
