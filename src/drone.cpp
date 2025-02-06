#include "drone.hpp"

Drone make_drone(float x, float y, float vx, float vy)
{
	Drone drone = {x, y, vx, vy};
	return drone;
}

void update_drone_position(Drone *drone, float fx, float fy)
{
	// Update velocities based on force and damping
	drone->vx += (fx - global_params.damping * drone->vx) / global_params.mass * global_params.time_step;
	drone->vy += (fy - global_params.damping * drone->vy) / global_params.mass * global_params.time_step;

	// Update positions based on velocity
	drone->x += drone->vx * global_params.time_step;
	drone->y += drone->vy * global_params.time_step;
}

float calculate_repulsive_force(int32_t obstacle_x, int32_t obstacle_y, Drone *drone)
{
	// Calculate distance to obstacle
	float dx = drone->x - obstacle_x;
	float dy = drone->y - obstacle_y;
	float distance = sqrt(dx * dx + dy * dy);

	// Apply repulsion only within a certain radius
	if (distance < global_params.repulsion_radius && distance > 0)
	{
		float force_magnitude = global_params.eta * (1.0 / distance - 1.0 / global_params.repulsion_radius) / (distance * distance);
		return force_magnitude;
	}
	return 0.0;
}

void receive_obstacles(int pipe_fd, Obstacles &obstacles, int size)
{
	obstacles.obstacles_number(size);
	std::vector<int32_t> vecx = std::vector<int32_t>(size);
	std::vector<int32_t> vecy = std::vector<int32_t>(size);

	ssize_t bytes_size;

	// read x values
	bytes_size = read(pipe_fd, vecx.data(), size * sizeof(int32_t));
	handle_pipe_read_error(bytes_size);

	// read y values
	bytes_size = read(pipe_fd, vecy.data(), size * sizeof(int32_t));
	handle_pipe_read_error(bytes_size);

	// update object
	obstacles.obstacles_x(vecx);
	obstacles.obstacles_y(vecy);
}

void drone_component(int read_fd, int write_fd)
{
	register_signal_handler();

	fd_set readfds;
	struct timeval timeout;

	ssize_t bytes_size;

	// Get terminal size
	int COLS, LINES;
	bytes_size = read(read_fd, &COLS, sizeof(int));
	handle_pipe_read_error(bytes_size);

	bytes_size = read(read_fd, &LINES, sizeof(int));
	handle_pipe_read_error(bytes_size);

	Drone drone = {10.0, 10.0, 0.0, 0.0}; // Initialize drone at origin with zero velocity
	WorldState world_state;
	Obstacles obstacles;
	float force_x = 0.0, force_y = 0.0;

	int received = 0;
	while (1)
	{
		received = 0;

		FD_ZERO(&readfds);

		// Add the FDs to the fd_set
		FD_SET(read_fd, &readfds);

		timeout.tv_sec = 0; // Wait for up to 5 seconds
		timeout.tv_usec = 0;

		// Reset forces
		force_x = 0.0;
		force_y = 0.0;

		int result = select(read_fd + 1, &readfds, NULL, NULL, &timeout);
		handle_select_error(result);
		if (result == 0)
		{
			// printf("Drone received: no input\n");
		}
		else if (result > 0)
		{
			if (FD_ISSET(read_fd, &readfds))
			{
				// Read control forces from the pipe (e.g., from blackboard server)
				bytes_size = read(read_fd, &world_state, sizeof(world_state));
				handle_pipe_read_error(bytes_size);

				if (world_state.obstacle_count > 0)
					receive_obstacles(read_fd, obstacles, world_state.obstacle_count);

				received = 1;

				// convert input into control forces
				if (world_state.input.n)
					force_y -= global_params.command_force;
				if (world_state.input.s)
					force_y += global_params.command_force;
				if (world_state.input.e)
					force_x += global_params.command_force;
				if (world_state.input.w)
					force_x -= global_params.command_force;
				if (world_state.input.reset)
				{
					drone.vx = 0;
					drone.vy = 0;
					drone.x = 10;
					drone.y = 10;
				}
			}
		}

		// Update drone position based on control forces
		for (int i = 0; i < world_state.obstacle_count; i++)
		{
			int32_t obstacle_x = obstacles.obstacles_x().at(i);
			int32_t obstacle_y = obstacles.obstacles_y().at(i);

			// Calculate repulsive force from obstacle
			float repulsion_force = calculate_repulsive_force(
				obstacle_x,
				obstacle_y,
				&drone);

			// Decompose repulsive force into x and y components
			float dx = drone.x - obstacle_x;
			float dy = drone.y - obstacle_y;
			float distance = sqrt(dx * dx + dy * dy);
			if (distance > 0)
			{
				force_x += repulsion_force * (dx / distance);
				force_y += repulsion_force * (dy / distance);
			}
		}

		// Create virtual obstacles at the borders
		Obstacle borders[4] = {};
		borders[0] = make_obstacle(0, drone.x, 0);		   // Top border
		borders[1] = make_obstacle(0, COLS - 1, drone.y);  // Right border
		borders[2] = make_obstacle(0, drone.x, LINES - 1); // Bottom border
		borders[3] = make_obstacle(0, 0, drone.y);		   // Left border

		for (int i = 0; i < 4; i++)
		{
			Obstacle obstacle = borders[i];

			// Calculate repulsive force from obstacle
			float repulsion_force = calculate_repulsive_force(obstacle.x, obstacle.y, &drone);

			// Decompose repulsive force into x and y components
			float dx = drone.x - obstacle.x;
			float dy = drone.y - obstacle.y;
			float distance = sqrt(dx * dx + dy * dy);
			if (distance > 0)
			{
				force_x += repulsion_force * (dx / distance);
				force_y += repulsion_force * (dy / distance);
			}
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
		usleep(global_params.time_step * 500000); // Convert TIME_STEP to microseconds
	}
}
