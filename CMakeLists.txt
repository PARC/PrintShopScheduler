project(PrintShopScheduler)
cmake_minimum_required(VERSION 2.8.8)
set(PROJECT_NAME_STR PrintShopScheduler)
# says the PrintShopScheduler project uses C++
project(${PROJECT_NAME_STR} CXX)

# load the modules in this directory before looking
# for packages
#set(CMAKE_MODULE_PATH ${PrintShopScheduler_SOURCE_DIR}/cmake)

find_package(Boost REQUIRED COMPONENTS system)
find_package(JNI)

if(CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-m64 -Wall -ansi -Wno-deprecated -pthread)
endif()

if(UNIX)
    add_definitions(-D_LINUX_ -DUNIX_ENV)
endif()

#-------------------
# set common include folder for module
#-------------------
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
set(EXT_PROJECTS_DIR ${PROJECT_SOURCE_DIR}/ext)
set(PSS_DATA_PATH ${PROJECT_SOURCE_DIR}/data)

#-------------------
# Core dependencies
#-------------------
# directory for 3rd party packages used by PrintShopScheduler.
add_subdirectory(${EXT_PROJECTS_DIR}/rapidxml)

# Add both shared and static libraries for ${NAME}
# For example, if ${NAME} = pss, then:
# 1. the share library target is called 'pss'
# 2. the static library target is called 'pss_static'
# 3. for target_link_libraries(), one can use ${PSS_LIB} for
#    linking the shared library and ${PSS_STATIC_LIB} for
#    linking the static library
macro(add_shared_and_static_libs NAME)
  add_library(${NAME} SHARED ${ARGN})
  add_library(${NAME}_static STATIC ${ARGN})
  set_property(TARGET ${NAME}_static PROPERTY POSITION_INDEPENT_CODE TRUE)
  string(TOUPPER ${NAME}_lib SHARED_LIB)
  string(TOUPPER ${NAME}_static_lib STATIC_LIB)
  set(${SHARED_LIB} ${PROJECT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(${STATIC_LIB} ${PROJECT_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${NAME}_static${CMAKE_STATIC_LIBRARY_SUFFIX})
endmacro(add_shared_and_static_libs)

#-------------------
# Module source
#-------------------
include_directories(${COMMON_INCLUDES} ${RAPIDXML_INCLUDE_DIRS})
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
# Compile the source files referenced above and
# put the results into a shared library and a static library.
# In Unix, the shared library will be named lib.so, and
# the static library will be named libpss_static.a
add_shared_and_static_libs(pss ${SRC_FILES})

add_dependencies(pss rapidxml)

# This makes a special pss executable by
# calling the normal add_executable command on
# the newly named executable, saying that
# we're using C99, linking the libraries for
# this application and setting the dependencies
# for this application.
macro(add_pss_executable NAME)
  add_executable(${NAME} ${ARGN})
  target_link_libraries(${NAME} ${PSS_LIB} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} m)
endmacro(add_pss_executable)

# Like the previous macro except it makes a
# shared library (.so or .dll) instead of an
# executable.
macro(add_pss_shared_library NAME)
  add_library(${NAME} SHARED ${ARGN})
  target_link_libraries(${NAME} ${PSS_LIB} ${CMAKE_THREAD_LIBS_INIT})
endmacro(add_pss_shared_library)

add_subdirectory(${PROJECT_SOURCE_DIR}/apps)

# Set installation rules
install(DIRECTORY include/
  DESTINATION include/PrintShopScheduler
  FILES_MATCHING PATTERN "*.hpp"
  PATTERN ".git" EXCLUDE)

install(TARGETS pss pss_static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
 
