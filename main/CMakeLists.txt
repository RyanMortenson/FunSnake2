idf_component_register(SRCS main.c
                           game.c
                           snake.c
                           nav.c
                           menu.c
                           com.c
                           ../audio/Chomp.c
                           ../audio/Crash.c
                           ../audio/LevelUp.c
                       INCLUDE_DIRS . ../audio
                       PRIV_REQUIRES esp_timer config lcd cursor pin sound)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

