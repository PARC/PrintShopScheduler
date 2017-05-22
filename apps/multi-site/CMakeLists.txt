project(pssm)
add_pss_executable(pssm pss_multisite_scheduler_main.cpp)
configure_file(run.sh.in run.sh @ONLY)
