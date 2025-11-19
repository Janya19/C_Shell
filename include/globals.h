#ifndef GLOBAL_H
#define GLOBAL_H

#include <signal.h>

typedef enum{name, pipes, amp, semi_colon, redirect_in, redirect_out, append, invalid, eof} token_type;

typedef struct{
    token_type type;
    char* value;
}Token;


#define MAX_TOKENS 4096
extern Token final_tokens[MAX_TOKENS];

#define MAX_BUFFER 4096


#define MAX_LOG 15

extern char LOG_FILE_PATH[PATH_MAX+1];
#define LOG_FILE LOG_FILE_PATH


#define MAX_JOBS 1024

typedef enum {
    JOB_UNUSED = 0,
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} JobState;

typedef struct {
    int job_id;             
    pid_t pgid;             
    char cmd[MAX_BUFFER];   
    JobState state;         
} Job;


extern Job jobs[MAX_JOBS];
extern int next_job_id;     
extern pid_t fg_pgid;       


void add_job_entry_by_pgid(pid_t pgid, const char *cmd_name);
void remove_job_entry_by_pgid(pid_t pgid);
int find_job_index_by_pgid(pid_t pgid);


void add_job_entry(pid_t pid, const char *cmd_name); 
void remove_job_entry_by_pid(pid_t pid);
int find_job_index_by_pid(pid_t pid);


int find_job_index_by_jobid(int jobid);


void check_background_processes(void);

extern int pos;


extern char home_dir[PATH_MAX+1]; 
extern char prev_dir[PATH_MAX+1];
extern char curr_dir[PATH_MAX+1];
extern int tokenIndex;

extern volatile sig_atomic_t got_sigchld;

void shell_prompt();
char** tokenizer(char* input);
Token classifier(char* tok);
int atomic_parser();
int command_group_parser();
int shell_command_parser();

void install_signal_handlers(void);
void sigint_handler(int signo);
void sigtstp_handler(int signo);

void valid(int start, int end, int is_background);

void execute_tokens(void);

void exe_hop(int start, int end);
void exe_reveal(int start, int end);
void exe_log(int start, int end);
void exe_other(int start, int end, int is_background);
void exe_ping(int start, int end);

void add_to_log(const char* cmd);
void print_log();
void purge_log();
void execute_log_index(int index);

pid_t handle_pipes(int start, int end);


#endif
