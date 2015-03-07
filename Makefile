#project name
TARGET = OpenCLInfo

CC = g++
CFLAGS = -Wall -W -O2 

is_64=$(shell s=`uname -m`; if (echo $$s | grep x86_64 > /dev/null); then echo 1; fi)

# amd
ifeq ($(is_64), 1)
SDK_LIB_AMD=$(AMDAPPSDKROOT)/lib/x86_64
else
SDK_LIB_AMD=$(AMDAPPSDKROOT)/lib/x86
endif
SDK_INC_AMD=$(AMDAPPSDKROOT)/include

# intel
ifeq ($(is_64), 1)
SDK_LIB_INTEL=/usr/lib64
endif
SDK_INC_INTEL=/usr/include

# nvidia
SDK_LIB_NVIDIA=/usr/lib
SDK_INC_NVIDIA=/usr/local/cuda/include

# libraries for make
INCLUDELIBS=$(SDK_LIB_AMD) $(SDK_LIB_INTEL) $(SDK_LIB_NVIDIA)

# includes for make
INCLUDEDIRS=$(SDK_INC_AMD) $(SDK_INC_INTEL) $(SDK_INC_NVIDIA)

# OpenCL SDK libraries
LLIBS:= OpenCL

SDKLIBS  := $(foreach f, $(LLIBS), -l$(f))
CPPFLAGS := $(foreach f, $(INCLUDELIBS), -L$(f))
CPPFLAGS += $(foreach f, $(INCLUDEDIRS), -I$(f))

SRCFILES += $(TARGET).cpp

OBJS = $(SRCFILES:.cpp=.o)

$(TARGET) : $(OBJS)
$(TARGET).o : $(TARGET).cpp

all default: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CFLAGS) $(SRCS) $(CPPFLAGS) $(OBJS) -o $@ $(SDKLIBS)

clobber:
	rm -rf $(TARGET) $(OBJS)

clean:
	rm -rf $(OBJS)
