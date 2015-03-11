ifndef CPPC
	CPPC=g++
endif

CCFLAGS= -g
LIBS= -lOpenCL

DATA_DIR = data

PLATFORM = $(shell uname -s)
ifeq ($(PLATFORM), Darwin)
	CPPC = clang++
	LIBS = -framework OpenCL
endif

convolve:	CLHelpers.cpp\
		StatFile.cpp\
		Timer.cpp\
		main.cpp

	$(CPPC) $^ $(CCFLAGS) $(LIBS) -o $@

clean:
	rm -f convolve
	rm -rf $(DATA_DIR)
