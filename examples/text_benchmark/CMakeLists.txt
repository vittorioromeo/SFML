# all source files
set(SRC TextBenchmark.cpp)

# define the text_benchmark target
sfml_add_example(text_benchmark GUI_APP
                 SOURCES ${SRC}
                 DEPENDS SFML::Graphics
                 RESOURCES_DIR resources)
