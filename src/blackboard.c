#include <ncurses.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For exit

#define DELAY 50000 // Microseconds delay for refresh

// #define DRONE '+'
// #define OBSTACLE '*'
// #define TARGET '5'

#define DRONE_PAIR 1
#define OBSTACLE_PAIR 2
#define TARGET_PAIR 3

typedef struct
{
	int x, y;
} Object;

// Function to initialize ncurses
void init_ncurses()
{
	initscr();
	noecho();
	curs_set(FALSE);
	keypad(stdscr, TRUE);
	timeout(0); // Non-blocking input
}

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
	// init_pair(PLAYER_PAIR, COLOR_RED, COLOR_MAGENTA);
}

void draw_drone(Object drone)
{
	attron(COLOR_PAIR(DRONE_PAIR));
	mvprintw(drone.y, drone.x, "+");
	attroff(COLOR_PAIR(DRONE_PAIR));
}

void draw_obstacles(Object obstacle)
{
	attron(COLOR_PAIR(OBSTACLE_PAIR));
	mvprintw(obstacle.y, obstacle.x, "*");
	attroff(COLOR_PAIR(OBSTACLE_PAIR));
}

// Function to display the obstacle and drone
void display(Object drone, Object obstacle)
{
	clear();
	draw_drone(drone);		  // Display drone
	draw_obstacles(obstacle); // Display obstacle
	// mvprintw(drone.y, drone.x, "+");
	// mvprintw(obstacle.y, obstacle.x, "*");
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

int main()
{
	int ch;
	init_ncurses();
	init_colors();

	Object drone = {COLS / 2, LINES / 2};	 // Start drone in the center
	Object obstacle = {COLS / 3, LINES / 3}; // Place obstacle at a fixed point

	while (1)
	{
		ch = getch(); // Capture user input
		if (ch == 'q')
			break; // Exit if 'q' is pressed

		move_drone(&drone, ch);
		display(drone, obstacle);
		usleep(DELAY); // Delay to control refresh rate
	}

	endwin(); // Close ncurses mode
	printf("Window size: %d x %d\n", COLS, LINES);

	// print window size
	return 0;
}
