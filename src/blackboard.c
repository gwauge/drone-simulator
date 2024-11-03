#include <ncurses.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For exit
#include <time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <types.h>
#include <constants.h>

void init_colors()
{
	/* initialize colors */
	if (has_colors() == FALSE)
	{
		endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

	start_color();
	init_pair(DRONE_PAIR, COLOR_BLUE, COLOR_BLACK);
	init_pair(OBSTACLE_PAIR, COLOR_YELLOW, COLOR_BLACK);
	init_pair(TARGET_PAIR, COLOR_GREEN, COLOR_BLACK);
}

// Function to initialize ncurses
void init_ncurses()
{
	initscr();
	init_colors();
	noecho();
	curs_set(FALSE);
	keypad(stdscr, TRUE);
	timeout(0); // Non-blocking input
}

void draw_drone(Drone *drone)
{
	attron(COLOR_PAIR(DRONE_PAIR));
	mvprintw(drone->y, drone->x, DRONE);
	attroff(COLOR_PAIR(DRONE_PAIR));
}

void draw_obstacles(Object *obstacle)
{
	attron(COLOR_PAIR(OBSTACLE_PAIR));
	mvprintw(obstacle->y, obstacle->x, OBSTACLE);
	attroff(COLOR_PAIR(OBSTACLE_PAIR));
}

// Function to display the obstacle and drone
void display(Drone *drone, Object *obstacle)
{
	clear();
	draw_drone(drone);		  // Display drone
	draw_obstacles(obstacle); // Display obstacle
	refresh();
}

void reset_input(Input *input)
{
	input->n = 0;
	input->e = 0;
	input->s = 0;
	input->w = 0;
	input->reset = 0;
}

int main()
{
	int pipe_out[2], pipe_in[2]; // pipes for communication
	pid_t pid;

	// Create pipes
	if (pipe(pipe_out) == -1 || pipe(pipe_in) == -1)
	{
		perror("Failed to create pipes");
		return 1;
	}

	// Fork a new process for the Drone Dynamics
	pid = fork();
	if (pid == -1)
	{
		perror("Fork failed");
		return 1;
	}
	else if (pid == 0)
	{
		// In child process - Drone Dynamics
		// Close unnecessary pipe ends
		close(pipe_out[1]); // Close write end of output pipe
		close(pipe_in[0]);	// Close read end of input pipe

		// Redirect pipes to standard input/output for easy reading/writing in dynamics
		dup2(pipe_out[0], STDIN_FILENO);
		dup2(pipe_in[1], STDOUT_FILENO);

		// Run the dynamics process
		execl("./build/drone", "drone", NULL);
		perror("Failed to exec drone");
		exit(1);
	}
	else
	{
		// In parent process - Blackboard Server
		// Close unnecessary pipe ends
		close(pipe_out[0]); // Close read end of output pipe
		close(pipe_in[1]);	// Close write end of input pipe

		Drone drone;
		Input input;
		Object obstacle = {COLS / 3, LINES / 3}; // Place obstacle at a fixed point

		int ch;
		init_ncurses();

		// send map size once at the beginning
		write(pipe_out[1], &COLS, sizeof(int));
		write(pipe_out[1], &LINES, sizeof(int));

		while (1)
		{
			ch = getch(); // Capture user input
			if (ch == 'q')
				break; // Exit if 'q' is pressed

			switch (ch)
			{
			// check for W keys
			case KEY_UP:
				if (drone.y > 0)
					drone.y--;
				break;
			case KEY_DOWN:
				if (drone.y < LINES - 1)
					drone.y++;
				break;
			case KEY_LEFT:
				if (drone.x > 0)
					drone.x--;
				break;
			case KEY_RIGHT:
				if (drone.x < COLS - 1)
					drone.x++;
				break;

			case 'w':
				input.n = 1;
				input.w = 1;
				break;
			case 'e':
				input.n = 1;
				break;
			case 'r':
				input.n = 1;
				input.e = 1;
				break;
			case 's':
				input.e = 1;
				break;
			case 'd':
				input.reset = 1;
				break;
			case 'f':
				input.e = 1;
				break;
			case 'x':
				input.s = 1;
				input.w = 1;
				break;
			case 'c':
				input.s = 1;
				break;
			case 'v':
				input.s = 1;
				input.e = 1;
				break;
			}

			// Send input to Drone process
			write(pipe_out[1], &input, sizeof(Input));

			// Read updated position and velocity from Drone Dynamics
			read(pipe_in[0], &drone, sizeof(Drone));

			display(&drone, &obstacle);
			// mvprintw(0, 0, "Drone updated: Position (%.2f, %.2f), Velocity (%.2f, %.2f)\n",
			// 		 drone.x, drone.y, drone.vx, drone.vy);
			refresh();

			reset_input(&input);

			usleep(TIME_STEP * 1000000); // Delay to control refresh rate
		}

		// Close pipes and wait for the child process
		close(pipe_out[1]);
		close(pipe_in[0]);
		wait(NULL); // Wait for child process to finish
	}

	return 0;
}
