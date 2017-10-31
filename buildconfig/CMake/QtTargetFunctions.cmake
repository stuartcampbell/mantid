# Utility functions to add libraries/executables parametrized by
# the required version of Qt
#
# name: mtd_add_qt_library
# brief: create a library target for linked against Qt
# The global ENABLE_MANTIDPLOT option controls if a Qt4 target
# is created.
# To limit the Qt version for a specfic library use
# QT_VERSION, e.g.
#
# mtd_add_qt_library ( QT_VERSION 5
#   ...
#   ...
# )
#
# For all options see mtd_add_qt_target
function (mtd_add_qt_library)
  _qt_versions(_qt_vers ${ARGN})
  # Create targets
  foreach(_ver ${_qt_vers})
    mtd_add_qt_target (LIBRARY QT_VERSION ${_ver} ${ARGN})
  endforeach()
endfunction()

# name: mtd_add_qt_executable
# brief: create a library target for linked against Qt
# The global ENABLE_MANTIDPLOT option controls if a Qt4 target
# is created.
# To limit the Qt version for a specfic library use
# QT_VERSION, e.g.
#
# mtd_add_qt_executable ( QT_VERSION 5
#   ...
#   ...
# )
# For all options see mtd_add_qt_target
function (mtd_add_qt_executable)
  _qt_versions(_qt_vers ${ARGN})
  # Create targets
  foreach(_ver ${_qt_vers})
    mtd_add_qt_target (LIBRARY QT_VERSION ${_ver} ${ARGN})
  endforeach()
endfunction()

# Target agnostic function to add either an executable or library linked to Qt
# option: LIBRARY If included define a library target
# option: EXECUTABLE If included define an executable target
# option: NO_SUFFIX If included, no suffix is added to the target name
# option: EXCLUDE_FROM_ALL If included, the target is excluded from target ALL
# keyword: TARGET_NAME The name of the target. The target will have -Qt{QT_VERSION} appended to it.
# keyword: QT_VERSION The major version of Qt to build against
# keyword: SRC .cpp files to include in the target build
# keyword: QT4_SRC .cpp files to include in a Qt4 build
# keyword: QT5_SRC .cpp files to include in a Qt5 build
# keyword: MOC Header files that are to be parsed by moc
# keyword: UI Qt designer ui files that are to be parsed by the UI compiler
# keyword: NOMOC Additional headers that are not to be passed to moc
# keyword: RES Any resource .qrc files
# keyword: DEFS Compiler definitions to set to the target
# keyword: QT_PLUGIN If included, the target is a Qt plugin. The value is the name of the plugin library
# keyword: INCLUDE_DIRS A list of include directories to add to the target
# keyword: SYSTEM_INCLUDE_DIRS A list of include directories to add to the target and marked as system headers
# keyword: PRECOMPILED A name of the precompiled header
# keyword: LINK_LIBS A list of additional libraries to link to the
#          target that are not dependent on Qt
# keyword: QT4_LINK_LIBS A list of additional Qt libraries to link to.
#          QtGui islinked to by default
# keyword: QT5_LINK_LIBS A list of additional Qt libraries to link to.
#          QtWidgets islinked to by default
# keyword: MTD_QT_LINK_LIBS A list of additional libraries to link to the
#          target. It is assumed each was produced with this function and
#          will have the -Qt{QT_VERSION} suffix appended.
# keyword: INSTALL_DIR A destination directory for the install command.
# keyword: OSX_INSTALL_RPATH Install path for osx version > 10.8
function (mtd_add_qt_target)
  set (options LIBRARY EXECUTABLE NO_SUFFIX EXCLUDE_FROM_ALL)
  set (oneValueArgs
    TARGET_NAME QT_VERSION QT_PLUGIN INSTALL_DIR PRECOMPILED)
  set (multiValueArgs SRC QT4_SRC QT5_SRC UI MOC
    NOMOC RES DEFS INCLUDE_DIRS SYSTEM_INCLUDE_DIRS LINK_LIBS
    QT4_LINK_LIBS QT5_LINK_LIBS MTD_QT_LINK_LIBS OSX_INSTALL_RPATH)
  cmake_parse_arguments (PARSED "${options}" "${oneValueArgs}"
                         "${multiValueArgs}" ${ARGN})
  if (PARSED_UNPARSED_ARGUMENTS)
    message ( FATAL_ERROR "Unexpected arguments found: ${PARSED_UNPARSED_ARGUMENTS}" )
  endif()

  if (${PARSED_LIBRARY} AND ${PARSED_EXECUTABLE})
    message (FATAL_ERROR "Both LIBRARY and EXECUTABLE options specified. Please choose only one.")
  endif()

  # uic needs to run against the correct version of Qt. Keep the generated files in
  # a subdirectory of the binary directory labelled by the version
  set (_binary_dir_on_entry ${CMAKE_CURRENT_BINARY_DIR})
  _append_qt_suffix (AS_DIR VERSION ${PARSED_QT_VERSION} OUTPUT_VARIABLE _ui_dir
                     ${CMAKE_CURRENT_BINARY_DIR})
  set (CMAKE_CURRENT_BINARY_DIR ${_ui_dir})
  if (PARSED_QT_VERSION EQUAL 4)
    qt4_wrap_ui (UI_HEADERS ${PARSED_UI})
    _internal_qt_wrap_cpp ( 4 MOC_GENERATED ${PARSED_MOC})
    set (ALL_SRC ${PARSED_SRC} ${PARSED_QT4_SRC} ${MOC_GENERATED})
    qt4_add_resources (RES_FILES ${PARSED_RES})
    set (_qt_link_libraries Qt4::QtGui ${PARSED_QT4_LINK_LIBS})
  elseif (PARSED_QT_VERSION EQUAL 5)
    qt5_wrap_ui (UI_HEADERS ${PARSED_UI})
    _internal_qt_wrap_cpp ( 5 MOC_GENERATED ${PARSED_MOC} )
    set (ALL_SRC ${PARSED_SRC} ${PARSED_QT5_SRC} ${MOC_GENERATED})
    qt5_add_resources (RES_FILES ${PARSED_RES})
    set (_qt_link_libraries Qt5::Widgets ${PARSED_QT5_LINK_LIBS})
  else ()
    message (FATAL_ERROR "Unknown Qt version. Please specify only the major version.")
  endif()
  set (CMAKE_CURRENT_BINARY_DIR ${_binary_dir_on_entry})

  if (${PARSED_NO_SUFFIX})
    set (_target ${PARSED_TARGET_NAME})
  else()
    _append_qt_suffix (VERSION ${PARSED_QT_VERSION} OUTPUT_VARIABLE _target
                       ${PARSED_TARGET_NAME})
  endif()
  _append_qt_suffix (VERSION ${PARSED_QT_VERSION} OUTPUT_VARIABLE _mtd_qt_libs
                     ${PARSED_MTD_QT_LINK_LIBS})

  if (${PARSED_EXCLUDE_FROM_ALL})
    set(_target_exclude_from_all "EXCLUDE_FROM_ALL")
  else()
    set(_target_exclude_from_all)
  endif()

  # Use a precompiled header where they are supported
  if (${PARSED_PRECOMPILED})
    enable_precompiled_headers( ${PARSED_PRECOMPILED}  ALL_SRC )
  endif()

  if (${PARSED_LIBRARY})
    add_library (${_target} ${_target_exclude_from_all} ${ALL_SRC} ${UI_HEADERS} ${PARSED_NOMOC} ${RES_FILES})
  elseif (${PARSED_EXECUTABLE})
    add_executable (${_target} ${_target_exclude_from_all} ${ALL_SRC} ${UI_HEADERS} ${PARSED_NOMOC} ${RES_FILES})
  else ()
    message (FATAL_ERROR "Unknown target type. Options=LIBRARY,EXECUTABLE")
  endif()

  # Target properties
  _disable_suggest_override( ${PARSED_QT_VERSION} ${_target} )
  # Use public headers to populate the INTERFACE_INCLUDE_DIRECTORIES target property
  target_include_directories (${_target} PUBLIC ${_ui_dir} ${_other_ui_dirs}
                              ${PARSED_INCLUDE_DIRS})
  if ( PARSED_SYSTEM_INCLUDE_DIRS )
    target_include_directories (${_target} SYSTEM PUBLIC ${PARSED_SYSTEM_INCLUDE_DIRS})
  endif()
  target_link_libraries (${_target} PRIVATE ${_qt_link_libraries}
                         ${PARSED_LINK_LIBS} ${_mtd_qt_libs})
  if(PARSED_DEFS)
    set_target_properties ( ${_target} PROPERTIES COMPILE_DEFINITIONS "${PARSED_DEFS}" )
  endif()

  if (OSX_VERSION VERSION_GREATER 10.8)
    if (PARSED_OSX_INSTALL_RPATH)
      set_target_properties ( ${_target} PROPERTIES INSTALL_RPATH  "${PARSED_OSX_INSTALL_RPATH}" )
    endif()
  endif ()

  if (PARSED_QT_PLUGIN)
    # Define a compiler variable to set the name of the library within the code. This
    # is required by the Qt plugin mechanism
    #set( LIB_NAME MantidWidgetPlugins )
    add_definitions( -DLIBRARY_NAME=${PARSED_QT_PLUGIN} )

    # change the destination of the target as Qt expects this in a directory called "designer"
    set_target_output_directory( ${_target} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/designer )

    # Set the name of the generated library
    set_target_properties ( ${_target} PROPERTIES OUTPUT_NAME ${PARSED_QT_PLUGIN} )
  endif()

  if (NOT ${PARSED_EXCLUDE_FROM_ALL})
    if (NOT ${PARSED_INSTALL_DIR})
        set(INSTALL_DESTINATION_DIR ${PARSED_INSTALL_DIR})
    else()
        set(INSTALL_DESTINATION_DIR ${LIB_DIR})
    endif()
    install ( TARGETS ${_target} ${SYSTEM_PACKAGE_TARGET} DESTINATION ${INSTALL_DESTINATION_DIR} )
  endif()
endfunction()

function (mtd_add_qt_tests)
  _qt_versions(_qt_vers ${ARGN})
  # Create targets
  foreach(_ver ${_qt_vers})
    mtd_add_qt_test_executable (QT_VERSION ${_ver} ${ARGN})
  endforeach()
endfunction()

# Create an executable target for running a set of unit tests
# linked to Qt
# keyword: TEST_NAME The name of the test suite. The version of Qt will be appended to it
# keyword: SRC The list of test headers containing the unit test code
# keyword: QT_VERSION The major version of Qt to build against
# keyword: INCLUDE_DIRS A list of include directories to add to the target
# keyword: TEST_HELPER_SRCS A list of test helper files to compile in with the target
# keyword: LINK_LIBS A list of additional libraries to link to the
#          target that are not dependent on Qt
# keyword: QT4_LINK_LIBS A list of additional Qt libraries to link to.
#          QtGui islinked to by default
# keyword: QT5_LINK_LIBS A list of additional Qt libraries to link to.
#          QtWidgets islinked to by default
# keyword: MTD_QT_LINK_LIBS A list of additional libraries to link to the
#          target. It is assumed each was produced with this function and
#          will have the Qt{QT_VERSION} suffix appended.
# keyword: PARENT_DEPENDENCIES Any targets listed here will have this new target
#          as a dependency
function (mtd_add_qt_test_executable)
  set (options)
  set (oneValueArgs TARGET_NAME QT_VERSION)
  set (multiValueArgs SRC QT4_SRC INCLUDE_DIRS TEST_HELPER_SRCS LINK_LIBS
       QT4_LINK_LIBS QT5_LINK_LIBS MTD_QT_LINK_LIBS PARENT_DEPENDENCIES)
  cmake_parse_arguments (PARSED "${options}" "${oneValueArgs}"
                         "${multiValueArgs}" ${ARGN})
  if (PARSED_UNPARSED_ARGUMENTS)
    message ( FATAL_ERROR "Unexpected arguments found: ${PARSED_UNPARSED_ARGUMENTS}" )
  endif()
  _append_qt_suffix (VERSION ${PARSED_QT_VERSION} OUTPUT_VARIABLE _target_name
                     ${PARSED_TARGET_NAME})
  # test generation
  list (APPEND PARSED_SRC ${PARSED_QT${PARSED_QT_VERSION}_SRC})
  set (CXXTEST_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Qt${PARSED_QT_VERSION})
  set (TESTHELPER_SRCS ${PARSED_TEST_HELPER_SRCS})
  cxxtest_add_test ( ${_target_name} ${PARSED_SRC} )

  # Warning suppression
  _disable_suggest_override( ${PARSED_QT_VERSION} ${_target_name} )

  # libraries
  set (_link_libs ${PARSED_LINK_LIBS} ${_mtd_qt_libs} )
  if (PARSED_QT_VERSION EQUAL 4)
   set (_link_libs Qt4::QtGui ${PARSED_QT4_LINK_LIBS} ${_link_libs})
  elseif (PARSED_QT_VERSION EQUAL 5)
   set (_link_libs Qt5::Widgets ${PARSED_QT5_LINK_LIBS} ${_link_libs})
  else ()
   message (FATAL_ERROR "Unknown Qt version. Please specify only the major version.")
  endif()
  _append_qt_suffix (VERSION ${PARSED_QT_VERSION} OUTPUT_VARIABLE _mtd_qt_libs
                     ${PARSED_MTD_QT_LINK_LIBS})

  # client and system headers
  target_include_directories ( ${_target_name} PRIVATE ${PARSED_INCLUDE_DIRS} )
  target_include_directories ( ${_target_name} SYSTEM PRIVATE ${CXXTEST_INCLUDE_DIR}
                               ${GMOCK_INCLUDE_DIR} ${GTEST_INCLUDE_DIR} )

  target_link_libraries (${_target_name} LINK_PRIVATE ${LINK_LIBS}
    ${_link_libs} ${_mtd_qt_libs} )

  # Add dependency to any parents
  foreach (_dep ${PARSED_PARENT_DEPENDENCIES})
    add_dependencies (${_dep} ${_target_name})
  endforeach()
endfunction ()

# Given a list of arguments decide which Qt versions
# should be built. If ENABLE_MANTIPLOT is true the Qt4 is built.
# QT_VERSION can be specified to override this.
function (_qt_versions output_list)
  # process argument list
  list (FIND ARGN "QT_VERSION" _ver_idx)
  if (_ver_idx EQUAL -1)
    # default versions
    set (_qt_vers 4 5)
  else()
    math (EXPR _ver_value_idx "${_ver_idx}+1")
    list (GET ARGN ${_ver_value_idx} _ver_value)
    list (APPEND _qt_vers ${_ver_value})
  endif()
  if(NOT ENABLE_MANTIDPLOT)
    list (REMOVE_ITEM _qt_vers 4)
  endif()
  if(NOT ENABLE_WORKBENCH)
    list (REMOVE_ITEM _qt_vers 5)
  endif()
  set (${output_list} ${_qt_vers} PARENT_SCOPE)
endfunction()

# Appends a string to the given variable to define the Qt version
# the library is built against.
# version: Version number of the library
# output_variable: The name of an output variable. This will be set
#                 on the parent scope
# option: as_dir If true then treat the inputs as directories at append a subdirectory
# ARGN: A list of strings to process
function (_append_qt_suffix)
  set (options AS_DIR)
  set (oneValueArgs VERSION OUTPUT_VARIABLE)
  cmake_parse_arguments (PARSED "${options}" "${oneValueArgs}"
                         "${multiValueArgs}" ${ARGN})

 set (_target_suffix "Qt${PARSED_VERSION}")
  if(PARSED_AS_DIR)
    set (_sep "/")
  else()
    set (_sep "")
  endif()
  set (_out)
  foreach (_item ${PARSED_UNPARSED_ARGUMENTS})
      list (APPEND _out ${_item}${_sep}${_target_suffix})
  endforeach ()
  set (${PARSED_OUTPUT_VARIABLE} ${_out} PARENT_SCOPE)
endfunction ()

# Wrap generation of moc files
# We call the qt{4|5}_wrap_cpp individually for each file and force the include
# path to be absolute to avoid relative paths whose length exceed the maximum
# allowed limit on Windows (260 chars). It is assumed that the input paths
# can be made absolute by prefixing them with ${CMAKE_CURRENT_LIST_DIR}
# It assumes that the unnamed arguments are the input files to run through moc
function(_internal_qt_wrap_cpp qtversion moc_generated)
  foreach (_infile ${ARGN})
    if(qtversion EQUAL 4)
      qt4_wrap_cpp (moc_generated ${_infile} OPTIONS -i -f${CMAKE_CURRENT_LIST_DIR}/${_infile} )
    elseif (qtversion EQUAL 5)
      qt5_wrap_cpp (moc_generated ${_infile} OPTIONS -i -f${CMAKE_CURRENT_LIST_DIR}/${_infile} )
    else()
      message (FATAL_ERROR "Unknown Qt version='${qtversion}'.")
    endif()
  endforeach()
  # Pass the output variable out
  set (${moc_generated} ${${moc_generated}} PARENT_SCOPE)
endfunction()

# Disables suggest override for versions of Qt < 5.6.2 as
# Q_OBJECT produces them and the cannot be avoided.
function (_disable_suggest_override _qt_version _target)
  # For Qt < 5.6.2 gcc-5 produces warnings when it encounters the Q_OBJECT
  # macro. We disable the warning in this case
  if ( _qt_version EQUAL 5 AND CMAKE_COMPILER_IS_GNUCXX AND
       Qt5Core_VERSION_STRING VERSION_LESS "5.6.2" )
    get_target_property ( _options ${_target} COMPILE_OPTIONS )
    string ( REPLACE "-Wsuggest-override" "" _options "${_options}" )
    set_target_properties ( ${_target} PROPERTIES
      COMPILE_OPTIONS "${_options}" )
  endif()
endfunction ()
