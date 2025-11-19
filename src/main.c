#include "all.h"

int pos=0;
Token final_tokens[MAX_TOKENS];

int main(){
    //getting the home directory (the folder our shell program is in)
    if (getcwd(home_dir, sizeof(home_dir)) == NULL) {
        perror("getcwd");
        return 1;
    }
        /* Put the shell into its own process group and take control of the terminal */
    pid_t shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        /* Non-fatal; do NOT print to the controlling terminal (pty) because
           expect/autograder will treat that as program output.
           If you need debugging output during development, set the environment
           variable DEBUG_SHELL=1 before running and it will print there. */
        if (getenv("DEBUG_SHELL")) {
            perror("setpgid");
        }
    }
    /* Prevent the shell itself from being stopped by background terminal I/O */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    /* Try to make the shell the foreground process group of the terminal */
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
        /* not fatal; ignore */
    }


    /* Now install signal handlers (these will be adjusted so handler only
       sets a flag; main loop will do the actual reaping/printing). */
    install_signal_handlers();

    
        const char *homeenv = getenv("HOME");
        if (homeenv && strlen(homeenv) < PATH_MAX - 32) {
            snprintf(LOG_FILE_PATH, sizeof(LOG_FILE_PATH), "%s/.modular_shell_history", homeenv);
        } else {
            /* fallback to current directory */
            strncpy(LOG_FILE_PATH, "shell_history.txt", sizeof(LOG_FILE_PATH));
            LOG_FILE_PATH[sizeof(LOG_FILE_PATH)-1] = '\0';
        }
    

    //main function loop
    char input[4096];
    while(1){
        //printing the shell prompt
        shell_prompt();
        
        //getting the input command
        if(fgets(input,sizeof(input),stdin)==NULL){
            // Kill all child processes before exiting
            for (int i = 0; i < MAX_JOBS; i++) {
                if (jobs[i].state != JOB_UNUSED) {
                    kill(-jobs[i].pgid, SIGKILL); // Note the '-' to signal the group
                }
            }
            printf("logout\n");
            exit(0);
        }

        // check_background_processes();
                /* If a SIGCHLD arrived, handle background completions now from the
           main thread (async-safe). The signal handler will only set the
           got_sigchld flag — it must NOT call check_background_processes(). */
        if (got_sigchld) {
            check_background_processes();
            got_sigchld = 0;
        }
        
        add_to_log(input);

        //splitting it into tokens
        char** tokens=tokenizer(input);

        //parsing it
        int n=0;
        for(int i=0;tokens[i]!=NULL;i++){
            final_tokens[n++]=classifier(tokens[i]);
        }
        final_tokens[n].type=eof;

        //valid vs invalid
        pos=0;
        if (shell_command_parser()==1 && final_tokens[pos].type == eof){
            // printf("Valid Syntax!\n");
            execute_tokens();
        } else {
            printf("Invalid Syntax!\n");
        }

        //freeing the tokens
        for(int i=0;tokens[i]!=NULL;i++){
            free(tokens[i]);
        }
        free(tokens);
    }
    return 0;
}