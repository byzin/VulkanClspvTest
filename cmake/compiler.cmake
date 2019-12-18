# file: compiler.cmake
# author: Sho Ikeda
#
# Copyright (c) 2015-2019 Sho Ikeda
# This software is released under the MIT License.
# http://opensource.org/licenses/mit-license.php
#


#
function(initCompilerOption)
  set(option_description "Enable compiler full warning (if supported).")
  setBooleanOption(Z_ENABLE_COMPILER_WARNING ON ${option_description})

  set(option_description "The compiler treat all warnings as build errors (if supported).")
  setBooleanOption(Z_TREAT_COMPILER_WARNING_AS_ERROR OFF ${option_description})

  set(option_description "Clang uses libc++ instead of libstdc++ (if compiler supports).")
  setBooleanOption(Z_CLANG_USES_LIBCXX OFF ${option_description})

  set(option_description "Enable C++ address sanitizer (if compiler supports).")
  setBooleanOption(Z_ENABLE_SANITIZER_ADDRESS OFF ${option_description})

  set(option_description "Enable C++ thread sanitizer (if compiler supports).")
  setBooleanOption(Z_ENABLE_SANITIZER_THREAD OFF ${option_description})

  set(option_description "Enable C++ memory sanitizer (if compiler supports).")
  setBooleanOption(Z_ENABLE_SANITIZER_MEMORY OFF ${option_description})

  set(option_description "Enable C++ undefined sanitizer (if compiler supports).")
  setBooleanOption(Z_ENABLE_SANITIZER_UNDEF_BEHAVIOR OFF ${option_description})

  set(option_description "Enable C++ undefined sanitizer (if compiler supports).")
  setBooleanOption(Z_ENABLE_SANITIZER_UNDEF_BEHAVIOR_FULL OFF ${option_description})

  set(option_description "Enable C++ leak sanitizer (if compiler supports).")
  setBooleanOption(Z_ENABLE_SANITIZER_LEAK OFF ${option_description})

  set(option_description "Enable static analyzer (clang-tidy, include-what-you-use, link-what-you-use).")
  setBooleanOption(Z_ENABLE_STATIC_ANALYZER OFF ${option_description})
endfunction(initCompilerOption)


function(getMsvcCompilerFlags cxx_compile_flags cxx_linker_flags cxx_definitions)
  set(compile_flags "")
  set(linker_flags "")
  set(definitions "")

  list(APPEND compile_flags /constexpr:depth${constexpr_depth}
                            /constexpr:backtrace${constexpr_backtrace}
                            /constexpr:steps${constexpr_steps})

  # Output variables
  set(${cxx_compile_flags} ${compile_flags} PARENT_SCOPE)
  set(${cxx_linker_flags} ${linker_flags} PARENT_SCOPE)
  set(${cxx_definitions} ${definitions} PARENT_SCOPE)
endfunction(getMsvcCompilerFlags)


function(appendClangFlags cxx_compile_flags)
  set(compile_flags ${${cxx_compile_flags}})
  foreach(compile_flag IN LISTS ARGN)
    if(Z_VISUAL_STUDIO)
      string(REPLACE "=" ";" compile_flag ${compile_flag})
      foreach(flag IN LISTS compile_flag)
        list(APPEND compile_flags "SHELL:-Xclang ${flag}")
      endforeach(flag)
    else()
      list(APPEND compile_flags ${compile_flag})
    endif()
  endforeach(compile_flag)
  set(${cxx_compile_flags} ${compile_flags} PARENT_SCOPE)
endfunction(appendClangFlags)


function(getSanitizerFlags compile_sanitizer_flags linker_sanitizer_flags)
  set(check_list "")

  # Collect sanitizer checks
  if(Z_ENABLE_SANITIZER_ADDRESS)
    list(APPEND check_list address)
  endif()
  if(Z_ENABLE_SANITIZER_THREAD)
    list(APPEND check_list thread)
  endif()
  if(Z_ENABLE_SANITIZER_MEMORY)
    list(APPEND check_list memory)
  endif()
  if(Z_ENABLE_SANITIZER_UNDEF_BEHAVIOR OR Z_ENABLE_SANITIZER_UNDEF_BEHAVIOR_FULL)
    list(APPEND check_list alignment
                           bool
                           builtin
                           bounds
                           enum
                           float-cast-overflow
                           float-divide-by-zero
                           integer-divide-by-zero
                           nonnull-attribute
                           null
                           nullability-arg
                           nullability-assign
                           nullability-return
                           object-size
                           pointer-overflow
                           return
                           returns-nonnull-attribute
                           unreachable
                           vla-bound)
    if(NOT Z_WINDOWS)
      list(APPEND check_list function
                             vptr)
    endif()
  endif()
  if(Z_ENABLE_SANITIZER_UNDEF_BEHAVIOR_FULL)
    list(APPEND check_list implicit-unsigned-integer-truncation
                           implicit-signed-integer-truncation
                           shift
                           signed-integer-overflow
                           unsigned-integer-overflow)
  endif()
  if(Z_ENABLE_SANITIZER_LEAK)
    list(APPEND check_list leak)
  endif()

  # Make a sanitizer option string from check list 
  set(compile_flags "")
  set(linker_flags "")
  if(check_list)
    string(REPLACE ";" "," check_flag "${check_list}")
    set(compile_flags "-fsanitize=${check_flag}" -fno-omit-frame-pointer)
    set(linker_flags "-fsanitize=${check_flag}")

    # Output
    set(${compile_sanitizer_flags} ${compile_flags} PARENT_SCOPE)
    set(${linker_sanitizer_flags} ${linker_flags} PARENT_SCOPE)
  endif()
endfunction(getSanitizerFlags)


function(getClangCompilerFlags cxx_compile_flags cxx_linker_flags cxx_definitions)
  set(compile_flags "")
  set(linker_flags "")
  set(definitions "")

  appendClangFlags(compile_flags
      -fconstexpr-depth=${constexpr_depth}
      -fconstexpr-backtrace-limit=${constexpr_backtrace}
      -fconstexpr-steps=${constexpr_steps}
      -ftemplate-depth=${recursive_template_depth})

  if(Z_CLANG_USES_LIBCXX)
    list(APPEND compile_flags -stdlib=libc++)
    list(APPEND linker_flags -stdlib=libc++)
  endif()

  # Sanitizer
  getSanitizerFlags(compile_sanitizer_flags linker_sanitizer_flags)
  list(APPEND compile_flags ${compile_sanitizer_flags})
  list(APPEND linker_flags ${linker_sanitizer_flags})

  # Output variables
  set(${cxx_compile_flags} ${compile_flags} PARENT_SCOPE)
  set(${cxx_linker_flags} ${linker_flags} PARENT_SCOPE)
  set(${cxx_definitions} ${definitions} PARENT_SCOPE)
endfunction(getClangCompilerFlags)


function(getGccCompilerFlags cxx_compile_flags cxx_linker_flags cxx_definitions)
  set(compile_flags "")
  set(linker_flags "")
  set(definitions "")

  list(APPEND compile_flags -fconstexpr-depth=${constexpr_depth}
                            -fconstexpr-loop-limit=${constexpr_steps}
                            -ftemplate-depth=${recursive_template_depth})

  # Sanitizer
  getSanitizerFlags(compile_sanitizer_flags linker_sanitizer_flags)
  list(APPEND compile_flags ${compile_sanitizer_flags})
  list(APPEND linker_flags ${linker_sanitizer_flags})

  # Output variables
  set(${cxx_compile_flags} ${compile_flags} PARENT_SCOPE)
  set(${cxx_linker_flags} ${linker_flags} PARENT_SCOPE)
  set(${cxx_definitions} ${definitions} PARENT_SCOPE)
endfunction(getGccCompilerFlags)


function(getMsvcWarningFlags compiler_warning_flags)
  set(warning_flags "")
  list(APPEND warning_flags /W4)
  if(Z_TREAT_COMPILER_WARNING_AS_ERROR)
    list(APPEND warning_flags /WX)
  endif()
  # Output variables
  set(${compiler_warning_flags} ${warning_flags} PARENT_SCOPE)
endfunction(getMsvcWarningFlags)


function(getClangWarningFlags compiler_warning_flags)
  set(warning_flags "")
  list(APPEND warning_flags -Weverything
                            -Wno-c++98-compat
                            -Wno-c++98-compat-pedantic
                            )
  if(Z_TREAT_COMPILER_WARNING_AS_ERROR)
    list(APPEND warning_flags -Werror)
  endif()
  # Output variables
  set(${compiler_warning_flags} ${warning_flags} PARENT_SCOPE)
endfunction(getClangWarningFlags)


function(getGccWarningFlags compiler_warning_flags)
  set(warning_flags "")
  list(APPEND warning_flags -pedantic
                            -Wall
                            -Wextra
                            -Wcast-align
                            -Wcast-qual
                            -Wctor-dtor-privacy
                            -Wdisabled-optimization
                            -Wformat=2
                            -Winit-self
                            -Wlogical-op
                            -Wmissing-declarations
                            -Wmissing-include-dirs
                            -Wnoexcept
                            -Wold-style-cast
                            -Woverloaded-virtual
                            -Wredundant-decls
                            -Wshadow
                            -Wsign-conversion
                            -Wsign-promo
                            -Wstrict-null-sentinel
                            -Wstrict-overflow=5
                            -Wswitch-default
                            -Wundef
                            )
  if(Z_TREAT_COMPILER_WARNING_AS_ERROR)
    list(APPEND warning_flags -Werror)
  endif()
  # Output variables
  set(${compiler_warning_flags} ${warning_flags} PARENT_SCOPE)
endfunction(getGccWarningFlags)


# Get C++ specific compile flags
function(getCxxCompilerFlags cxx_compile_flags cxx_linker_flags cxx_definitions)

  set(constexpr_depth 512)
  set(constexpr_backtrace 16)
  set(constexpr_steps 2147483647)
  set(recursive_template_depth 2048)

  if(Z_GCC)
    getGccCompilerFlags(compile_flags linker_flags definitions)
  elseif(Z_CLANG)
    getClangCompilerFlags(compile_flags linker_flags definitions)
  elseif(Z_MSVC)
    getMsvcCompilerFlags(compile_flags linker_flags definitions)
  endif()
  # Output variables
  set(${cxx_compile_flags} ${compile_flags} PARENT_SCOPE)
  set(${cxx_linker_flags} ${linker_flags} PARENT_SCOPE)
  set(${cxx_definitions} ${definitions} PARENT_SCOPE)
endfunction(getCxxCompilerFlags)


#
function(getCxxWarningFlags compiler_warning_flags)
  set(compiler_version ${CMAKE_CXX_COMPILER_VERSION})
  set(environment "${CMAKE_SYSTEM_NAME} ${CMAKE_CXX_COMPILER_ID} ${compiler_version}")

  set(warning_flags "")
  if(Z_ENABLE_COMPILER_WARNING)
    if(Z_GCC)
      getGccWarningFlags(warning_flags)
    elseif(Z_CLANG)
      getClangWarningFlags(warning_flags)
    elseif(Z_MSVC)
      getMsvcWarningFlags(warning_flags)
    else()
      message(WARNING "${environment}: Warning option isn't supported.")
    endif()
  endif()
  # Output variables
  set(${compiler_warning_flags} ${warning_flags} PARENT_SCOPE)
endfunction(getCxxWarningFlags)
