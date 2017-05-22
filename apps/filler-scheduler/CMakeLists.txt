project(pssf)
add_pss_executable(pssf pss_filler_scheduler_main.cpp)
configure_file(run.sh.in run.sh @ONLY)
