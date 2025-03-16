#ifndef MENU_H
#define MENU_H

//Menu config
typedef enum {
    MENU_SELECT_BLITZ,
    MENU_SELECT_RAPID,
    MENU_SELECT_CLASSICAL,
    MENU_SELECT_FISCHER,
    MENU_SELECT_BRONSTEIN,
    MENU_SELECT_DELAY,
    MENU_SELECT_CUSTOM,
    MENU_SELECT_BACK,
    MENU_SELECT_COUNT // Ce compteur permet de connaître le nombre total de menus
} menu_state_t;

typedef enum {
    INPUT_UP,    
    INPUT_DOWN,  
    INPUT_OK,   
} input_event_t;

#endif 
