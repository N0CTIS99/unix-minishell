#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/wait.h>

//pre-defining the command length
#define MAX_cmd_LENGTH 1024

int bgPid = -1; //this variable will contain the pid of the process sent to the background

//function for getting the command from the terminal
char *getCmd() {

    char *cmd = (char *)malloc(MAX_cmd_LENGTH * sizeof(char));
    if (cmd == NULL) {
        perror("\nerror in malloc\n");
        exit(1);
    }
    if (fgets(cmd, MAX_cmd_LENGTH, stdin) == NULL) {    //0 for std input
        free(cmd);
        return NULL;
    }
    //the following variable checks if user has pressed enter
    char *checkEnter = strchr(cmd, '\n');
    
    //fgets gets the "\n" as well when we press enter, so the following conditional statement checks for the "\n", replaces it with "\0" aka EOF and then returns the cmd to the main()
    if (checkEnter != NULL) {
        *checkEnter = '\0';
    }
    return cmd;
}

//function for opening a new copy of shell24
int newTerminal(){

    int f = fork();

    if(f<0){
        printf("\nError in fork\n");
        return 1;
    }
    else if(f==0){  //child process

        //using exec to open up a new terminal when "newt" is entered, I'm using xterm as the terminal emulator
        if (execlp("xterm", "xterm", "-e", "./shell24", "shell24$newt", NULL) == -1) {
        
            printf("\nError in exec\n");
            return 1;
        }
    }

    return 0;
}

//this function checks the number of arguments in a command
int countArgs(char *cmd){

    //making a copy of the original cmd variable so that the original does not get changed by strtok
    char *cmdCpy = strdup(cmd);

    char *token; //variable to store argument
    int numOfArg = 0; //number of arguments

    token = strtok(cmdCpy, " ");  //tokenizing using space
    while(token != NULL){
        // printf("\nToken at countArgs is: %s\n", token);
        token = strtok(NULL, " ");
        numOfArg++;
        
    }

    return numOfArg;
    
}

//this function breaks each command down to separate arguments and puts them in the array
void getArgs(char *cmd, char *args[]){

    char *token; //variable to store argument
    int count = 0; //index of argArray
    char *cmdCpy = strdup(cmd);
    // printf("\ncmdCpy: %s\n", cmdCpy);

    token = strtok(cmdCpy, " ");  //tokenizing using space
    while(token != NULL){
        // printf("\nToken at getArgs is: %s\n", token);
        args[count] = token;
        // printf("\nargArray[%d]: %s", count, args[count]);

        token = strtok(NULL, " ");
        count++;
        
    }
    
}

//this function handles the text concatanation bit of the entire program
int concatTxt(char *cmd){

    char *token;    //variable for tokenization
    char *tokArray[6];  //array to store the commands segregated by the "#" character; since only 5 concatanations are allowed, I've set the array size to 6
    int numOfCmd = 0;   //total number of tokens/commands found

    token = strtok(cmd, "#");   //tokenizing based on "#"
    while(token != NULL && numOfCmd < 6){
        
        char *noSpaceCmd = token;   //this variable will hold the command without any spaces before and after it
        
        //removing any leading spaces before the commands, if there is are leading spaces then the pointer is moved forward to a non-space character
        while (isspace(*noSpaceCmd))
        {
            noSpaceCmd++;
        }

        //removing any trailing space characters
        int len = strlen(noSpaceCmd);
        while (len > 0 && isspace(noSpaceCmd[len - 1])) {
            noSpaceCmd[--len] = '\0';
        }
        
        tokArray[numOfCmd++] = noSpaceCmd; //command is stored in the array after space trimming and then numOfCmd is incremented by 1
        token = strtok(NULL, "#");

        //only allowing 5 concatanations
        if (numOfCmd == 6 && token != NULL)
        {
            printf("\nError. Only 5 concatanation operations are allowed at a time.\n");
            return 1;
        }
    }

    // printf("\nTotal number of commands are: %d\n", numOfCmd);

    if (numOfCmd < 2)
    {
        printf("\nPlease enter the name of at least two text files for concatanation\n");
        return 1;
    }

    char *concatCmd[numOfCmd + 2]; //the extra 2 is for cat at the front and NULL at the back

    concatCmd[0] = "cat";
    

    for (int i = 0; i < numOfCmd; i++)
    {
        concatCmd[i + 1] = tokArray[i];   
    }

    concatCmd[numOfCmd + 1] = NULL;

    // printf("\nCurrent State of the command\n");
    // for (int i = 0; i < numOfCmd+2; i++)
    // {
    //     printf("%s ", concatCmd[i]);   
    // }

    //forking so that shell24 doesn't end after executing a specific command
    int f = fork();
    if (f == 0) //child process
    {
        if (execvp("cat", concatCmd) != 0)
        {
            printf("\nExec failed.\n");
            return 1;
        }
    }
    else if (f < 0)
    {
        printf("\nFork error.\n");
        return 1;
    }
    else{   //parent process
        wait(NULL); //parent waits for child processes to finish
    }
    
    return 0;

}

//function for handling piping operation
int pipeOp(char *cmd){

    char *token;    //variable for tokenization
    char *tokArray[7];  //array to store the commands segregated by the "|" character; since only 6 piping operations are allowed, I've set the array size to 7
    int numOfCmd = 0;   //total number of tokens/commands found

    token = strtok(cmd, "|");   //tokenizing based on "|"
    while(token != NULL && numOfCmd < 7){
        
        char *noSpaceCmd = token;   //this variable will hold the command without any spaces before and after it
        
        //removing any leading spaces before the commands, if there is are leading spaces then the pointer is moved forward to a non-space character
        while (isspace(*noSpaceCmd))
        {
            noSpaceCmd++;
        }

        //removing any trailing space characters
        int len = strlen(noSpaceCmd);
        while (len > 0 && isspace(noSpaceCmd[len - 1])) {
            noSpaceCmd[--len] = '\0';
        }
        
        tokArray[numOfCmd++] = noSpaceCmd; //command is stored in the array after space trimming and then numOfCmd is incremented by 1
        token = strtok(NULL, "|");

        //only allowing 6 piping operations
        if (numOfCmd == 7 && token != NULL)
        {
            printf("\nError. Only 6 piping operations are allowed at a time.\n");
            return 1;
        }
    }

    // printf("\nTotal number of commands are: %d\n", numOfCmd);

    if (numOfCmd < 2)
    {
        printf("\nPlease enter at least 2 commands for piping operations.\n");
        return 1;
    }
    
    //actual piping operation begins here
    for (int i = 0; i < numOfCmd; i++)
    {
        // printf("\nSub-command: %s", tokArray[i]);

        //checking the number of arguments per command
        int numOfArg = countArgs(tokArray[i]);
        // printf("\nTotal arguments in the token are: %d\n", numOfArg);

        //checking if the number of arguments per command is >=1 and <=5 or not
        if (numOfArg < 1 || numOfArg > 5)
        {
            printf("\nThe number of arguments per command must be >=1 or <=5.\n");
            return 1;
        }

        //getting all the arguments for each command in a separate array
        char *argArray[numOfArg + 1];   //this array will contain the arguments of each separate command
        getArgs(tokArray[i], argArray);

        // printf("\nthe arguments in %s are:\n", tokArray[i]);
        // for (int j = 0; j < numOfArg; j++)
        // {
        //     printf("%s\n", argArray[j]);
        // }

        argArray[numOfArg] = NULL;

        //this ensures that the loop runs until the 2nd last command after which the last command is printed to the terminal
        if (i == numOfCmd - 2)
        {
            int fd[2];

            if(pipe(fd) == -1){ //opening pipe
                return 1;
            }

            int f = fork();
            if(f == 0 ){    //child process
                close(fd[0]);
                dup2(fd[1], 1); //stdout to write end
                execvp(argArray[0], argArray);
                
            }
            else if (f < 0)
            {
                printf("\nFork error.\n");
                return 1;
            }
            else{   //parent process
                int numOfArg = countArgs(tokArray[i+1]);

                //checking if the number of arguments per command is >=1 and <=5 or not
                if (numOfArg < 1 || numOfArg > 5)
                {
                    printf("\nThe number of arguments per command must be >=1 or <=5.\n");
                    return 1;
                }

                getArgs(tokArray[i+1], argArray);
                argArray[numOfArg] = NULL;
                // printf("\nargArray in final parent is: %s\n", argArray[0]);
                close(fd[1]);
                dup2(fd[0], 0); //stdin to read end
                execvp(argArray[0], argArray);
                wait(NULL); //parent waits for its child process to finish
                break;
            }
        }
        else{
            int fd[2];

            if(pipe(fd) == -1){ //opening pipe
                return 1;
            }

            int f = fork();
            if(f == 0 ){    //child process
                close(fd[0]);
                dup2(fd[1], 1); //stdout to write end
                execvp(argArray[0], argArray);
            }
            else if (f < 0)
            {
                printf("\nFork error.\n");
                return 1;
            }
            else{   //parent process
                close(fd[1]);
                dup2(fd[0], 0); //stdin to read end
                wait(NULL); //parent waits for its child process to finish
            }
        }
        
    }

    return 0;
    
}

//this function handles the ">" aka output redirection operation
int outputRedir(char *cmd){

    char *token;    //variable for tokenization
    char *tokArray[2];  //array to store the commands segregated by the ">" character
    int numOfCmd = 0;   //total number of tokens/commands found

    token = strtok(cmd, ">");   //tokenizing based on ">"
    while(token != NULL && numOfCmd < 2){
        
        char *noSpaceCmd = token;   //this variable will hold the command without any spaces before and after it
        
        //removing any leading spaces before the commands, if there is are leading spaces then the pointer is moved forward to a non-space character
        while (isspace(*noSpaceCmd))
        {
            noSpaceCmd++;
        }

        //removing any trailing space characters
        int len = strlen(noSpaceCmd);
        while (len > 0 && isspace(noSpaceCmd[len - 1])) {
            noSpaceCmd[--len] = '\0';
        }
        
        tokArray[numOfCmd++] = noSpaceCmd; //command is stored in the array after space trimming and then numOfCmd is incremented by 1
        token = strtok(NULL, ">");

        if (numOfCmd == 2 && token != NULL)
        {
            printf("\nError. Please enter arguments in the following way: [arguments] > [output-file].\n");
            return 1;
        }
    }

    // printf("\nTotal number of commands are: %d\n", numOfCmd);

    if (numOfCmd < 2)
    {
        printf("\nPlease enter your arguments followed by > followed by the name of the output file.\n");
        return 1;
    }

    //checking the number of arguments for the command before ">"
    int numOfArg = countArgs(tokArray[0]);
    

    //checking if the number of arguments per command is >=1 and <=5 or not
    if (numOfArg < 1 || numOfArg > 5)
    {
        printf("\nThe number of arguments before > must be >=1 or <=5.\n");
        return 1;
    }

    char *argArray[numOfArg + 1];   //this array will contain the arguments and NULL 
    getArgs(tokArray[0], argArray);

    argArray[numOfArg] = NULL;

    //opening file for writing; if file doesn't exist it gets created; if file already has content in it, it gets overwritten 
    int outFd = open(tokArray[1], O_WRONLY | O_CREAT | O_TRUNC, 0777);

    if(outFd == -1)
    {
        printf("\nError opening %s\n", tokArray[1]);
        return 1;
    }

    int cp1 = dup(1);   //variable for reversing redirection
    dup2(outFd, 1); //stdout to file
    close(outFd);

    int f = fork();
    if (f == 0) //child process
    {
        if (execvp(argArray[0], argArray) != 0)
        {
            dup2(cp1, 1);   //reversing redirection
            printf("\nExec failed. Please recheck your command.\n");
            return 1;
        }
    }
    else if (f < 0)
    {
        printf("\nFork error.\n");
        return 1;
    }
    else{   //parent process
        wait(NULL); //parent waits for child processes to finish
        dup2(cp1, 1);   //reversing redirection
    }

       
    return 0;

}


//this function handles the ">>" aka append output redirection operation
int appendRedir(char *cmd){

    char *token;    //variable for tokenization
    char *tokArray[2];  //array to store the commands segregated by the ">>" substring
    int numOfCmd = 0;   //total number of tokens/commands found

    token = strtok(cmd, ">>");   //tokenizing based on ">>"
    while(token != NULL && numOfCmd < 2){
        
        char *noSpaceCmd = token;   //this variable will hold the command without any spaces before and after it
        
        //removing any leading spaces before the commands, if there is are leading spaces then the pointer is moved forward to a non-space character
        while (isspace(*noSpaceCmd))
        {
            noSpaceCmd++;
        }

        //removing any trailing space characters
        int len = strlen(noSpaceCmd);
        while (len > 0 && isspace(noSpaceCmd[len - 1])) {
            noSpaceCmd[--len] = '\0';
        }
        
        tokArray[numOfCmd++] = noSpaceCmd; //command is stored in the array after space trimming and then numOfCmd is incremented by 1
        token = strtok(NULL, ">>");

        if (numOfCmd == 2 && token != NULL)
        {
            printf("\nError. Please enter arguments in the following way: [arguments] >> [output-file].\n");
            return 1;
        }
    }

    // printf("\nTotal number of commands are: %d\n", numOfCmd);

    if (numOfCmd < 2)
    {
        printf("\nPlease enter your arguments followed by >> followed by the name of the output file.\n");
        return 1;
    }

    //checking the number of arguments for the command before ">>"
    int numOfArg = countArgs(tokArray[0]);
    

    //checking if the number of arguments per command is >=1 and <=5 or not
    if (numOfArg < 1 || numOfArg > 5)
    {
        printf("\nThe number of arguments before > must be >=1 or <=5.\n");
        return 1;
    }

    char *argArray[numOfArg + 1];   //this array will contain the arguments and NULL
    getArgs(tokArray[0], argArray);

    argArray[numOfArg] = NULL;

    //opening file for writing; if file doesn't exist it gets created; if file already has content in it, the new output gets appended
    int outAppendFd = open(tokArray[1], O_WRONLY | O_CREAT | O_APPEND, 0777);

    if(outAppendFd == -1)
    {
        printf("\nError opening %s\n", tokArray[1]);
        return 1;
    }

    int cp1 = dup(1);   //variable for reversing redirection
    dup2(outAppendFd, 1); //stdout to file
    close(outAppendFd);

    int f = fork();
    if (f == 0) //child process
    {
        if (execvp(argArray[0], argArray) != 0)
        {
            dup2(cp1, 1);   //reversing redirection
            printf("\nExec failed. Please recheck your command.\n");
            return 1;
        }
    }
    else if (f < 0)
    {
        printf("\nFork error.\n");
        return 1;
    }
    else{   //parent process
        wait(NULL); //parent waits for child processes to finish
        dup2(cp1, 1);   //reversing redirection
    }
    
       
    return 0;

}


//this function handles the "<" aka input redirection operation
int inputRedir(char *cmd){

    char *token;    //variable for tokenization
    char *tokArray[2];  //array to store the commands segregated by the "<" character
    int numOfCmd = 0;   //total number of tokens/commands found

    token = strtok(cmd, "<");   //tokenizing based on "<"
    while(token != NULL && numOfCmd < 2){
        
        char *noSpaceCmd = token;   //this variable will hold the command without any spaces before and after it
        
        //removing any leading spaces before the commands, if there is are leading spaces then the pointer is moved forward to a non-space character
        while (isspace(*noSpaceCmd))
        {
            noSpaceCmd++;
        }

        //removing any trailing space characters
        int len = strlen(noSpaceCmd);
        while (len > 0 && isspace(noSpaceCmd[len - 1])) {
            noSpaceCmd[--len] = '\0';
        }
        
        tokArray[numOfCmd++] = noSpaceCmd; //command is stored in the array after space trimming and then numOfCmd is incremented by 1
        token = strtok(NULL, "<");

        if (numOfCmd == 2 && token != NULL)
        {
            printf("\nError. Please enter arguments in the following way: [arguments] < [input-file].\n");
            return 1;
        }
    }

    // printf("\nTotal number of commands are: %d\n", numOfCmd);

    if (numOfCmd < 2)
    {
        printf("\nPlease enter your arguments followed by < followed by the name of the input file.\n");
        return 1;
    }

    //checking the number of arguments for the command before ">"
    int numOfArg = countArgs(tokArray[0]);
    

    //checking if the number of arguments per command is >=1 and <=5 or not
    if (numOfArg < 1 || numOfArg > 5)
    {
        printf("\nThe number of arguments before > must be >=1 or <=5.\n");
        return 1;
    }

    char *argArray[numOfArg + 1];   //this array will contain the arguments and NULL
    getArgs(tokArray[0], argArray);

    argArray[numOfArg] = NULL;

    //opening file for reading;  
    int inFd = open(tokArray[1], O_RDONLY);

    if(inFd == -1)
    {
        printf("\nError opening %s\n", tokArray[1]);
        return 1;
    }

    int cp0 = dup(0);   //variable for reversing redirection
    dup2(inFd, 0); //stdin from file
    close(inFd);

    int f = fork();
    if (f == 0) //child process
    {
        if (execvp(argArray[0], argArray) != 0)
        {
            dup2(cp0, 1);   //reversing redirection
            printf("\nExec failed. Please recheck your command.\n");
            return 1;
        }
    }
    else if (f < 0)
    {
        printf("\nFork error.\n");
        return 1;
    }
    else{   //parent process
        wait(NULL); //parent waits for child processes to finish
        dup2(cp0, 0);   //reversing redirection
    }


    
       
    return 0;

}


//function for handling sequential operation
int seqExe(char *cmd){

    char *token;    //variable for tokenization
    char *tokArray[6];  //array to store the commands segregated by the ";" and since only 5 sequential operations are allowed, I've set the array size to 6
    int numOfCmd = 0;   //total number of tokens/commands found

    token = strtok(cmd, ";");   //tokenizing based on ";" 
    while(token != NULL && numOfCmd < 6){
        
        char *noSpaceCmd = token;   //this variable will hold the command without any spaces before and after it
        
        //removing any leading spaces before the commands, if there is are leading spaces then the pointer is moved forward to a non-space character
        while (isspace(*noSpaceCmd))
        {
            noSpaceCmd++;
        }

        //removing any trailing space characters
        int len = strlen(noSpaceCmd);
        while (len > 0 && isspace(noSpaceCmd[len - 1])) {
            noSpaceCmd[--len] = '\0';
        }
        
        tokArray[numOfCmd++] = noSpaceCmd; //command is stored in the array after space trimming and then numOfCmd is incremented by 1
        token = strtok(NULL, ";");

        //only allowing 5 sequential operations
        if (numOfCmd == 6 && token != NULL)
        {
            printf("\nError. Only 5 sequential operations are allowed at a time.\n");
            return 1;
        }
    }

    // printf("\nTotal number of commands are: %d\n", numOfCmd);

    if (numOfCmd < 2)
    {
        printf("\nPlease enter at least 2 commands for sequential operations.\n");
        return 1;
    }
    
    //actual sequential operation begins here
    for (int i = 0; i < numOfCmd; i++)
    {
        int f = fork();
        if (f == 0) //child process
        {
            // printf("\nSub-command: %s", tokArray[i]);

            //checking the number of arguments per command
            int numOfArg = countArgs(tokArray[i]);
            // printf("\nTotal arguments in the token are: %d\n", numOfArg);

            //checking if the number of arguments per command is >=1 and <=5 or not
            if (numOfArg < 1 || numOfArg > 5)
            {
                printf("\nThe number of arguments per command must be >=1 or <=5.\n");
                return 1;
            }

            //getting all the arguments for each command in a separate array
            char *argArray[numOfArg + 1];   //this array will contain the arguments of each separate command
            getArgs(tokArray[i], argArray);

            argArray[numOfArg] = NULL;

            printf("\n");

            if (execvp(argArray[0], argArray) != 0)
            {
                printf("\nExec failed. Please recheck your command.\n");
                return 1;
            }  
        }
        else if (f < 0)
        {
            printf("\nFork error.\n");
            return 1;
        }
        else{   //parent process
            wait(NULL); //parent waits for child processes to finish
        }
        
    }

    return 0;
    
}


//this function handles the background processing bit of the entire program
int backgroundPr(char *cmd){

    char *token;    //variable for tokenization
    char *tokArray[1];  //allowing only one program to be sent to the process at a time. So, I've set the array size to 1
    int numOfCmd = 0;   //total number of tokens/commands found

    token = strtok(cmd, "&");   //tokenizing based on "&"
    while(token != NULL && numOfCmd < 1){
        
        char *noSpaceCmd = token;   //this variable will hold the command without any spaces before and after it
        
        //removing any leading spaces before the commands, if there is are leading spaces then the pointer is moved forward to a non-space character
        while (isspace(*noSpaceCmd))
        {
            noSpaceCmd++;
        }

        //removing any trailing space characters
        int len = strlen(noSpaceCmd);
        while (len > 0 && isspace(noSpaceCmd[len - 1])) {
            noSpaceCmd[--len] = '\0';
        }
        
        tokArray[numOfCmd++] = noSpaceCmd; //command is stored in the array after space trimming and then numOfCmd is incremented by 1
        token = strtok(NULL, "&");

        //only allowing 5 concatanations
        if (numOfCmd == 1 && token != NULL)
        {
            printf("\nError. Only 1 program can be sent to the background at a time.\n");
            return 1;
        }
    }

    // printf("\nTotal number of commands are: %d\n", numOfCmd);

    if (numOfCmd < 1)
    {
        printf("\nPlease enter the name of at least one exectable program for background processing.\n");
        return 1;
    }

    // char *pwd = "./";
    // char *bgCmd = malloc(strlen(pwd) + strlen(tokArray[0]) + + 1); //this variable will contain the executable name alongside "./"

    // strcpy(bgCmd, pwd);
    // strcat(bgCmd, tokArray[0]);

    int numOfArg = countArgs(tokArray[0]);

    char *argArray[numOfArg + 1];   //this array will contain the arguments of each separate command
    getArgs(tokArray[0], argArray);

    argArray[numOfArg] = NULL;

    //forking so that shell24 doesn't end after executing a specific command
    int f = fork();
    if (f == 0) //child process
    {
        if (execvp(argArray[0], argArray) != 0)
        {
            printf("\nExec failed.\n");
            // free(bgCmd);
            return 1;
        }
    }
    else if (f < 0)
    {
        printf("\nFork error.\n");
        // free(bgCmd);
        return 1;
    }
    else{   //parent process
        bgPid = f; //storing the pid of the process sent to the background
        // free(bgCmd);
    }

    return 0;

}

//function to bring back the last process sent to the background
int foregroundPr(){
    int st;
    waitpid(bgPid, &st, WUNTRACED); //WUNTRACED is used here so that it waits for the child/background process
    bgPid = -1;

    return 0;
}

//function to handle conditional (&&/||) operations; for "||" operation I have used recursion and for "&&" operations I have called the sequential execution (seqExe) function
int conditionOp(char *cmd){

    //if the user inputted ||
    if(strstr(cmd, "||") != NULL){
        char *token;    //variable for tokenization
        char *tokArray[2];  //array to store the commands segregated by the "||" character
        int numOfCmd = 0;   //total number of tokens/commands found

        token = strtok(cmd, "||");   //tokenizing based on "||"
        while(token != NULL && numOfCmd < 2){
            
            char *noSpaceCmd = token;   //this variable will hold the command without any spaces before and after it
            
            //removing any leading spaces before the commands, if there is are leading spaces then the pointer is moved forward to a non-space character
            while (isspace(*noSpaceCmd))
            {
                noSpaceCmd++;
            }

            //removing any trailing space characters
            int len = strlen(noSpaceCmd);
            while (len > 0 && isspace(noSpaceCmd[len - 1])) {
                noSpaceCmd[--len] = '\0';
            }
            
            tokArray[numOfCmd++] = noSpaceCmd; //command is stored in the array after space trimming and then numOfCmd is incremented by 1
            token = strtok(NULL, "||");

            if (numOfCmd == 2 && token != NULL)
            {
                printf("\nError. Please enter arguments in the following way: [arguments] || [arguments].\n");
                return 1;
            }
        }

        // printf("\nTotal number of commands are: %d\n", numOfCmd);

        if (numOfCmd < 2)
        {
            printf("\nPlease enter your arguments followed by || followed by other arguments.\n");
            return 1;
        }

        //actual || operation begins here; here the left hand side is recursively sent back to conditionOp() to check if there are are || operators in the sub-commands after they are already segregated and tokenized by "||"
        // if so then it further segregates and tokenizes it by "||", otherwise it moves on to executing based on "&&"

        for(int i=0 ; i < numOfCmd ; i++){
            int success = conditionOp(tokArray[i]);
            if(success != 0){
                continue;
            }
            //if left hand side of "||" works then no need to run right hand side
            else{
                break;
            }
        }
          
    }
    //&& operations
    else{

        char *token;    //variable for tokenization
        char *tokArray[6];  //array to store the commands segregated by the "&&" characters; since only 5 sequential operations are allowed, I've set the array size to 6
        int numOfCmd = 0;   //total number of tokens/commands found

        token = strtok(cmd, "&&");   //tokenizing based on "&&" for conditional operation
        while(token != NULL && numOfCmd < 6){
            
            char *noSpaceCmd = token;   //this variable will hold the command without any spaces before and after it
            
            //removing any leading spaces before the commands, if there is are leading spaces then the pointer is moved forward to a non-space character
            while (isspace(*noSpaceCmd))
            {
                noSpaceCmd++;
            }

            //removing any trailing space characters
            int len = strlen(noSpaceCmd);
            while (len > 0 && isspace(noSpaceCmd[len - 1])) {
                noSpaceCmd[--len] = '\0';
            }
            
            tokArray[numOfCmd++] = noSpaceCmd; //command is stored in the array after space trimming and then numOfCmd is incremented by 1
            token = strtok(NULL, "&&");

            //only allowing 5 sequential operations
            if (numOfCmd == 6 && token != NULL)
            {
                printf("\nError. Only 5 sequential operations are allowed at a time.\n");
                return 1;
            }
        }

        // printf("\nTotal number of commands are: %d\n", numOfCmd);
        
        //actual && operations begins here
        for (int i = 0; i < numOfCmd; i++)
        {
            int f = fork();
            if (f == 0) //child process
            {
                //checking the number of arguments per command
                int numOfArg = countArgs(tokArray[i]);
                // printf("\nTotal arguments in the token are: %d\n", numOfArg);

                //checking if the number of arguments per command is >=1 and <=5 or not
                if (numOfArg < 1 || numOfArg > 5)
                {
                    printf("\nThe number of arguments per command must be >=1 or <=5.\n");
                    return 1;
                }

                //getting all the arguments for each command in a separate array
                char *argArray[numOfArg + 1];   //this array will contain the arguments of each separate command
                getArgs(tokArray[i], argArray);

                argArray[numOfArg] = NULL;

                printf("\n");

                if (execvp(argArray[0], argArray) != 0)
                {
                    printf("\nExec failed. Please recheck your command.\n");
                    exit(1);    //if LHS of "&&" fails then send exit failed
                }  
            }
            else if (f < 0)
            {
                printf("\nFork error.\n");
                return 1;
            }
            else{   //parent process
                int st;
                waitpid(f, &st, 0);

                //check status of child process
                if (WIFEXITED(st))
                {
                    int exitSt = WEXITSTATUS(st);
                    if (exitSt != 0)
                    {
                        return 1;
                    }
                    
                }
                
            }
            
        }
        
    }

    return 0;
}



int main() {
    char *cmd;

    for(;;) {
        printf("\nshell24$ ");
        fflush(stdout); //flushing the output buffer so that the results of the functionalities do not appear after "shell24$"
        
        cmd = getCmd(); //getting the command inputted by the user

        //optional
        if (cmd == NULL) {
            // End of input (e.g., EOF reached)
            printf("\n");
            break;
        }

        // printf("\nThe cmd you entered is: %s\n", cmd);

        //functionalities

        //opening a new terminal
        if(strcmp(cmd, "newt") == 0){
            printf("\nOpening another copy of shell24.\n");
            newTerminal();
        }
        else if((strcmp(cmd, "fg") == 0) && bgPid != -1){
            printf("\nBringing the process with pid %d to the foreground\n", bgPid);
            foregroundPr();
        }
        //if the user did not decide to open a new terminal the program moves on to the other functionalities
        else{

            // handling special characters
            printf("\nOnto the main functionality\n");
            char *option;
            option = strpbrk(cmd, "#|&;><");    //searching for the 1st occurence of any of the following characters: #, |, &, ;, >, <
            if (option != NULL) {
                // if found, appropriate function related to the special character will be called
                if (*option == '#') {
                    printf("\nConcatenation operation according to the the given command has been executed.\n\n");
                    concatTxt(cmd);

                } 
                else if (*option == '|') {
                    char *subStr = strstr(cmd, "||");   //variable to check if the user inputted | or ||
                    if (subStr != NULL) {
                        printf("\nConditional operation on the given command has been executed.\n\n");
                        conditionOp(cmd);
                    } else {
                        printf("\nPiping operation on the given command has been executed.\n\n");
                        pipeOp(cmd);
                    }
                    
                }
                else if (*option == '>') {
                    char *subStr = strstr(cmd, ">>");   //variable to check if the user inputted > or >>
                    if (subStr != NULL) {
                        printf("\nOutput redirection and appending (>>) according to the given command has been executed.\n\n");
                        appendRedir(cmd);
                    } else {
                        printf("\nOuput redirection (>) according to the given command has been executed.\n\n");
                        outputRedir(cmd);
                    }
                     
                }
                else if (*option == '<') {
                    printf("\nInput redirection (<) according to the given command has been executed.\n\n");
                    inputRedir(cmd);
                }
                else if (*option == ';') {
                    printf("Sequential execution of the given command has been executed.\n\n", cmd);
                    seqExe(cmd);
                }
                else if (*option == '&') {
                    char *subStr = strstr(cmd, "&&");   //variable to check if the user inputted & or &&
                    if (subStr != NULL) {
                        printf("\nConditional operation on the given command has been executed.\n\n");
                        conditionOp(cmd);
                    } else {
                        printf("\nExecuting the given program in the background.\n\n");
                        backgroundPr(cmd);
                    }
                } 
                else{
                    printf("\nInvalid command entered.\n\n");
                }
                // You can add similar conditions for other special characters
            }
            else{
                printf("\nNo commands entered\n\n");
            }
        }

        free(cmd);
    }

    return 0;
}
