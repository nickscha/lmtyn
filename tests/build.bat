@echo off

set DEF_FLAGS_COMPILER=-std=c89 -pedantic -Wall -Wextra -Werror -Wvla -Wconversion -Wdouble-promotion -Wsign-conversion -Wmissing-field-initializers -Wuninitialized -Winit-self -Wunused -Wunused-macros -Wunused-local-typedefs
set DEF_FLAGS_LINKER=
set SOURCE_NAME=lmtyn_test

cc -s -O2 %DEF_FLAGS_COMPILER% -o %SOURCE_NAME%.exe %SOURCE_NAME%.c %DEF_FLAGS_LINKER%
%SOURCE_NAME%.exe
ffmpeg -hide_banner -loglevel error -y -framerate 30 -i test_%%5d.ppm -c:v libx264 -pix_fmt yuv420p test.mp4
cp test_00000.ppm out.ppm
rm test_*.ppm