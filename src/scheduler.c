#include "scheduler.h"
#include "process.h"
#include "serial.h"

/* Currently running process ID */
static int32_t current_pid = -1;

/* ---------------------------------------------------
 * Initialize scheduler
 * --------------------------------------------------- */
void scheduler_init(void)
{
    current_pid = -1;
}

/* ---------------------------------------------------
 * Find next READY process (Round Robin)
 * --------------------------------------------------- */
static int32_t find_next_ready(void)
{
    int32_t start = current_pid;

    for (int i = 0; i < MAX_PROCS; i++)
    {
        start = (start + 1) % MAX_PROCS;

        if (proc_get_state(start) == PR_READY)
        {
            return start;
        }
    }
    return -1;
}

/* ---------------------------------------------------
 * Run scheduler loop (cooperative, no context switch)
 * --------------------------------------------------- */
void scheduler_run(void)
{
    serial_puts("\n[Scheduler] Starting Round-Robin scheduling\n");

    while (1)
    {
        int32_t next = find_next_ready();

        if (next < 0)
        {
            serial_puts("[Scheduler] No READY process. CPU idle.\n");
            break; /* Exit if no processes */
        }

        current_pid = next;
        proc_set_state(next, PR_RUNNING);

        serial_puts("[Scheduler] Running process PID ");
        serial_putc('0' + (next % 10));
        serial_puts("\n");

        /* Get PCB and run the process function */
        pcb_t *pcb = proc_get_pcb(next);
        if (pcb && pcb->entry)
        {
            pcb->entry();
        }

        /* If process returns, terminate it */
        proc_terminate(next);

        serial_puts("[Scheduler] Process PID ");
        serial_putc('0' + (next % 10));
        serial_puts(" terminated\n");
    }
}

/* ---------------------------------------------------
 * Cooperative yield (mark current as READY)
 * --------------------------------------------------- */
void scheduler_yield(void)
{
    if (current_pid >= 0 && proc_is_alive(current_pid))
    {
        proc_set_state(current_pid, PR_READY);
    }
}
