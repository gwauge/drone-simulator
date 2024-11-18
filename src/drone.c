#include <sys/select.h>
#include "drone.h"
#include "pipes.h"

void update_drone_position(Drone *drone, float fx, float fy)
{
	// Update velocities based on force and damping
	drone->vx += (fx - DAMPING * drone->vx) / MASS * TIME_STEP;
	drone->vy += (fy - DAMPING * drone->vy) / MASS * TIME_STEP;

	// Update positions based on velocity
	drone->x += drone->vx * TIME_STEP;
	drone->y += drone->vy * TIME_STEP;
}

float calculate_repulsive_force(Object *obstacle, Drone *drone)
{
	// Calculate distance to obstacle
	float dx = drone->x - obstacle->x;
	float dy = drone->y - obstacle->y;
	float distance = sqrt(dx * dx + dy * dy);

	// Apply repulsion only within a certain radius
	if (distance < REPULSION_RADIUS && distance > 0)
	{
		float force_magnitude = ETA * (1.0 / distance - 1.0 / REPULSION_RADIUS) / (distance * distance);
		return force_magnitude;
	}
	return 0.0;
}

void drone_component(int read_fd, int write_fd)
{
	printf("Drone component - PID: %d\n", getpid());

	fd_set readfds;
	struct timeval timeout;

	ssize_t bytes_size;

	// Get terminal size
	int COLS, LINES;
	bytes_size = read(read_fd, &COLS, sizeof(int));
	handle_pipe_read_error(bytes_size);

	bytes_size = read(read_fd, &LINES, sizeof(int));
	handle_pipe_read_error(bytes_size);

	bytes_size = write(write_fd, "ACK", 3 + 1);
	handle_pipe_write_error(bytes_size);

	Drone drone = {10.0, 10.0, 0.0, 0.0};	 // Initialize drone at origin with zero velocity
	Input input = {0, 0, 0, 0, 0};			 // Initialize input commands
	Object obstacle = {COLS / 3, LINES / 3}; // Place obstacle at a fixed point
	float force_x = 0.0, force_y = 0.0;

	int received = 0;
	while (1)
	{
		received = 0;

		FD_ZERO(&readfds);
		FD_SET(read_fd, &readfds);

		timeout.tv_sec = 0; // Wait for up to 5 seconds
		timeout.tv_usec = 0;

		// Reset forces
		force_x = 0.0;
		force_y = 0.0;

		int result = select(read_fd + 1, &readfds, NULL, NULL, &timeout);
		if (result == -1)
		{
			perror("select");
			exit(EXIT_FAILURE);
		}
		else if (result == 0)
		{
			// printf("Drone received: no input\n");
		}
		else if (result > 0)
		{
			if (FD_ISSET(read_fd, &readfds))
			{
				// Read control forces from the pipe (e.g., from blackboard server)
				bytes_size = read(read_fd, &input, sizeof(Input));
				handle_pipe_read_error(bytes_size);

				received = 1;

				printf(
					"Drone received: n=%d, e=%d, s=%d, w=%d, reset=%d\n",
					input.n, input.e, input.s, input.w, input.reset);

				// convert input into control forces
				if (input.n)
					force_y -= COMMAND_FORCE;
				if (input.s)
					force_y += COMMAND_FORCE;
				if (input.e)
					force_x += COMMAND_FORCE;
				if (input.w)
					force_x -= COMMAND_FORCE;
				if (input.reset)
				{
					drone.vx = 0;
					drone.vy = 0;
					drone.x = 10;
					drone.y = 10;
				}
			}
		}

		// Calculate repulsive force from obstacle
		float repulsion_force = calculate_repulsive_force(&obstacle, &drone);

		// Decompose repulsive force into x and y components
		float dx = drone.x - obstacle.x;
		float dy = drone.y - obstacle.y;
		float distance = sqrt(dx * dx + dy * dy);
		if (distance > 0)
		{
			force_x += repulsion_force * (dx / distance);
			force_y += repulsion_force * (dy / distance);
		}

		// Update drone position based on control forces
		update_drone_position(&drone, force_x, force_y);

		// only write back to the blackboard server if we received new input
		if (received)
		{
			// Write updated position and velocity back to the blackboard server
			bytes_size = write(write_fd, &drone, sizeof(Drone));
			handle_pipe_write_error(bytes_size);
		}

		// reset force
		force_x = 0.0;
		force_y = 0.0;

		// Simulate time step
		usleep(TIME_STEP * 500000); // Convert TIME_STEP to microseconds
	}
}
