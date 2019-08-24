# Makefile for sample program
.PHONY			: all clean

# the program to build
NAME			:= main

VPATH = ImageProcessing:serialSend:canInit
# Build tools and flags
CXX			:= /usr/bin/g++
LD			:= /usr/bin/g++

CPPFLAGS		:= -w -std=c++11 -I/usr/include/ \
			-I./serialSend　-I./ImageProcessing -I./canInit -I./


LDFLAGS			:=-lgxiapi -lpthread \
-L/usr/lib/arm-linux-gnueabihf \
-lopencv_calib3d -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_video -lopencv_videoio -lopencv_videostab


all			: $(NAME)


$(NAME)			: $(NAME).o serialSend.o ImageProcessing.o canInit.o
	$(LD) -o $@ $^ $(CPPFLAGS) $(LDFLAGS)

*.o : *.cpp
	$(CXX) $(CPPFLAGS) -c -o $@ $<

clean			:
	$(RM) *.o $(NAME)
clean_obj		:
	$(RM) *.o



