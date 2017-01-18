#
# HMCSIM TOP-LEVEL MAKEFILE
#


include Makefile.inc

LIBNAME  := hmcsim
SRCDIR   := src
BLDDIR   := build
LIBS     :=
TARGET   := lib/lib$(LIBNAME).a

.PHONY   : $(TARGET)

SRC      := $(wildcard $(SRCDIR)/*.cpp)
ifeq (,$(findstring HMC_USES_BOBSIM, $(HMCSIM_MACROS)))
SRC      := $(filter-out $(SRCDIR)/hmc_bobsim.cpp, $(SRC))
endif
ifeq (,$(findstring HMC_USES_GRAPHVIZ, $(HMCSIM_MACROS)))
SRC      := $(filter-out $(SRCDIR)/hmc_graphviz.cpp, $(SRC))
endif

ifeq (,$(findstring HMC_LOGGING_SQLITE3, $(HMCSIM_MACROS)))
SRC      := $(filter-out $(SRCDIR)/hmc_trace_sqlite3.cpp, $(SRC))
else
LIBS     += -lsqlite3
endif
ifeq (,$(findstring HMC_LOGGING_POSTGRESQL, $(HMCSIM_MACROS)))
SRC      := $(filter-out $(SRCDIR)/hmc_trace_postgresql.cpp, $(SRC))
else
LIBS     += -lpqxx -lpq
endif
ifeq (,$(findstring HMC_LOGGING, $(HMCSIM_MACROS)))
SRC      := $(filter-out $(SRCDIR)/hmc_trace.cpp, $(SRC))
else
CFLAGS   += -DHMC_LOGGING
CXXFLAGS += -DHMC_LOGGING
endif
OBJ      := $(SRC:$(SRCDIR)/%.cpp=$(BLDDIR)/%.o)
DEPS     := $(SRC:$(SRCDIR)/%.cpp,$(BLDDIR)/%.deps)

-include $(DEPS)

default: $(TARGET)

####### Conditional ##############################

ifneq (,$(findstring HMC_DEBUG, $(HMCSIM_MACROS)))
CFLAGS   += -O -g -pg #-fsanitize=address -fno-omit-frame-pointer -fsanitize=alignment -fsanitize=bounds -fsanitize=object-size -fsanitize=shift
CXXFLAGS += -O -g -pg #-fsanitize=address -fsanitize=alignment -fsanitize=bounds -fsanitize=object-size -fsanitize=shift -fsanitize=undefined 
else
CFLAGS   += -O3 -ffast-math -fPIC
CXXFLAGS += -O3 -ffast-math -fPIC
HMCSIM_MACROS += -DNDEBUG
endif

ifneq (,$(findstring HMC_USES_GRAPHVIZ, $(HMCSIM_MACROS)))
LIBS      += -lboost_system -lboost_graph -lboost_regex
endif

ifneq (,$(findstring HMC_USES_BOBSIM, $(HMCSIM_MACROS)))
BOBSRCDIR := extern/bobsim/src
BOBSRC    := $(wildcard $(BOBSRCDIR)/*.cpp)
BOBOBJ    := $(BOBSRC:$(BOBSRCDIR)/%.cpp=$(BLDDIR)/%.o)
CFLAGS    += -DHMCSIM_SUPPORT=1
CXXFLAGS  += -DHMCSIM_SUPPORT=1
ifneq (,$(findstring HMC_FAST_BOBSIM, $(HMCSIM_MACROS)))
CFLAGS    += -DBOBSIM_NO_LOG=1
CXXFLAGS  += -DBOBSIM_NO_LOG=1
endif

$(BOBOBJ): $(BLDDIR)/%.o : $(BOBSRCDIR)/%.cpp
	@echo "[$(CXX)]" $@
	@uncrustify  --no-backup -c tools/uncrustify.cfg -q --replace $<
	@$(CXX) $(CXXFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<
endif

####### Build targets ############################

cppcheck:
	cppcheck --enable=all --enable=information $(wildcard $(SRCDIR)/*.h) $(SRCDIR) main.cpp 2>&1 >/dev/null | less

$(BLDDIR):
	@echo "[MKDIR] $@"
	@mkdir -p $@

$(OBJ): $(BLDDIR)/%.o : $(SRCDIR)/%.cpp
	@echo "[$(CXX)]" $@
	@uncrustify  --no-backup -c tools/uncrustify.cfg -q --replace $<
	@$(CXX) $(CXXFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

$(TARGET): $(BLDDIR) $(OBJ) $(BOBOBJ)
	@echo "[$(AR)] $@"
	@-$(RM) -f $(TARGET).ar
	@echo " Linking..."; echo "CREATE $@" > $(TARGET).ar
	@for o in $(BOBOBJ); do (echo "ADDMOD $$o" >> $(TARGET).ar); done
	@for o in $(OBJ); do (echo "ADDMOD $$o" >> $(TARGET).ar); done
	@echo "SAVE" >> $(TARGET).ar
	@echo "END" >> $(TARGET).ar
	@$(AR) -M < $(TARGET).ar
	@-$(RM) -f $(TARGET).ar


TESTBIN := main.elf
$(TESTBIN): $(TARGET)
	$(CXX) $(CFLAGS) -o $@ main.cpp $(TARGET) -lz -ltcmalloc $(LIBS)

bldall:
	make clean
	make $(TESTBIN)

runall: 
	./$(TESTBIN)
	#gprof $(TESTBIN) gmon.out > $(TESTBIN).anal.txt
	#gprof $(TESTBIN) | gprof2dot | dot -Tpng -o output.png

clean:
	rm -rf $(TARGET) $(BLDDIR) lib/*
