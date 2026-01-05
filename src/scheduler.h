#ifndef KACCHI_SCHEDULER_H
#define KACCHI_SCHEDULER_H

/* Initialize scheduler */
void scheduler_init(void);

/* Pick next process and run it */
void scheduler_run(void);

/* Yield CPU voluntarily */
void scheduler_yield(void);

#endif
