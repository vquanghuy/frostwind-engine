# Header-only, pinned local GLM. No dependency downloads during configuration.
set(GLM_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/external/glm")
if(NOT EXISTS "${GLM_INCLUDE_DIR}/glm/glm.hpp")
    message(FATAL_ERROR "Missing pinned GLM headers; see external/glm/README.integration.md")
endif()
add_library(frostwind-glm INTERFACE)
add_library(GLM::GLM ALIAS frostwind-glm)
target_include_directories(frostwind-glm SYSTEM INTERFACE "${GLM_INCLUDE_DIR}")
target_compile_definitions(frostwind-glm INTERFACE GLM_FORCE_RADIANS)
