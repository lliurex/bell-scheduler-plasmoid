if(NOT WIN32)
    # uses the command line tool 'bell-scheduler', does not make sense on Windows
    add_subdirectory(bellschedulernotifier)
endif(NOT WIN32)
