#VARIANT?=G
CC=clang
CXX=clang++
DEVICE?=DDR3_1333
CXXFLAGS=-O3 -g
LIB_NAME=libbobsim.so
EXE_NAME=BOBSim
LINKFLAGS=

SRC = $(wildcard *.cpp)

OBJ = $(subst LibraryStubs.o,,$(addsuffix .o, $(basename $(SRC))))

LOBJ = $(patsubst RandomStreamSim%.lo,,$(addsuffix .lo, $(basename $(SRC))))

all: ${EXE_NAME}

#   $@ target name, $^ target deps, $< matched pattern
$(EXE_NAME): $(OBJ)
	$(CXX) $(LINK_FLAGS) -o $@ $^ 
	@echo "Built $@ successfully" 

#for now, I'm assuming that -ltcmalloc will be linked with the binary, not the library
$(LIB_NAME): $(LOBJ)
	$(CXX) -shared -Wl,-soname,$@ -o $@ $^
	@echo "Built $@ successfully" 

#include the autogenerated dependency files for each .o file
-include $(OBJ:.o=.dep)
-include $(LOBJ:.lo=.dep)

# build dependency list via gcc -M and save to a .dep file
%.dep : %.cpp
	@$(CXX) -M $(CXXFLAGS) $< > $@

%.po : %.cpp
	$(CXX) -O3 -ffast-math -fPIC -DNO_STORAGE -o $@ -c $<

# build all .cpp files to .o files
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -D${DEVICE} -o $@ -c $<

%.lo : %.cpp
	$(CXX) $(CXXFLAGS) -D${DEVICE} -DLOG_OUTPUT -fPIC -o $@ -c $< 

clean: 
	-rm -f $(EXE_NAME) $(LIB_NAME) *.lo *.o *.so *.dep *.txt tmp.log

test: $(EXE_NAME)
	@ls -lh $(EXE_NAME)
	@ls -l $(EXE_NAME)
	@./$(EXE_NAME) -c 2000 > tmp.log
	@diff tmp.log zz_ref_C2000.log
  
