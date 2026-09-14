if(NOT DEFINED STATUS_FILE OR NOT EXISTS "${STATUS_FILE}")
  message(FATAL_ERROR "CUDA gate status file is missing")
endif()

file(READ "${STATUS_FILE}" CUDA_STATUS)
foreach(REQUIRED_FIELD requested compiler toolkit_found targets_available)
  string(FIND "${CUDA_STATUS}" "${REQUIRED_FIELD}=" FIELD_POSITION)
  if(FIELD_POSITION EQUAL -1)
    message(FATAL_ERROR "CUDA gate status is missing ${REQUIRED_FIELD}: ${CUDA_STATUS}")
  endif()
endforeach()

if(CUDA_STATUS MATCHES "targets_available=ON" AND
   NOT CUDA_STATUS MATCHES "toolkit_found=ON")
  message(FATAL_ERROR "CUDA targets cannot be available without CUDAToolkit")
endif()

message(STATUS "TinyServe CUDA gate status validated: ${CUDA_STATUS}")
