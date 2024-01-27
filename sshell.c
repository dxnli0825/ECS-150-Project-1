#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define CMDARGU_MAX 16
#define PATHLEN_MAX 32
#define RUN_FAIL 256
#define ERROR 2

int check_cmd(char *cmd)
{
        char *nl;
        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO)) {            //isatty ->tests whether fildes is associated with a terminal device.
                printf("%s", cmd);
                fflush(stdout);
        }

        /* Remove trailing newline from command line */
        nl = strchr(cmd, '\n');         //strchr -> search \n in cmd
        if (nl)
                *nl = '\0';

        /* Builtin command */
        if (!strcmp(cmd, "exit")) {
                fprintf(stderr, "Bye...\n");
                fprintf(stderr, "+ completed 'exit' [0]\n");
                return 1;
        }

        return 0;
}

void remove_space(char *word)
{
        char *copy = word;
        while(*copy)
        {
                while (*copy == ' ')
                {
                        ++copy;
                }
                *word++ = *copy++;
        }
        *word = '\0';
}

int str_to_arr(char *cmdLine, char *argv[CMDARGU_MAX], char *key)
{
        int count = 0;
        char *token = strtok(cmdLine, key);
        while(token != NULL) 
        {
                argv[count] = token;
                count = count +1;
                token = strtok(NULL, key);
        }
        argv[count] = NULL;
        return count;
}

void file_redir(char *filename, int redir)
{
        int fd;
        remove_space(filename);
        
        if (redir ==1)
        {
                fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
        } else 
        {
                fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0644);
        }

        if (fd < 0)
        {
                fprintf(stderr, "Error: cannot write to the file\n");
                exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
}

void pipeline3(char *command1, char *command2, char *command3, char *original_cmd)
{
        pid_t p1, p2, p3;
        int fd1[2];
        int fd2[2];
        int status1, status2, status3;
        status1 = 0;
        status2 = 0;
        status3 = 0;
        char *argv[CMDARGU_MAX];
        pipe(fd1);
        pipe(fd2);

        p1 = fork();
        if(p1 < 0)
        {
                exit(1);
        }
        if (p1 == 0) { 
                str_to_arr(command1, argv, " ");

                dup2(fd1[1], STDOUT_FILENO); 
                
                close(fd2[0]);
                close(fd2[1]);
                close(fd1[0]);
                close(fd1[1]);

                execvp(argv[0], argv);
                fprintf(stderr, "Error: command not found\n");
                status1 =1;
                exit(1);
        }

        p2 = fork();
        if(p2 < 0)
        {
                exit(1);
        }
        if (p2 == 0) { // Child #2 
                str_to_arr(command2, argv, " ");
                
                dup2(fd1[0], STDIN_FILENO); // Replace stdin with pipe 
                dup2(fd2[1], STDOUT_FILENO);

                close(fd1[1]);
                close(fd2[0]);
                close(fd2[1]);
                close(fd1[0]); // Close now unused FD 
                execvp(argv[0], argv); // Child #2 becomes command2 
                fprintf(stderr, "Error: command not found\n");
                status2 = 1;
                exit(1);
        }

        p3 = fork();
        if(p3 < 0)
        {
                exit(1);
        }
        if (p3 == 0) { // Child #3 
                str_to_arr(command3, argv, " ");

                dup2(fd2[0], STDIN_FILENO); // Replace stdin with pipe 

                close(fd1[1]); // No need for write access 
                close(fd1[0]);
                close(fd2[1]);
                close(fd2[0]); // Close now unused FD 

                execvp(argv[0], argv); // Child #2 becomes command2 
                fprintf(stderr, "Error: command not found\n");
                status3 = 1;
                exit(1);
        }

        close(fd1[0]); // Pipe no longer needed in parent 
        close(fd1[1]);
        close(fd2[0]);
        close(fd2[1]);
        waitpid(p1, NULL, 0); // Parent waits for two children 
        waitpid(p2, NULL, 0);
        waitpid(p3, NULL, 0);

        fprintf(stderr, "+ completed '%s' [%d] [%d] [%d]\n", original_cmd, status1, status2, status3);
}

void pipeline2(char *command1, char *command2, char *original_cmd)
{
        pid_t p1, p2;
        int fd[2];
        int status1, status2;
        status1 = 0;
        status2 = 0;
        char *argv[CMDARGU_MAX];
        pipe(fd);

        p1 = fork();
        if(p1 < 0)
        {
                exit(1);
        }
        if (!(p1)) { // Child #1 
                str_to_arr(command1, argv, " ");
                close(fd[0]); // No need for read access
                dup2(fd[1], STDOUT_FILENO); // Replace stdout with pipe
                close(fd[1]); // Close now unused FD
                execvp(argv[0], argv); // Child #1 becomes command1
                fprintf(stderr, "Error: command not found\n");
                status1 =1;
                exit(1);
        }

        p2 = fork();
        if(p2 < 0)
        {
                exit(1);
        }
        if (!(p2)) { 
                str_to_arr(command2, argv, " ");
                close(fd[1]); 
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]); 
                execvp(argv[0], argv);
                fprintf(stderr, "Error: command not found\n");
                status2 = 1;
                exit(1);
        }

        close(fd[0]); 
        close(fd[1]);
        waitpid(p1, NULL, 0);
        waitpid(p2, NULL, 0);

        fprintf(stderr, "+ completed '%s' [%d] [%d]\n", original_cmd, status1, status2);
}

int cd_function(char *argv)
{
        int status = 0;
        char cwd[PATHLEN_MAX];

        DIR *dir = opendir(argv);
        if ( dir == NULL && !strstr(argv, ".."))
        {
                fprintf(stderr, "Error: cannot cd into directory\n");
                status = RUN_FAIL;
        } else
        {
                chdir(argv);
                getcwd(cwd, PATHLEN_MAX);
        }
        return status;
}

void piping(char *cmd)
{
        char *argv[CMDARGU_MAX];
        int pipe_count = str_to_arr(cmd, argv, "|");
        if (pipe_count == 1)
        {
                fprintf(stderr, "Error: missing command\n");
        } else if(pipe_count == 2)
        {
                pipeline2(argv[0], argv[1], cmd);
        } else
        {
                pipeline3(argv[0], argv[1], argv[2], cmd);
        }
}

void lenth_check(int len)
{
        if(len >= 17)
        {       //check the numbr of argument number 
                fprintf(stderr, "Error: too many process arguments\n");
                exit(ERROR);
        }
}

void error_checking(int argument_len, char *filename)
{
        if( argument_len == 1)
        {
                fprintf(stderr, "Error: missing command\n");
                exit(ERROR);        //missing executable command
        }
                                
        if(filename == NULL)
        {
                fprintf(stderr, "Error: no output file\n");
                 exit(ERROR);        //missing output file
        }

        if(strchr(filename, '|') != NULL)
        {
                fprintf(stderr, "Error: mislocated output redirection\n");
                exit(ERROR);
        }
}

int check_redirection(char *token, int redir_to, char *cmd)
{
        if (token == NULL)
        {
                fprintf(stderr, "Error: missing command\n");
                exit(ERROR);
        } else if(strstr(cmd, ">>"))
        {
                //append to file
                redir_to = 2;
        } else
        {
                //create new file
                redir_to = 1;
        }
        return redir_to;
}

void sls_command() {
        DIR *directory;
        struct dirent *dir_entry;

        if ((directory = opendir(".")) != NULL) {
                while ((dir_entry = readdir(directory)) != NULL) 
                {
                        char filename[259];
                        struct stat sstat;

                        if (!strcmp(dir_entry->d_name, ".") || !strcmp(dir_entry->d_name, ".."))
                        {
                                continue;
                        }
                        
                        // Get the full path of the file
                        snprintf(filename, sizeof(filename), "%s/%s", ".", dir_entry->d_name);

                        // Get file information
                        if (stat(filename, &sstat) == 0) 
                        {
                                printf("%s (%ld bytes)\n", dir_entry->d_name, (long)sstat.st_size);
                        }
                }
                closedir(directory);
        } else {
                perror("Error: cannot open directory");
        }
}


int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                /* Print prompt */
                printf("sshell@ucd$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);
                
                if (check_cmd(cmd))
                        break;

                char *token;
                char *argv[CMDARGU_MAX];
                char *original_cmd = strdup(cmd);
                int redir = 0;
                int use_pipe = 0;
                int len = 0;
                int is_sls = 0;
                int status;

                token = strtok(cmd, ">");
                if (strcmp(token, original_cmd))
                {
                        //command contains redirection
                        redir = check_redirection(token, redir, original_cmd);

                } else if (strcmp(strtok(cmd, "|"), original_cmd))
                {
                        use_pipe = 1;
                } else if (strstr(original_cmd, "sls"))
                {
                        sls_command();
                        is_sls = 1;
                } else
                {
                        len = str_to_arr(cmd, argv, " ");
                }

                pid_t pid = fork();
                if (pid == 0) {
                        /* Child */
                        lenth_check(len);

                        if (use_pipe == 1 || is_sls ==1)
                        {
                                exit(0);        //run pipe in different child
                        } else if (redir >= 1) 
                        {       //rediecate from stdout to file
                                char* cmd_pointer = token;      //command you want to execute
                                char* filename = strtok(NULL, ">");
                                int argu_len = str_to_arr(cmd_pointer, argv, " ");

                                error_checking(argu_len, filename);
                                file_redir(filename, redir);

                        } else if(!strcmp(argv[0], "pwd") || (!strcmp(argv[0], "cd")))
                        {       //run cd and pwd on parent
                                exit(0);
                        }
                        execvp(argv[0], argv);
                        fprintf(stderr, "Error: command not found\n");
                        exit(ERROR);

                } else if (pid > 0) {
                        /* Parent */ 
                        wait(&status);

                        if(use_pipe == 1)
                        {
                                piping(original_cmd);
                                status = ERROR;
                        } else if(!strcmp(argv[0], "pwd") )
                        {
                                char cwd[PATHLEN_MAX];
                                getcwd(cwd, PATHLEN_MAX);
                                fprintf(stdout, "%s \n", cwd);
                        } else if (!strcmp(argv[0], "cd") && argv[1] != NULL)
                        {
                                printf("argv 1 = %s\n", argv[1]);
                                status = cd_function(argv[1]);
                        }

                        if (WIFEXITED(status) && (WEXITSTATUS(status) != ERROR) && use_pipe == 0) 
                        {
                                fprintf(stderr, "+ completed '%s' [%d]\n", original_cmd, WEXITSTATUS(status));
                        }
                } else {
                        perror("fork");
                        exit(1);
                }
        }
        return EXIT_SUCCESS;
}