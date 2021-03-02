#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

typedef int BOOL;
#define FALSE 0
#define TRUE 1

// mode flags
typedef int mysh_mode_t;
#define PROMT_MODE 0
#define BATCH_MODE 1

// pipe fd index
#define RD_END 0
#define WR_END 1

BOOL getcmd(char s[], int size);
BOOL built_in_cmd(int argc, char *argv[], int bg_pid[], int *p_bg_cnt);
int str_contains(char s[], char c);
void print(const char message[]);
void printErr(void);

int main(int argc, char *argv[]) {
    const int maxlen = 512;
    mysh_mode_t mysh_mode;
    char cmd[maxlen + 2];           // including "\n\0"
    BOOL redir_flag;
    char *redir_path;
    int cmd_cnt;                    // number of piped cmds
    int pipefd[maxlen >> 1][2];     // pipe file discriptor
    int pipe_pid[maxlen >> 1];      // store pid of pipe cmds
    int myargc[maxlen >> 1];        // argc of each piped cmd
    // linked list is better, but I use arrays here for simplicity.
    // in the ith pipe: myargc[i] <= maxlen / 2 (a b c ...)
    char *myargv[maxlen >> 1][maxlen >> 1];
    // myargv[i][j] means the jth arg in the ith cmd of the pipe
    int rc_fork;
    BOOL bg_flag;
    int bg_cnt = 0;                 // count background jobs
    int bg_pid[maxlen >> 1];        // store pid of bg jobs
    BOOL err_in_loop;

    // mode select
    if (argc == 1) mysh_mode = PROMT_MODE;
    else if (argc == 2) {
        mysh_mode = BATCH_MODE;
        int fd = open(argv[1], O_RDONLY);
        if (fd < 0 || dup2(fd, STDIN_FILENO) < 0) {
            print("An error has occurred\n");
            printErr();
            exit(1);
        }
    } else {
        print("An error has occurred\n");
        printErr();
        exit(1);
    }

    // main loop
    while (1) {
        // prompt and user input
        if (mysh_mode == PROMT_MODE) print("mysh> ");
        if (!getcmd(cmd, maxlen + 2)) continue;
        if (mysh_mode == BATCH_MODE) print(cmd);

        // detect background jobs
        bg_flag = FALSE;
        for (int i = strlen(cmd) - 1; i >= 0; --i) {
            if (cmd[i] == ' ' || cmd[i] == '\t' || cmd[i] == '\n')
                continue;
            if (cmd[i] == '&') {
                bg_flag = TRUE;
                cmd[i] = '\n';      // should not be '\0'
                break;
            }
            break;
        }

        // detect redirection
        redir_flag = FALSE;
        if (str_contains(cmd, '>')) {
            redir_flag = TRUE;
            if (cmd[0] == '>' || cmd[strlen(cmd) - 1] == '>') {
                // no cmd specified: > foo
                // no file specified: cmd >(EOF) or cmd > foo >(EOF)
                printErr();
                continue;
            }
            strtok(cmd, ">");
            redir_path = strtok(NULL, ">");
            // if (redir_path == NULL) {
            //     // no file specified: cmd >(EOF)
            //     printErr();
            //     continue;
            // }
            if (redir_path - cmd > strlen(cmd) + 1) {
                // mltiple redir: cmd >> foo
                // works in real shell to append output
                // not implemented in this project
                printErr();
                continue;
            }
            if (strtok(NULL, ">") != NULL) {
                // multiple redir: cmd > foo > foo
                // works in real shell to redir output to multiple files
                // not implemented in this project
                printErr();
                continue;
            }
            // tokenize path (should contain only one token)
            redir_path = strtok(redir_path, " \t\n");
            if (redir_path == NULL) {
                // no file specified: cmd > (whitespace)
                printErr();
                continue;
            }
            if (strtok(NULL, " \t\n") != NULL) {
                // multiple file specified: cmd > foo1 foo2
                // works in real shell where foo2 is ignored (maybe?)
                // not implemented in this project
                printErr();
                continue;
            }
    
        }

        // detect pipe
        cmd_cnt = 0;
        myargv[cmd_cnt][myargc[cmd_cnt] = 0] = strtok(cmd, "|");
        while (myargv[cmd_cnt][myargc[cmd_cnt]] != NULL) {
            ++cmd_cnt;
            myargv[cmd_cnt][myargc[cmd_cnt] = 0] = strtok(NULL, "|");
        }
        if (!cmd_cnt) {
            // weird, should never happen: cmd == |||||||||||(EOF)
            printErr();
            continue;
        }
        if (myargv[0][0] != cmd) {
            // begin with |: |cmd
            printErr();
            continue;
        }
        err_in_loop = FALSE;
        for (int cmd_idx = 1; cmd_idx < cmd_cnt; ++cmd_idx) {
            if (myargv[cmd_idx][0] - myargv[cmd_idx - 1][0] >
                strlen(myargv[cmd_idx - 1][0]) + 1) {
                    // multiple |: cmd1 || cmd2
                    err_in_loop = TRUE;
                    break;
                }
        }
        if (err_in_loop) {
            printErr();
            continue;
        }

        // tokenize
        for (int cmd_idx = 0; cmd_idx < cmd_cnt; ++cmd_idx) {
            myargv[cmd_idx][0] = strtok(myargv[cmd_idx][0], " \t\n");
            while (myargv[cmd_idx][myargc[cmd_idx]] != NULL) {
                myargv[cmd_idx][++myargc[cmd_idx]] = strtok(NULL, " \t\n");
            }
            // check whether tokenize runs well:
            // printf("argc[%d]: %d\n", cmd_idx, myargc[cmd_idx]);
            // for (int i = 0; i < myargc[cmd_idx]; ++i) 
            //     printf("argv[%d][%d] = %s\n", cmd_idx, i, myargv[cmd_idx][i]);
        }
        // printf("cmd_cnt: %d\n", cmd_cnt);

        err_in_loop = FALSE;
        // handle commands
        for (int cmd_idx = 0; cmd_idx < cmd_cnt; ++cmd_idx) {
            if (!myargc[cmd_idx]) {
                if (cmd_cnt == 1) continue;         // empty line
                // reach here if the ith cmd of pipe is empty
                err_in_loop = TRUE;
                break;
            }
            if (built_in_cmd(myargc[cmd_idx], myargv[cmd_idx], bg_pid, &bg_cnt)) {
                if (cmd_cnt == 1) {
                    if (bg_flag) {
                        // background of built in command is not supported
                        err_in_loop = TRUE;
                        break;
                    }
                    // correctly handled built in cmd
                    // set cmd_cnt = 0 to avoid wait
                    cmd_cnt = 0;
                    // break for loop which runs only once
                    break;
                }
                // reach here if find built in cmd in pipe
                err_in_loop = TRUE;
                break;
            }
            // when enter a loop, only the read end of prev pipe is opened
            if (cmd_idx < cmd_cnt - 1) {
                // not the last cmd of the pipe, need new pipe
                if (pipe(pipefd[cmd_idx]) < 0) {
                    // pipe creation failed
                    err_in_loop = TRUE;
                    break;
                }
            }
            rc_fork = fork();
            if (rc_fork < 0) {
                // fork failed
                err_in_loop = TRUE;
                break;
            } else if (rc_fork == 0) {
                // child
                // handle redirection and pipe
                if (cmd_idx == 0 && cmd_cnt != 1) {
                    close(pipefd[cmd_idx][RD_END]);     // close unused read end
                    // pipe output
                    if (dup2(pipefd[cmd_idx][WR_END], STDOUT_FILENO) < 0) {
                        printErr();
                        exit(1);
                    }
                }
                if (0 < cmd_idx && cmd_idx < cmd_cnt - 1) {
                    close(pipefd[cmd_idx][RD_END]);     // close unused read end
                    // need pipe in and out
                    if (dup2(pipefd[cmd_idx][WR_END], STDOUT_FILENO) < 0) {
                        printErr();
                        exit(1);
                    }
                    if (dup2(pipefd[cmd_idx - 1][RD_END], STDIN_FILENO) < 0) {
                        printErr();
                        exit(1);
                    }
                }
                if (cmd_idx == cmd_cnt - 1) {
                    if (redir_flag) {
                        int fd = open(redir_path, 
                                    O_CREAT|O_WRONLY|O_TRUNC,
                                    S_IRUSR | S_IWUSR);
                        if (fd < 0 || dup2(fd, STDOUT_FILENO) < 0) {
                            // open error or dup error 
                            // will not execute dup if open err occurs
                            printErr();
                            exit(1);
                        }
                    }
                    if (cmd_cnt != 1) {
                        if (dup2(pipefd[cmd_idx - 1][RD_END], STDIN_FILENO) < 0) {
                            printErr();
                            exit(1);
                        }
                    }
                }
                execvp(myargv[cmd_idx][0], myargv[cmd_idx]);
                // reach here if command does not exist or cannot be executed
                printErr();
                exit(1);
            } else {
                // parent
                pipe_pid[cmd_idx] = rc_fork;
                // close outdated pipes
                if (cmd_cnt != 1 && cmd_idx < cmd_cnt - 1)
                    close(pipefd[cmd_idx][WR_END]);
                if (cmd_idx > 0)
                    close(pipefd[cmd_cnt - 1][RD_END]);
            }
        }
        if (err_in_loop) {
            printErr();
            continue;
        }
        if (bg_flag) for (int cmd_idx = 0; cmd_idx < cmd_cnt; ++cmd_idx) {
            // background job: store pid, no wait
            bg_pid[bg_cnt++] = pipe_pid[cmd_idx];
        } else for (int cmd_idx = 0; cmd_idx < cmd_cnt; ++cmd_idx) {
            // not background: wait every pid in pipe (or single cmd)
            if (waitpid(pipe_pid[cmd_idx], NULL, 0) < 0)
                printErr();
        }
    }
    return 0;
}

BOOL getcmd(char s[], int size) {
    // read a command until '\n'
    // return TRUE if succeed
    // return FALSE if exceed max length
    // exit on EOF
    BOOL rc = TRUE;
    if (fgets(s, size, stdin) == NULL) {        // EOF
        exit(0);
    }
    while (strlen(s) == size - 1 && s[size - 2] != '\n') {
        // exceed maxlen, consume extra characters
        if (rc) {       // print error message once
            printErr();
            rc = FALSE;
        } 
        fgets(s, size, stdin);
    }
    return rc;
}


BOOL built_in_cmd(int argc, char *argv[], int bg_pid[], int *p_bg_cnt) {
    // check and handle built-in commands
    // return TRUE if argv is a built-in command
    // return FALSE otherwise.
    if (strcmp(argv[0], "exit") == 0) {
        if (argc == 1) exit(0);
        else printErr();
        return TRUE;
    }
    if (strcmp(argv[0], "cd") == 0) {
        if (argc == 1) {
            if (chdir(getenv("HOME")) == -1)
                printErr();
        } else if (argc == 2) {
            if (chdir(argv[1]) == -1)
                printErr();
        } else printErr();
        return TRUE;
    }
    if (strcmp(argv[0], "pwd") == 0) {
        if (argc == 1) {
            char *cwd = getcwd(NULL, 0);
            if (cwd != NULL) {
                print(cwd);
                print("\n");
                free(cwd);
            } else printErr();
        } else printErr();
        return TRUE;
    }
    if (strcmp(argv[0], "wait") == 0) {
        if (argc == 1) {
            for (int i = 0; i < *p_bg_cnt; ++i) {
                if (waitpid(bg_pid[i], NULL, 0) < 0)
                    printErr();
            }
            // printf("wait: %d\n", *p_bg_cnt);
            *p_bg_cnt = 0;
        } else printErr();
        return TRUE;
    }
    return FALSE;
}

BOOL str_contains(char s[], char c) {
    int len = strlen(s);
    for (int i = 0; i < len; ++i)
        if (s[i] == c)
            return TRUE;
    return FALSE;
}

void print(const char message[]) {
    // a simple interface for printing message
    write(STDOUT_FILENO, message, strlen(message));
}

void printErr() {
    // a simple interface for printing error
    static const char error_message[] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}