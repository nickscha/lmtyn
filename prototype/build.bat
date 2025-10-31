gcc -s -O2 test.c -o test.exe
test.exe
ffmpeg -y -framerate 30 -i test_%%5d.ppm -c:v libx264 -pix_fmt yuv420p test.mp4
cp test_00000.ppm out.ppm
rm test_*.ppm
