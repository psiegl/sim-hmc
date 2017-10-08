#
# HMCSIM TOP-LEVEL MAKEFILE
#


include Makefile.inc

LIBNAME  := hmcsim
SRCDIR   := src
BLDDIR   := bld
LIBDIR   := lib
LIBS     := -lm -ltcmalloc 
CXXFLAGS += -ggdb -fPIC
TARGET   := lib/lib$(LIBNAME).a

.PHONY   : $(TARGET)

SRC      := $(wildcard $(SRCDIR)/*.cpp)

ifeq (,$(findstring HMC_USES_BOBSIM, $(HMCSIM_MACROS)))
SRC      := $(filter-out $(SRCDIR)/hmc_bobsim.cpp, $(SRC))
endif

ifeq (,$(findstring HMC_USES_GRAPHVIZ, $(HMCSIM_MACROS)))
SRC      := $(filter-out $(SRCDIR)/hmc_graphviz.cpp, $(SRC))
else
LIBS     += -lboost_system -lboost_graph -lboost_regex
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
CXXFLAGS += -DHMC_LOGGING
endif

OBJ      := $(SRC:$(SRCDIR)/%.cpp=$(BLDDIR)/%.o)
DEPS     := $(SRC:$(SRCDIR)/%.cpp,$(BLDDIR)/%.deps)

-include $(DEPS)

default: $(TARGET)
	make -C extern/wrapper-gc64hmcsim2.0

####### Conditional ##############################

ifneq (,$(findstring HMC_DEBUG, $(HMCSIM_MACROS)))
CXXFLAGS += -O -g #-fsanitize=address -fsanitize=alignment -fsanitize=bounds -fsanitize=object-size -fsanitize=shift -fsanitize=undefined 
else
CXXFLAGS += -Ofast -ffast-math -march=native
HMCSIM_MACROS += -DNDEBUG
ifneq (,$(findstring HMC_USES_CRC, $(HMCSIM_MACROS)))
LIBS     += -lz
endif
endif
ifneq (,$(findstring HMC_PROF, $(HMCSIM_MACROS)))
CXXFLAGS += -pg
endif

ifneq (,$(findstring HMC_USES_BOBSIM, $(HMCSIM_MACROS)))
BOBSRCDIR := extern/bobsim/src
BOBSRC    := $(wildcard $(BOBSRCDIR)/*.cpp)
BOBOBJ    := $(BOBSRC:$(BOBSRCDIR)/%.cpp=$(BLDDIR)/%.o)
CXXFLAGS  += -DHMCSIM_SUPPORT=1
ifneq (,$(findstring HMC_FAST_BOBSIM, $(HMCSIM_MACROS)))
CXXFLAGS  += -DBOBSIM_NO_LOG=1
endif

$(BOBOBJ): $(BLDDIR)/%.o : $(BOBSRCDIR)/%.cpp
	@echo "[$(CXX)]" $@
	@uncrustify  --no-backup -c tools/uncrustify.cfg -q --replace $<
	@$(CXX) $(CXXFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<
endif

####### Build targets ############################

cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem --enable=information $(wildcard $(SRCDIR)/*.h) $(SRCDIR) main.cpp 2>&1 >/dev/null | less

$(BLDDIR):
	@echo "[mkdir] $@"
	@mkdir -p $@

$(LIBDIR):
	@echo "[mkdir] $@"
	@mkdir -p $@

$(OBJ): $(BLDDIR)/%.o : $(SRCDIR)/%.cpp
	@echo "[$(CXX)]" $@
	@uncrustify  --no-backup -c tools/uncrustify.cfg -q --replace $<
	@$(CXX) $(CXXFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

$(BLDDIR)/asm.ar: $(BLDDIR) $(LIBDIR) $(LIB) $(OBJ) $(BOBOBJ)
	@echo "[echo] $@"
	@echo "CREATE $(TARGET)" > $@
	@for o in $(BOBOBJ); do (echo "ADDMOD $$o" >> $@); done
	@for o in $(OBJ); do (echo "ADDMOD $$o" >> $@); done
	@echo "SAVE" >> $@
	@echo "END" >> $@

$(TARGET): $(BLDDIR)/asm.ar
	@echo "[$(AR)] $@"
	@$(AR) -M < $(BLDDIR)/asm.ar


TESTBIN := main.elf
$(TESTBIN): $(TARGET)
	@echo "[$(CXX)]" $@
	@$(CXX) $(CXXFLAGS) $(HMCSIM_MACROS) -o $@ main.cpp $(TARGET) $(LIBS) # -static

run: $(TESTBIN)
	@./$(TESTBIN)

ifneq (,$(findstring HMC_PROF, $(HMCSIM_MACROS)))
prof: runall
	@gprof $(TESTBIN) gmon.out > $(TESTBIN).prof.txt
	@gprof $(TESTBIN) | gprof2dot -w | dot -Tpng -o $(TESTBIN).prof.png
endif

clean:
	@rm -rf $(TARGET) $(BLDDIR) lib/* *.prof.* hmcsim.db gmon.out

perf_anno: $(TESTBIN)
	perf record -e cpu-clock,faults,cycles ./$(TESTBIN)
	perf report

perf_stat: $(TESTBIN)
	perf stat -r 30 ./$(TESTBIN)
