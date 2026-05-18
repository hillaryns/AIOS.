#include "include/shell.h"
#include "include/vga.h"
#include "include/pit.h"
#include "pmm.h"

#include <stdint.h>

#define SHELL_BUFFER_SIZE 256

static char shell_buffer[SHELL_BUFFER_SIZE];
static int shell_index = 0;

static uint64_t last_page = 0;

/* -------------------------------------------------- */

static int strcmp(const char *a, const char *b)
{
    while (*a && *b) {

        if (*a != *b)
            return 0;

        a++;
        b++;
    }

    return (*a == 0 && *b == 0);
}

/* -------------------------------------------------- */

static void shell_prompt(void)
{
    vga_puts("\nAIOS > ");
}

/* -------------------------------------------------- */

static void shell_execute(void)
{
    shell_buffer[shell_index] = 0;

    if (strcmp(shell_buffer, "help")) {

        vga_puts(
            "\nCommands:\n"
            "help\n"
            "clear\n"
            "uptime\n"
            "meminfo\n"
	    "allocpage\n"
	    "freepage\n"
            "echo\n"
	    "crash\n"
	    "pagefault\n"
        );
    }

    else if (strcmp(shell_buffer, "clear")) {

        vga_clear();

    }

    else if (strcmp(shell_buffer, "uptime")) {

        vga_puts("\nUptime(ms): ");
        vga_putdec((uint32_t)pit_get_ms());
    }

   else if (strcmp(shell_buffer, "crash")) {

        __asm__ volatile ("int $0");

  }

  else if (strcmp(shell_buffer, "pagefault")) {

       volatile uint64_t *ptr =
          (uint64_t*)0xFFFFFFFFFFFFULL;

       *ptr = 123;
  }

    else if (strcmp(shell_buffer, "meminfo")) {

       uint64_t free_pages = pmm_get_free_pages();

       uint64_t free_mb =
           (free_pages * PAGE_SIZE) / 1024 / 1024;

       vga_puts("\nFree memory: ");

       vga_putdec((uint32_t)free_mb);

       vga_puts(" MB");
    }

   else if (strcmp(shell_buffer, "allocpage")) {

      last_page = pmm_alloc_page();

      vga_puts("\nAllocated page: ");

      vga_puthex(last_page);
   }

   else if (strcmp(shell_buffer, "freepage")) {

        if (last_page) {

            pmm_free_page(last_page);

            vga_puts("\nPage freed");

            last_page = 0;

        } else {

            vga_puts("\nNo page allocated");
        }
    }

    else if (shell_buffer[0] == 'e' &&
             shell_buffer[1] == 'c' &&
             shell_buffer[2] == 'h' &&
             shell_buffer[3] == 'o' &&
             shell_buffer[4] == ' ') {

        vga_puts("\n");
        vga_puts(&shell_buffer[5]);
    }

    else {

        vga_puts("\nUnknown command");
    }

    shell_index = 0;

    shell_prompt();
}

/* -------------------------------------------------- */

void shell_input(char c)
{
    if (c == '\n') {

        shell_execute();
        return;
    }

    if (c == '\b') {

        if (shell_index > 0) {

            shell_index--;
            vga_backspace();
        }

        return;
    }

    if (shell_index >= SHELL_BUFFER_SIZE - 1)
        return;

    shell_buffer[shell_index++] = c;

    char s[2] = { c, 0 };

    vga_puts(s);
}

/* -------------------------------------------------- */

void shell_init(void)
{
    vga_puts(
        "\nAIOS v0.1\n"
        "Type 'help' for commands\n"
    );

    shell_prompt();
}
