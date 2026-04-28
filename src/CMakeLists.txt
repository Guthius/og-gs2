add_library(gs2lib
  ast.cpp
  context.cpp
  environment.cpp
  eval.cpp
  lexer.cpp
  parser.cpp
  script.cpp
  value.cpp
)

target_include_directories(gs2lib PUBLIC ../include)

add_executable(gs2 main.cpp)

target_link_libraries(
  gs2 PRIVATE gs2lib
)
