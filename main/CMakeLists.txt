idf_component_register(SRCS main.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES esp_timer config lcd cursor pin sound)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

