if(NOT WIN32)
    # uses the command line tool 'quota', does not make sense on Windows
    add_subdirectory(bellschedulernotifier)
endif(NOT WIN32)
