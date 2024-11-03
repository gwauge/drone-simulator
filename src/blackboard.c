#include <ncurses.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For exit
#include <math.h>
#include <time.h>

#define DELAY 50000 // Microseconds delay for refresh

#define DRONE "+"
#define OBSTACLE "*"

#define DRONE_PAIR 1
#define OBSTACLE_PAIR 2
#define TARGET_PAIR 3

#define COMMAND_FORCE 10.0	 // Force magnitude for key press
#define MASS 1.0			 // Drone mass in kg
#define DAMPING 0.5			 // Damping coefficient
#define TIME_STEP 0.1		 // Time step for simulation (100 ms)
#define REPULSION_RADIUS 5.0 // Max distance for obstacle repulsion
#define ETA 10.0			 // Strength of the repulsive force

typedef struct
{
	int x, y;
} Object;

typedef struct
{
	float x, y;	  // Position
	float vx, vy; // Velocity
} Drone;

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

// Function to handle movement based on user input
void move_drone(Object *drone, int ch)
{
	switch (ch)
	{
	case KEY_UP:
		if (drone->y > 0)
			drone->y--;
		break;
	case KEY_DOWN:
		if (drone->y < LINES - 1)
			drone->y++;
		break;
	case KEY_LEFT:
		if (drone->x > 0)
			drone->x--;
		break;
	case KEY_RIGHT:
		if (drone->x < COLS - 1)
			drone->x++;
		break;
	}
}

void update_drone_position(Drone *drone, float fx, float fy)
{
	// Update velocities based on force and damping
	drone->vx += (fx - DAMPING * drone->vx) / MASS * TIME_STEP;
	drone->vy += (fy - DAMPING * drone->vy) / MASS * TIME_STEP;

	// Update positions based on velocity
	drone->x += drone->vx * TIME_STEP;
	drone->y += drone->vy * TIME_STEP;

	// check for collision with walls
	if (drone->x < 0 || drone->x >= COLS || drone->y < 0 || drone->y >= LINES)
	{
		// bounce back
		drone->vx = -drone->vx;
		drone->vy = -drone->vy;
	}
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

int main()
{
	int ch;
	init_ncurses();

	Object obstacle = {COLS / 3, LINES / 3}; // Place obstacle at a fixed point

	Drone drone = {COLS / 2, LINES / 2, 0.0, 0.0}; // Start drone in the center
	float force_x = 0.0, force_y = 0.0;

	while (1)
	{
		ch = getch(); // Capture user input
		if (ch == 'q')
			break; // Exit if 'q' is pressed

		// move_drone(&drone, ch);
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
			force_y -= COMMAND_FORCE;
			force_x -= COMMAND_FORCE;
			break;
		case 'e':
			force_y -= COMMAND_FORCE;
			break;
		case 'r':
			force_y -= COMMAND_FORCE;
			force_x += COMMAND_FORCE;
			break;
		case 's':
			force_x -= COMMAND_FORCE;
			break;
		case 'd':
			drone.vx = 0.0;
			drone.vy = 0.0;
			break;
		case 'f':
			force_x += COMMAND_FORCE;
			break;
		case 'x':
			force_y += COMMAND_FORCE;
			force_x -= COMMAND_FORCE;
			break;
		case 'c':
			force_y += COMMAND_FORCE;
			break;
		case 'v':
			force_y += COMMAND_FORCE;
			force_x += COMMAND_FORCE;
			break;
		}

		// DRONE DYNAMICS
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

		// Update drone position based on total forces
		update_drone_position(&drone, force_x, force_y);

		// Reset command force for next iteration (only apply once)
		force_x = 0.0;
		force_y = 0.0;

		display(&drone, &obstacle);
		// print velocity
		mvprintw(0, 0, "Velocity: (%.2f, %.2f)", drone.vx, drone.vy);
		refresh();
		// usleep(DELAY); // Delay to control refresh rate
		usleep(TIME_STEP * 1000000); // Delay to control refresh rate
	}

	endwin(); // Close ncurses mode
	printf("Window size: %d x %d\n", COLS, LINES);

	// print window size
	return 0;
}
