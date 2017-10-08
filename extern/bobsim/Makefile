#VARIANT?=G
CC=clang
CXX=clang++
SRCDIR=src
DEVICE?=DDR3_1333
CXXFLAGS += -g -fPIC
LIB_NAME=bobsim
EXE_NAME=BOBSim
LINKFLAGS=
INC=-Iinclude/ -Iinclude/cfg

SRC = $(wildcard $(SRCDIR)/*.cpp)

EXE_OBJ = $(wildcard example/*.cpp)

OBJ = $(addsuffix .o, $(basename $(SRC)))
#LOBJ = $(addsuffix .lo, $(basename $(SRC)))

all: lib${LIB_NAME}.a

$(EXE_NAME): $(EXE_OBJ) lib$(LIB_NAME).a
	$(CXX) $(CXXFLAGS) -D${DEVICE} $(INC) $(LINKFLAGS) -o $@ $< lib$(LIB_NAME).a
	@echo "Built $@ successfully" 

#for now, I'm assuming that -ltcmalloc will be linked with the binary, not the library
#lib$(LIB_NAME).so: $(LOBJ)
#	$(CXX) -shared -Wl,-soname,$@ -o $@ $^
#	@echo "Built $@ successfully" 

lib$(LIB_NAME).a: $(OBJ)
	ar rcs -o $@ $^
	@echo "Built $@ successfully"

%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -D${DEVICE} -o $@ -c $<

clean: 
	-rm -f $(EXE_NAME) $(EXE_NAME).shared lib$(LIB_NAME).* $(SRCDIR)/*.lo $(SRCDIR)/*.o *.so $(SRCDIR)/*.dep *.txt tmp.log

test: $(EXE_NAME)
	@ls -lh $(EXE_NAME)
	@ls -l $(EXE_NAME)
	@LD_LIBRARY_PATH=$(shellpwd) ./$(EXE_NAME) -c 2000 > tmp.log
	@diff tmp.log example/zz_ref_C2000.log

cppcheck:
	cppcheck --enable=all --enable=information $(INC) example $(SRCDIR) 2>&1 >/dev/null | less
