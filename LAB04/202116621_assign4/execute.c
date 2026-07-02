/*---------------------------------------------------------------------------*/
/* execute.c                                                                 */
/* Author: Jongki Park, Kyoungsoo Park                                       */
/*---------------------------------------------------------------------------*/

#include "dynarray.h"
#include "token.h"
#include "util.h"
#include "lexsyn.h"
#include "snush.h"
#include "execute.h"
#include "job.h"
#include "unistd.h"

#define CHILD(pid) (pid == 0)
#define PARENT(pid) (pid > 0)

extern struct job_manager *manager;
/*---------------------------------------------------------------------------*/
void redout_handler(char *fname) {
	int fd;

	fd = open(fname, 
			  O_WRONLY | O_CREAT | O_TRUNC, 
			  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		error_print(NULL, PERROR);
		exit(EXIT_FAILURE);
	}
	else {
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}
}
/*---------------------------------------------------------------------------*/
void redin_handler(char *fname) {
	int fd;

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		error_print(NULL, PERROR);
		exit(EXIT_FAILURE);
	}
	else {
		dup2(fd, STDIN_FILENO);
		close(fd);
	}
}
/*---------------------------------------------------------------------------*/
int build_command_partial(DynArray_T oTokens, int start, 
						int end, char *args[]) {
	int i, ret = 0, redin = FALSE, redout = FALSE, cnt = 0;
	struct Token *t;

	/* Build command */
	for (i = start; i < end; i++) {

		t = dynarray_get(oTokens, i);

		if (t->token_type == TOKEN_WORD) {
			if (redin == TRUE) {
				redin_handler(t->token_value);
				redin = FALSE;
			}
			else if (redout == TRUE) {
				redout_handler(t->token_value);
				redout = FALSE;
			}
			else
				args[cnt++] = t->token_value;
		}
		else if (t->token_type == TOKEN_REDIN)
			redin = TRUE;
		else if (t->token_type == TOKEN_REDOUT)
			redout = TRUE;
	}
	args[cnt] = NULL;

#ifdef DEBUG
	for (i = 0; i < cnt; i++)
	{
		if (args[i] == NULL)
			printf("CMD: NULL\n");
		else
			printf("CMD: %s\n", args[i]);
	}
	printf("END\n");
#endif
	return ret;
}
/*---------------------------------------------------------------------------*/
int build_command(DynArray_T oTokens, char *args[]) {
	return build_command_partial(oTokens, 0, 
								dynarray_get_length(oTokens), args);
}
/*---------------------------------------------------------------------------*/
void execute_builtin(DynArray_T oTokens, enum BuiltinType btype) {
	int ret;
	char *dir = NULL;
	struct Token *t1;

	switch (btype) {
	case B_EXIT:
		if (dynarray_get_length(oTokens) == 1) {
			// printf("\n");
			dynarray_map(oTokens, free_token, NULL);
			dynarray_free(oTokens);

			exit(EXIT_SUCCESS);
		}
		else
			error_print("exit does not take any parameters", FPRINTF);

		break;

	case B_CD:
		if (dynarray_get_length(oTokens) == 1) {
			dir = getenv("HOME");
			if (dir == NULL) {
				error_print("cd: HOME variable not set", FPRINTF);
				break;
			}
		}
		else if (dynarray_get_length(oTokens) == 2) {
			t1 = dynarray_get(oTokens, 1);
			if (t1->token_type == TOKEN_WORD)
				dir = t1->token_value;
		}

		if (dir == NULL) {
			error_print("cd takes one parameter", FPRINTF);
			break;
		}
		else {
			ret = chdir(dir);
			if (ret < 0)
				error_print(NULL, PERROR);
		}
		break;

	default:
		error_print("Bug found in execute_builtin", FPRINTF);
		exit(EXIT_FAILURE);
	}
}
/*---------------------------------------------------------------------------*/
void wait_fg(int jobid) {
	while (1) {
        struct job *job = find_job_by_jid(jobid);

        if (job == NULL) {
            break;
        }

        if (job->state != foreground) {
            break;
        }
        sleep(0);
    }

    return;
}
/*---------------------------------------------------------------------------*/
void print_job(int jobid, pid_t pgid) {
    fprintf(stdout, 
		"[%d] Process group: %d running in the background\n", jobid, pgid);
}
/*---------------------------------------------------------------------------*/
int fork_exec(DynArray_T oTokens, int is_background) {
    pid_t pid;
    int   job_id = -1;     

	int sync_pipe[2];
	if (pipe(sync_pipe) < 0) {
		error_print("fork_exec: pipe error", PERROR);
		exit(EXIT_FAILURE);
	}

    pid = fork();
    if (pid < 0) {
        error_print("fork_exec: fork error", FPRINTF);
        exit(EXIT_FAILURE);
    }
    else if (PARENT(pid)) {
        if (setpgid(pid, pid) < 0) {
            error_print("fork_exec: setpgid (parent)", PERROR);
        }

        job_id = add_job_no_pipe(pid, is_background);
        if (job_id < 0) exit(EXIT_FAILURE);

		close(sync_pipe[0]);
		close(sync_pipe[1]);

		pid_t shell_pgid = getpgrp();  
		int tty_fd = open("/dev/tty", O_RDWR | __O_CLOEXEC);
		if (tty_fd < 0) {
			error_print("fork_exec: open tty error", PERROR);
			exit(EXIT_FAILURE);
		} 

        if (is_background) {
            print_job(job_id, pid);
        } else {
            tcsetpgrp(tty_fd, pid);
            wait_fg(job_id);
			tcsetpgrp(tty_fd, shell_pgid);
        }
		close(tty_fd);

        return job_id;
    }
	else if (CHILD(pid)) {
		close(sync_pipe[1]);
		char dummy;
		if (read(sync_pipe[0], &dummy, 1) < 0) {
			error_print("fork_exec: read sync pipe error", PERROR);
			exit(EXIT_FAILURE);
		}
		close(sync_pipe[0]);
		
        if (setpgid(0, getpid()) < 0) {
            error_print("fork_exec: setpgid (child)", PERROR);
            exit(EXIT_FAILURE);
        }

        char *cmd[MAX_ARGS_CNT];
        build_command(oTokens, cmd);
		
		signal(SIGPIPE, SIG_DFL);
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);
        execvp(cmd[0], cmd);
		
		error_print(cmd[0], PERROR);
		if (is_background) {
			int fd = open("/dev/null", O_RDONLY | __O_CLOEXEC);
			if (fd < 0) {
				error_print("fork_exec: open /dev/null error", PERROR);
				exit(EXIT_FAILURE);
			}
			if (dup2(fd, STDIN_FILENO) < 0) {
				error_print("fork_exec: dup2 stdout error", PERROR);
				exit(EXIT_FAILURE);
			}
			close(fd);
		}
		exit(EXIT_FAILURE);
    }

	error_print("unexpected case", FPRINTF);
	exit(EXIT_FAILURE);
	return -1;
}
/*---------------------------------------------------------------------------*/
int iter_pipe_fork_exec(int n_pipe, DynArray_T oTokens, int is_background) {  
	int toks = dynarray_get_length(oTokens);
	int pos[n_pipe + 2];
	pos[0] = 0;
	pos[n_pipe + 1] = toks;

	int j = 1;
	for (int i = 0; i < toks; i++) {
		struct Token *t = dynarray_get(oTokens, i);
		if (t->token_type == TOKEN_PIPE)
			pos[j++] = i;
	}

	// pipe
	int pipe_fd[n_pipe][2];
	for (int i = 0; i < n_pipe; i++) {
		if (pipe(pipe_fd[i]) < 0) {
			error_print("iter_pipe_fork_exec: pipe error", PERROR);
			exit(EXIT_FAILURE);
		}
	}
	int sync_pipe[2];
	if (pipe(sync_pipe) < 0) {
		error_print("iter_pipe_fork_exec: sync pipe error", PERROR);
		exit(EXIT_FAILURE);
	}

	pid_t pids[n_pipe + 1];
	struct job *job;
	for (int i = 0; i < (n_pipe + 1); i++) {
		pids[i] = fork();
		if (pids[i] < 0) {
			error_print("iter_pipe_fork_exec: fork error", FPRINTF);
			exit(EXIT_FAILURE);
		}
		else if (PARENT(pids[i])) {
			if (i == 0) { // First command
				if (setpgid(pids[i], pids[i]) < 0) {
					error_print("iter_pipe_fork_exec: setpgid error", PERROR);
					exit(EXIT_FAILURE);
				}
				job = add_job_with_pipe(pids, n_pipe + 1, is_background);
				if (!job) {
					error_print("iter_pipe_fork_exec: add_job_with_pipe error", FPRINTF);
					exit(EXIT_FAILURE);
				}
			} else { // Not the first command
				if (setpgid(pids[i], pids[0]) < 0) {
					error_print("iter_pipe_fork_exec: setpgid error", PERROR);
					exit(EXIT_FAILURE);
				}
				job->pid_list[i] = pids[i];
			}
			continue;
		}
		else if (CHILD(pids[i])) {
			close(sync_pipe[1]);
			char dummy;
			if (read(sync_pipe[0], &dummy, 1) < 0) {
				error_print("iter_pipe_fork_exec: read sync pipe error", PERROR);
				exit(EXIT_FAILURE);
			}
			close(sync_pipe[0]);

			if (i == 0) {
				if (setpgid(0, getpid()) < 0) {
					error_print("iter_pipe_fork_exec: setpgid error", PERROR);
					exit(EXIT_FAILURE);
				}
			} 
			if (i > 0) { // Not the first command
				if (setpgid(0, pids[0]) < 0) {
					error_print("iter_pipe_fork_exec: setpgid error", PERROR);
					exit(EXIT_FAILURE);
				}
				if (dup2(pipe_fd[i - 1][0], STDIN_FILENO) < 0) {
					error_print("iter_pipe_fork_exec: dup2 error", PERROR);
					exit(EXIT_FAILURE);
				}
			}
			if (i < n_pipe) { // Not the last command
				if (dup2(pipe_fd[i][1], STDOUT_FILENO) < 0) {
					error_print("iter_pipe_fork_exec: dup2 error", PERROR);
					exit(EXIT_FAILURE);
				}
			}

			for (int j = 0; j < n_pipe; j++) {
				close(pipe_fd[j][0]);
				close(pipe_fd[j][1]);
			}

			char *cmd[MAX_ARGS_CNT];
			build_command_partial(oTokens, pos[i], pos[i + 1], cmd);
			
			signal(SIGPIPE, SIG_DFL);
			signal(SIGTTIN, SIG_DFL);
			signal(SIGTTOU, SIG_DFL);
			execvp(cmd[0], cmd);

			error_print(cmd[0], PERROR);
			if (is_background && i == 0) {
				int fd = open("/dev/null", O_RDONLY | __O_CLOEXEC);
				if (fd < 0) {
					error_print("fork_exec: open /dev/null error", PERROR);
					exit(EXIT_FAILURE);
				}
				if (dup2(fd, STDIN_FILENO) < 0) {
					error_print("fork_exec: dup2 stdout error", PERROR);
					exit(EXIT_FAILURE);
				}
				close(fd);
			}
			usleep(100000); // Give time for the error to be printed
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < n_pipe; i++) {
		close(pipe_fd[i][0]);
		close(pipe_fd[i][1]);
	}
	close(sync_pipe[0]);
	close(sync_pipe[1]);

	int tty_fd = open("/dev/tty", O_RDWR | __O_CLOEXEC);
	if (tty_fd < 0) {
		error_print("iter_pipe_fork_exec: open tty error", PERROR);
		exit(EXIT_FAILURE);
	}
	pid_t shell_pgid = getpgrp(); 
	
	if (is_background) {
		print_job(job->job_id, pids[0]);
	}
	else {
		tcsetpgrp(tty_fd, pids[0]);
		wait_fg(job->job_id);
		tcsetpgrp(tty_fd, shell_pgid);
	}
	close(tty_fd);

	// return the first child process ID
	return pids[0];
}
/*---------------------------------------------------------------------------*/