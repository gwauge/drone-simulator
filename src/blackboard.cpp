#include "blackboard.hpp"

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

void draw_drone(WINDOW *win, Drone *drone)
{
	wattron(win, COLOR_PAIR(DRONE_PAIR));
	mvwprintw(win, drone->y, drone->x, &global_params.drone_symbol);
	wattroff(win, COLOR_PAIR(DRONE_PAIR));
}

void draw_obstacles(WINDOW *win, Obstacle *obstacles)
{
	wattron(win, COLOR_PAIR(OBSTACLE_PAIR));
	for (int i = 0; i < global_params.num_obstacles; i++)
	{
		if (obstacles[i].lifetime != OBSTACLE_UNSET)
		{
			mvwprintw(win, obstacles[i].y, obstacles[i].x, &global_params.obstacle_symbol);
		}
	}
	wattroff(win, COLOR_PAIR(OBSTACLE_PAIR));
}

void draw_targets(WINDOW *win, Target *targets)
{
	wattron(win, COLOR_PAIR(TARGET_PAIR));
	for (int i = 0; i < global_params.num_targets; i++)
	{
		if (targets[i].number != TARGET_UNSET)
		{
			mvwprintw(win, targets[i].y, targets[i].x, "%d", targets[i].number);
		}
	}
	wattroff(win, COLOR_PAIR(TARGET_PAIR));
}

// Function to display the obstacle and drone
void display(WINDOW *win, WorldState *world_state)
{
	wclear(win);
	draw_drone(win, &world_state->drone);		 // Display drone
	draw_obstacles(win, world_state->obstacles); // Display obstacles
	draw_targets(win, world_state->targets);	 // Display targets
	wrefresh(win);
}

void reset_input(Input *input)
{
	input->n = 0;
	input->e = 0;
	input->s = 0;
	input->w = 0;
	input->reset = 0;
}

void send_map_size(Process *process, int LINES, int COLS)
{
	size_t bytes_size;
	// send map size once at the beginning
	bytes_size = write(process->parent_to_child.write_fd, &COLS, sizeof(int));
	handle_pipe_write_error(bytes_size);
	bytes_size = write(process->parent_to_child.write_fd, &LINES, sizeof(int));
	handle_pipe_write_error(bytes_size);
}

double compute_score(
	double time_elapsed,
	int targets_reached,
	int obstacles_encountered,
	double distance_traveled)
{
	// Define weights for each factor
	const double time_weight = 0.1;
	const double targets_weight = 10.0;
	const double obstacles_weight = -5.0;
	const double distance_weight = 0.5;
	const double penalty_weight = -20.0;

	// Compute the score
	double score = 0.0;
	score += time_weight * time_elapsed;
	score += targets_weight * targets_reached;
	score += obstacles_weight * obstacles_encountered;
	score += distance_weight * distance_traveled;

	// Add penalties if any (example: if obstacles_encountered > 5, apply a penalty)
	if (obstacles_encountered > 5)
	{
		score += penalty_weight;
	}

	return score;
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

void blackboard(
	Process *watchdog_process,
	Process *drone_process,
	Process *obstacle_process,
	Process *targets_process,
	Process *keyboard_process)
{
	size_t bytes_size;

	int read_fds[NUM_COMPONENTS]; // Array for read ends of the pipes
	fd_set read_set;
	int max_fd = 0;

	read_fds[0] = watchdog_process->child_to_parent.read_fd;
	read_fds[1] = drone_process->child_to_parent.read_fd;
	read_fds[2] = obstacle_process->child_to_parent.read_fd;
	read_fds[3] = targets_process->child_to_parent.read_fd;
	read_fds[4] = keyboard_process->child_to_parent.read_fd;

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
	if (global_params.debug)
	{
		COLS = 157;
		LINES = 39;
	}
	else
	{
		init_ncurses();
	}

	int height = LINES;
	int width = COLS;

	int inspection_win_height = 5;
	int main_win_height = height - inspection_win_height;
	int main_win_width = width;

	WINDOW *main_win = newwin(main_win_height, main_win_width, 0, 0);
	WINDOW *inspection_win = newwin(inspection_win_height, width, height - inspection_win_height, 0);

	int active = 1; // boolean indicating if all processes are active

	Drone old_drone;
	WorldState world_state;
	reset_input(&world_state.input);

	send_map_size(drone_process, main_win_height, main_win_width);
	send_map_size(obstacle_process, main_win_height, main_win_width);
	send_map_size(targets_process, main_win_height, main_win_width);

	int counter = 0;
	int collision_counter = 0;
	double distance_traveled = 0;

	struct timeval timeout;

	while (1)
	{
		counter++;

		if (!active)
		{
			printf("Exiting due to inactivity of at least one process.\n");
			break;
		}

		// Send input to Drone process
		bytes_size = write(drone_process->parent_to_child.write_fd, &world_state, sizeof(WorldState));
		handle_pipe_write_error(bytes_size);

		reset_input(&world_state.input);

		timeout.tv_sec = 0;
		timeout.tv_usec = 10000; // 10ms

		// select file descriptors
		FD_ZERO(&read_set);
		// Add the read ends to the fd_set
		for (int i = 0; i < NUM_COMPONENTS; ++i)
		{
			FD_SET(read_fds[i], &read_set);
		}

		if (!global_params.debug)
			clear(); // Clear the screen

		// Wait for data to be ready
		int ready_count = select(max_fd + 1, &read_set, NULL, NULL, &timeout);
		handle_select_error(ready_count);

		// Check which file descriptors are ready
		for (int i = 0; i < NUM_COMPONENTS; ++i)
		{
			int fd = read_fds[i];
			if (FD_ISSET(fd, &read_set))
			{
				if (fd == drone_process->child_to_parent.read_fd)
				{
					// save old drone state
					old_drone = world_state.drone;

					// Read drone position
					bytes_size = read(fd, &world_state.drone, sizeof(Drone));
					handle_pipe_read_error(bytes_size);

					// calculate distance traveled
					distance_traveled += sqrt(pow(world_state.drone.x - old_drone.x, 2) + pow(world_state.drone.y - old_drone.y, 2));

					// check for target collisions
					int collision_idx = -1;
					for (int i = 0; i < global_params.num_targets; i++)
					{
						if (world_state.targets[i].number != TARGET_UNSET)
						{
							if (
								(int)world_state.drone.x == world_state.targets[i].x &&
								(int)world_state.drone.y == world_state.targets[i].y)
							{
								collision_idx = i;
								collision_counter++;
								break;
							}
						}
					}

					if (global_params.debug)
					{
						printf("[drone] updated: Position (%.2f, %.2f), Velocity (%.2f, %.2f)\n",
							   world_state.drone.x, world_state.drone.y, world_state.drone.vx, world_state.drone.vy);
						if (collision_idx >= 0)
						{
							printf("\tCollision detected with target %d\n", collision_idx);
						}
					}

					if (collision_idx >= 0)
					{
						// Send collision index to targets component
						bytes_size = write(targets_process->parent_to_child.write_fd, &collision_idx, sizeof(int));
						handle_pipe_write_error(bytes_size);
					}
				}
				else if (fd == watchdog_process->child_to_parent.read_fd)
				{
					// Read watchdog message
					bytes_size = read(fd, &active, sizeof(active));
					handle_pipe_read_error(bytes_size);

					if (global_params.debug)
						printf("[watchdog] active: %d\n", active);
				}
				else if (fd == obstacle_process->child_to_parent.read_fd)
				{
					bytes_size = read(fd, &world_state.obstacles, sizeof(Obstacle) * global_params.num_obstacles);
					handle_pipe_read_error(bytes_size);

					if (global_params.debug)
						printf("[obstacles] updated\n");
				}
				else if (fd == targets_process->child_to_parent.read_fd)
				{
					bytes_size = read(fd, &world_state.targets, sizeof(Target) * global_params.num_targets);
					handle_pipe_read_error(bytes_size);

					if (global_params.debug)
						printf("[targets] updated\n");
				}
				else if (fd == keyboard_process->child_to_parent.read_fd)
				{
					// Read keyboard input
					bytes_size = read(fd, &ch, sizeof(ch));
					handle_pipe_read_error(bytes_size);

					if (global_params.debug)
						printf("[keyboard] Received input: %c\n", ch);

					if (ch == 'q')
					{
						active = 0; // Exit the main loop
					}
					else if (ch == 'o')
					{
						printf("Terminating obstacles process %d\n", obstacle_process->pid);
						kill(obstacle_process->pid, SIGINT);
					}

					// handle user input
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
						world_state.input.w = 1;
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
				}
			}
		}

		if (!global_params.debug)
		{
			// update inspection window
			wclear(inspection_win);

			// print line of '=' to visually separate the main window from the inspection window
			for (int i = 0; i < width - 1; i++)
			{
				wprintw(inspection_win, "=");
			}
			wprintw(inspection_win, "\n");

			wprintw(inspection_win, "Main window size: %d x %d\n", main_win_height, main_win_width);
			wprintw(inspection_win, "[drone] updated: Position (%.2f, %.2f), Velocity (%.2f, %.2f)\n",
					world_state.drone.x, world_state.drone.y, world_state.drone.vx, world_state.drone.vy);
			double score = compute_score(counter, collision_counter, 0, distance_traveled);
			wprintw(inspection_win, "Score: %.2f\n", score);
			wrefresh(inspection_win);

			display(main_win, &world_state);
		}

		// Rotate file descriptors to ensure fairness
		rotate_fds(read_fds, NUM_COMPONENTS);
	}

	if (!global_params.debug)
	{
		delwin(main_win);
		delwin(inspection_win);
		endwin(); // Close ncurses mode
	}
}
