cmake_minimum_required(VERSION 3.0)
project(PSE C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)
find_package(PkgConfig REQUIRED)

# ALSA (system)
pkg_check_modules(ALSA REQUIRED alsa)

# FFTW (optional)
pkg_check_modules(FFTW3 QUIET fftw3)

add_executable(alsa_fft_demo src/alsa_fft_demo.c)
target_compile_options(alsa_fft_demo PRIVATE -Wall -Wextra -O2)
target_include_directories(alsa_fft_demo PRIVATE ${ALSA_INCLUDE_DIRS})
target_link_libraries(alsa_fft_demo PRIVATE ${ALSA_LIBRARIES} m)
if(FFTW3_FOUND)
  target_compile_definitions(alsa_fft_demo PRIVATE USE_FFTW=1)
  target_include_directories(alsa_fft_demo PRIVATE ${FFTW3_INCLUDE_DIRS})
  target_link_libraries(alsa_fft_demo PRIVATE ${FFTW3_LIBRARIES})
endif()
