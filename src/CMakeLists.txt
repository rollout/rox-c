include_directories(.)
add_library(libroxc roxparser.c roxapi.c)
add_library(libroxcpp roxparser.c roxapi.c roxapi.cxx)
