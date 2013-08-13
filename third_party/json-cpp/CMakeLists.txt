set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib)

include_directories(include)

add_library(json-cpp STATIC
    src/lib_json/json_reader.cpp
    src/lib_json/json_value.cpp
    src/lib_json/json_writer.cpp)

