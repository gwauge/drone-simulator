#include "blackboard.h"

int counter = 0;
Input input = {0, 0, 0, 0, 0};

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

void send_map_size(Process *process)
{
	size_t bytes_size;
	// send map size once at the beginning
	bytes_size = write(process->parent_to_child.write_fd, &COLS, sizeof(int));
	handle_pipe_write_error(bytes_size);
	bytes_size = write(process->parent_to_child.write_fd, &LINES, sizeof(int));
	handle_pipe_write_error(bytes_size);

	// read ACK from drone
	char ack_message[256];
	read_from_pipe(process->child_to_parent.read_fd, ack_message, sizeof(ack_message));
	// printf("Parent received from '%s': %s\n", process->name, ack_message);
}

void rotate_fds(int *fds, int count)
{
	if (count > 1)
	{
		int first = fds[0];
		for (int i = 0; i < count - 1; ++i)
		{
			fds[i] = fds[i + 1];
		}
		fds[count - 1] = first;
	}
}

void blackboard(Process *drone_process, Process *watchdog_process)
{
	size_t bytes_size;

	int read_fds[NUM_COMPONENTS]; // Array for read ends of the pipes
	fd_set read_set;
	int max_fd = 0;

	read_fds[0] = watchdog_process->child_to_parent.read_fd;
	read_fds[1] = drone_process->child_to_parent.read_fd;

	// find max fd
	for (int i = 0; i < NUM_COMPONENTS; i++)
	{
		if (read_fds[i] > max_fd)
		{
			max_fd = read_fds[i];
		}
	}

	// send pids to watchdog
	for (int i = 0; i < NUM_COMPONENTS; i++)
	{
		bytes_size = write(watchdog_process->parent_to_child.write_fd, &read_fds[i], sizeof(int));
		handle_pipe_write_error(bytes_size);
	}

	pid_t pid = getpid();

	int ch;
	init_ncurses();

	int active = 1; // boolean indicating if all processes are active

	Drone drone;
	Object obstacle = {COLS / 3, LINES / 3}; // Place obstacle at a fixed point

	send_map_size(drone_process);

	int counter = 0;

	while (1)
	{
		counter++;
		// printf("Counter: %d\n", counter);

		if (!active)
		{
			printf("Exiting due to inactivity of at least one process.\n");
			break;
		}

		// /*
		// handle user input
		ch = getch(); // Capture user input
		if (ch == 'q')
		{
			printf("Received Q input\n");
			break; // Exit if 'q' is pressed
		}

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
		// */

		// Send input to Drone process
		bytes_size = write(drone_process->parent_to_child.write_fd, &input, sizeof(Input));
		handle_pipe_write_error(bytes_size);

		reset_input(&input);

		// select file descriptors
		FD_ZERO(&read_set);
		// Add the read ends to the fd_set
		for (int i = 0; i < NUM_COMPONENTS; ++i)
		{
			FD_SET(read_fds[i], &read_set);
		}

		// Wait for data to be ready
		int ready_count = select(max_fd + 1, &read_set, NULL, NULL, NULL);
		handle_select_error(ready_count);

		// Check which file descriptors are ready
		for (int i = 0; i < NUM_COMPONENTS; ++i)
		{
			int fd = read_fds[i];
			if (FD_ISSET(fd, &read_set))
			{
				if (fd == drone_process->child_to_parent.read_fd)
				{
					// Read drone position
					bytes_size = read(fd, &drone, sizeof(Drone));
					handle_pipe_read_error(bytes_size);
					// printf("[drone - %d] Position (%.2f, %.2f), Velocity (%.2f, %.2f)\n",
					// 	   drone_process->pid, drone.x, drone.y, drone.vx, drone.vy);

					clear();
					display(&drone, &obstacle);
					mvprintw(0, 0, "Drone updated: Position (%.2f, %.2f), Velocity (%.2f, %.2f)\n",
							 drone.x, drone.y, drone.vx, drone.vy);
					mvprintw(1, 0, "Received char: %d\n", ch);
					mvprintw(2, 0, "PID: %d - Counter: %d\n", pid, counter);
					mvprintw(3, 0, "Active: %d\n", active);
					mvprintw(4, 0, "Last received: drone (%d)\n", drone_process->pid);
				}
				else if (fd == watchdog_process->child_to_parent.read_fd)
				{
					// Read watchdog message
					bytes_size = read(fd, &active, sizeof(active));
					printf("[watchdog - %d] %d\n", watchdog_process->pid, active);
					mvprintw(4, 0, "Last received: watchdog (%d)\n", watchdog_process->pid);
				}
			}
		}

		refresh();

		// Rotate file descriptors to ensure fairness
		rotate_fds(read_fds, NUM_COMPONENTS);

		// usleep(TIME_STEP * 1000000); // Delay to control refresh rate
	}

	endwin(); // Close ncurses mode
}
