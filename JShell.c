#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/unistd.h>
#include <sys/wait.h>

/* Things to do */
// Separate findAndExecuteCommand to findCommand and executeCommand.

//Constants.
int COMMAND_LIMIT = 80;
char PROMPT[] = "JShell$ ";

//Global Variables.
char *PATH;
char **listOfPaths;

//Functions.
char* findPATH(char **);
char** getPaths();
char** getArguments(char line[]);
char**getPipedCommands(char *line);
void findAndExecuteCommand(char**);
void executePipedCommand(char**);

int main(int argC, char **argV, char **envP) {
    PATH = findPATH(envP);
    listOfPaths = getPaths();

    printf("Address %p\n", &listOfPaths);

    int count = 0;
    while(listOfPaths[count] != NULL)
    {
        printf("Base[%d] Address %p Pointer Value %p String=%s\n",
               count, &listOfPaths[count], listOfPaths[count], listOfPaths[count]);
        count++;
    }

    //Shell initialization.
    char line[COMMAND_LIMIT];
    char **arguments; // Will contain all the arguments of the command.
    bool isPipe = false;
    printf("%s", PROMPT);

    //Main loop.
    int processID;
    //while(scanf("%s", line) != EOF) {
    while(fgets(line, COMMAND_LIMIT, stdin) != NULL) {
        //DEBUG printf("Command = %s\nArgC = %d\n", line, argC)
        for(int i = 0; i < sizeof(line); i++)
        {
            if(line[i] == '|')
            {
                isPipe = true;
                break;
            }
        }
        if(!isPipe)
        {
            //Determine the command name and construct the parameter list.
            arguments = getArguments(line);
            processID = fork();
            if (processID == -1) //If a -1 is returned, the fork failed.
                puts("Error in the forking process.\n");
            else if (processID == 0) //Child process is given an id of 0;
            {
                //Find the full path name for the file.
                findAndExecuteCommand(arguments);
                //Launch the executable file with the specified parameters.
            }
            else //Parent process.
            {
                wait(&processID);
            }
        }
        else
        {
            isPipe = false;
            arguments = getPipedCommands(line);
            processID = fork();
            if (processID == -1) //If a -1 is returned, the fork failed.
                puts("Error in the forking process.\n");
            else if (processID == 0) //Child process is given an id of 0;
            {
                executePipedCommand(arguments);
            }
            else //Parent process.
            {
                wait(&processID);
            }
        }
        printf("%s", PROMPT);
    }

    //Terminate the shell.
    free(listOfPaths); //Clean up!!
    free(arguments);
    return 0;
}

char* findPATH(char **envP) {
    char test[] = "****"; //Automatically makes the array include the '\0' character, need 4 spaces for "PATH".
    int count = 0;
    while(envP[count] != NULL)
    {
        for(int dex = 0; dex < 4; dex++)
        {
            test[dex] = envP[count][dex];
        }
        if(strcmp(test, "PATH") == 0)
        {
            //printf("FOUND IT: %s\n", envP[count]);
            break;
        }
        count++;
    }
    return envP[count];
}

char** getPaths() {
    char *pPATH = PATH; //So the global variable PATH isn't affected.
    char **parsedPATH; //The return.

    pPATH += 5; //To skip over 'PATH='.

    //DEBUG printf("PATH Variable=%s\n", pPATH);
    int pathsCounter = 0;
    int lengthOfPATH = strlen(pPATH) + 1; //For malloc, add 1 since strlen ignores the null character.

    /* Allocate enough memory for enough 'array spaces' */
    for(int dex = 0; dex < lengthOfPATH; dex++)
    {
        if(pPATH[dex] == ':' || pPATH[dex] == '\0')
        {
            pathsCounter++;
        }
    }
    pathsCounter++; //Add one more since we are basing on the colons.
    //DEBUG printf("%d\n", pathsCounter);
    parsedPATH = malloc(pathsCounter * sizeof(char*)); //Allocate enough memory for the pointer array.


    /* Allocate enough memory for each string */
    int charCounter = 0, arrayCounter = 0;
    for(int dex = 0; dex < lengthOfPATH; dex++)
    {
        if(pPATH[dex] == ':' || pPATH[dex] == '\0')
        {
            charCounter++; //Include a null character for each string.
            parsedPATH[arrayCounter] = malloc(charCounter); //Allocate the memory for the string.
            //DEBUG printf("%d\n", charCounter);
            charCounter = 0;
            arrayCounter++; //Move on to the next string.
        }
        else
        {
            charCounter++;
        }
    }

    /* Store the strings in the array */
    int stringDex = 0, arrayDex = 0;
    for(int dex = 0; dex < lengthOfPATH; dex++)
    {
        if(pPATH[dex] == ':' || pPATH[dex] == '\0')
        {
            //Include the null character.
            stringDex++;
            parsedPATH[arrayDex][stringDex] = '\0';
            //String is done.
            arrayDex++;
            stringDex = 0;
        }
        else
        {
            parsedPATH[arrayDex][stringDex] = pPATH[dex];
            stringDex++;
        }
    }

    parsedPATH[pathsCounter] = NULL; //Finally set the last pointer to NULL.
    return parsedPATH;
}

char** getArguments(char line[]) {
    char **args;

    /* Get the number of arguments */
    int lineLength = strlen(line) + 1; //fgets included the /n.
    int numberOfArguments = 1; //Include the extra whitespace.

    for(int dex = 0; dex < lineLength; dex++)
    {
        if(line[dex] == ' ' || line[dex] == '\0')
            numberOfArguments++;
    }
    args = malloc(numberOfArguments * sizeof(char*));

    /* Get te length of each word. */
    int charCount = 0, arrayCount = 0;
    for(int dex = 0; dex < lineLength; dex++)
    {
        if(line[dex] == ' ' || line[dex] == '\0')
        {
            charCount++;
            args[arrayCount] = malloc(charCount * sizeof(char*));
            arrayCount++;
            charCount = 0;
        }
        else if(line[dex] == '\n')
        {
            continue;
        }
        else
        {
            charCount++;
        }
    }

    /* Assign the words to the array. */
    int stringDex = 0, arrayDex = 0;
    for(int dex = 0; dex < lineLength; dex++)
    {
        if(line[dex] == ' ' || line[dex] == '\0')
        {
            stringDex++;
            args[arrayDex][stringDex] = '\0';
            arrayDex++;
            stringDex = 0;
        }
        else if(line[dex] == '\n')
        {
            continue;
        }
        else
        {
            args[arrayDex][stringDex] = line[dex];
            stringDex++;
        }
    }
    return args;
}

char** getPipedCommands(char *line) {
    int numberOfCommands = 1;
    int lineSize = strlen(line) + 1;
    for(int i = 0; i < lineSize; i++)
    {
        //printf("%c ", line[i]);
        if(line[i]  == '|')
        {
            numberOfCommands++;
        }
    }
    char **commands = malloc(numberOfCommands * sizeof(char*));
    int commandCount = 0, argumentCount = 0;
    for(int dex = 0; dex < lineSize; dex++)
    {
        if(line[dex] == '|' || line[dex] == '\0')
        {
            commandCount++; //Include the null character.
            commands[argumentCount] = malloc(commandCount * sizeof(char*));
            commandCount = 0; //Reset the command size counter.
            argumentCount++;
        }
        else if(line[dex] == '\n')
        {
            continue;
        }
        else
        {
            commandCount++;
        }
    }
    commandCount = 0, argumentCount = 0;
    for(int dex = 0; dex < lineSize; dex++)
    {
        if(line[dex] == '|' || line[dex] == '\0')
        {
            commandCount++;
            commands[argumentCount][commandCount] = '\0';
            commandCount = 0;
            argumentCount++;
        }
        else if(line[dex] == '\n')
        {
            continue;
        }
        else
        {
            commands[argumentCount][commandCount] = line[dex];
            commandCount++;
        }
    }
    return commands;
}

void findAndExecuteCommand(char** arguments) {
    char searchPath[200];

    int count = 0;
    while(listOfPaths[count] != NULL)
    {
        strcpy(searchPath, listOfPaths[count]);
        strcat(searchPath, "/");
        strcat(searchPath, arguments[0]);
        printf("Searching %s\n", searchPath);

        //If it exists, execute!
        if(access(searchPath, F_OK) != -1)
        {
            puts("Found!!");
            execv(searchPath, arguments);
            break;
        }
        count++;
    }
}

void executePipedCommand(char** arguments) {
    char** firstArguments = getArguments(arguments[0]);
    char** secondArguments = getArguments(arguments[1]);

    const int READ = 0, WRITE = 1; //For readability.
    int thePipe[2];
    pipe(thePipe);

    int childID = fork();
    if(childID == 0) //Child process AKA the in side of the pipe.
    {
        dup2(thePipe[WRITE], STDOUT_FILENO); //Move system stdin to the pipe's input.
        close(thePipe[READ]);
        close(thePipe[WRITE]);
        findAndExecuteCommand(firstArguments);
    }
    else if(childID == -1)
        puts("Error");
    else
    {
        wait(&childID);
        dup2(thePipe[READ], STDIN_FILENO); //Move system stdout to the pipe's output.
        close(thePipe[WRITE]);
        close(thePipe[READ]);
        findAndExecuteCommand(secondArguments);
    }
    close(thePipe[READ]);
    close(thePipe[WRITE]);
}
