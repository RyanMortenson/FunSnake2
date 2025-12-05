idf_component_register(
SRCS
  Chomp.c Crash.c
INCLUDE_DIRS
  .
)
# PRIV_REQUIRES driver
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
