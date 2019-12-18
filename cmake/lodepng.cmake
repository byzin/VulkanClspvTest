# file: lodepng.cmake
# author: Sho Ikeda
#
# Copyright (c) 2015-2019 Sho Ikeda
# This software is released under the MIT License.
# http://opensource.org/licenses/mit-license.php
# 


function(loadLodepng lodepng_dir lodepng_include_dir lodepng_library)
  set(source_files ${lodepng_dir}/lodepng.cpp ${lodepng_dir}/lodepng.h)
  set(lodepng_name "lodepng")
  source_group(${lodepng_name} FILES ${source_files})
  add_library(${lodepng_name} STATIC ${source_files})
  ## Set lodepng properties
  set_target_properties(${lodepng_name} PROPERTIES CXX_STANDARD 17
                                                   CXX_STANDARD_REQUIRED ON)
  target_compile_options(${lodepng_name} PRIVATE ${cxx_compiler_flags})
  target_include_directories(${lodepng_name} PRIVATE ${lodepng_dir})
  target_link_libraries(${lodepng_name} ${CMAKE_THREAD_LIBS_INIT}
                                        ${cxx_linker_flags})
  target_compile_definitions(${lodepng_name} PRIVATE ${cxx_definitions})


  set(${lodepng_include_dir} ${lodepng_dir} PARENT_SCOPE)
  set(${lodepng_library} ${lodepng_name} PARENT_SCOPE)
endfunction(loadLodepng)
