UNAME 				= $(shell uname -a)
CRAY_PLATFORM_NAME 		= nakuru

ifeq (${CRAY_PLATFORM_NAME}, $(findstring ${CRAY_PLATFORM_NAME}, ${UNAME}))
	#Variables for Nakuru Cray system
	MPICXX 			= CC 
	CXXFLAGS 		= -O3 -Wno-deprecated-register -I/sw/UNCLASSIFIED/glog-master/include
else
	#Default variables
	MPICXX 			= mpic++
	CXXFLAGS 		= -g -O3 -Wno-deprecated-register
endif

SRC := .
OBJ := .

SOURCES := $(wildcard $(SRC)/*.cc)
OBJECTS := $(patsubst $(SRC)/%.cc, $(OBJ)/%.o, $(SOURCES))

all: $(OBJECTS);

$(OBJ)/%.o: $(SRC)/%.cc
	$(MPICXX) $(CPPFLAGS) $(CXXFLAGS) -I$(SRC) -c $< -o $@	

clean:
	rm -f $(OBJECTS)

