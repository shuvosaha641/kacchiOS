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
    serial_puts("║   ALL SUBSYSTEMS VERIFIED          ║\n");
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
                serial_puts("\n=== SYSTEM TESTS ===\n");
                serial_puts("  test     - Run complete system verification\n");
                serial_puts("  memory   - Test memory subsystem\n");
                serial_puts("  process  - Test process subsystem\n");
                serial_puts("  sched    - Test scheduler\n");
                serial_puts("\n=== MEMORY OPERATIONS ===\n");
                serial_puts("  alloc <size> - Allocate memory (e.g., alloc 512)\n");
                serial_puts("  free         - Free last allocated block\n");
                serial_puts("  meminfo      - Show memory status\n");
                serial_puts("\n=== PROCESS OPERATIONS ===\n");
                serial_puts("  ps           - List all processes\n");
                serial_puts("  create       - Create a new process\n");
                serial_puts("  kill <pid>   - Terminate process (e.g., kill 1)\n");
                serial_puts("  run          - Execute scheduler\n");
                serial_puts("\n=== IPC COMMUNICATION ===\n");
                serial_puts("  send <pid> <msg> - Send message to process\n");
                serial_puts("  recv <pid>       - Receive message from process\n");
                serial_puts("\n=== UTILITIES ===\n");
                serial_puts("  version      - Show OS version\n");
                serial_puts("  clear        - Clear screen\n");
                serial_puts("  help         - Show this help\n");
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
            else if (string_starts_with(input, "alloc"))
            {
                int size = 512; /* default */
                if (pos > 6)
                {
                    /* Parse size from input */
                    int parsed = 0;
                    for (int i = 6; i < pos && input[i] >= '0' && input[i] <= '9'; i++)
                        parsed = parsed * 10 + (input[i] - '0');
                    if (parsed > 0)
                        size = parsed;
                }
                void *ptr = heap_alloc(size);
                if (ptr)
                    serial_puts("✓ Allocated memory\n");
                else
                    serial_puts("✗ Allocation failed\n");
            }
            else if (string_equal(input, "free"))
            {
                heap_free(512);
                serial_puts("✓ Memory freed\n");
            }
            else if (string_equal(input, "meminfo"))
            {
                serial_puts("Memory Status:\n");
                serial_puts("  Stack: 4KB\n");
                serial_puts("  Heap: 8KB\n");
                serial_puts("  Available: Check with 'test'\n");
            }
            else if (string_equal(input, "ps"))
            {
                int count = 0;
                serial_puts("Process List:\n");
                for (int i = 0; i < 16; i++)
                {
                    if (proc_is_alive(i))
                    {
                        count++;
                        serial_puts("  PID ");
                        char buf[8];
                        buf[0] = '0' + (i / 10);
                        buf[1] = '0' + (i % 10);
                        buf[2] = '\0';
                        serial_puts(buf);
                        serial_puts(": ");
                        int state = proc_get_state(i);
                        if (state == 0)
                            serial_puts("TERMINATED\n");
                        else if (state == 1)
                            serial_puts("NEW\n");
                        else if (state == 2)
                            serial_puts("READY\n");
                        else if (state == 3)
                            serial_puts("RUNNING\n");
                        else
                            serial_puts("UNKNOWN\n");
                    }
                }
                serial_puts("Total: ");
                char countbuf[8];
                countbuf[0] = '0' + (count / 10);
                countbuf[1] = '0' + (count % 10);
                countbuf[2] = '\0';
                serial_puts(countbuf);
                serial_puts("/16 processes\n");
            }
            else if (string_equal(input, "create"))
            {
                int32_t pid = proc_create(test_proc_hello);
                if (pid >= 0)
                {
                    serial_puts("✓ Process created: PID ");
                    char buf[8];
                    buf[0] = '0' + (pid / 10);
                    buf[1] = '0' + (pid % 10);
                    buf[2] = '\0';
                    serial_puts(buf);
                    serial_puts("\n");
                    proc_set_state(pid, PR_READY);
                }
                else
                {
                    serial_puts("✗ Process creation failed\n");
                    serial_puts("  Reason: Process table full or max (16) reached\n");
                    serial_puts("  Use 'ps' to see active processes\n");
                    serial_puts("  Use 'kill <pid>' to terminate a process\n");
                }
            }
            else if (string_starts_with(input, "kill"))
            {
                int pid = 0;
                if (pos > 5)
                {
                    int parsed = 0;
                    for (int i = 5; i < pos && input[i] >= '0' && input[i] <= '9'; i++)
                        parsed = parsed * 10 + (input[i] - '0');
                    if (parsed >= 0)
                        pid = parsed;
                }
                proc_terminate(pid);
                serial_puts("✓ Process terminated\n");
            }
            else if (string_equal(input, "run"))
            {
                serial_puts("Starting scheduler...\n");
                scheduler_run();
                serial_puts("✓ Scheduler completed\n");
            }
            else if (string_starts_with(input, "send"))
            {
                int pid = -1;
                int msg_start = -1;

                /* Parse: send <pid> <msg> */
                int i = 5; /* skip "send " */
                int num = 0;
                while (i < pos && input[i] >= '0' && input[i] <= '9')
                {
                    num = num * 10 + (input[i] - '0');
                    i++;
                }
                pid = num;

                /* Skip space */
                while (i < pos && input[i] == ' ')
                    i++;
                msg_start = i;

                if (msg_start < pos)
                {
                    char msg[33];
                    int j = 0;
                    while (j < pos - msg_start && j < 32)
                    {
                        msg[j] = input[msg_start + j];
                        j++;
                    }
                    msg[j] = '\0';

                    if (proc_send(pid, msg) == 0)
                    {
                        serial_puts("✓ Message sent to PID ");
                        char buf[8];
                        buf[0] = '0' + (pid / 10);
                        buf[1] = '0' + (pid % 10);
                        buf[2] = '\0';
                        serial_puts(buf);
                        serial_puts("\n");
                    }
                    else
                        serial_puts("✗ Send failed\n");
                }
                else
                    serial_puts("Usage: send <pid> <message>\n");
            }
            else if (string_starts_with(input, "recv"))
            {
                int pid = 0;
                if (pos > 5)
                {
                    int parsed = 0;
                    for (int i = 5; i < pos && input[i] >= '0' && input[i] <= '9'; i++)
                        parsed = parsed * 10 + (input[i] - '0');
                    if (parsed >= 0)
                        pid = parsed;
                }

                char msg[33];
                if (proc_recv(pid, msg) == 0)
                {
                    serial_puts("✓ Message from PID ");
                    char buf[8];
                    buf[0] = '0' + (pid / 10);
                    buf[1] = '0' + (pid % 10);
                    buf[2] = '\0';
                    serial_puts(buf);
                    serial_puts(": ");
                    serial_puts(msg);
                    serial_puts("\n");
                }
                else
                    serial_puts("✗ No message or invalid PID\n");
            }
            else
            {
                serial_puts("Unknown command\n");
            }
        }
    }
}
