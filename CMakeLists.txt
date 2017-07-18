
cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)
project(al_sound_grabber)
find_package(qibuild)

option(SOUNDPROCESSING_IS_REMOTE
  "module is compiled as a remote module (ON or OFF)"
  ON)

set(_srcs
    main.cpp
    alsoundprocessing.h
    alsoundprocessing.cpp
)

if(SOUNDPROCESSING_IS_REMOTE)
  add_definitions(" -DSOUNDPROCESSING_IS_REMOTE ")
  qi_create_bin(al_sound_grabber ${_srcs})
else()
  qi_create_lib(al_sound_grabber SHARED ${_srcs} SUBFOLDER naoqi)
endif()

set_source_files_properties(main.cpp PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")
set_source_files_properties(alsoundprocessing.cpp PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations")

qi_use_lib(al_sound_grabber ALCOMMON ALPROXIES ALAUDIO)

