#include <blackboard.h>

#include <ncurses.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For exit
#include <sys/wait.h>

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
	mvprintw(drone->y, drone->x, DRONE_SYMBOL);
	attroff(COLOR_PAIR(DRONE_PAIR));
}

void draw_obstacles(Object *obstacle)
{
	attron(COLOR_PAIR(OBSTACLE_PAIR));
	mvprintw(obstacle->y, obstacle->x, OBSTACLE_SYMBOL);
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

int blackboard()
{
	// create drone process
	ProcessInfo *drone_process = create_process_and_pipe("drone");
	printf("Drone process created: %s\n", drone_process->read_pipe_name);

	Drone drone;
	Input input;

	int ch;
	init_ncurses();

	Object obstacle = {COLS / 3, LINES / 3}; // Place obstacle at a fixed point

	// send map size once at the beginning
	write(drone_process->write_fd, &COLS, sizeof(int));
	write(drone_process->write_fd, &LINES, sizeof(int));

	// Read response from parent
	char msg[100];
	ssize_t bytes_read = read(drone_process->read_fd, msg, sizeof(msg));
	if (bytes_read > 0)
	{
		printf("Parent received: %s\n", msg);
	}
	else if (bytes_read == -1)
	{
		perror("read");
	}

	printf("ACK received\n");

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
		write(drone_process->write_fd, &input, sizeof(Input));

		// Read updated position and velocity from Drone Dynamics
		read(drone_process->read_fd, &drone, sizeof(Drone));

		display(&drone, &obstacle);
		mvprintw(0, 0, "Drone updated: Position (%.2f, %.2f), Velocity (%.2f, %.2f)\n",
				 drone.x, drone.y, drone.vx, drone.vy);
		refresh();

		reset_input(&input);

		usleep(TIME_STEP * 1000000); // Delay to control refresh rate
	}

	close_process_and_pipe(drone_process);
	wait(NULL); // Wait for child process to finish

	endwin(); // Close ncurses mode

	return 0;
}
