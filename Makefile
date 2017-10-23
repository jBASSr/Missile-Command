# cs335 missileCommand Project
# to compile the project, type make and press enter
# JBC Added "-g" to compile line to add debugging info 5/5/16

all: missileCommand

missileCommand:  missileCommand.cpp johnC.cpp danielT.cpp joseR.cpp joseG.cpp ppm.cpp
	g++ -g missileCommand.cpp johnC.cpp danielT.cpp joseR.cpp joseG.cpp ppm.cpp \
	libggfonts.a -Wall -lX11 -lGL -lGLU -lalut -lm \
	/usr/lib/x86_64-linux-gnu/libopenal.so \
	-o missileCommandMain

clean:
	rm -f missileCommand
	rm -f *.o

