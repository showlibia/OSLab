#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

void shell_loop();
char* shell_read_line();
char** shell_split_line(char* line);
int shell_execute(char** args);
int shell_cd(char** args);
int shell_help(char** args);
int shell_exit(char** args);
int shell_history(char** args);

#define MAX_LINE 80
#define MAX_HISTORY 10

char* history[MAX_LINE];
int history_count = 0;

char* builtin_str[] = {
    "cd",
    "help",
    "exit",
    "history",
};

int main()
{
    printf("Please enter \"help\" to get the command\n");

    shell_loop();
    
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return EXIT_SUCCESS;
}

void shell_loop()
{
    char* line;
    char** args;
    int status;

    do {
        printf("osh> ");
        line = shell_read_line();

        if (history_count < MAX_HISTORY) {
            if (strcmp(line, "\n") != 0) {
                history[history_count++] = strdup(line);
            }
        } else {
            for (int i = 0;i < MAX_HISTORY; i++) {
                history[i] = history[i + 1];
            }
            history[MAX_HISTORY] = strdup(line);
        }

        args = shell_split_line(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while (status);
}

#define READLINE_BUFSIZE 1024

char* shell_read_line()
{
    size_t buffsize = READLINE_BUFSIZE;
    char* buff = NULL;
    int read;

    read = getline(&buff, &buffsize, stdin);
    if (read == -1) {
        if (feof(stdin)) {
            exit(EXIT_SUCCESS); // We recieved an EOF
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    if (strcmp(buff, "!!\n") == 0) {
        if (history_count > 0) {
            return strdup(history[history_count - 1]);
        } else {
            printf("No commands in history.\n");
            return NULL;
        }
    } else if (buff[0] == '!' && isdigit(buff[1])) {
        int num = atoi(&buff[1]);
        if (num > 0 && num <= history_count) {
            return strdup(history[num - 1]);
        } else {
            printf("No such command in history.\n");
            return NULL;
        }
    }
    return buff;
}

#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

char** shell_split_line(char* line)
{
    size_t lineSize = strlen(line);
    char** toks = (char**)malloc(sizeof(char*) * lineSize);
    char* token;
    int counter = 0;

    if (!toks) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SHELL_TOK_DELIM);
    while (token != NULL) {
        toks[counter++] = strdup(token); // strdup:copy the token into toks[counter]
        token = strtok(NULL, SHELL_TOK_DELIM); // get next token;
    }
    toks[counter] = token;
    return toks;
}

int shell_launch(char** args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("shell");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("shell");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int (*builtin_func[])(char**) = {
    &shell_cd,
    &shell_help,
    &shell_exit,
    &shell_history
};

int shell_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char*);
}

int shell_cd(char** args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "shell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int shell_help(char** args)
{
    int i;
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < shell_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int shell_exit(char** args)
{
    return 0;
}

int shell_history(char** args)
{    
    for (int i = history_count - 1; i >= history_count - 10 && i >= 0; i--) {
        printf("%d %s", i + 1, history[i]);
    }
    return 1;
}

int shell_execute(char** args)
{
    int i;
    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < shell_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        } 
    }

    return shell_launch(args);
}
