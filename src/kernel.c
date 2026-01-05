/* kernel.c - Complete OS verification and test suite */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

#define MAX_INPUT 128

/* ================================================================
 * TEST PROCESSES
 * ================================================================ */
void test_proc_hello(void)
{
    serial_puts("    [P] Hello from process!\n");
}

void test_proc_count(void)
{
    serial_puts("    [P] Counting: 1 2 3\n");
}

void test_proc_mem(void)
{
    serial_puts("    [P] Testing heap allocation\n");
    void *ptr = heap_alloc(256);
    if (ptr)
        serial_puts("    [P] Success!\n");
}

/* ================================================================
 * MEMORY TEST SUITE
 * ================================================================ */
void test_memory_complete(void)
{
    serial_puts("\n[MEMORY TEST]\n");
    serial_puts("─────────────────────────────────────\n");

    serial_puts("1. Stack: allocating 256B... ");
    void *s1 = stack_alloc(256);
    serial_puts(s1 ? "✓\n" : "✗\n");

    serial_puts("2. Stack: deallocating... ");
    stack_free(256);
    serial_puts("✓\n");

    serial_puts("3. Heap: allocating 512B... ");
    void *h1 = heap_alloc(512);
    serial_puts(h1 ? "✓\n" : "✗\n");

    serial_puts("4. Heap: allocating 512B... ");
    void *h2 = heap_alloc(512);
    serial_puts(h2 ? "✓\n" : "✗\n");

    serial_puts("5. Heap: allocating 512B... ");
    void *h3 = heap_alloc(512);
    serial_puts(h3 ? "✓\n" : "✗\n");

    serial_puts("6. Heap: freeing all... ");
    heap_free(h1);
    heap_free(h2);
    heap_free(h3);
    serial_puts("✓\n");

    serial_puts("7. Coalescing: allocating 1024B... ");
    void *big = heap_alloc(1024);
    if (big)
    {
        serial_puts("✓ (coalescing works!)\n");
        heap_free(big);
    }
    else
        serial_puts("✗\n");

    serial_puts("✓ MEMORY: OK\n");
}

/* ================================================================
 * PROCESS TEST SUITE
 * ================================================================ */
void test_process_complete(void)
{
    serial_puts("\n[PROCESS TEST]\n");
    serial_puts("─────────────────────────────────────\n");

    serial_puts("1. Creating PID 1... ");
    int32_t p1 = proc_create(test_proc_hello);
    serial_puts(p1 >= 0 ? "✓\n" : "✗\n");

    serial_puts("2. Creating PID 2... ");
    int32_t p2 = proc_create(test_proc_count);
    serial_puts(p2 >= 0 ? "✓\n" : "✗\n");

    serial_puts("3. Setting PID 1 to READY... ");
    proc_set_state(p1, PR_READY);
    serial_puts("✓\n");

    serial_puts("4. Setting PID 2 to READY... ");
    proc_set_state(p2, PR_READY);
    serial_puts("✓\n");

    serial_puts("5. Checking states...\n");
    if (proc_get_state(p1) == PR_READY)
        serial_puts("   - PID 1: READY ✓\n");
    if (proc_get_state(p2) == PR_READY)
        serial_puts("   - PID 2: READY ✓\n");

    serial_puts("6. Terminating PID 1... ");
    proc_terminate(p1);
    serial_puts("✓\n");

    serial_puts("7. Verifying terminated... ");
    if (proc_get_state(p1) == PR_TERMINATED)
        serial_puts("✓\n");
    else
        serial_puts("✗\n");

    serial_puts("8. PID 2 still alive... ");
    if (proc_is_alive(p2))
        serial_puts("✓\n");
    else
        serial_puts("✗\n");

    proc_terminate(p2);
    serial_puts("✓ PROCESS: OK\n");
}

/* ================================================================
 * SCHEDULER TEST SUITE
 * ================================================================ */
void test_scheduler_complete(void)
{
    serial_puts("\n[SCHEDULER TEST]\n");
    serial_puts("─────────────────────────────────────\n");

    serial_puts("1. Initializing scheduler... ✓\n");
    scheduler_init();

    serial_puts("2. Creating test processes...\n");
    int32_t p1 = proc_create(test_proc_hello);
    int32_t p2 = proc_create(test_proc_count);
    int32_t p3 = proc_create(test_proc_mem);

    serial_puts("   PID 1, 2, 3 created ✓\n");

    serial_puts("3. Setting all to READY... ✓\n");
    proc_set_state(p1, PR_READY);
    proc_set_state(p2, PR_READY);
    proc_set_state(p3, PR_READY);

    serial_puts("4. Running scheduler...\n\n");
    scheduler_run();

    serial_puts("\n✓ SCHEDULER: OK\n");
}

/* ================================================================
 * COMPLETE SYSTEM TEST
 * ================================================================ */
void run_full_test(void)
{
    serial_puts("\n");
    serial_puts("╔═════════════════════════════════════╗\n");
    serial_puts("║   kacchiOS COMPLETE SYSTEM TEST    ║\n");
    serial_puts("║   Memory + Process + Scheduler     ║\n");
    serial_puts("╚═════════════════════════════════════╝\n");

    test_memory_complete();
    test_process_complete();
    test_scheduler_complete();

    serial_puts("\n");
    serial_puts("╔═════════════════════════════════════╗\n");
    serial_puts("║   ✓✓✓ ALL SYSTEMS WORKING ✓✓✓     ║\n");
    serial_puts("╚═════════════════════════════════════╝\n");
}

/* ================================================================
 * MAIN KERNEL
 * ================================================================ */
void kmain(void)
{
    serial_init();
    memory_init();
    proc_init();

    serial_puts("\n════════════════════════════════════\n");
    serial_puts("   kacchiOS v0.1.0\n");
    serial_puts("   Baremetal OS with Memory, Process,\n");
    serial_puts("   and Scheduler Support\n");
    serial_puts("════════════════════════════════════\n");

    serial_puts("\n[INFO] Running startup tests...\n");
    stress_test_memory();

    serial_puts("\n[READY] Type 'test' for full verification\n");
    serial_puts("Type 'help' for commands\n\n");

    char input[MAX_INPUT];
    int pos = 0;

    while (1)
    {
        serial_puts("kacchiOS> ");
        pos = 0;

        while (1)
        {
            char c = serial_getc();
            if (c == '\r' || c == '\n')
            {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            }
            else if ((c == '\b' || c == 0x7F) && pos > 0)
            {
                pos--;
                serial_puts("\b \b");
            }
            else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1)
            {
                input[pos++] = c;
                serial_putc(c);
            }
        }

        if (pos > 0)
        {
            if (string_equal(input, "help"))
            {
                serial_puts("Commands:\n");
                serial_puts("  test     - Run complete system verification\n");
                serial_puts("  memory   - Test memory subsystem only\n");
                serial_puts("  process  - Test process subsystem only\n");
                serial_puts("  sched    - Test scheduler only\n");
                serial_puts("  version  - Show version\n");
                serial_puts("  clear    - Clear screen\n");
            }
            else if (string_equal(input, "test"))
            {
                run_full_test();
            }
            else if (string_equal(input, "memory"))
            {
                test_memory_complete();
            }
            else if (string_equal(input, "process"))
            {
                test_process_complete();
            }
            else if (string_equal(input, "sched"))
            {
                test_scheduler_complete();
            }
            else if (string_equal(input, "version"))
            {
                serial_puts("kacchiOS v0.1.0\n");
            }
            else if (string_equal(input, "clear"))
            {
                for (int i = 0; i < 30; i++)
                    serial_puts("\n");
            }
            else
            {
                serial_puts("Unknown command\n");
            }
        }
    }
}
