// Seth Hilaski
// CIS 452 - Operating Systems
// Program 2 - Server Process Management System
// A basis process manager that can spawn servers and server processes and
// manage them.

#include "procMan.h"

//Parses user input into command -> argument array form
int prsipt(char *input, char** argv, int argv_size);
//Checks for valid user commands and executes them
int input_mux(char** argv, svrinfo_t *servers);
//Spawn a new server process using passed arguments
int fork_svr(char** argv);
//Add a server to the servers list
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
//Display process hierarchy of the process manager
void disp_ph(svrinfo_t *servers);
//Help menu
void help(void);
//Quit the process manager
void quit(svrinfo_t *servers);
//NOP Handler for SIGUSR1 and SIGUSR2
static void nop_handler(int, siginfo_t*, void*);

int num_servers = 0;

int main(void)
{
    //Welcome message//
    printf("Welcome to the basic process manager.\n");
    printf("Type \"help\" to display a list of commands.\n");
    char input[MAXINPUT];       //Buffer for user input
    char *argv[MAXARGS];        //Parsed user commands
    int status = 0;
    svrinfo_t servers[MAXSERV];
    int i;
    for(i = 0; i < MAXSERV; i++) {  //Initilize data for each server.
        svrinfo_fact(&servers[i]);
    }

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

/* Parse the user input into commands and arguments.
 *     input: character array containint user input
 *     argv: array of strings used to store input as tokenized parameters
 *     argv_size: the number of string elements in the argv array.
 */
int prsipt(char *input, char** argv, int argv_size)
{
    char* t_arg = strtok(input, " \n");   // Get first argument from input
    if(t_arg == NULL) return -1; //Return negative if no input
    if(!strcmp(t_arg, "quit")) return 1; //Return true if quit was entered

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

/* Function checks for valid commands and calls relevant handlers when a command
 * was recognized.
 *     argv: parsed input from the user in command and argument form
 *     servers: array containting information of active servers.
 */
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
    } else if( !strcmp(argv[0], "ph") ) {   //Display Status Command
        disp_ph(servers);
        return 1;
    } else if( !strcmp(argv[0], "help")) {  //Display help menu
        help();
        return 1;
    }

    printf("Command \"%s\" not recognized\n", argv[0]);
    return 0;
}

/* Verify input for the cs command. Checks for valid number of arguments and
 * correct servers name (with verify_svrn helper function).
 *     argv: tokenized user input command
 *     servers: array of information containing active server info
 */
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

/* Verify the uniqueness of a passed in name and that correct number of arguments
 * passed for the cs command.
 *     svr_name: server name to check for duplicates in active servers
 *     servers: array of information containing active server info
 */
int verify_svrn(char *svr_name, svrinfo_t *servers) {
    int i;
    if(svr_name == NULL || svr_name[0] == '\0') return 0;   //Check for empty
    for( i = 0; i < MAXSERV; i++) {
        if(!strcmp(svr_name, servers[i].name)) {
            return -1;      //Return false if server name found in list
        }
    }

    return 1;           //Return true if passed server name unique in list
}

/* Create a new server process using fork.
 *     argv: tokenized user input command
 */
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

/* Add a new server to the list of active servers.
 *     pid: The process id of the new server
 *     svr_name: The name of the new server
 *     servers: array of information containing active server info
 */
void add_svr(int pid, char *svr_name, svrinfo_t *servers)
{
    int i;
    for (i = 0; i < MAXSERV; i++) {         //Find the first empty array element
        if(servers[i].pid <= 0) {
            servers[i].pid = pid;                   //Set pid
            memcpy(servers[i].name, svr_name, strlen(svr_name) + 1);      //Set server name
            num_servers++;
            printf("\tServer \"%s\" created.\n", svr_name);
            break;
        }
    }
}

/* Delete a server and all its processes.
 *     svr_name: The name of the server to delete
 *     servers: array of information containing active server info
 */
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
        num_servers--;
        kill(d_pid, SIGINT); // send terminate signal
        waitpid(d_pid, 0, 0);

    } else {    //otherwise match was not found
        printf("Delete server command failed. Server %s not found.\n", svr_name);
    }
}

/* Create a new process for the indicated server.
 *     svr_name: The name of the server to create a new process for
 *     servers: array of information containing active server info
 */
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

/* Delete a process from the indicated server.
 *     svr_name: The name of the server to remove a process from
 *     servers: array of information containing active server info
 */
void del_proc(char *svr_name, svrinfo_t *servers)
{
    if(svr_name == NULL || svr_name[0] == '\0') {
        printf("ERROR: Invalid number of arguments for \"dp\" command.\n");
        return;
    }
    int i;
    for( i = 0; i < MAXSERV; i++) {
        if(!strcmp(servers[i].name, svr_name)) {
            kill(servers[i].pid, SIGUSR2);
            return;
        }
    }

    printf("Server name \"%s\" not found\n", svr_name);
}

/* Routine that cleans up process manager resources before quitting.
 *     servers: array of information containing active server info
 */
void quit(svrinfo_t *servers) {
    int i;
    for(i = 0; i < MAXSERV; i++) {
        if(servers[i].pid > 0) {
            kill(servers[i].pid, SIGINT);
        }
    }
    // Wait for all child processes to terminate (blocking)
    while(waitpid((pid_t)-1, 0, 0) > 0);
    printf("\tShutting down process manager...\n");
}

/* Displays the help menu with a list of valid commands to the user.
 */
void help(void) {
    printf("----- HELP MENU ----\n");
    printf("cs <minp> <maxp> <name>\n");
    printf("\tCreate server <name> with at least <minp> processes.\n");
    printf("ds <name>\n\tDelete server <name> and all its processs.\n");
    printf("cp <name>\n\tCreate a new process on server <name>.\n");
    printf("dp <name>\n\tDelete a process on server <name>.\n");
    printf("ph\n\tDisplay the process hierarchy.\n");
    printf("quit\n\tQuit the process manager.\n");
}

/* Display the process hierarchy of the process manager.
 *     servers: array of information containing active server info
 */
void disp_ph(svrinfo_t *servers)
{
    pid_t pid;
    char *argv[3] = {"ps", "-H", '\0'};

    if((pid = fork()) < 0) {     // Check if error occured during fork

      perror("The fork failed.\n");
      exit(1);
    }
    else if(pid == 0) {      //Child process code
        printf("---- PROCESS HIERARCHY ----\n");
        //Run the ps -H command to display the process hierarchy.
        if(execvp(*argv, argv) < 0) {
            printf("Error executing command: %s\n", strerror(errno));
            exit(1);
        }
    }
    waitpid((pid_t)-1, 0, 0);   // wait for child process

}

/* A signal handler that does nothing to prevent erratic behavior on process
 * manager when SIGUSR1 and SIGUSR2 signaled to a server.
 */
static void nop_handler(int sigint, siginfo_t* siginfo, void* context)
{
}
