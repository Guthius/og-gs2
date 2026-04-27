add_library(gs2lib
  ast_print.cpp
  ast_string.cpp
  context.cpp
  environment.cpp
  eval_expr.cpp
  lexer.cpp
  main.cpp
  parser.cpp
  script.cpp
  value.cpp
)

add_executable(gs2
  ast_print.cpp
  lexer.cpp
  main.cpp
  parser.cpp
)

target_link_libraries(
  gs2 PRIVATE gs2lib
)
