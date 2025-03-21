#ifndef MENU_H
#define MENU_H

//Menu config
typedef enum {
    MENU_CLOSED,
    MENU_OPEN
} menu_state_t;

typedef enum {
    MENU_SELECT_BLITZ,
    MENU_SELECT_BULLET,
    MENU_SELECT_RAPID,
    MENU_SELECT_CLASSICAL,
    MENU_SELECT_CUSTOM,
    MENU_SELECT_BACK,
    MENU_SELECT_COUNT // counter for the menu cursor
} menu_options_t;

typedef enum {
    INPUT_UP,    
    INPUT_DOWN,  
    INPUT_OK,   
} input_event_t;

void pause_clk();
void reset_clk();
void set_clk_settings(int p_time, int i_time);
void display_menu_cursor(menu_options_t menu_options);

#endif 
