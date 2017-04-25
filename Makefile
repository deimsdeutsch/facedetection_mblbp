##################################################
# Copyright (c) 2017 Xiaozhe Yao                 # 
# xiaozhe.yao@gmail.com                          #
##################################################/

CXX = g++

objects = common.o image-resize.o mblbp-detect.o mblbp-detect-mview.o tinyxml2.o main.o

main : $(objects)
	$(CXX) -Wall -std=c++11 -o main $(objects)

main.o : mblbp-internal.h mblbp-detect-mview.h
common.o : common.h
image-resize.o : 
tinyxml2.o : tinyxml2.h
mblbp-detect.o : mblbp-detect.h mblbp-internal.h tinyxml2.h
mblbp-detect-mview.o : mblbp-internal.h mblbp-detect-mview.h 


.PHONY: clean help
clean:
	rm *.o
	rm main
	@echo Cleaned!

help:
	@echo ===============================================
	@echo = Make Tools for Face Detection Program V1.0  =
	@echo = Author Xiaozhe Yaoi xiaozhe.yaoi@gmail.com  =
	@echo ===============================================