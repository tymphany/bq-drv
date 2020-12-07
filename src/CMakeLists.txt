AUX_SOURCE_DIRECTORY(. src_files)

ADD_EXECUTABLE(bq25703-drv-r1 ${src_files})

find_package(Threads)

target_link_libraries(bq25703-drv-r1 ${CMAKE_THREAD_LIBS_INIT})

INSTALL(TARGETS bq25703-drv-r1
	RUNTIME DESTINATION bin
)


