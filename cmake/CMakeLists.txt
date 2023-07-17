set(TEREBINTH_PACKAGE_DIR
  "${INSTALL_LIB_DIR}/cmake/terebinth-${PROJECT_VERSION}"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/terebinth-config.cmake
  INSTALL_DESTINATION ${TEREBINTH_PACKAGE_DIR}
  PATH_VARS INSTALL_INCLUDE_DIR
            INSTALL_LIB_DIR
            INSTALL_BIN_DIR
            INSTALL_DOC_DIR
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/terebinth-config-version.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)
