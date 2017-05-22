cmake_minimum_required(VERSION 2.8.8)
project(rapidxml_builder CXX)
include(ExternalProject)

ExternalProject_Add(rapidxml
    GIT_REPOSITORY https://github.com/dwd/rapidxml.git
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    # Disable install step
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

# Specify RAPIDXML include dir
ExternalProject_Get_Property(rapidxml source_dir)
set(RAPIDXML_INCLUDE_DIRS ${source_dir}/ PARENT_SCOPE)

