#ifndef MENU_H
#define MENU_H

//Menu config
typedef enum {
    MENU_CLOSED,
    MENU_OPEN
} menu_state_t;

typedef enum {
    CUSTOM_CLOSED,
    CUSTOM_OPEN
} custom_state_t;

typedef enum {
    MENU_SELECT_BLITZ,
    MENU_SELECT_BULLET,
    MENU_SELECT_RAPID,
    MENU_SELECT_CLASSICAL,
    MENU_SELECT_CUSTOM,
    MENU_SELECT_LICHESS,
    MENU_SELECT_RESET,
    MENU_SELECT_BACK,
    MENU_SELECT_COUNT // counter for the menu cursor
} menu_options_t;

typedef enum {
    CUSTOM_SELECT_TIME,
    CUSTOM_SELECT_INC,
    CUSTOM_SELECT_CONFIRM,
    CUSTOM_SELECT_RESET,
    CUSTOM_SELECT_BACK,
    CUSTOM_SELECT_COUNT // counter for the menu cursor
} custom_options_t;

typedef enum {
    INPUT_MINUS,    
    INPUT_PLUS,  
    INPUT_LEFT,    
    INPUT_RIGHT,  
    INPUT_OK,   
} input_event_t;

void pause_clk();
void reset_clk();
void set_clk_settings(int p_time, int i_time);
void display_menu_cursor(menu_options_t menu_options);
void display_custom_cursor(custom_options_t custom_options);

#endif 
