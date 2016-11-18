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

SOURCES  := $(shell find $(SRCDIR) -type f -name "*.cpp")
OBJECTS  := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))
DEPS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.deps))

-include $(DEPS)

default: $(TARGET)

####### Conditional ##############################

ifneq (,$(findstring HMC_DEBUG, $(HMCSIM_MACROS)))
CFLAGS   += -g
CXXFLAGS += -g
else
HMCSIM_MACROS += -DNDEBUG
endif

ifneq (,$(findstring HMC_USES_BOBSIM, $(HMCSIM_MACROS)))
LIBS     += extern/bobsim/libbobsim.a

extern/bobsim/libbobsim.a:
	@echo "-- BUILDING -- $@"; make -C extern/bobsim/ libbobsim.a
endif

####### Build targets ############################

cppcheck:
	cppcheck --enable=all --enable=information $(INC) $(SRCDIR) 2>&1 >/dev/null | less

$(BUILDDIR):
	@echo "[MKDIR] $@"
	@mkdir -p $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(BUILDDIR)
	@echo "[$(CXX)] $@"
	@uncrustify  --no-backup -c tools/uncrustify.cfg -q --replace $<
	@$(CXX) $(CFLAGS) -MD -MF $(@:.o=.deps) -c -o $@ $<

$(TARGET): $(OBJECTS) $(LIBS)
	@echo "[$(AR)] $@"
	@-$(RM) -f $(TARGET).ar
	@echo " Linking..."; echo "CREATE $@" > $(TARGET).ar
	@for l in $(LIBS); do (echo "ADDLIB $$l" >> $(TARGET).ar); done
	@for o in $(OBJECTS); do (echo "ADDMOD $$o" >> $(TARGET).ar); done
	@echo "SAVE" >> $(TARGET).ar
	@echo "END" >> $(TARGET).ar
	@$(AR) -M < $(TARGET).ar
	@-$(RM) -f $(TARGET).ar
