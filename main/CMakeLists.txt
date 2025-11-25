idf_component_register(SRCS main.c
                           game.c
                           snake.c
                           nav.c
                           menu.c
                           com.c
                           ../audio/Chomp.c
                           ../audio/Crash.c
                           ../../components/joy/joy.c
                           ../../components/net/net.c
                           ../sprites/apple.c
                            ../sprites/bluebody.c
                            ../sprites/bluedown.c
                            ../sprites/blueleft.c
                            ../sprites/blueright.c
                            ../sprites/blueup.c
                            ../sprites/cherry.c
                            ../sprites/goldapple.c
                            ../sprites/graybody.c
                            ../sprites/graydown.c
                            ../sprites/grayleft.c
                            ../sprites/grayright.c
                            ../sprites/grayup.c
                            ../sprites/greenapple.c
                            ../sprites/greenbody.c
                            ../sprites/greendown.c
                            ../sprites/greenleft.c
                            ../sprites/greenright.c
                            ../sprites/greenup.c
                       INCLUDE_DIRS . ../audio
                                     .
                                    ../sprites
                       PRIV_REQUIRES nvs_flash esp_wifi driver freertos net esp_adc joy esp_timer config lcd cursor pin sound)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

