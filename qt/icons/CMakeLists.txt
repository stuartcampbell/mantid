set(
  SRC_FILES
  src/CharIconPainter.cpp
  src/CharIconEngine.cpp
  src/Icon.cpp
)

set(
  INC_FILES
  inc/MantidQtIcons/CharIconPainter.h
  inc/MantidQtIcons/CharIconEngine.h
  inc/MantidQtIcons/Icon.h
)

if(ENABLE_MANTIDPLOT)
  find_package(Qt4 COMPONENTS QtScript REQUIRED)
  mtd_add_qt_library (TARGET_NAME MantidQtIcons
                      QT_VERSION 4
                      SRC ${SRC_FILES}
                      NOMOC ${INC_FILES}
                      RES ${CMAKE_CURRENT_LIST_DIR}/resources/icons.qrc
                      DEFS
                        IN_MANTIDQT_ICONS
                      INCLUDE_DIRS
                        ${CMAKE_CURRENT_SOURCE_DIR}/inc
                      LINK_LIBS
                        ${TCMALLOC_LIBRARIES_LINKTIME}
                        ${POCO_LIBRARIES}
                        ${Boost_LIBRARIES}
                        ${JSONCPP_LIBRARIES}
                        Qt4::QtScript
                      INSTALL_DIR
                        ${LIB_DIR}
                      OSX_INSTALL_RPATH
                        @loader_path/../MacOS
                        @loader_path/../Frameworks
                      LINUX_INSTALL_RPATH
                        "\$ORIGIN/../${LIB_DIR}")
endif()

if(ENABLE_WORKBENCH)
  mtd_add_qt_library (TARGET_NAME MantidQtIcons
                      QT_VERSION 5
                      SRC ${SRC_FILES}
                      NOMOC ${INC_FILES}
                      RES ${CMAKE_CURRENT_LIST_DIR}/resources/icons.qrc
                      DEFS
                        IN_MANTIDQT_ICONS
                      INCLUDE_DIRS
                        ${CMAKE_CURRENT_SOURCE_DIR}/inc
                      LINK_LIBS
                        ${TCMALLOC_LIBRARIES_LINKTIME}
                        ${POCO_LIBRARIES}
                        ${Boost_LIBRARIES}
                        ${JSONCPP_LIBRARIES}
                      INSTALL_DIR
                        ${WORKBENCH_LIB_DIR}
                      OSX_INSTALL_RPATH
                        @loader_path/../MacOS
                        @loader_path/../Frameworks
                      LINUX_INSTALL_RPATH
                        "\$ORIGIN/../${WORKBENCH_LIB_DIR}")
endif()

set(QT5_TEST_FILES test/IconTest.h)

set(
  CXXTEST_EXTRA_HEADER_INCLUDE
  "${CMAKE_CURRENT_LIST_DIR}/test/IconsTestInitialization.h"
)

mtd_add_qt_tests(
  TARGET_NAME MantidQtIconsTest
  SRC ${QT5_TEST_FILES}
  INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../widgets/common/inc
  LINK_LIBS
    ${TARGET_LIBRARIES}
    ${Boost_LIBRARIES}
    ${GMOCK_LIBRARIES}
    ${GTEST_LIBRARIES}
  MTD_QT_LINK_LIBS MantidQtIcons
  PARENT_DEPENDENCIES GUITests
)
