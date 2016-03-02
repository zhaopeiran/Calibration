all: selectPupil

INCDIR=../../inc
OPENCV=/opt/local/include
OPENCVL=/opt/local/lib

selectPupil: selectPupil.cpp 
	g++ -g -o selectPupil.bin selectPupil.cpp -I$(INCDIR) -I$(OPENCV) -L$(OPENCVL) -std=c++11 -lopencv_core -lopencv_highgui -lopencv_imgproc

clean:
	rm -f selectPupil.bin
