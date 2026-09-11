cmake_minimum_required(VERSION 3.16)

project(TinyServe VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(tinyserve_enable_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endfunction()

add_library(tinyserve_runtime
  runtime/attention.cpp
  runtime/generator.cpp
  runtime/layers.cpp
  runtime/model.cpp
  runtime/model_config.cpp
  runtime/sampling.cpp
  runtime/tensor.cpp
  runtime/tokenizer.cpp
  runtime/weight_loader.cpp
)
target_include_directories(tinyserve_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
tinyserve_enable_warnings(tinyserve_runtime)

add_executable(tinyserve runtime/main.cpp runtime/generate.cpp)
target_compile_definitions(tinyserve PRIVATE TINYSERVE_VERSION="${PROJECT_VERSION}")
target_link_libraries(tinyserve PRIVATE tinyserve_runtime)
tinyserve_enable_warnings(tinyserve)

include(CTest)
if(BUILD_TESTING)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  add_executable(tinyserve_tests tests/test_smoke.cpp)
  tinyserve_enable_warnings(tinyserve_tests)
  add_test(NAME tinyserve.smoke COMMAND tinyserve_tests)

  add_executable(tinyserve_model_config_tests tests/test_model_config.cpp)
  target_link_libraries(tinyserve_model_config_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_model_config_tests)
  add_test(NAME tinyserve.model_config COMMAND tinyserve_model_config_tests)

  set(TINYSERVE_FIXTURE_DIR ${CMAKE_CURRENT_BINARY_DIR}/fixtures)
  set(TINYSERVE_DUMMY_FIXTURE ${TINYSERVE_FIXTURE_DIR}/dummy.tserve)
  add_custom_command(
    OUTPUT ${TINYSERVE_DUMMY_FIXTURE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TINYSERVE_FIXTURE_DIR}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_weights.py
            --dummy-output ${TINYSERVE_DUMMY_FIXTURE}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_weights.py
    VERBATIM
  )
  add_custom_target(tinyserve_dummy_fixture ALL DEPENDS ${TINYSERVE_DUMMY_FIXTURE})
  add_dependencies(tinyserve tinyserve_dummy_fixture)

  add_executable(tinyserve_weight_loader_tests tests/test_weight_loader.cpp)
  target_link_libraries(tinyserve_weight_loader_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_weight_loader_tests)
  add_dependencies(tinyserve_weight_loader_tests tinyserve_dummy_fixture)
  add_test(
    NAME tinyserve.weight_loader
    COMMAND tinyserve_weight_loader_tests ${TINYSERVE_DUMMY_FIXTURE}
  )

  add_executable(tinyserve_tokenizer_tests tests/test_tokenizer.cpp)
  target_link_libraries(tinyserve_tokenizer_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_tokenizer_tests)
  add_test(NAME tinyserve.tokenizer COMMAND tinyserve_tokenizer_tests)

  add_executable(tinyserve_tensor_tests tests/test_tensor.cpp)
  target_link_libraries(tinyserve_tensor_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_tensor_tests)
  add_test(NAME tinyserve.tensor COMMAND tinyserve_tensor_tests)

  add_executable(tinyserve_attention_tests tests/test_attention.cpp)
  target_link_libraries(tinyserve_attention_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_attention_tests)
  add_test(NAME tinyserve.attention COMMAND tinyserve_attention_tests)

  add_executable(tinyserve_layers_tests tests/test_layers.cpp)
  target_link_libraries(tinyserve_layers_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_layers_tests)
  add_test(NAME tinyserve.layers COMMAND tinyserve_layers_tests)

  add_executable(tinyserve_model_tests tests/test_model.cpp)
  target_link_libraries(tinyserve_model_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_model_tests)
  add_dependencies(tinyserve_model_tests tinyserve_dummy_fixture)
  add_test(NAME tinyserve.model COMMAND tinyserve_model_tests ${TINYSERVE_DUMMY_FIXTURE})

  add_executable(tinyserve_generation_tests tests/test_generation.cpp)
  target_link_libraries(tinyserve_generation_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_generation_tests)
  add_dependencies(tinyserve_generation_tests tinyserve_dummy_fixture)
  add_test(
    NAME tinyserve.generation
    COMMAND tinyserve_generation_tests ${TINYSERVE_DUMMY_FIXTURE}
  )

  add_test(
    NAME tinyserve.cli
    COMMAND tinyserve generate --model ${TINYSERVE_DUMMY_FIXTURE} --prompt A --max-tokens 2
  )
  set_tests_properties(tinyserve.cli PROPERTIES PASS_REGULAR_EXPRESSION "generated_token_ids=")
  add_test(
    NAME tinyserve.cli_invalid
    COMMAND tinyserve generate --model ${TINYSERVE_DUMMY_FIXTURE} --prompt A --tokens 65
  )
  set_tests_properties(tinyserve.cli_invalid PROPERTIES WILL_FAIL TRUE)

  add_test(
    NAME tinyserve.converter_determinism
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_convert_weights.py
            --converter ${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_weights.py
            --work-dir ${CMAKE_CURRENT_BINARY_DIR}/converter-test
  )

  set(TINYSERVE_SAVED_REFERENCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/fixture_reference.txt)
  add_test(
    NAME tinyserve.reference_dump
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_dump_reference.py
            --dumper ${CMAKE_CURRENT_SOURCE_DIR}/tools/dump_reference.py
            --checker ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_reference_outputs.py
            --model ${TINYSERVE_DUMMY_FIXTURE}
            --saved-reference ${TINYSERVE_SAVED_REFERENCE}
            --work-dir ${CMAKE_CURRENT_BINARY_DIR}/reference-test
  )

  add_executable(tinyserve_reference_tests tests/test_reference.cpp)
  target_link_libraries(tinyserve_reference_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_reference_tests)
  add_dependencies(tinyserve_reference_tests tinyserve_dummy_fixture)
  add_test(
    NAME tinyserve.reference_compare
    COMMAND tinyserve_reference_tests ${TINYSERVE_DUMMY_FIXTURE} ${TINYSERVE_SAVED_REFERENCE}
  )
endif()

option(TINYSERVE_BUILD_BENCHMARKS "Build TinyServe benchmark placeholders" ON)
if(TINYSERVE_BUILD_BENCHMARKS)
  add_executable(tinyserve_bench bench/placeholder_bench.cpp)
  tinyserve_enable_warnings(tinyserve_bench)
endif()
