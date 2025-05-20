idf_component_register(SRCS "menu.c" "i2c-lcd.c" "buttons.c" "lichess.c" "main.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES lichess_cert.pem)
