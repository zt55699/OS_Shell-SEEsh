/***************************
 CSC 360 AS1PartB
 Name:Tong Zhang
 StuID: V00926513
 Date: 01/31/2020
 Reference:"Tutorial -
        Write a Shell in C"
        by Stephen Brennan
****************************/
//.SEEshrc: run commands

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>  // Linux
#include <stdbool.h> // bool
#include <stdio.h> // printf()
#include <stdlib.h> // malloc()
#include <string.h>
#include <signal.h>

#define SEEsh_TOK_BUFFERSIZE 512
#define SEEsh_TOK_DELIM " ,\t\r\n\a"

typedef void (*ctrlC_handler_t)(int);
extern char **environ;
char** SEEsh_split_line(char*);
char* SEEsh_read_line(void);
void SEEsh_loop(void);
int SEEsh_launch(char**);

//builtin commands
char* builtin_str[] = {"cd", "help", "exit", "pwd", "set", "unset"};
int SEEsh_cd(char** args);
int SEEsh_help(char** args);
int SEEsh_exit(char** args);
int SEEsh_pwd(char** args);
int SEEsh_set(char** args);
int SEEsh_unset(char** args);

int (*builtin_fun[]) (char **) = {
    &SEEsh_cd,
    &SEEsh_help,
    &SEEsh_exit,
    &SEEsh_pwd,
    &SEEsh_set,
    &SEEsh_unset
};

int SEEsh_num_builtins() {
    return sizeof(builtin_str)/ sizeof(char*);
}

int SEEsh_cd(char **args){
    if(args[1]==NULL){
        if(chdir(getenv("HOME")) != 0)
            perror("SEEsh");
    }
    else if(args[1]!=NULL){
        if(chdir(args[1]) != 0)
            perror("SEEsh");
    }
    return 1;
}

int SEEsh_help(char **args){
    int i;
    printf("Tong's CSC360 AS1 SEEsh Shell\n");
    printf("Input program names and arguments, and hit enter.\n");
    printf("Builtin commands are the followings:\n");
    
    for (i=0; i<SEEsh_num_builtins(); i++) {
        printf("   %s\n", builtin_str[i]);
    }
    printf("\"HOME\" is initialized to current directory.\n");
    printf("Use the man command for Linux's programs.\n");
    return 1;
}

int SEEsh_exit(char **args){
    return 0;
}

int SEEsh_pwd(char **args){
    char *buf = NULL;
    size_t size = 0;
    char *path = getcwd(buf,size);
    if (path == NULL){
        perror("SEEsh");
    }
    printf ("%s\n", path);
    free(buf);
    return 1;
}
int SEEsh_set(char **args){
    //setenv(); unsetenv(); getenv();
    //env VAR = something
    //int setenv(const char *name, const char *value, int overwrite);
    if(args[1]==NULL){
        printf ("Show env list: \n");
        char *string =*environ;
        int i =1;
        while(string!=NULL){
            printf ("%s\n", string);
            string = *(environ+i);
            i++;
        }
        return 1;
    }
    else if(args[2] ==NULL){
        args[2] = "";
        if(setenv (args[1],args[2],1)!=0){
            perror("SEEsh");
        }
        else
            printf ("SEEsh: set %s to empty string\n", args[1]);
    }
    else{
        fprintf(stderr, "SEEsh: expected argument format: [VARIABLE] [newVALUE]\n");
        return 1;
    }
    if(setenv (args[1],args[2],1)!=0){
        perror("SEEsh");
    }
    return 1;
}
int SEEsh_unset(char **args){
    //int unsetenv(const char *name);
    if(args[1]==NULL){
        fprintf(stderr, "SEEsh: expected argument to \"unset\"\n");
        return 1;
    }
    if(unsetenv (args[1])!=0){
        perror("SEEsh");
    }
    return 1;
}

void ctrlC_handler(int sig_num){
    printf("receive signal %d \n", sig_num);
    exit(EXIT_FAILURE);
}

void ctrlC_handler2(int sig_num){
    //printf("Use CTRL+D to terminate SEEsh\n");
}

int SEEsh_execute(char **args){
    int i;
    if(args[0]==NULL){
        return 1;
    }
    if(args[0][0]==0){
        printf("SEEsh exits by CTRL+D \n");
        //possible memory leak here.
        return 0;
    }
    for(i = 0; i < SEEsh_num_builtins();i++) {
        if (strcmp(args[0], builtin_str[i]) == 0){
            //printf("run builtin\n");
            return (*builtin_fun[i]) (args);
        }
    }
    
    return SEEsh_launch(args);
}



int SEEsh_launch(char ** args){
    pid_t pid;
    int status;
    
    pid = fork();
    if(pid==0){
        //this is child
        if(execvp(args[0], args)== -1){ //variant of exec, v- vector, p-give name for search
            perror("SEEsh");
        }
        
        exit(EXIT_FAILURE);
    } else if(pid<0){
        //forking error
        perror("SEEsh");
    } else{
        //this is parent process
        //signal(SIGINT, ctrlC_handler2);
        //kill(pid, SIGINT);
        //signal(SIGINT, ctrlC_handler2);
        do{
            waitpid(pid, &status, WUNTRACED);    //
        }while(!WIFEXITED(status)&& !WIFSIGNALED(status));
    }
    
    return 1;
}


void SEEsh_config(){
    char *buf = NULL;
    size_t size = 0;
    char *currPath = getcwd(buf,size);
    if(setenv("HOME", currPath, 1)!=0)  //Set HOME to current directory
        perror("SEEsh");
    FILE *fp;
    char *line = NULL;
    size_t buffersize = 0;      //ssize_t
    ssize_t read;
    char **args;
    fp = fopen(".SEEshrc", "r");
    if( fp==NULL){
        printf("Cannot open .SEEshrc\n");
        exit(EXIT_FAILURE);
    }
    while((read = getline(&line, &buffersize, fp))!=-1){
        printf("%s\n", line);
        args = SEEsh_split_line(line);
        SEEsh_execute(args);
    }
    if(line)
        free(line);
    free(buf);
    
}


char **SEEsh_split_line(char *line){
    int buffersize = SEEsh_TOK_BUFFERSIZE, position = 0;
    char** tokens = malloc(buffersize * sizeof(char*));
    char* token;
    if (!tokens){
        fprintf(stderr, "SEEsh: allocation error\n");
        exit (EXIT_FAILURE);
    }
    else if(line[0]== 0){
        char** ctrlD = malloc(buffersize * sizeof(char*));
        ctrlD[0] =  &line[0];
        return ctrlD;
    }
    token = strtok(line, SEEsh_TOK_DELIM);
    while (token!=NULL){
        tokens[position] = token;
        position ++;
        if(position >= buffersize){
            buffersize += SEEsh_TOK_BUFFERSIZE;
            tokens = realloc (tokens, buffersize*sizeof(char*));
            if(!tokens){
                fprintf(stderr, "SEEsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SEEsh_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

char *SEEsh_read_line(){
    char* line = NULL;
    size_t buffersize = 0; //getline allocate memory    ssize_t
    
    getline(&line, &buffersize, stdin);
    return line;
}

void SEEsh_loop(){
    char* line;
    char** args;
    int status = 1;
    do{
        signal(SIGINT, ctrlC_handler2);
        printf("? ");
        line = SEEsh_read_line();
        args = SEEsh_split_line(line);
        status = SEEsh_execute(args);
        
        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv){
    // Load configurations
    // .SEEshrc means run command
    printf("Welcome to SEEsh!\n");
    SEEsh_config();
    
    SEEsh_loop();
    
    // Perform any shutdown/cleanup
    
    return EXIT_SUCCESS;
}
