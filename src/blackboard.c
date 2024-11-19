#include "blackboard.h"

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

void draw_obstacles(Obstacle *obstacles)
{
	attron(COLOR_PAIR(OBSTACLE_PAIR));
	for (int i = 0; i < NUM_OBSTACLES; i++)
	{
		if (obstacles[i].lifetime != OBSTACLE_UNSET)
		{
			mvprintw(obstacles[i].y, obstacles[i].x, OBSTACLE_SYMBOL);
		}
	}
	attroff(COLOR_PAIR(OBSTACLE_PAIR));
}

// Function to display the obstacle and drone
void display(WorldState *world_state)
{
	clear();
	draw_drone(&world_state->drone);		// Display drone
	draw_obstacles(world_state->obstacles); // Display obstacles
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

void blackboard(Process *watchdog_process, Process *drone_process, Process *obstacle_process)
{
	size_t bytes_size;

	int read_fds[NUM_COMPONENTS]; // Array for read ends of the pipes
	fd_set read_set;
	int max_fd = 0;

	read_fds[0] = watchdog_process->child_to_parent.read_fd;
	read_fds[1] = drone_process->child_to_parent.read_fd;
	read_fds[2] = obstacle_process->child_to_parent.read_fd;

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

	WorldState world_state;
	reset_input(&world_state.input);

	send_map_size(drone_process);
	send_map_size(obstacle_process);

	int counter = 0;

	while (1)
	{
		counter++;

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
		case 'w':
			world_state.input.n = 1;
			world_state.input.w = 1;
			break;
		case 'e':
			world_state.input.n = 1;
			break;
		case 'r':
			world_state.input.n = 1;
			world_state.input.e = 1;
			break;
		case 's':
			world_state.input.e = 1;
			break;
		case 'd':
			world_state.input.reset = 1;
			break;
		case 'f':
			world_state.input.e = 1;
			break;
		case 'x':
			world_state.input.s = 1;
			world_state.input.w = 1;
			break;
		case 'c':
			world_state.input.s = 1;
			break;
		case 'v':
			world_state.input.s = 1;
			world_state.input.e = 1;
			break;
		}
		// */

		// Send input to Drone process
		bytes_size = write(drone_process->parent_to_child.write_fd, &world_state, sizeof(WorldState));
		handle_pipe_write_error(bytes_size);

		reset_input(&world_state.input);

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
					bytes_size = read(fd, &world_state.drone, sizeof(Drone));
					handle_pipe_read_error(bytes_size);

					clear();
					display(&world_state);
					mvprintw(0, 0, "Drone updated: Position (%.2f, %.2f), Velocity (%.2f, %.2f)\n",
							 world_state.drone.x, world_state.drone.y, world_state.drone.vx, world_state.drone.vy);
					mvprintw(1, 0, "Received char: %d\n", ch);
					mvprintw(2, 0, "PID: %d - Counter: %d\n", pid, counter);
					mvprintw(3, 0, "Active: %d\n", active);
					mvprintw(4, 0, "Last received: drone (%d)\n", drone_process->pid);
				}
				else if (fd == watchdog_process->child_to_parent.read_fd)
				{
					// Read watchdog message
					bytes_size = read(fd, &active, sizeof(active));
					mvprintw(4, 0, "Last received: watchdog (%d)\n", watchdog_process->pid);
				}
				else if (fd == obstacle_process->child_to_parent.read_fd)
				{
					bytes_size = read(fd, &world_state.obstacles, sizeof(Obstacle) * NUM_OBSTACLES);
					handle_pipe_read_error(bytes_size);
					mvprintw(4, 0, "Last received: obstacles (%d)\n", obstacle_process->pid);
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
