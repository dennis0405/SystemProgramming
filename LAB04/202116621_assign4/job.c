/*---------------------------------------------------------------------------*/
/* job.c                                                                     */
/* Author: Jongki Park, Kyoungsoo Park                                       */
/*---------------------------------------------------------------------------*/

#include "job.h"

#define TRUE 1
#define FALSE 0

extern struct job_manager *manager;
/*---------------------------------------------------------------------------*/
void init_job_manager() {
	manager = (struct job_manager *)calloc(1, sizeof(struct job_manager));
	if (manager == NULL) {
		fprintf(stderr, "[Error] job manager allocation failed\n");
		exit(EXIT_FAILURE);
	}
}
/*---------------------------------------------------------------------------*/
struct job *find_job_by_jid(int job_id) {
    if (manager == NULL) {
        fprintf(stderr, "[Error] find_job_by_jid: Job manager is NULL\n");
        return NULL;
    }

    struct job *current = manager->jobs;

    while (current != NULL) {
        if (current->job_id == job_id) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}
/*---------------------------------------------------------------------------*/
struct job *find_job_by_pid(pid_t pid) {
    if (manager == NULL) {
        fprintf(stderr, "[Error] find_job_by_pid: Job manager is NULL\n");
        return NULL;
    }

    struct job *current = manager->jobs;

    while (current != NULL) {
        for (int i = 0; i < current->total_num; i++) {
            if (current->pid_list[i] == pid) {
                return current;
            }
        }
        current = current->next;
    }
    return NULL;
}
/*---------------------------------------------------------------------------*/
struct job *find_job_fg() {
    if (manager == NULL) {
        fprintf(stderr, "[Error] find_job_fg: Job manager is NULL\n");
        return NULL;
    }

    struct job *current = manager->jobs;

    while (current != NULL) {
        if (current->state == foreground) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}
/*---------------------------------------------------------------------------*/
char add_job_to_manager(struct job *job) {
    if (manager == NULL) {
        fprintf(stderr, "[Error] add_job_to_manager: Job manager is NULL\n");
        return FALSE;
    }

    if (manager->jobs == NULL) {
        manager->jobs = job;
    } else {
        struct job *current = manager->jobs;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = job;
    }
    manager->n_jobs++;
    return TRUE;
}
/*---------------------------------------------------------------------------*/
int add_job_no_pipe(pid_t pgid, int is_background) {
    struct job *job = (struct job *)malloc(sizeof(struct job));
    if (job == NULL) {
        fprintf(stderr, "[Error] add_job: Job allocation failed\n");
        return -1;
    }

    job->job_id = manager->n_jobs;
    job->state = is_background ? background : foreground;
    job->pgid = pgid;
    job->pid_list = (pid_t *)malloc(sizeof(pid_t));
    if (job->pid_list == NULL) {
        fprintf(stderr, "[Error] add_job: PID list allocation failed\n");
        free(job);
        return -1;
    }
    job->pid_list[0] = pgid;
    job->total_num = 1;
    job->curr_num = 0;
    job->next = NULL;
    if (add_job_to_manager(job) == FALSE) {
        fprintf(stderr, "[Error] add_job: Failed to add job to manager\n");
        free(job->pid_list);
        free(job);
        return -1;
    }
    
    return job->job_id;
}
/*---------------------------------------------------------------------------*/
struct job *add_job_with_pipe(pid_t *pids, int n_pids, int is_background) {
    if (n_pids <= 0) {
        fprintf(stderr, "[Error] add_job_with_pipe: No PIDs provided\n");
        return NULL;
    }

    struct job *job = (struct job *)malloc(sizeof(struct job));
    if (job == NULL) {
        fprintf(stderr, "[Error] add_job_with_pipe: Job allocation failed\n");
        return NULL;
    }

    job->job_id = manager->n_jobs;
    job->state = is_background ? background : foreground;
    job->pgid = pids[0];
    job->pid_list = (pid_t *)calloc(1, n_pids * sizeof(pid_t));
    if (job->pid_list == NULL) {
        fprintf(stderr, "[Error] add_job_with_pipe: PID list allocation failed\n");
        free(job);
        return NULL;
    }
    job->pid_list[0] = pids[0];

    job->total_num = n_pids;
    job->curr_num = 0;
    job->next = NULL;

    if (add_job_to_manager(job) == FALSE) {
        fprintf(stderr, "[Error] add_job_with_pipe: Failed to add job to manager\n");
        free(job->pid_list);
        free(job);
        return NULL;
    }

    return job;
}
/*---------------------------------------------------------------------------*/
char erase_done_bg_jobs(struct job *job) {
    if (manager == NULL) {
        fprintf(stderr, "[Error] erase_done_bg_jobs: Job manager is NULL\n");
        return FALSE;
    }

    if (manager->done_bg_jobs == NULL) {
        fprintf(stderr, "[Error] erase_done_bg_jobs: No done background jobs\n");
        return FALSE;
    }

    struct job *current = manager->done_bg_jobs;
    struct job *prev = NULL;

    while (current != NULL) {
        if (current == job) {
            if (!prev)
                manager->done_bg_jobs = current->next;
            else
                prev->next = current->next;
            current->next = NULL;

            free(current->pid_list);
            free(current);
            return TRUE;
        }
        prev = current;
        current = current->next;
    }

    fprintf(stderr, "[Error] erase_done_bg_jobs: Job not found in done background jobs\n");
    return FALSE;
}
/*---------------------------------------------------------------------------*/
char erase_job_from_manager(struct job *job) {
    if (manager == NULL) {
        fprintf(stderr, "[Error] erase_job_from_manager: Job manager is NULL\n");
        return FALSE;
    }

    struct job *current = manager->jobs;
    struct job *prev = NULL;

    while (current != NULL) {
        if (current == job) {
            if (!prev)
                manager->jobs = current->next;
            else
                prev->next = current->next;
            current->next = NULL;

            free(current->pid_list);
            free(current);
            manager->n_jobs--;
            return TRUE;
        }
        prev = current;
        current = current->next;
    }

    fprintf(stderr, "[Error] erase_job_from_manager: Job not found\n");
    return FALSE;
}
/*---------------------------------------------------------------------------*/
char move_to_done_bg_jobs(struct job *job) {
    if (manager == NULL) {
        fprintf(stderr, "[Error] move_to_done_bg_jobs: Job manager is NULL\n");
        return FALSE;
    }
    
    struct job *current = manager->jobs;
    struct job *prev = NULL;
    while (current != NULL) {
        if (current == job) {
            if (!prev)
                manager->jobs = current->next;
            else
                prev->next = current->next;
            current->next = NULL;
            break;
        }
        prev = current;
        current = current->next;
    }
    if (current == NULL) {
        fprintf(stderr, "[Error] move_to_done_bg_jobs: Job not found in manager\n");
        return FALSE;
    }
    manager->n_jobs--;

    if (manager->done_bg_jobs == NULL)
        manager->done_bg_jobs = job;
    else {
        struct job *current = manager->done_bg_jobs;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = job;
    }
    return TRUE;
}
/*---------------------------------------------------------------------------*/
char erase_process_from_job(struct job *job, pid_t pid) {
    if (job == NULL) {
        fprintf(stderr, "[Error] erase_process_from_job: Job is NULL\n");
        return FALSE;
    }
    
    int idx = -1;
    for (int i = 0; i < job->total_num; i++) {
        if (job->pid_list[i] == pid) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        fprintf(stderr, "[Error] erase_process_from_job: PID not found in job\n");
        return FALSE;
    }
    job->pid_list[idx] = -1;
    job->curr_num++;

    if (job->curr_num == job->total_num) {
        job_state s = job->state;
        switch(s) {
            case background:
                if(move_to_done_bg_jobs(job) == FALSE) {
                    fprintf(stderr, "[Error] erase_process_from_job: Failed to move job to done background jobs\n");
                    return FALSE;
                }
                break;
            case foreground:
                if(erase_job_from_manager(job) == FALSE) {
                    fprintf(stderr, "[Error] erase_process_from_job: Failed to erase job from manager\n");
                    return FALSE;
                }
                break;
            default:
                fprintf(stderr, "[Error] erase_process_from_job: Unknown job state\n");
                return FALSE;
        }
    }
    return TRUE;
}
/*---------------------------------------------------------------------------*/

