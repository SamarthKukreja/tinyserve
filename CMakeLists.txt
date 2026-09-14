cmake_minimum_required(VERSION 3.16)

project(TinyServe VERSION 0.1.0 LANGUAGES CXX)

option(TINYSERVE_ENABLE_CUDA "Probe for optional TinyServe CUDA microkernels" OFF)
set(TINYSERVE_CUDA_AVAILABLE OFF)
set(TINYSERVE_CUDA_TOOLKIT_FOUND OFF)
set(TINYSERVE_CUDA_COMPILER "not-probed")
if(TINYSERVE_ENABLE_CUDA)
  include(CheckLanguage)
  check_language(CUDA)
  if(CMAKE_CUDA_COMPILER)
    set(TINYSERVE_CUDA_COMPILER "${CMAKE_CUDA_COMPILER}")
    if(CMAKE_VERSION VERSION_LESS 3.17)
      message(WARNING
        "TinyServe found a CUDA compiler, but optional CUDA targets require CMake 3.17+; "
        "continuing with the CPU-only build")
    else()
      enable_language(CUDA)
      find_package(CUDAToolkit QUIET)
      if(CUDAToolkit_FOUND)
        set(TINYSERVE_CUDA_TOOLKIT_FOUND ON)
        set(TINYSERVE_CUDA_AVAILABLE ON)
        if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.18 AND NOT CMAKE_CUDA_ARCHITECTURES)
          message(WARNING
            "CUDA is available but CMAKE_CUDA_ARCHITECTURES is unset; set it explicitly "
            "for reproducible GPU binaries (for example, -DCMAKE_CUDA_ARCHITECTURES=86)")
        endif()
      else()
        message(WARNING
          "TinyServe found a CUDA compiler but not CUDAToolkit; continuing CPU-only")
      endif()
    endif()
  else()
    set(TINYSERVE_CUDA_COMPILER "not-found")
    message(WARNING
      "TINYSERVE_ENABLE_CUDA=ON but no CUDA compiler was found; optional CUDA targets "
      "will be skipped and the CPU runtime remains available")
  endif()
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cuda_status.txt.in
  ${CMAKE_CURRENT_BINARY_DIR}/tinyserve_cuda_status.txt
  @ONLY
)

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
  runtime/kv_cache.cpp
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

  add_executable(tinyserve_kv_cache_tests tests/test_kv_cache.cpp)
  target_link_libraries(tinyserve_kv_cache_tests PRIVATE tinyserve_runtime)
  tinyserve_enable_warnings(tinyserve_kv_cache_tests)
  add_test(NAME tinyserve.kv_cache COMMAND tinyserve_kv_cache_tests)

  add_test(
    NAME tinyserve.cli
    COMMAND tinyserve generate --model ${TINYSERVE_DUMMY_FIXTURE} --prompt A --max-tokens 2
  )
  set_tests_properties(tinyserve.cli PROPERTIES PASS_REGULAR_EXPRESSION "generated_token_ids=")
  add_test(
    NAME tinyserve.cli_kv_cache
    COMMAND tinyserve generate --model ${TINYSERVE_DUMMY_FIXTURE} --prompt A --max-tokens 2
            --decode-mode kv-cache
  )
  set_tests_properties(tinyserve.cli_kv_cache PROPERTIES
                       PASS_REGULAR_EXPRESSION "decode_mode=kv-cache")
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

  add_test(
    NAME tinyserve.kv_results
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/render_kv_results.py
            --results ${CMAKE_CURRENT_SOURCE_DIR}/bench/results.csv
            --raw-results ${CMAKE_CURRENT_SOURCE_DIR}/bench/raw_results.csv
            --kv-doc ${CMAKE_CURRENT_SOURCE_DIR}/docs/kv_cache_results.md
            --report ${CMAKE_CURRENT_SOURCE_DIR}/docs/final_benchmark_report.md
            --readme ${CMAKE_CURRENT_SOURCE_DIR}/README.md
            --check
  )

  add_test(
    NAME tinyserve.cuda_gate_status
    COMMAND ${CMAKE_COMMAND}
            -DSTATUS_FILE=${CMAKE_CURRENT_BINARY_DIR}/tinyserve_cuda_status.txt
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cuda_gate.cmake
  )
endif()

if(TINYSERVE_CUDA_AVAILABLE)
  add_executable(tinyserve_cuda_check cuda/cuda_check.cu)
  target_link_libraries(tinyserve_cuda_check PRIVATE CUDA::cudart)
  set_target_properties(tinyserve_cuda_check PROPERTIES
    CUDA_STANDARD 17
    CUDA_STANDARD_REQUIRED ON
  )
  if(BUILD_TESTING)
    add_test(NAME tinyserve.cuda_availability COMMAND tinyserve_cuda_check)
    set_tests_properties(tinyserve.cuda_availability PROPERTIES SKIP_RETURN_CODE 77)
  endif()
endif()

option(TINYSERVE_BUILD_BENCHMARKS "Build TinyServe fixture benchmarks" ON)
if(TINYSERVE_BUILD_BENCHMARKS)
  add_executable(tinyserve_bench bench/kv_cache_bench.cpp)
  target_link_libraries(tinyserve_bench PRIVATE tinyserve_runtime)
  target_compile_definitions(tinyserve_bench PRIVATE
    TINYSERVE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
  )
  tinyserve_enable_warnings(tinyserve_bench)
endif()
