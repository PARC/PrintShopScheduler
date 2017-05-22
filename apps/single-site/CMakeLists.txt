project(psss)
add_pss_executable(psss pss_scheduler_main.cpp)
configure_file(run.sh.in run.sh @ONLY)
