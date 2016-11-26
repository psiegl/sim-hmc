#
# HMCSIM TOP-LEVEL MAKEFILE
#


include Makefile.inc

LIBNAME  := hmcsim
SRCDIR   := src
BUILDDIR := build
LIBS     :=
TARGET   := lib$(LIBNAME).a

.PHONY   : $(TARGET)

SRC      := $(wildcard $(SRCDIR)/*.cpp)
OBJ      := $(SRC:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
DEPS     := $(SRC:$(SRCDIR)/%.cpp,$(BUILDDIR)/%.deps)

-include $(DEPS)

default: $(TARGET)

####### Conditional ##############################

ifneq (,$(findstring HMC_DEBUG, $(HMCSIM_MACROS)))
CFLAGS   += -O0 -g
CXXFLAGS += -O0 -g
else
CFLAGS   += -O3 -ffast-math
CXXFLAGS += -O3 -ffast-math
HMCSIM_MACROS += -DNDEBUG
endif

ifneq (,$(findstring HMC_USES_BOBSIM, $(HMCSIM_MACROS)))
#LIBS     += extern/bobsim/libbobsim.a
CFLAGS   += -DHMCSIM_SUPPORT=1
CXXFLAGS += -DHMCSIM_SUPPORT=1

BOBSRCDIR := extern/bobsim/src
BOBSRC   := $(wildcard $(BOBSRCDIR)/*.cpp)
BOBOBJ   := $(BOBSRC:$(BOBSRCDIR)/%.cpp=$(BOBSRCDIR)/%.o)

$(BOBOBJ):
	@echo "-- BUILDING -- $@"; make -C extern/bobsim/ libbobsim.a CXXFLAGS='-DHMCSIM_SUPPORT -g'
endif

####### Build targets ############################

cppcheck:
	cppcheck --enable=all --enable=information $(wildcard $(SRCDIR)/*.h) $(SRCDIR) main.cpp 2>&1 >/dev/null | less

$(BUILDDIR):
	@echo "[MKDIR] $@"
	@mkdir -p $@

$(OBJ): $(BUILDDIR)/%.o : $(SRCDIR)/%.cpp
	@echo "[$(CXX)]" $@
	@uncrustify  --no-backup -c tools/uncrustify.cfg -q --replace $<
	@$(CXX) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

$(TARGET): $(BUILDDIR) $(OBJ) $(LIBS) $(BOBOBJ)
	@echo "[$(AR)] $@"
	@-$(RM) -f $(TARGET).ar
	@echo " Linking..."; echo "CREATE $@" > $(TARGET).ar
	@for l in $(LIBS); do (echo "ADDLIB $$l" >> $(TARGET).ar); done
	@for o in $(BOBOBJ); do (echo "ADDMOD $$o" >> $(TARGET).ar); done
	@for o in $(OBJ); do (echo "ADDMOD $$o" >> $(TARGET).ar); done
	@echo "SAVE" >> $(TARGET).ar
	@echo "END" >> $(TARGET).ar
	@$(AR) -M < $(TARGET).ar
	@-$(RM) -f $(TARGET).ar

all:
	make clean -C extern/bobsim/ && make && g++ main.cpp libhmcsim.a -lz -g && ./a.out

