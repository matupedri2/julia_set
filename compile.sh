
# Para compilar en linux x86/x64
gcc -c -g -Wall -O2 -o obj/julia.o julia.c
# Para compilar ARM
#gcc -c -g -Wall -mfpu=vfp -O2 -o obj/dot_canvas.o dot_canvas.c

gcc -c -g -Wall -O2 -o obj/chrono.o chrono.c

cd obj

gcc -o ../julia.elf julia.o chrono.o  -lm -lSDL 
