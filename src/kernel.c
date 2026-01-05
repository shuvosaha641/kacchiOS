/* kernel.c - Main kernel with null process */
#include "types.h"
#include "serial.h"
#include "string.h"
#include "memory.h"
#include "process.h"

#define MAX_INPUT 128

static uint32_t uptime_ticks = 0;

/* Simple command parser */
void handle_command(char *cmd)
{
    /* help - show available commands */
    if (string_equal(cmd, "help"))
    {
        serial_puts("Available commands:\n");
        serial_puts("  help       - Show this help message\n");
        serial_puts("  clear      - Clear screen (visual only)\n");
        serial_puts("  alloc SIZE - Allocate SIZE bytes on heap\n");
        serial_puts("  free       - Show free memory info\n");
        serial_puts("  version    - Show OS version\n");
        serial_puts("  info       - Show system info\n");
        serial_puts("  ps         - Show process info\n");
        serial_puts("  uptime     - Show uptime\n");
        return;
    }

    /* clear - visual clear */
    if (string_equal(cmd, "clear"))
    {
        for (int i = 0; i < 25; i++)
        {
            serial_puts("\n");
        }
        return;
    }

    /* version - show OS info */
    if (string_equal(cmd, "version"))
    {
        serial_puts("kacchiOS v0.1.0\n");
        serial_puts("Memory: 4KB stack, 8KB heap\n");
        serial_puts("Max processes: 16\n");
        return;
    }

    /* info - show detailed system info */
    if (string_equal(cmd, "info"))
    {
        serial_puts("=== System Information ===\n");
        serial_puts("OS: kacchiOS v0.1.0\n");
        serial_puts("Arch: x86 (32-bit)\n");
        serial_puts("Boot: Multiboot\n");
        serial_puts("Stack: 4096 bytes\n");
        serial_puts("Heap: 8192 bytes\n");
        serial_puts("Max PIDs: 16\n");
        serial_puts("===========================\n");
        return;
    }

    /* uptime - show system uptime in ticks */
    if (string_equal(cmd, "uptime"))
    {
        serial_puts("Uptime: ");
        /* Simple number print */
        uint32_t ticks = uptime_ticks++;
        if (ticks == 0)
        {
            serial_puts("0 ticks\n");
        }
        else
        {
            char buf[16];
            int idx = 0;
            uint32_t temp = ticks;
            while (temp > 0)
            {
                buf[idx++] = '0' + (temp % 10);
                temp /= 10;
            }
            for (int i = idx - 1; i >= 0; i--)
            {
                serial_putc(buf[i]);
            }
            serial_puts(" ticks\n");
        }
        return;
    }

    /* free - show memory status */
    if (string_equal(cmd, "free"))
    {
        serial_puts("Memory Usage:\n");
        serial_puts("  Stack: 4096 bytes total\n");
        serial_puts("  Heap:  8192 bytes total\n");
        serial_puts("  Use 'alloc SIZE' to test allocation\n");
        return;
    }

    /* ps - process status */
    if (string_equal(cmd, "ps"))
    {
        serial_puts("Process Status:\n");
        serial_puts("  PID 0: kernel (RUNNING)\n");
        for (int i = 1; i < 5; i++)
        {
            pr_state_t state = proc_get_state(i);
            if (state != PR_TERMINATED)
            {
                serial_puts("  PID ");
                serial_putc('0' + i);
                serial_puts(": (");
                if (state == PR_NEW)
                    serial_puts("NEW");
                else if (state == PR_READY)
                    serial_puts("READY");
                else if (state == PR_RUNNING)
                    serial_puts("RUNNING");
                else if (state == PR_BLOCKED)
                    serial_puts("BLOCKED");
                else if (state == PR_SLEEPING)
                    serial_puts("SLEEPING");
                serial_puts(")\n");
            }
        }
        return;
    }

    /* alloc SIZE - test heap allocation */
    if (string_starts_with(cmd, "alloc "))
    {
        int size = 0;
        char *ptr = cmd + 6; /* Skip "alloc " */
        while (*ptr >= '0' && *ptr <= '9')
        {
            size = size * 10 + (*ptr - '0');
            ptr++;
        }
        if (size > 0)
        {
            void *mem = heap_alloc(size);
            if (mem)
            {
                serial_puts("Allocated ");
                serial_puts(cmd + 6);
                serial_puts(" bytes OK\n");
            }
            else
            {
                serial_puts("Allocation failed (out of heap)\n");
            }
        }
        else
        {
            serial_puts("Usage: alloc SIZE (e.g., alloc 256)\n");
        }
        return;
    }

    /* Unknown command */
    serial_puts("Unknown command: ");
    serial_puts(cmd);
    serial_puts("\nType 'help' for commands.\n");
}

void kmain(void)
{
    char input[MAX_INPUT];
    int pos = 0;

    /* Initialize hardware */
    serial_init();

    /* Print welcome message */
    serial_puts("\n");
    serial_puts("========================================\n");
    serial_puts("    kacchiOS - Minimal Baremetal OS\n");
    serial_puts("========================================\n");
    serial_puts("Hello from kacchiOS!\n");
    serial_puts("Running null process...\n");
    serial_puts("Type 'help' for available commands.\n\n");
    /* Initialize memory subsystem and run allocator self-test */
    memory_init();
    stress_test_memory();

    /* Initialize process manager */
    proc_init();

    /* Main loop - the "null process" */
    while (1)
    {
        serial_puts("kacchiOS> ");
        pos = 0;

        /* Read input line */
        while (1)
        {
            char c = serial_getc();

            /* Handle Enter key */
            if (c == '\r' || c == '\n')
            {
                input[pos] = '\0';
                serial_puts("\n");
                break;
            }
            /* Handle Backspace */
            else if ((c == '\b' || c == 0x7F) && pos > 0)
            {
                pos--;
                serial_puts("\b \b"); /* Erase character on screen */
            }
            /* Handle normal characters */
            else if (c >= 32 && c < 127 && pos < MAX_INPUT - 1)
            {
                input[pos++] = c;
                serial_putc(c); /* Echo character */
            }
        }

        /* Process command if not empty */
        if (pos > 0)
        {
            handle_command(input);
        }
    }

    /* Should never reach here */
    for (;;)
    {
        __asm__ volatile("hlt");
    }
}