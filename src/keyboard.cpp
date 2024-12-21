#include "keyboard.hpp"

// Global flag for graceful termination
volatile sig_atomic_t sigint_received = 0;

// Example: Terminal settings to reset on exit
struct termios original_termios;

// Custom SIGINT handler
void handle_sigint(int sig)
{
    // Set the flag to indicate SIGINT was received
    sigint_received = 1;
}

// Function to reset terminal settings
void reset_terminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
    printf("\nTerminal settings restored.\n");
}

// Function to set up raw terminal mode (if needed)
void enable_raw_mode()
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &original_termios); // Save original settings
    raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

// Function to restore terminal to normal mode
void disable_raw_mode()
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);

    tty.c_lflag |= (ICANON | ECHO); // Re-enable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void keyboard_component(int read_fd, int write_fd)
{
    register_signal_handler();
    signal(SIGINT, handle_sigint);

    ssize_t bytes_size;

    // Enable raw mode for keyboard input
    enable_raw_mode();
    printf("Keyboard input process started. Press 'q' to quit.\n");

    char input;
    while (!sigint_received)
    {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        struct timeval timeout = {0, POLL_INTERVAL}; // Set polling interval

        // Use select to wait for input or timeout
        int result = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
        if (result > 0 && FD_ISSET(STDIN_FILENO, &read_fds))
        {
            // Read a single character from stdin
            if (read(STDIN_FILENO, &input, 1) > 0)
            {
                // write the input to the pipe
                bytes_size = write(write_fd, &input, sizeof(char));
                handle_pipe_write_error(bytes_size);
            }
        }

        // Sleep for the polling interval (optional; adjust if using select timeout only)
        usleep(POLL_INTERVAL);
    }

    // Clean up
    disable_raw_mode();

    printf("Keyboard input process terminated.\n");
}
