ifndef CPPC
	CPPC=g++
endif

CPP_COMMON = ../../Cpp_common
CCFLAGS= -g

INC = -I $(CPP_COMMON)
LIBS = -lOpenCL

DATA_DIR = data

# Check our platform and make sure we define the APPLE variable
# and set up the right compiler flags and libraries
PLATFORM = $(shell uname -s)
ifeq ($(PLATFORM), Darwin)
	CPPC = clang++
	LIBS = -framework OpenCL
endif

convolve:	StatFile.cpp\
		Timer.cpp\
		main.cpp

	$(CPPC) $^ $(INC) $(CCFLAGS) $(LIBS) -o $@

clean:
	rm -f convolve
	rm -rf $(DATA_DIR)
