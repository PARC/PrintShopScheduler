if (JNI_FOUND)
  project(pssjni)
  include_directories(${JNI_INCLUDE_DIRS})
  add_pss_shared_library(pssjni pss_jni_main.cpp)
  target_link_libraries(pssjni ${JNI_LIBRARIES})
endif()
