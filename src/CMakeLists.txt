add_library(gs2lib
  ast_print.cpp
  ast_string.cpp
  context.cpp
  environment.cpp
  eval_expr.cpp
  eval_stmt.cpp
  lexer.cpp
  main.cpp
  parser.cpp
  script.cpp
  value.cpp
)

target_include_directories(gs2lib INTERFACE .)

add_executable(gs2 main.cpp)

target_link_libraries(
  gs2 PRIVATE gs2lib
)
