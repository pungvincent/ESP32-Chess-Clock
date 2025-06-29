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
    CUSTOM_SET_CLOSED,
    CUSTOM_SET_OPEN
} custom_set_state_t;

typedef enum {
    LICHESS_CLOSED,
    LICHESS_OPEN
} Lichess_state_t;

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
    CUSTOM_SELECT_SET,
    CUSTOM_SELECT_BACK,
    CUSTOM_SELECT_COUNT // counter for the menu cursor
} custom_options_t;

typedef enum {
    CUSTOM_SET_SELECT_P1_MIN_TENTH,
    CUSTOM_SET_SELECT_P1_MIN_UNIT,
    CUSTOM_SET_SELECT_P1_SEC_TENTH,
    CUSTOM_SET_SELECT_P1_SEC_UNIT,
    CUSTOM_SET_SELECT_P2_MIN_TENTH,
    CUSTOM_SET_SELECT_P2_MIN_UNIT,
    CUSTOM_SET_SELECT_P2_SEC_TENTH,
    CUSTOM_SET_SELECT_P2_SEC_UNIT,
    CUSTOM_SET_SELECT_INC,
    CUSTOM_SET_SELECT_CONFIRM,
    CUSTOM_SET_SELECT_BACK,
    CUSTOM_SET_SELECT_COUNT // counter for the menu cursor
} custom_set_options_t;

typedef enum {
    INPUT_MINUS,    
    INPUT_PLUS,  
    INPUT_LEFT,    
    INPUT_RIGHT,  
    INPUT_OK,   
} input_event_t;

void pause_clk();
void reset_clk();
void set_clk_settings(int p_time, int i_time); //Set the same timer for each player
void set_custom_clk_settings(int p1_time, int p2_time, int i_time); //Set different timer for each player
void display_menu_cursor(menu_options_t menu_options);
void display_custom_cursor(custom_options_t custom_options);
void display_custom_set_cursor(custom_set_options_t custom_set_options); 

#endif 
