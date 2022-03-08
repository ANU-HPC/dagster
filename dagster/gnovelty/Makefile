
#Default variables
MPICXX 			= mpic++
CXXFLAGS 		= -g -O3 -Wno-deprecated-register

SRC := .
OBJ := .

SOURCES := $(wildcard $(SRC)/*.cc)
OBJECTS := $(patsubst $(SRC)/%.cc, $(OBJ)/%.o, $(SOURCES))

all: $(OBJECTS);

$(OBJ)/%.o: $(SRC)/%.cc
	$(MPICXX) $(CPPFLAGS) $(CXXFLAGS) -I$(SRC) -c $< -o $@	

clean:
	rm -f $(OBJECTS)

