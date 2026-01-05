#include "scheduler.h"
#include "process.h"
#include "io.h"

/* Currently running process ID */
int current_pid = -1;

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
static int find_next_ready(void)
{
    int start = current_pid;

    for (int i = 0; i < MAX_PROCESS; i++)
    {
        start = (start + 1) % MAX_PROCESS;

        if (process_table[start].state == PROC_READY)
        {
            return start;
        }
    }
    return -1;
}

/* ---------------------------------------------------
 * Run scheduler loop
 * --------------------------------------------------- */
void scheduler_run(void)
{
    serial_puts("\n[Scheduler] Starting Round-Robin scheduling\n");

    while (1)
    {
        int next = find_next_ready();

        if (next < 0)
        {
            serial_puts("[Scheduler] No READY process. CPU idle.\n");
            continue;
        }

        current_pid = next;
        process_table[next].state = PROC_RUNNING;

        serial_puts("[Scheduler] Running process ");
        serial_putint(next);
        serial_puts("\n");

        /* Run the process function */
        process_table[next].entry();

        /* If process returns, it is terminated */
        process_table[next].state = PROC_TERMINATED;

        serial_puts("[Scheduler] Process ");
        serial_putint(next);
        serial_puts(" terminated\n");
    }
}

/* ---------------------------------------------------
 * Cooperative yield (optional)
 * --------------------------------------------------- */
void scheduler_yield(void)
{
    if (current_pid >= 0)
    {
        process_table[current_pid].state = PROC_READY;
    }
}
