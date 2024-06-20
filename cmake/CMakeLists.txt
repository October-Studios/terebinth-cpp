set(TEREBINTH_PACKAGE_DIR
  "${CMAKE_INSTALL_DIR}/cmake/terebinth-${PROJECT_VERSION}"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/terebinth-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/terebinth-config.cmake
  INSTALL_DESTINATION ${TEREBINTH_PACKAGE_DIR}
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR
            CMAKE_INSTALL_LIBDIR
            CMAKE_INSTALL_BINDIR
            CMAKE_INSTALL_DOCDIR
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/terebinth-config-version.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMinorVersion
)

export(EXPORT terebinth-targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/terebinth-targets.cmake"
  NAMESPACE terebinth::
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/terebinth-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/terebinth-config-version.cmake
              ${CMAKE_CURRENT_SOURCE_DIR}/terebinth-api.cmake
        DESTINATION ${TEREBINTH_PACKAGE_DIR}
        COMPONENT devel
)

install(EXPORT terebinth-targets
  FILE terebinth-targets.cmake
  NAMESPACE terebinth::
  DESTINATION ${TEREBINTH_PACKAGE_DIR}
  COMPONENT devel
)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/Modules/CodeCoverage.cmake
  DESTINATION ${TEREBINTH_PACKAGE_DIR}/Modules
  COMPONENT devel
)
