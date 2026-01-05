#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

/* Process Manager Config */
#define MAX_PROCS        16
#define PROC_STACK_SIZE  4096

/* process states */
typedef enum {
    PR_TERMINATED = 0,  /* free/empty slot */
    PR_NEW,  /* created but not scheduled */
    PR_READY,  /* waiting for CPU */
    PR_RUNNING  /* currently running*/
} pr_state_t;

/*process control block */
typedef struct {
    int32_t     pid;
    pr_state_t  state;
    void      (*entry)(void);

    void       *stack_base;
    uint32_t   *esp;
    uint32_t    stack_size;
} pcb_t;


void    proc_init(void);
int32_t proc_create(void (*func)(void));

/* state transition */
int     proc_set_state(int32_t pid, pr_state_t new_state);

/* terminate + cleanup */
int     proc_terminate(int32_t pid);

/* NULL if invalid/terminated */
pcb_t*  proc_get_pcb(int32_t pid);

#endif
