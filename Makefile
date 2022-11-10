CUDA_ROOT_DIR := /usr/local/cuda

NVCC := $(CUDA_ROOT_DIR)/bin/nvcc
NVCC_FLAGS := --ptxas-options=-v --compiler-options '-fPIC'

CUDA_INCLUDE := $(CUDA_ROOT_DIR)/include
CUDA_LIB_DIR=$(CUDA_ROOT_DIR)/lib64


PYTHON_INCLUDE := $(shell python -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())")

GNU_LIB := /usr/lib/x86_64-linux-gnu


INCLUDES := -I$(CUDA_INCLUDE) -I$(PYTHON_INCLUDE) -I./src/include
LIBRARIES := -L$(GNU_LIB) -lunwind -L$(CUDA_LIB_DIR) -lcuda -lcupti

SOURCE := $(wildcard ./src/*.cpp)
OBJECTS := $(SOURCE:.cpp=.o)
TARGET := daydream.so

all: $(OBJECTS)
	$(NVCC) -shared -o $(TARGET) $(OBJECTS) $(LIBRARIES) $(INCLUDES)
	rm -f $(OBJECTS)

%.o: %.cpp
	$(NVCC) $(NVCC_FLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean

clean:
	rm -f $(OBJECTS) $(TARGET)