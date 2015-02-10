#include "procMan.h"

void init_svr_arr(void);
//Parses user input into command -> argument array form
int prsipt(char *input, char** argv, int argv_size);
//Checks for valid user commands and executes them
int input_mux(char** argv);
//Spawn a new server process using passed arguments
int fork_svr(char** argv);
//Add a servers to the servers list
void add_server(int pid, char *svr_name);
//Delete a servers from the servers list
void delete_server(char *svr_name);
//Verify input for the create server command
int verify_cs(char **argv);
//Verify server name passed is unique in the list
int verify_svrn(char *svr_name);
//Quit
void quit(void);

svrinfo_t servers[MAXSERV];
int num_servers = 0;

int main(void)
{
    char input[MAXINPUT];       //Buffer for user input
    char *argv[MAXARGS];        //Parsed user commands
    int status = 0;

    // Input loop
    while(1) {
        // Get input from the user
        if( fgets(input, MAXINPUT, stdin) == NULL) continue;
        // Parse the input and return a status
        status = prsipt(input, argv, MAXARGS);

        if(status > 0) {            // Quit program if quit command entered
            quit();
            return 0;
        } else {
            input_mux(argv);
        }
    }

    return 0;
}

// void init_svr_arr(void) {
//     int i;
//     for(i = 0; i < MAXSERV)
// }

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

int input_mux(char** argv)
{
    if(!strcmp(argv[0], "cs")) {            //Create Server Command
        if(!verify_cs(argv)) return 0;      //Verify command input

        int s_pid = fork_svr(argv);           //Create server
        if(s_pid > 0) {
            add_server(s_pid, argv[3]);     //Add server to list
        }
        return 1;

    } else if ( !strcmp(argv[0], "as") ) {  //Abort Server Command
        delete_server(argv[1]);
        return 1;
    } else if ( !strcmp(argv[0], "cp") ) {  //Create Process Command
        return 1;
    } else if ( !strcmp(argv[0], "ap") ) {  //Abort Process Command
        return 1;
    } else if( !strcmp(argv[0], "ds") ) {   //Display Status Command
        return 1;
    }

    printf("Command \"%s\" not recognized\n", argv[0]);
    return 0;
}

// Verify input for the cs command
int verify_cs(char **argv) {
    if(num_servers == MAXSERV) {
        printf("ERROR: Server creation aborted. Max servers reached.\n");
        return 0;
    }
    //Verify correct number of arguments and server name
    int vf = verify_svrn(argv[3]);
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
int verify_svrn(char *svr_name) {
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
    else if(pid == 0)       // Fork child code
    {
        argv[0] = "./svr";      //File name for server code
        // Create a new server using the passed in commands
        if(execvp(*argv, argv) < 0)
        {
            printf("Error executing command: %s\n", strerror(errno));
            //printf("%s %s %s\n", argv[0], argv[1], argv[2]);
            exit(1);
        }
    }
    return pid;
}

void add_server(int pid, char *svr_name)
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
    printf("Server %s created.\nNum Servers: %d\n----\n", svr_name, num_servers);
}

void delete_server(char *svr_name)
{
    if(svr_name == NULL || svr_name[0] == '\0') {
        printf("ERROR: Invalid number of arguments for \"as\" command.\n");
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
        printf("Server %s aborted.\nNum Servers: %d\n-----\n", svr_name, num_servers);
    } else {    //otherwise match was not found
        printf("Abort server command failed. Server %s not found.\n", svr_name);
    }
}

//Routine that cleans up process manager resources before quitting.
void quit(void) {
    int i;
    for(i = 0; i < MAXSERV; i++) {
        if(servers[i].pid > 0) {
            kill(servers[i].pid, SIGTERM);
        }
    }
    // Wait for all child processes to terminate
    while(waitpid((pid_t)-1, 0, WNOHANG) > 0);
    printf("Shutting down process manager...\n");
}
