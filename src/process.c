#include "process.h"
#include "memory.h"

#include <stddef.h>
#include <stdint.h>  

/* process table */
static pcb_t proctab[MAX_PROCS];

static int valid_pid(int32_t pid) {
    return (pid >= 0 && pid < MAX_PROCS);
}

static int32_t find_free_pid(void) {
    for (int32_t i = 0; i < MAX_PROCS; i++) {
        if (proctab[i].state == PR_TERMINATED) {
            return i;
        }
    }
    return -1;
}

void proc_init(void) {
    for (int i = 0; i < MAX_PROCS; i++) {
        proctab[i].pid    = i;
        proctab[i].state = PR_TERMINATED;
        proctab[i].entry  = NULL;
        proctab[i].stack_base = NULL;
        proctab[i].esp  = NULL;
        proctab[i].stack_size = 0;
        proctab[i].has_msg = 0;
    }
}
/* process creation */
int32_t proc_create(void (*func)(void)) {
    if (func == NULL) return -1;

    int32_t pid = find_free_pid();
    if (pid < 0) return -1;

    void *stack = mem_alloc(PROC_STACK_SIZE);
    if (stack == NULL) return -1;

  
    uint32_t *sp = (uint32_t *)((uintptr_t)stack + PROC_STACK_SIZE);
    sp = (uint32_t *)((uintptr_t)sp & ~0xFUL);

    proctab[pid].entry  = func;
    proctab[pid].stack_base = stack;
    proctab[pid].esp  = sp;
    proctab[pid].stack_size = PROC_STACK_SIZE;
    proctab[pid].has_msg = 0;


    proctab[pid].state = PR_NEW;

    return pid;
}

/* state transition */
int proc_set_state(int32_t pid, pr_state_t new_state) {
    if (!valid_pid(pid)) return -1;
    if (proctab[pid].state == PR_TERMINATED) return -1;

    if (new_state == PR_TERMINATED) return -1;

    proctab[pid].state = new_state;
    return 0;
}

/* process termination */
int proc_terminate(int32_t pid) {
    if (!valid_pid(pid)) return -1;

    if (proctab[pid].state == PR_TERMINATED) {
        return 0; /* already terminated */
    }

    if (proctab[pid].stack_base != NULL) {
        mem_free(proctab[pid].stack_base);
    }

    proctab[pid].entry = NULL;
    proctab[pid].stack_base = NULL;
    proctab[pid].esp = NULL;
    proctab[pid].stack_size = 0;
    proctab[pid].has_msg = 0;
    proctab[pid].state = PR_TERMINATED;

    return 0;
}

pcb_t* proc_get_pcb(int32_t pid) {
    if (!valid_pid(pid)) return NULL;
    if (proctab[pid].state == PR_TERMINATED) return NULL;
    return &proctab[pid];
}

pr_state_t proc_get_state(int32_t pid) {
    if (!valid_pid(pid)) return PR_TERMINATED;
    return proctab[pid].state;
}

int32_t proc_is_alive(int32_t pid) {
    if (!valid_pid(pid)) return 0;
    return (proctab[pid].state != PR_TERMINATED);
}

int proc_send(int32_t dst_pid, const char *msg) {
    if (!valid_pid(dst_pid)) return -1;
    if (proctab[dst_pid].state == PR_TERMINATED) return -1;

    int i = 0;
    while (msg[i] && i < IPC_MSG_SIZE - 1) {
        proctab[dst_pid].msg[i] = msg[i];
        i++;
    }
    proctab[dst_pid].msg[i] = '\0';
    proctab[dst_pid].has_msg = 1;

    return 0;
}

int proc_recv(int32_t pid, char *out) {
    if (!valid_pid(pid)) return -1;
    if (!proctab[pid].has_msg) return -1;

    int i = 0;
    while (i < IPC_MSG_SIZE) {
        out[i] = proctab[pid].msg[i];
        if (out[i] == '\0') break;
        i++;
    }

    proctab[pid].has_msg = 0;
    return 0;
}