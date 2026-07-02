/*---------------------------------------------------------------------------*/
/* snush.c                                                                   */
/* Author: Jongki Park, Kyoungsoo Park                                       */
/*---------------------------------------------------------------------------*/

#include "util.h"
#include "token.h"
#include "dynarray.h"
#include "execute.h"
#include "lexsyn.h"
#include "snush.h"
#include "job.h"

#define EV_BUF_SIZE 32
#define FALSE 0

struct job_manager *manager;
volatile sig_atomic_t error_flag[EV_BUF_SIZE] = {0};
volatile sig_atomic_t err_idx = 0;
volatile sig_atomic_t is_error = 0;

/*---------------------------------------------------------------------------*/
static void check_error_flag() {    
    if (err_idx == 0) {
        return;
    }
    if (err_idx > EV_BUF_SIZE) {
        fprintf(stderr, "[Error] Too many errors occurred, exiting...\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < err_idx; i++) {
        if (error_flag[i] == 1) {
            fprintf(stderr, "[Error] sigchld_handler: find_job_by_pid\n");
            fflush(stderr);
            is_error = 1;
        }
        else if (error_flag[i] == 2) {
            fprintf(stderr, "[Error] sigchld_handler: erase_process_from_job\n");
            fflush(stderr);
            is_error = 1;
        }
        else if (error_flag[i] == 3) {
            fprintf(stderr, "[Error] sigchld_handler: waitpid failed\n");
            fflush(stderr);
            is_error = 1;
        }
    }
    err_idx = 0;

    if (is_error) {
        fprintf(stderr, "[Error] Exiting due to errors\n");
        exit(EXIT_FAILURE);
    }
}
/*---------------------------------------------------------------------------*/
static void check_bg_status() {
    struct job *job;

    while((job = manager->done_bg_jobs) != NULL) {
        fprintf(stdout, 
            "[%d] Process group: %d finished in the background\n", 
            job->job_id, job->pgid);
        fflush(stdout);

        if (erase_done_bg_jobs(job) == FALSE) {
            fprintf(stderr, "[Error] check_bg_status: Failed to erase done background jobs\n");
            exit(EXIT_FAILURE);
        }
    }
}
/*---------------------------------------------------------------------------*/
static void terminate_jobs() {
    struct job *curr = manager->jobs;
    struct job *next = NULL;

    while (curr != NULL) {
        next = curr->next;
        free(curr->pid_list);
        free(curr);
        curr = next;
    }
    manager->n_jobs = 0;
    manager->jobs = NULL;

    curr = manager->done_bg_jobs;
    while (curr != NULL) {
        next = curr->next;
        free(curr->pid_list);
        free(curr);
        curr = next;
    }
    
    manager->done_bg_jobs = NULL;
}
/*---------------------------------------------------------------------------*/
void cleanup() {
    if (manager) {
        struct job *j;
        for (j = manager->jobs; j != NULL; j = j->next) {
            if (j->pgid > 0) {
                kill(-j->pgid, SIGTERM);
            }
        }
        
        for (j = manager->done_bg_jobs; j != NULL; j = j->next) {
            if (j->pgid > 0) {
                kill(-j->pgid, SIGTERM);
            }
        }
    }

    int shell_pgid = getpgrp();
    tcsetpgrp(STDIN_FILENO, shell_pgid);
    tcsetpgrp(STDOUT_FILENO, shell_pgid);
    tcsetpgrp(STDERR_FILENO, shell_pgid);
    terminate_jobs();
    free(manager);
}
/*---------------------------------------------------------------------------*/
static void sigint_handler(int signo) {
    struct job *job = NULL;
    pid_t pgid;

    if (signo == SIGINT) {
        job = find_job_fg();
        if (job == NULL) {
            return;
        }

        pgid = job->pgid;
        if (pgid != 0) {
            kill(-pgid, SIGINT);
        }
    }

    return;
}
/*---------------------------------------------------------------------------*/
/* 
 * Whenever a child process terminates, this handler handles all terminated 
 * child processes (i.e. zombies). 
 * Do not use printf() in signal handler since this is called asynchronously.
 * Instead, print out the error message or the background process status 
 * message with check_error_flag() and check_bg_status().
 */
static void sigchld_handler(int signo) {
    pid_t pid;
    int stat;

    if (signo == SIGCHLD) {

        while((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
            struct job *job = find_job_by_pid(pid);

            if (job == NULL) {
                error_flag[err_idx++] = 1;
                break;
            }
            else {
                if (erase_process_from_job(job, pid) == FALSE) {
                    error_flag[err_idx++] = 2;
                    job->state = stopped;
                    break;
                }
                error_flag[err_idx++] = 0;
            }             
        }

        if (pid < 0 && errno != ECHILD && errno != EINTR) {
            error_flag[err_idx++] = 3;
            return;
        }
    }
    return;
}
/*---------------------------------------------------------------------------*/
static void shell_helper(const char *in_line) {
    DynArray_T oTokens;

    enum LexResult lexcheck;
    enum SyntaxResult syncheck;
    enum BuiltinType btype;
    int n_pipe;
    int ret_jid;
    int is_background;

    oTokens = dynarray_new(0);
    if (oTokens == NULL) {
        error_print("Cannot allocate memory", FPRINTF);
        exit(EXIT_FAILURE);
    }

    lexcheck = lex_line(in_line, oTokens);
    switch (lexcheck) {
    case LEX_SUCCESS:
        if (dynarray_get_length(oTokens) == 0)
            return;

        /* dump lex result when DEBUG is set */
        dump_lex(oTokens);

        syncheck = syntax_check(oTokens);
        if (syncheck == SYN_SUCCESS) {
            btype = check_builtin(dynarray_get(oTokens, 0));
            if (btype == NORMAL) {
                is_background = check_bg(oTokens);
                n_pipe = count_pipe(oTokens);

                if (manager->n_jobs + 1 > MAX_JOBS) {
                    fprintf(stderr, 
                        "[Error] Total number of jobs execeed the limit(%d)\n",
                        MAX_JOBS);
                    return;
                }

                if (n_pipe > 0) {
                    ret_jid = iter_pipe_fork_exec(n_pipe, oTokens, 
                                                is_background);
                }
                else {
                    ret_jid = fork_exec(oTokens, is_background);
                }

                if (ret_jid < 0) {
                    error_print("Invalid return value "\
                        "of external command execution", FPRINTF);
                }
            }
            else {
                /* Execute builtin command */
                execute_builtin(oTokens, btype);
            }
        }

        /* syntax error cases */
        else if (syncheck == SYN_FAIL_NOCMD)
            error_print("Missing command name", FPRINTF);
        else if (syncheck == SYN_FAIL_MULTREDOUT)
            error_print("Multiple redirection of standard out", FPRINTF);
        else if (syncheck == SYN_FAIL_NODESTOUT)
            error_print("Standard output redirection without file name", 
                        FPRINTF);
        else if (syncheck == SYN_FAIL_MULTREDIN)
            error_print("Multiple redirection of standard input", FPRINTF);
        else if (syncheck == SYN_FAIL_NODESTIN)
            error_print("Standard input redirection without file name", 
                        FPRINTF);
        else if (syncheck == SYN_FAIL_INVALIDBG)
            error_print("Invalid use of background", FPRINTF);
        break;

    case LEX_QERROR:
        error_print("Unmatched quote", FPRINTF);
        break;

    case LEX_NOMEM:
        error_print("Cannot allocate memory", FPRINTF);
        break;

    case LEX_LONG:
        error_print("Command is too large", FPRINTF);
        break;

    default:
        error_print("lex_line needs to be fixed", FPRINTF);
        exit(EXIT_FAILURE);
    }

    /* Free memories allocated to tokens */
    dynarray_map(oTokens, free_token, NULL);
    dynarray_free(oTokens);
}
/*---------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    sigset_t sigset;
    char c_line[MAX_LINE_SIZE + 2];

    atexit(cleanup);
    init_job_manager();
    error_print(argv[0], SETUP);

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
 
    /* Register signal handler */
    signal(SIGINT, sigint_handler);
    signal(SIGCHLD, sigchld_handler);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    while (1) {
        check_error_flag();
        check_bg_status();
        fprintf(stdout, "%% ");
        fflush(stdout);

        if (fgets(c_line, MAX_LINE_SIZE, stdin) == NULL) {
            fprintf(stdout, "\n");
            exit(EXIT_SUCCESS);
        }

        shell_helper(c_line);
    }

    return 0;
}
/*---------------------------------------------------------------------------*/