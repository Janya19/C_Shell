#include "all.h"

volatile sig_atomic_t got_sigchld = 0;
pid_t fg_pgid = -1; //stores the pgid of current foreground job, -1 means "no foreground job right now"

/* SIGCHLD handler: reap children and use the reaper routine. */
void sigchld_handler(int signo) {
    (void)signo;
    /* call the reaper that uses waitpid(WNOHANG) and prints completion messages */
    got_sigchld = 1; // tell main loop that a child has changed state
}

//ctrl+c
void sigint_handler(int signo){
    if(fg_pgid > 0){
        kill(-fg_pgid, SIGINT);
        // kill() with a negative pid sends the signal to the process group
        // whose PGID is |fg_pgid|. That way, every process in the foreground
        // pipeline (cmd1 | cmd2 | ...) gets SIGINT.
    }
}

//ctrl+z - Fixed to not print generic message
void sigtstp_handler(int signo){
    if(fg_pgid > 0){
        kill(-fg_pgid, SIGTSTP);
        // Don't print anything here - let the main shell handle job notification
        // The job control logic in execution.c will print the proper format
    }
}

void install_signal_handlers(void){
    struct sigaction sa_int, sa_tstp, sa_ch; // action descriptions
    
    /* --- Ignore terminal background I/O signals in the shell process --- */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    // --- SIGINT (Ctrl-C) handler setup ---
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);

    // --- SIGTSTP (Ctrl-Z) handler setup ---
    sa_tstp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    // sa_tstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_tstp, NULL);

    // --- SIGCHLD handler: reap children when they exit ---
    sa_ch.sa_handler = sigchld_handler;
    sigemptyset(&sa_ch.sa_mask);
    sa_ch.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa_ch, NULL);

    // Ignore SIGQUIT (Ctrl-\) so the shell itself doesn't quit from it.
    struct sigaction sa_quit = {0};
    sa_quit.sa_handler = SIG_IGN;
    sigaction(SIGQUIT, &sa_quit, NULL);
}