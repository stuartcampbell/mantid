
set(
  SRC_FILES
  src/BaseInstrumentModel.cpp
)

set(
  MOC_FILES
  inc/MantidQtWidgets/CustomInstrumentView/BaseInstrumentModel.h
)

set(
  INC_FILES
  inc/MantidQtWidgets/CustomInstrumentView/BaseInstrumentModel.h
)

# Target
mtd_add_qt_library(TARGET_NAME MantidQtWidgetsCustomInstrumentView
                   QT_VERSION 4
                   SRC
                     ${SRC_FILES}
                     ${QT4_SRC_FILES}
                   MOC
                     ${MOC_FILES}
                     ${QT4_MOC_FILES}
                   NOMOC
                     ${INC_FILES}
                     ${QT4_INC_FILES}
                   DEFS
                     IN_MANTIDQT_CUSTOMINSTRUMENTVIEW
                   INCLUDE_DIRS
                     inc
                   SYSTEM_INCLUDE_DIRS
                     ${Boost_INCLUDE_DIRS}
                   LINK_LIBS
                     ${TCMALLOC_LIBRARIES_LINKTIME}
                     ${CORE_MANTIDLIBS}
                     ${POCO_LIBRARIES}
                     ${Boost_LIBRARIES}
                     ${PYTHON_LIBRARIES}
                   QT4_LINK_LIBS
                     Qt4::QtOpenGL
                     Qwt5
                   MTD_QT_LINK_LIBS
                     MantidQtWidgetsCommon
                     MantidQtWidgetsPlotting
					 MantidQtWidgetsInstrumentView
                   INSTALL_DIR
                     ${LIB_DIR}
                   OSX_INSTALL_RPATH
                     @loader_path/../MacOS
                     @loader_path/../Frameworks
                   LINUX_INSTALL_RPATH
                     "\$ORIGIN/../${LIB_DIR}")

find_package(BoostPython REQUIRED)
mtd_add_qt_library(TARGET_NAME MantidQtWidgetsCustomInstrumentView
                   QT_VERSION 5
                   SRC
                     ${SRC_FILES}
                     ${QT5_SRC_FILES}
                   MOC
                     ${MOC_FILES}
                     ${QT5_MOC_FILES}
                   NOMOC
                     ${INC_FILES}
                     ${QT5_INC_FILES}
                   DEFS
                     IN_MANTIDQT_CUSTOMINSTRUMENTVIEW
                   INCLUDE_DIRS
                     inc
                   LINK_LIBS
                     ${TCMALLOC_LIBRARIES_LINKTIME}
                     ${CORE_MANTIDLIBS}
                     PythonInterfaceCore
                     ${POCO_LIBRARIES}
                     ${Boost_LIBRARIES}
                     ${PYTHON_LIBRARIES}
                   QT5_LINK_LIBS
                     Qt5::OpenGL
                   MTD_QT_LINK_LIBS
                     MantidQtWidgetsCommon
					 MantidQtWidgetsPlotting
                     MantidQtWidgetsMplCpp
					 MantidQtWidgetsInstrumentView
                   INSTALL_DIR
                     ${WORKBENCH_LIB_DIR}
                   OSX_INSTALL_RPATH
                     @loader_path/../MacOS
                     @loader_path/../Frameworks
                   LINUX_INSTALL_RPATH
                     "\$ORIGIN/../${WORKBENCH_LIB_DIR}")

# Testing
#set(
#  TEST_FILES
  #test/InstrumentWidgetDecoderTest.h
#)

#mtd_add_qt_tests(
  #TARGET_NAME MantidQtWidgetsInstrumentWidgetTest
  #QT_VERSION 5
  #SRC ${TEST_FILES}
  #INCLUDE_DIRS
  #  ../../../Framework/DataObjects/inc
  #  ../../../Framework/TestHelpers/inc
  #TEST_HELPER_SRCS
  #  ../../../Framework/TestHelpers/src/TearDownWorld.cpp
  #  ../../../Framework/TestHelpers/src/ComponentCreationHelper.cpp
  #  ../../../Framework/TestHelpers/src/InstrumentCreationHelper.cpp
  #  ../../../Framework/TestHelpers/src/WorkspaceCreationHelper.cpp
  #LINK_LIBS
  #  ${CORE_MANTIDLIBS}
  #  PythonInterfaceCore
  #  DataObjects
  #  ${POCO_LIBRARIES}
  #  ${Boost_LIBRARIES}
  #  ${PYTHON_LIBRARIES}
  #  ${GMOCK_LIBRARIES}
  #  ${GTEST_LIBRARIES}
  #QT5_LINK_LIBS Qt5::OpenGL
  #MTD_QT_LINK_LIBS
  #  MantidQtWidgetsCommon
  #  MantidQtWidgetsInstrumentView
  #  MantidQtWidgetsMplCpp
  #PARENT_DEPENDENCIES GUITests
#)
