#include "all.h"
#include <errno.h>

int apply_redirections_and_save(int start, int end, int *saved_stdin, int *saved_stdout);
void restore_redirections(int saved_stdin, int saved_stdout);
void exe_activities(void);
void exe_bg(int start, int end);
void exe_fg(int start, int end);
int is_builtin(const char *cmd);
void run_builtin(char **argv, int start, int end);
static void build_cmd_string(int start, int end, char *buf, size_t buflen, int append_amp);



/* Build argv array excluding redirection and pipe tokens */
char **build_argv_filtered(int start, int end, int *argc_out) {
    int argc_max = end - start + 2;
    char **argv = malloc(sizeof(char*) * argc_max);
    if (!argv) return NULL;

    int argc = 0;
    for (int i = start; i <= end; i++) {
        char *tok = final_tokens[i].value;
        if (!tok) continue;

        // skip redirection operators + their filenames
        if (strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0 || strcmp(tok, ">>") == 0) {
            if (i + 1 <= end) i++; // skip filename too, but check bounds
            continue;
        }

        // skip pipes (handled outside)
        if (strcmp(tok, "|") == 0) continue;

        argv[argc++] = tok;
    }
    argv[argc] = NULL;
    if (argc_out) *argc_out = argc;
    return argv;
}

int is_builtin(const char *cmd) {
    if (!cmd) return 0;
    return (strcmp(cmd, "hop") == 0) ||
           (strcmp(cmd, "reveal") == 0) ||
           (strcmp(cmd, "log") == 0) ||
           (strcmp(cmd, "activities") == 0) ||
           (strcmp(cmd, "bg") == 0) ||
           (strcmp(cmd, "fg") == 0) ||
           (strcmp(cmd, "echo") == 0);
}

/* Central builtin dispatcher used by handle_pipes and other places.
 *
 * argv: usual argv array built with build_argv_filtered for the segment (argv[0] is command)
 * start, end: indices into final_tokens for the token-range of this command segment
 *
 * This function runs the builtin and prints/writes to stdout/stderr as appropriate.
 * It DOES NOT exit the process — the caller (child in pipeline) typically will _exit().
 */
void run_builtin(char **argv, int start, int end) {
    if (!argv || !argv[0]) return;

    /* echo: print joined arguments separated by spaces, preserving tokens as-is */
    if (strcmp(argv[0], "echo") == 0) {
        for (int i = 1; argv[i]; i++) {
            if (i > 1) putchar(' ');
            fputs(argv[i], stdout);
        }
        putchar('\n');
        fflush(stdout);
        return;
    }

    /* For other builtins, use the token-index based handlers */
    if (strcmp(argv[0], "hop") == 0) {
        exe_hop(start, end);
        return;
    }
    if (strcmp(argv[0], "reveal") == 0) {
        exe_reveal(start, end);
        return;
    }
    if (strcmp(argv[0], "log") == 0) {
        exe_log(start, end);
        return;
    }
    if (strcmp(argv[0], "activities") == 0) {
        exe_activities();
        return;
    }
    if (strcmp(argv[0], "bg") == 0) {
        exe_bg(start, end);
        return;
    }
    if (strcmp(argv[0], "fg") == 0) {
        exe_fg(start, end);
        return;
    }
    if (strcmp(argv[0], "ping") == 0) { // Added this block
        exe_ping(start, end);
        return;
    }
    /* Unknown builtin */
    fprintf(stderr, "Builtin not implemented: %s\n", argv[0]);
    fflush(stderr);
}



// void handle_pipes(int start, int end, int is_background) {
//     int cmd_start = start;
//     int pipefd[2];
//     int prev_fd = -1;

//     for (int i = start; i <= end; i++) {
//         if (final_tokens[i].type == pipes || i == end) {
//             int cmd_end = (final_tokens[i].type == pipes) ? i - 1 : i;

//             // if (final_tokens[i].type != eof && final_tokens[i].type != pipes)
                

//             if (final_tokens[i].type != eof) {
//                 if (pipe(pipefd) < 0) {
//                     perror("pipe");
//                     return;
//                 }
//             }

//             pid_t pid = fork();
//             if (pid < 0) { perror("fork"); return; }
//             else if (pid == 0) {
//                 /* CHILD */
//                 if (prev_fd != -1) {
//                     dup2(prev_fd, STDIN_FILENO);
//                     close(prev_fd);
//                 }

//                 if (final_tokens[i].type == pipes) {
//                     dup2(pipefd[1], STDOUT_FILENO);
//                     close(pipefd[0]);
//                     close(pipefd[1]);
//                 }

//                 int s_in=-1, s_out=-1;
//                 if (apply_redirections_and_save(cmd_start, cmd_end, &s_in, &s_out) < 0) _exit(1);

//                 // char *argv[cmd_end - cmd_start + 2];
//                 // int argc = 0;
//                 // for (int j = cmd_start; j <= cmd_end; j++) argv[argc++] = final_tokens[j].value;
//                 // argv[argc] = NULL;

//                 // execvp(argv[0], argv);
//                 int argc;
//                 char **argv = build_argv_filtered(cmd_start, cmd_end, &argc);
//                 if (!argv || argc == 0) _exit(1);

//                 execvp(argv[0], argv);
//                 perror("Command not found!");
//                 free(argv);
//                 _exit(127);

//                 // perror("Command not found!");
//                 // _exit(127);
//             } else {
//                 /* PARENT */
//                 if (prev_fd != -1) close(prev_fd);
//                 if (final_tokens[i].type == pipes) {
//                     close(pipefd[1]);
//                     prev_fd = pipefd[0];
//                 }
//             }

//             cmd_start = i + 1;
//         }
//     }

//     /* wait for all children */
//     while (wait(NULL) > 0);
// }

// Helper to extract argv segment for a pipe
// static char **build_argv_segment(char **argv, int start, int end) {
//     int len = end - start;
//     char **seg = malloc((len + 1) * sizeof(char *));
//     if (!seg) { perror("malloc"); exit(1); }
//     for (int i = 0; i < len; i++) seg[i] = argv[start + i];
//     seg[len] = NULL;
//     return seg;
// }

/* Replace with this implementation (uses final_tokens indices) */
// NEW handle_pipes function
// This function is now run by the MAIN shell process.
// It forks all children for a pipeline and returns the Process Group ID (PGID) of the job.
pid_t handle_pipes(int start, int end) {
    int cmd_starts[128], cmd_ends[128], num_cmds = 0;
    int cur = start;
    for (int i = start; i <= end; ++i) {
        if (final_tokens[i].type == pipes) {
            cmd_starts[num_cmds] = cur;
            cmd_ends[num_cmds] = i - 1;
            num_cmds++;
            cur = i + 1;
        }
    }
    cmd_starts[num_cmds] = cur;
    cmd_ends[num_cmds] = end;
    num_cmds++;

    if (num_cmds == 0) return -1;

    int pipes_array[num_cmds > 1 ? (num_cmds - 1) : 1][2];
    for (int i = 0; i < num_cmds - 1; ++i) {
        if (pipe(pipes_array[i]) < 0) {
            perror("pipe");
            return -1;
        }
    }

    // pid_t pids[num_cmds];
    pid_t pgid = -1;

    for (int s = 0; s < num_cmds; ++s) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return -1;
        }

        if (pid == 0) { // --- CHILD ---
            // Set the process group. The first child becomes the group leader.
            if (s == 0) {
                setpgid(0, 0);
            } else {
                setpgid(0, pgid);
            }
            
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (s > 0) {
                dup2(pipes_array[s - 1][0], STDIN_FILENO);
            }
            if (s < num_cmds - 1) {
                dup2(pipes_array[s][1], STDOUT_FILENO);
            }

            for (int j = 0; j < num_cmds - 1; ++j) {
                close(pipes_array[j][0]);
                close(pipes_array[j][1]);
            }

            int saved_in, saved_out;
            apply_redirections_and_save(cmd_starts[s], cmd_ends[s], &saved_in, &saved_out);
            
            int argc;
            char **argv = build_argv_filtered(cmd_starts[s], cmd_ends[s], &argc);
            if (is_builtin(argv[0])) {
                run_builtin(argv, cmd_starts[s], cmd_ends[s]);
                _exit(0);
            } else {
                execvp(argv[0], argv);
                fprintf(stderr, "Command not found!\n");
                _exit(127);
            }
        } else { // --- PARENT ---
            // pids[s] = pid;
            if (s == 0) {
                pgid = pid; // First child's PID becomes the PGID for the whole job
            }
            setpgid(pid, pgid);
        }
    }

    // Parent closes all pipe ends
    for (int i = 0; i < num_cmds - 1; ++i) {
        close(pipes_array[i][0]);
        close(pipes_array[i][1]);
    }

    return pgid; // Return the process group ID of the pipeline job
}

Job jobs[MAX_JOBS];
int next_job_id = 1;

/* comparator used by qsort for exe_activities */
static int jobs_cmp_idx(const void *a, const void *b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return strcmp(jobs[ia].cmd, jobs[ib].cmd);
}


void add_job_entry(pid_t pgid, const char *cmd_name) {
    for (int i = 0; i < MAX_JOBS; ++i) {
        /* consider a slot free when state == JOB_UNUSED */
        if (jobs[i].state == JOB_UNUSED) {
            jobs[i].job_id = next_job_id++;
            jobs[i].pgid = pgid;
            strncpy(jobs[i].cmd, cmd_name, MAX_BUFFER - 1);
            jobs[i].cmd[MAX_BUFFER - 1] = '\0';
            jobs[i].state = JOB_RUNNING;
            return;
        }
    }
    /* fallback: overwrite index 0 if table is full */
    jobs[0].job_id = next_job_id++;
    jobs[0].pgid = pgid;
    strncpy(jobs[0].cmd, cmd_name, MAX_BUFFER - 1);
    jobs[0].cmd[MAX_BUFFER - 1] = '\0';
    jobs[0].state = JOB_RUNNING;
}



/* Build a single space-separated command string from final_tokens[start..end].
 * If append_amp is non-zero we append " &" to the end (useful for background jobs).
 * buf must be at least MAX_BUFFER bytes.
 */
static void build_cmd_string(int start, int end, char *buf, size_t buflen, int append_amp) {
    buf[0] = '\0';
    size_t used = 0;
    for (int i = start; i <= end; ++i) {
        if (!final_tokens[i].value) continue;
        /* skip special separators if any (but tokenization should avoid putting ; & as part of range) */
        size_t need = strlen(final_tokens[i].value);
        if (used + need + 2 >= buflen) break; /* avoid overflow */
        if (used > 0) {
            strncat(buf, " ", buflen - used - 1);
            used += 1;
        }
        strncat(buf, final_tokens[i].value, buflen - used - 1);
        used += need;
    }
    if (append_amp) {
        if (used + 3 < buflen) {
            strncat(buf, " &", buflen - used - 1);
        }
    }
}


void remove_job_entry_by_pid(pid_t pgid) {
    for (int i = 0; i < MAX_JOBS; ++i) {
        if (jobs[i].pgid == pgid) {
            jobs[i].pgid = 0;
            jobs[i].cmd[0] = '\0';
            jobs[i].state = JOB_UNUSED;   /* mark slot free */
            jobs[i].job_id = 0;           /* optional reset */
            return;
        }
    }
}



int find_job_index_by_pid(pid_t pgid) {
    for (int i = 0; i < MAX_JOBS; ++i) {
        if (jobs[i].pgid == pgid && jobs[i].state != JOB_UNUSED) {
            return i;
        }
    }
    return -1;  /* not found */
}



/* reaper: call from main after input (and optionally from execute_tokens) */
void check_background_processes(void) {
    int status;
    pid_t pid;

    /* Watch for exits, stops and continues */
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        int idx = find_job_index_by_pid(pid);
        if (idx == -1) continue;

        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            /* process ended */
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
                fprintf(stderr, "%s with pid %d exited normally\n", jobs[idx].cmd, (int)pid);
            else
                fprintf(stderr, "%s with pid %d exited abnormally\n", jobs[idx].cmd, (int)pid);
                // fflush(stderr);
            remove_job_entry_by_pid(jobs[idx].pgid);
        } else if (WIFSTOPPED(status)) {
            /* stopped (Ctrl+Z) — mark stopped */
            jobs[idx].state = JOB_STOPPED;
            fprintf(stderr, "[%d] %s - Stopped\n", jobs[idx].job_id, jobs[idx].cmd);
            fflush(stderr);
        } else if (WIFCONTINUED(status)) {
            /* resumed */
            jobs[idx].state = JOB_RUNNING;
            fprintf(stderr, "[%d] %s - Running\n", jobs[idx].job_id, jobs[idx].cmd);
            fflush(stderr);
        }
    }
}



/* ---------- helper functions used by builtins (redirection + job helpers) ---------- */

/* Apply redirections found in final_tokens[start..end].
   On success returns 0 and sets saved_stdin/saved_stdout to -1 (no change) or
   to duplicated fds that should be restored by restore_redirections().
   On error prints the required error message and returns -1.
   This function supports tokens of the forms:
     <filename   or   < filename
     >filename   or   > filename
     >>filename  or   >> filename
*/
int apply_redirections_and_save(int start, int end, int *saved_stdin, int *saved_stdout) {
    *saved_stdin = -1;
    *saved_stdout = -1;

    for (int i = start; i <= end; ++i) {
        char *tok = final_tokens[i].value;
        if (!tok || tok[0] == '\0') continue;

        if (tok[0] == '<') {
            char *fname = NULL;
            if (tok[1] != '\0') {
                /* form: <name */
                fname = tok + 1;
            } else {
                /* form: < name */
                if (i + 1 <= end) fname = final_tokens[++i].value;
                else { fprintf(stderr, "No such file or directory\n"); return -1; }
            }
            int fd = open(fname, O_RDONLY);
            if (fd < 0) {
                fprintf(stderr, "No such file or directory\n");
                return -1;
            }
            if (*saved_stdin == -1) *saved_stdin = dup(STDIN_FILENO);
            dup2(fd, STDIN_FILENO);
            close(fd);
        } else if (tok[0] == '>') {
            int append = 0;
            char *fname = NULL;
            if (tok[1] == '>') {
                /* starts with >> */
                append = 1;
                if (tok[2] != '\0') fname = tok + 2;
                else {
                    if (i + 1 <= end) fname = final_tokens[++i].value;
                    else { fprintf(stderr, "Unable to create file for writing\n"); return -1; }
                }
            } else {
                /* starts with single > */
                if (tok[1] != '\0') fname = tok + 1;
                else {
                    if (i + 1 <= end) fname = final_tokens[++i].value;
                    else { fprintf(stderr, "Unable to create file for writing\n"); return -1; }
                }
            }
            int fd;
            if (append)
                fd = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else
                fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                fprintf(stderr, "Unable to create file for writing\n");
                return -1;
            }
            if (*saved_stdout == -1) *saved_stdout = dup(STDOUT_FILENO);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        /* other tokens are ignored here (flags, plain args) */
    }

    return 0;
}

void restore_redirections(int saved_stdin, int saved_stdout) {
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
}

/* find job index given a job id (job numbers assigned in add_job_entry) */
int find_job_index_by_jobid(int jobid) {
    for (int i = 0; i < MAX_JOBS; ++i) {
        if (jobs[i].job_id == jobid && jobs[i].pgid != 0) return i;
    }
    return -1;
}

/* activities builtin: list processes spawned by the shell, sorted lexicographically by command name.
   Print format: "command - Running" or "command - Stopped" (matches autograder regex expectations).
*/
void exe_activities(void) {
    int idxs[MAX_JOBS];
    int cnt = 0;
    for (int i = 0; i < MAX_JOBS; ++i) {
        if (jobs[i].state == JOB_RUNNING || jobs[i].state == JOB_STOPPED) {
            idxs[cnt++] = i;
        }
    }
    if (cnt == 0) return;

    qsort(idxs, cnt, sizeof(int), jobs_cmp_idx);

    for (int k = 0; k < cnt; ++k) {
        int i = idxs[k];
        const char *state =
            (jobs[i].state == JOB_RUNNING) ? "Running" :
            (jobs[i].state == JOB_STOPPED) ? "Stopped" : "Done";
        printf("[%d] : %s - %s\n", (int)jobs[i].pgid, jobs[i].cmd, state);
    }
    fflush(stdout);
}


void exe_ping(int start, int end) {
    // Requirement: Must have exactly 2 arguments (pid and signal)
    if (end - start != 2) {
        fprintf(stderr, "Invalid syntax!\n");
        return;
    }

    char* pid_str = final_tokens[start + 1].value;
    char* signal_str = final_tokens[start + 2].value;
    char* endptr;

    // Requirement: Handle invalid signal_number
    // strtol is used to safely convert string to long and check for non-numeric parts
    long signal_num_long = strtol(signal_str, &endptr, 10);
    if (*endptr != '\0' && !isspace((unsigned char)*endptr)) {
        fprintf(stderr, "Invalid syntax!\n");
        return;
    }

    pid_t pid = atoi(pid_str); // atoi is sufficient for pid
    int signal_number = (int)signal_num_long;

    // Requirement: Take signal number modulo 32
    int actual_signal = signal_number % 32;

    // The kill() system call sends a signal. kill(pid, 0) can be used to check if a process exists.
    // We send the actual signal directly.
    if (kill(pid, actual_signal) == -1) {
        // Check the global 'errno' variable after a system call fails
        if (errno == ESRCH) { // ESRCH means "No such process"
            // Requirement: Handle non-existent process
            fprintf(stderr, "No such process found\n");
        }
        // Note: We ignore other potential errors (like EPERM for permission denied)
        // as the spec only requires handling the ESRCH case.
    } else {
        // Requirement: Print success message
        printf("Sent signal %d to process with pid %d\n", signal_number, (int)pid);
    }
}


/* bg builtin: resume a stopped job in the background
   Syntax: bg [job_number]
   Prints error messages expected by autograder.
*/
void exe_bg(int start, int end) {
    int jobnum = -1;
    if (start + 1 <= end) {
        jobnum = atoi(final_tokens[start + 1].value);
    } else {
        /* pick most recent job id */
        int max_j = -1;
        for (int i = 0; i < MAX_JOBS; ++i) {
            if (jobs[i].state != JOB_UNUSED && jobs[i].job_id > max_j) {
                max_j = jobs[i].job_id;
            }
        }
        if (max_j == -1) { 
            fprintf(stderr, "No such job\n"); 
            // fflush(stdout);
            return; 
        }
        jobnum = max_j;
    }

    int idx = find_job_index_by_jobid(jobnum);
    if (idx == -1) { 
        fprintf(stderr, "No such job\n");

        // fflush(stdout);
        return; 
    }

    if (jobs[idx].state == JOB_RUNNING) {
        fprintf(stderr, "Job already running\n");
        // fflush(stderr);
        return;
    }

    /* signal the whole process group, not a single pid */
    if (kill(-jobs[idx].pgid, SIGCONT) < 0) {
        fprintf(stderr, "No such job\n");  // According to your test, even running jobs should say "No such job"
        // fflush(stdout);
        return;
    }

    jobs[idx].state = JOB_RUNNING;
    printf("[%d] %s &\n", jobs[idx].job_id, jobs[idx].cmd);
    fflush(stdout);
}


/* fg builtin: bring a background/stopped job to foreground
   Syntax: fg [job_number]
   Prints the command when bringing to foreground.
*/
void exe_fg(int start, int end) {
    int jobnum = -1;
    if (start + 1 <= end) jobnum = atoi(final_tokens[start + 1].value);
    else {
        int max_j = -1;
        for (int i = 0; i < MAX_JOBS; ++i) if (jobs[i].job_id > max_j) max_j = jobs[i].job_id;
        if (max_j == -1) { fprintf(stderr, "No such job\n"); return; }
        jobnum = max_j;
    }

    int idx = find_job_index_by_jobid(jobnum);
    if (idx == -1) { fprintf(stderr, "No such job\n"); return; }

    printf("%s\n", jobs[idx].cmd);

    pid_t pgid = jobs[idx].pgid;
    fg_pgid = pgid;

    /* resume the process group */
    if (kill(-pgid, SIGCONT) < 0) {
        printf("No such job\n");
        fg_pgid = -1;
        return;
    }

    /* put the job's process group in the foreground */
    if (tcsetpgrp(STDIN_FILENO, pgid) < 0) { /* ignore */ }

    /* wait for the job's processes (wait for any process in the pgid) */
    int status;
    if (waitpid(-pgid, &status, WUNTRACED) < 0) { /* ignore errors */ }

    if (WIFSTOPPED(status)) {
        jobs[idx].state = JOB_STOPPED;
        fprintf(stderr, "[%d] Stopped %s\n", jobs[idx].job_id, jobs[idx].cmd);
    } else {
        /* process exited — remove job entry by its pgid */
        remove_job_entry_by_pid(pgid);
    }

    /* restore terminal to the shell */
    if (tcsetpgrp(STDIN_FILENO, getpgrp()) < 0) { /* ignore */ }
    fg_pgid = -1;

}


// /* ---------- end helper functions ---------- */
// void exe_other(int start, int end, int is_background){
//     /* build argv from final_tokens[start..end] */
//     int argc_max = end - start + 2;
//     char **argv = malloc(sizeof(char*) * argc_max);
//     if (!argv) { perror("malloc"); return; }
//     // int argc = 0;
//     // for (int i = start; i <= end; i++) {
//     //     argv[argc++] = final_tokens[i].value;
//     // }
//     // argv[argc] = NULL;
//     int argc = 0;
//     for (int i = start; i <= end; i++) {
//         char *tok = final_tokens[i].value;

//         if (!tok) continue;

//         // skip redirection operators and their filenames
//         if (strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0 || strcmp(tok, ">>") == 0) {
//             i++; // skip filename too
//             continue;
//         }

//         // skip pipes (handled separately in handle_pipes)
//         if (strcmp(tok, "|") == 0) {
//             continue;
//         }

//         // otherwise, keep it in argv
//         argv[argc++] = tok;
//     }
//     argv[argc] = NULL;

//     pid_t pid = fork();
//     if (pid < 0) {
//         perror("fork");
//         free(argv);
//         return;
//     } else if (pid == 0) {
//         /* CHILD */
//         if (setpgid(0, 0) < 0) { /* ignore error */ }  /* best-effort; ignore error */

//         /* restore default terminal-related signal dispositions in the child */
//         signal(SIGINT, SIG_DFL);
//         signal(SIGQUIT, SIG_DFL);
//         signal(SIGTSTP, SIG_DFL);
//         signal(SIGTTIN, SIG_DFL);
//         signal(SIGTTOU, SIG_DFL);

//         /* If this command is running in background, prevent it from reading the terminal */
//         if (is_background) {
//             int fd = open("/dev/null", O_RDONLY);
//             if (fd >= 0) {
//                 dup2(fd, STDIN_FILENO);
//                 close(fd);
//             }
//         }

//         /* If there is a pipe anywhere in argv, handle the pipeline (handle_pipes forks children and exits) */
//         /* check if there is any pipe token in final_tokens range */
//         int has_pipe = 0;
//         for (int i = start; i <= end; i++) {
//             if (strcmp(final_tokens[i].value,"|")==0) { has_pipe=1; break; }
//         }

//         if (has_pipe) {
//             handle_pipes(start, end, is_background);
//             free(argv);
//             _exit(0); // parent already waits inside handle_pipes
//         }


//         /* No pipe: perform redirection (if any) and exec */
//         int s_in = -1, s_out = -1;
//         if (apply_redirections_and_save(start, end, &s_in, &s_out) < 0) {
//             _exit(1);
//         }

//         /* Exec the program */
//         execvp(argv[0], argv);
//         /* if exec fails */
//         fprintf(stderr, "Command not found!\n");
//         fflush(stderr);
//         /* restore redirections if you want (not necessary before _exit) */
//         _exit(127);
//     } 
//     else {
//         /* PARENT */

//         /* Put child in its own process group so signals target whole job */
//         if (setpgid(pid, pid) < 0) {
//             /* ignore error */
//         }

// if (is_background) {
//             /* ---- BACKGROUND ---- */
//             char cmdline[MAX_BUFFER];
//             build_cmd_string(start, end, cmdline, sizeof(cmdline), 1); /* append_amp=1 */

//             add_job_entry(pid, cmdline);

//             int idx = find_job_index_by_pid(pid);
//             int jobnum = (idx >= 0) ? jobs[idx].job_id : -1;
            
//             // Only print job info if it's not part of a command sequence
//             // Check if this is a standalone background command (not followed by ;)
//             int should_print = 1;
//             for (int i = start; i <= end; i++) {
//                 if (final_tokens[i].value && strcmp(final_tokens[i].value, "&") == 0) {
//                     // Check if there's more commands after the &
//                     if (i + 1 <= end && final_tokens[i + 1].type != eof) {
//                         should_print = 0;
//                         break;
//                     }
//                 }
//             }
            
//             if (should_print) {
//                 if (jobnum >= 0) {
//                     fprintf(stderr, "[%d] %d\n", jobnum, (int)pid);
//                 } else {
//                     printf("[?] %d\n", (int)pid);
//                 }
//                 fflush(stdout);
//             }

//             free(argv);
//             return;
//         }
//         else {
//             /* ---- FOREGROUND ---- */
//             fg_pgid = pid;

//             if (tcsetpgrp(STDIN_FILENO, pid) < 0) {
//                 /* ignore errors */
//             }

//             int status;
//             pid_t result;
//             do {
//                 result = waitpid(pid, &status, WUNTRACED);
//             } while (result == -1 && errno == EINTR);

//             if (result > 0 && WIFSTOPPED(status)) {
//                 /* If stopped with Ctrl-Z, add to jobs[] */
//                 char cmdline[MAX_BUFFER];
//                 build_cmd_string(start, end, cmdline, sizeof(cmdline), 0);
//                 add_job_entry(pid, cmdline);
//                 int idx = find_job_index_by_pid(pid);
//                 if (idx >= 0) jobs[idx].state = JOB_STOPPED;
//                 printf("[%d] %s - Stopped\n", jobs[idx].job_id, jobs[idx].cmd);
//                 fflush(stdout);
//             }

//             if (tcsetpgrp(STDIN_FILENO, getpgrp()) < 0) {
//                 /* ignore errors */
//             }

//             fg_pgid = -1;
//             free(argv);
//             return;
//         }
//     }
// }

void exe_hop(int start, int end) {
    char curr[PATH_MAX];
    if(getcwd(curr, sizeof(curr)) == NULL) {
        perror("getcwd");
        return;
    }

    // Process all arguments from start+1 to end
    for (int i = start + 1; i <= end; i++) {
        char* arg = final_tokens[i].value;
        if (!arg) continue;

        // Skip redirection tokens
        if (strcmp(arg, "<") == 0 || strcmp(arg, ">") == 0 || strcmp(arg, ">>") == 0) {
            i++; // skip filename too
            continue;
        }

        // Save current directory before each hop
        if(getcwd(curr, sizeof(curr)) == NULL) {
            perror("getcwd");
            return;
        }

        if(strcmp("~", arg) == 0) {
            if(chdir(home_dir) == 0) strcpy(prev_dir, curr);
            else perror("Error");
        }
        else if(strcmp(".", arg) == 0) {
            continue; // do nothing
        }
        else if(strcmp("..", arg) == 0) {
            if(chdir("..") == 0) strcpy(prev_dir, curr);
            else perror("Error");
        }
        else if (strcmp("-", arg) == 0) {
            if (prev_dir[0] == '\0') {
                fflush(stdout);
            } else {
                if (chdir(prev_dir) == 0) {
                    strcpy(prev_dir, curr);
                }
                else{
                    printf("\n");
                }
            }
        }
        else {
            if(chdir(arg) == 0) strcpy(prev_dir, curr);
            else {
                fprintf(stderr, "No such directory!\n");
            }
        }
    }
}


void exe_reveal(int start, int end) {
    int flag_a = 0;
    int flag_l = 0;
    char *path_arg = NULL;
    int nonflag_count = 0;

    /* parse flags and optional path; detect too many non-flag args */
    for (int i = start + 1; i <= end; ++i) {
        char *tok = final_tokens[i].value;
        if (!tok) continue;
        
        // Skip redirection tokens and their arguments
        if (strcmp(tok, "<") == 0 || strcmp(tok, ">") == 0 || strcmp(tok, ">>") == 0) {
            if (i + 1 <= end) i++; // skip the filename that follows
            continue;
        }
        
        // Skip pipe tokens
        if (strcmp(tok, "|") == 0) {
            continue;
        }
        
        if (tok[0] == '-' && tok[1] != '\0') {
            /* treat token as flags, e.g. -a, -l, -la */
            for (int j = 1; tok[j]; ++j) {
                if (tok[j] == 'a') flag_a = 1;
                else if (tok[j] == 'l') flag_l = 1;
                /* other letters are treated as filename characters per spec */
            }
        } else {
            /* non-flag token — candidate path */
            nonflag_count++;
            if (nonflag_count > 1) {
                /* too many arguments */
                fprintf(stderr, "reveal: Invalid Syntax!\n");
                return;
            }
            path_arg = tok;
        }
    }

    /* decide directory to list */
    char dirpath[PATH_MAX];
    if (path_arg == NULL) {
        /* no argument — use current directory */
        if (getcwd(dirpath, sizeof(dirpath)) == NULL) {
            fprintf(stderr, "No such directory!\n");
            return;
        }
    } else {
        if (strcmp(path_arg, "~") == 0) {
            strncpy(dirpath, home_dir, sizeof(dirpath)-1);
            dirpath[sizeof(dirpath)-1] = '\0';
        } else if (strcmp(path_arg, "-") == 0) {
            if (strlen(prev_dir) == 0) {
                fprintf(stderr, "No such directory!\n");
                return;
            }
            strncpy(dirpath, prev_dir, sizeof(dirpath)-1);
            dirpath[sizeof(dirpath)-1] = '\0';
        } else {
            strncpy(dirpath, path_arg, sizeof(dirpath)-1);
            dirpath[sizeof(dirpath)-1] = '\0';
        }
    }

    DIR *dir = opendir(dirpath);
    if (!dir) {
        fprintf(stderr, "No such directory!\n");
        return;
    }

    struct dirent *entry;
    char *files[4096];
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (!flag_a && entry->d_name[0] == '.') continue;
        files[count] = malloc(strlen(entry->d_name) + 1);
        if (files[count]) {
            strcpy(files[count], entry->d_name);
            count++;
        }
        if (count >= 4096) break; /* safety */
    }
    closedir(dir);

// /* sort lexicographically (case-insensitive) */
//     for (int i = 0; i < count - 1; ++i) {
//         for (int j = i + 1; j < count; ++j) {
//             if (strcasecmp(files[i], files[j]) > 0) {  // Case-insensitive
//                 char *tmp = files[i];
//                 files[i] = files[j];
//                 files[j] = tmp;
//             }
//         }
//     }

/* sort lexicographically (ASCII value based) */
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (strcmp(files[i], files[j]) > 0) {  // Case-sensitive
                char *tmp = files[i];
                files[i] = files[j];
                files[j] = tmp;
            }
        }
    }

    /* print — if -l set: one per line; otherwise space-separated like ls */
    for (int i = 0; i < count; ++i) {
        printf("%s", files[i]);
        if (flag_l == 1 || i == count - 1) printf("\n");
        else printf(" ");
    }

    fflush(stdout);

    /* free the allocated names */
    for (int i = 0; i < count; ++i) free(files[i]);
}

void exe_log(int start, int end) {
    
    if (start == end) {
        print_log();
        return;
    }

    char* arg1 = (start + 1 <= end) ? final_tokens[start + 1].value : NULL;
    if (!arg1) { print_log(); return; }

    if (strcmp(arg1, "purge") == 0){
        purge_log();
    }
    else if (strcmp(arg1, "execute")==0){
        if (start+2 > end) {
            printf("Usage: log execute <index>\n");
            return;
        }
        int idx = atoi(final_tokens[start+2].value);
        execute_log_index(idx);
    } else {
        fprintf(stderr, "log: Invalid Syntax!\n");
    }
}

// NEW valid function (replaces old valid() and exe_other())
void valid(int start, int end, int is_background) {
    if (start > end) return;

    // Check for pipes in the current command segment
    int has_pipes = 0;
    for (int i = start; i <= end; i++) {
        if (final_tokens[i].type == pipes) {
            has_pipes = 1;
            break;
        }
    }

    // Handle built-ins that MUST run in the parent shell (like hop, fg, bg)
    char *cmd = final_tokens[start].value;
    if (!has_pipes && !is_background) {
        if (strcmp(cmd, "hop") == 0) {
            exe_hop(start, end); return;
        }
        if (strcmp(cmd, "fg") == 0) {
            exe_fg(start, end); return;
        }
        if (strcmp(cmd, "bg") == 0) {
            exe_bg(start, end); return;
        }
    }

    pid_t pgid;
    if (has_pipes) {
        pgid = handle_pipes(start, end);
    } else {
        // This is the logic for a simple command (previously in exe_other)
        pgid = fork();
        if (pgid == 0) { // --- CHILD ---
            setpgid(0, 0); // Child puts itself in a new process group
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (is_background) {
                int fd = open("/dev/null", O_RDONLY);
                if (fd >= 0) { dup2(fd, STDIN_FILENO); close(fd); }
            }

            int saved_in, saved_out;
            apply_redirections_and_save(start, end, &saved_in, &saved_out);

            int argc;
            char **argv = build_argv_filtered(start, end, &argc);

            if (is_builtin(argv[0])) {
                run_builtin(argv, start, end);
                _exit(0);
            } else {
                execvp(argv[0], argv);
                fprintf(stderr, "Command not found!\n");
                _exit(127);
            }
        }
    }

    if (pgid < 0) return; // Forking or piping failed

    // --- PARENT'S WAITING AND JOB CONTROL LOGIC ---
    if (is_background) {
        char cmdline[MAX_BUFFER];
        build_cmd_string(start, end, cmdline, sizeof(cmdline), 1);
        add_job_entry(pgid, cmdline);
        int idx = find_job_index_by_pid(pgid);
        fprintf(stderr, "[%d] %d\n", jobs[idx].job_id, (int)pgid);
    } else {
        fg_pgid = pgid;
        if (tcsetpgrp(STDIN_FILENO, pgid) < 0) { /* ignore */ }

        int status;
        pid_t result;
        // Loop to handle interruptions by signals (EINTR)
        do {
            result = waitpid(-pgid, &status, WUNTRACED);
        } while (result == -1 && errno == EINTR);

        if (WIFSTOPPED(status)) {
            char cmdline[MAX_BUFFER];
            build_cmd_string(start, end, cmdline, sizeof(cmdline), 0);
            add_job_entry(pgid, cmdline);
            int idx = find_job_index_by_pid(pgid);
            if(idx != -1) {
                jobs[idx].state = JOB_STOPPED;
                fprintf(stderr, "\n[%d] %s - Stopped\n", jobs[idx].job_id, jobs[idx].cmd);
            }
        }
        
        if (tcsetpgrp(STDIN_FILENO, getpgrp()) < 0) { /* ignore */ }
        fg_pgid = -1;
    }
}

void execute_tokens() {
    int pos_local = 0;

    while(final_tokens[pos_local].type != eof) {
        int start = pos_local;
        int end = pos_local;

    
    while(final_tokens[end].type != semi_colon &&
        final_tokens[end].type != amp &&
        final_tokens[end].type != eof) {
        end++;
    }

    /* detect background operator: if separator is & then this command is background */
    int is_background = 0;
    if (final_tokens[end].type == amp) is_background = 1;

    /* execute from start to end-1, passing background flag */
    if (end > start) {
        valid(start, end - 1, is_background);
    }

    /* skip separator */
    if (final_tokens[end].type == eof) break;
    pos_local = end + 1;

    }
}
