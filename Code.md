# Functions
`void print_time()`: Displays the time remaining for each player.

`void player1_timer(void* arg)` and `void player2_timer(void* arg)`: Callback functions for each player's timer, which will be called every second to decrement the timer.


`void init_timer()`: Initializes each player's timers and sets their callback function.

`void IRAM_ATTR button_isr_handler(void* arg)`: Interrupt triggered by the falling edge of `BUTTON_PLAYER_GPIO`(GPIO 2). It changes the player's turn with the `player1_turn` boolean/flag, and thus, the timer starts decrementing every second. It also gives back the `xSemaphore` binary semaphore at the same time.

`void init_buttons()`: Sets `BUTTON_PLAYER_GPIO`(GPIO 2) as a INPUT which will trigger the `button_isr_handler` interruption routine.

`void player1_task(void* arg)` and `void player2_task(void* arg)`: manages the player timer, starting it when it's the player turn and suspending the task until a button press resumes it. If it's not the player turn, the timer is stopped, and the task waits briefly before checking again.