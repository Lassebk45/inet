#
# Do not edit this file -- it gets generated from Makefile.inc.in.
# Your changes will be lost the next time ./configure is run.
#
OMNETPP_PRODUCT = OMNeT++
OMNETPP_RELEASE = omnetpp-6.0.1
OMNETPP_VERSION = 6.0.1
OMNETPP_BUILDID = 220831-e4f07d04b8
OMNETPP_EDITION = Academic Public License -- NOT FOR COMMERCIAL USE

#
# Optional features (set to "yes" to enable the feature)
#
WITH_QTENV = yes
WITH_OSG = yes
WITH_OSGEARTH = no
WITH_NETBUILDER = yes
WITH_LIBXML = no
WITH_PARSIM = no
WITH_SYSTEMC = no
WITH_AKAROA = no
PREFER_SQLITE_RESULT_FILES = no

#
# SHARED_LIBS determines whether omnetpp is built as shared or static libs
# By default we use shared libs
#
SHARED_LIBS = yes

#
# Configure MODE specific flags/suffixes. You can add new modes at will.
# Do not forget to run ./configure after editing this file.
#
# Each mode will have a different subdirectory in the output folder (out/).
# $D is a suffix which will be appended to the names of the binaries.
#

# add custom modes
ALL_MODES = release debug sanitize
# this is the default mode
MODE = release
ifeq ($(MODE),release)
  CFLAGS = $(CFLAGS_RELEASE)
  D=
else ifeq ($(MODE),debug)
  CFLAGS = $(CFLAGS_DEBUG)
  D=_dbg
else ifeq ($(MODE),sanitize)
  CFLAGS = $(CFLAGS_DEBUG) $(SANITIZE_FLAGS)
  LDFLAGS = $(SANITIZE_FLAGS)
  D=_sanitize
else
  $(error Unknown mode: '$(MODE)'. Accepted modes: $(ALL_MODES); or define your own mode in 'Makefile.inc.in')
endif
CLEANALL_COMMAND=$(foreach m, $(ALL_MODES), $(MAKE) -s clean MODE=$m; )

PLATFORM = linux
ARCH = x86_64
#
# Directories
#

# NOTE: it is important to evaluate the current path immediately in this file using :=
OMNETPP_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
OMNETPP_IMAGE_PATH := $(OMNETPP_ROOT)/images
OMNETPP_BIN_DIR = $(OMNETPP_ROOT)/bin
OMNETPP_INCL_DIR = $(OMNETPP_ROOT)/include
OMNETPP_LIB_DIR = $(OMNETPP_ROOT)/lib$(OUTPUT_PREFIX)
OMNETPP_OUT_DIR = $(OMNETPP_ROOT)/out$(OUTPUT_PREFIX)
OMNETPP_SRC_DIR = $(OMNETPP_ROOT)/src
OMNETPP_UTILS_DIR = $(OMNETPP_SRC_DIR)/utils
OMNETPP_UI_DIR = $(OMNETPP_ROOT)/ui
OMNETPP_DOC_DIR = $(OMNETPP_ROOT)/doc
OMNETPP_SAMPLES_DIR = $(OMNETPP_ROOT)/samples
OMNETPP_TEST_DIR = $(OMNETPP_ROOT)/test
OMNETPP_TOOLS_DIR = $(OMNETPP_ROOT)/tools/$(PLATFORM).$(ARCH)

# platform specific code
ifeq ($(PLATFORM),win32)
  OMNETPP_IMAGE_PATH := $(shell cygpath -p -m '$(OMNETPP_IMAGE_PATH)')
endif

# use statically built omnetpp if we are building the IDE native helper library
ifeq ($(BUILDING_UILIBS),yes)
  SHARED_LIBS=no
  OUTPUT_PREFIX=/ui
endif

#
# Configname determines where (in which subdirectory of out/)
# makemake-generated makefiles create object files and other
# build artifacts.
#
CONFIGNAME = $(TOOLCHAIN_NAME)-$(MODE)

#
# Control verbosity. Specifiying V=1 will make the build output verbose. You can use:
# $(Q) to hide commands only in quiet mode
# $(qecho) to print out something only in quite mode
# $(vecho) to print out something only in verbose mode
#
ifeq ($(V),1)
  Q :=
  vecho = @echo
  qecho = @true
else
  Q := @
  vecho = @true
  qecho = @echo
endif

# Recursive wildcard function. Call like: $(call opp_rwildcard, src/, *.c *.h)
opp_rwildcard=$(foreach d,$(wildcard $1*),$(call opp_rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

#
# tools, libraries and compiler flags detected by ./configure
#
PERL = perl
YACC = bison -y -Wno-yacc
LEX = flex
CC = clang
CXX = clang++
TOOLCHAIN_NAME = clang
DEFINES = -DHAVE_SWAPCONTEXT
SANITIZE_FLAGS =  -fsanitize=leak,address,undefined
CFLAGS_DEBUG =  -O0 -Wall -glldb -fstandalone-debug
CFLAGS_RELEASE =  -O3 -DNDEBUG=1 -ffp-contract=off
CFLAGS += $(CFLAGS_EXTRA)  -MMD -MP -MF $@.d  -fPIC  -Wno-deprecated-register -Wno-unused-function -fno-omit-frame-pointer $(DEFINES)
CXXFLAGS = 
LDFLAG_LIBPATH = -L
LDFLAG_INCLUDE = -Wl,-u,
LDFLAG_LIB = -l
LDFLAG_IMPLIB = -Wl,--out-implib,
LDFLAG_IMPDEF = -Wl,--output-def,
LDFLAGS += $(LDFLAGS_EXTRA)  -fuse-ld=lld -Wl,-rpath,$(OMNETPP_LIB_DIR) -Wl,-rpath,$(OMNETPP_TOOLS_DIR)/lib -Wl,-rpath,.  -Wl,--export-dynamic $(LDFLAG_LIBPATH)$(OMNETPP_LIB_DIR)
WHOLE_ARCHIVE_ON = -Wl,--whole-archive
WHOLE_ARCHIVE_OFF = -Wl,--no-whole-archive
AS_NEEDED_ON = -Wl,--as-needed
AS_NEEDED_OFF = -Wl,--no-as-needed
PIC_FLAGS = -fPIC
MSGC = opp_msgc
NEDTOOL = opp_nedtool
AS = as
AR_CR = ar cr
ARFLAG_OUT = 
RANLIB = ranlib
SHLIB_LD = clang++ -shared -fPIC
DLLTOOL = dlltool
STRIP = strip
LN = ln -f
MKPATH = mkdir -p
SHLIB_POSTPROCESS = opp_shlib_postprocess
SWIG = not found
QMAKE = /usr/bin/qmake
MOC=/usr/lib/qt5/bin/moc
UIC=/usr/lib/qt5/bin/uic
RCC=/usr/lib/qt5/bin/rcc

HAVE_DLOPEN = 1
SYS_LIBS = -lstdc++ 
QT_CFLAGS = -isystem /usr/include/x86_64-linux-gnu/qt5
QT_LIBS = -lQt5Gui -lQt5Core -lQt5Widgets -lQt5PrintSupport -lQt5OpenGL -L/usr/lib/x86_64-linux-gnu
QTENV_LDFLAGS = -Wl,-rpath=/usr/lib/x86_64-linux-gnu
OSG_CFLAGS = 
OSG_LIBS = -losg -losgDB -losgGA -losgViewer -losgUtil -lOpenThreads
OSGEARTH_CFLAGS = 
OSGEARTH_LIBS = 
ZLIB_CFLAGS = 
ZLIB_LIBS = 
OPENMP_FLAGS = 
MPI_CFLAGS = 
MPI_LIBS = 
PTHREAD_CFLAGS = 
PTHREAD_LIBS = -lpthread
LIBXML_CFLAGS = 
LIBXML_LIBS = 
AKAROA_CFLAGS = 
AKAROA_LIBS = 

#
# platform specific library and executable prefixes and extensions
#
A_LIB_SUFFIX = .a
SO_LIB_SUFFIX = .so
DLL_LIB_SUFFIX = .dll
IMPLIB_SUFFIX = .dll.a
IMPDEF_SUFFIX = .def
SHARED_LIB_SUFFIX = $(SO_LIB_SUFFIX)
JNI_LIB_SUFFIX = $(SHARED_LIB_SUFFIX)
EXE_SUFFIX = 
LIB_PREFIX = lib

#
# omnetpp and system libraries linked with the simulations
#
QTENV_LIBS = $(LDFLAG_INCLUDE)_qtenv_lib $(AS_NEEDED_OFF) $(QTENV_LDFLAGS) -loppqtenv$D -loppenvir$D -lopplayout$D
CMDENV_LIBS = $(LDFLAG_INCLUDE)_cmdenv_lib $(AS_NEEDED_OFF) -loppcmdenv$D -loppenvir$D
ALL_ENV_LIBS = $(CMDENV_LIBS)
KERNEL_LIBS = -loppsim$D
OPPMAIN_LIB = $(LDFLAG_LIB)oppmain$D

#
# flags and libraries required for Qtenv
#
ifeq ($(WITH_QTENV),yes)
  DEFINES += -DWITH_QTENV
  # libraries required for static linking
  ifneq ($(SHARED_LIBS),yes)
    QTENV_LIBS += $(QT_LIBS)
    ifeq ($(WITH_OSG),yes)
      QTENV_LIBS += $(OSG_LIBS)
      KERNEL_LIBS += -losg -lOpenThreads
    endif
    ifeq ($(WITH_OSGEARTH),yes)
      QTENV_LIBS += $(OSGEARTH_LIBS)
      KERNEL_LIBS += -losgEarth
    endif
    ifeq ($(PLATFORM),macos)
      QTENV_LIBS += -framework Carbon
    endif
  endif
  ALL_ENV_LIBS += $(QTENV_LIBS)
endif

#
# Other defines
#
ifeq ($(WITH_PARSIM),yes)
  DEFINES += -DWITH_PARSIM
  # extra libs needed during static building
  ifneq ($(SHARED_LIBS),yes)
    KERNEL_LIBS += $(MPI_LIBS)
  endif
endif

ifeq ($(WITH_NETBUILDER),yes)
  DEFINES += -DWITH_NETBUILDER
endif

ifeq ($(WITH_LIBXML),yes)
  DEFINES += -DWITH_LIBXML
endif

# note: defines for OSG and osgEarth must be available even if WITH_QTENV=no
ifeq ($(WITH_OSG),yes)
  DEFINES += -DWITH_OSG
endif

ifeq ($(WITH_OSGEARTH),yes)
  DEFINES += -DWITH_OSGEARTH
endif


# this macro can be used to pass EXPORT macros to the compiler if omnet is built as shared library
ifeq ($(SHARED_LIBS),yes)
  IMPORT_DEFINES = -DOMNETPPLIBS_IMPORT
  EXPORT_DEFINES = $(EXPORT_MACRO)
endif

#
# handle differences between shared and static lib builds
#
ifeq ($(SHARED_LIBS),yes)
  LIB_SUFFIX =  $(SHARED_LIB_SUFFIX)
else
  LIB_SUFFIX = $(A_LIB_SUFFIX)
  # extra libraries needed when statically linking (because of indirect dependencies)
  KERNEL_LIBS += -loppnedxml$D -loppcommon$D $(LIBXML_LIBS)
endif

#
# Messages for the HELP target
#

define HELP_SYNOPSYS

SYNOPSYS
    make [TARGET] [VARIABLE1=value1 ...] [make-options]
endef
export HELP_SYNOPSYS

define HELP_TARGETS

TARGETS
    all       Build the simulation. (This is the default target.)
              If the MODE variable is not specified, both release and debug mode
              binaries will be created.
    clean     Clean build artifacts belonging to the selected mode
              (MODE=<modename>, or 'release' by default).
    cleanall  Clean build artifacts for all modes.
endef
export HELP_TARGETS

define HELP_OPP_TARGETS
    all       Build the simulator and the samples. (This is the default target.)
              If the MODE variable is not specified, both release and debug mode
              binaries will be created.
    clean     Clean build artifacts belonging to the selected mode
              (MODE=<modename>, or 'release' by default).
    cleanall  Clean build artifacts for all modes.
    apis      Build the API documentation.
    doc       Build the User Guide and the Manual.
    tests     Build and run the automated tests.
    samples   Build only the samples.
endef
export HELP_OPP_TARGETS

define HELP_VARIABLES

VARIABLES
    MODE      'release', 'debug', 'sanitize' or any other user defined string.
              Modes specify what compiler and linker flags are used during the
              build. User defined modes must be added in the 'Makefile.inc.in' file.
              (Look for the 'Configure MODE specific flags/suffixes' section.)
              After adding a new mode, run './configure' again to re-create
              the Makefile.inc file. The output will be available in the
              'out/$${TOOLCHAIN}-$${MODE}' folder.

              Predefined modes are:
              - release: Generate optimized code without debugging info.
              - debug: Generate code for debugging.
              - sanitize: Generate code which is instrumented with various
                sanitizers. The list of available sanitizers depend on the
                compiler. For example, Clang provides AddressSanitizer,
                MemorySanitizer, etc.
                Default options: $(SANITIZE_FLAGS)

    V         Use V=1 to echo all build commands on the console. This is useful
              for debugging the build process.

    CFLAGS_RELEASE, CFLAGS_DEBUG, SANITIZE_FLAGS
              Various flags used by default in 'release', 'debug' or 'sanitize'
              mode. Specifying these on the command line will override the
              default values for the given mode.

    CFLAGS_EXTRA, LDFLAGS_EXTRA
              Allows adding compiler and linker flags to the default command line
              options.

    CFLAGS, LDFLAGS
              Allows overriding the compiler and linker flags.

    CXXFLAGS  Compiler flags that should be passed to the C++ compiler
              only (e.g. -std=c++11).
endef
export HELP_VARIABLES

define HELP_OPP_VARIABLES

    SHARED_LIBS
              Use 'no' to build static libraries.

    WITH_QTENV, WITH_OSG, WITH_OSGEARTH, WITH_NETBUILDER, WITH_LIBXML, WITH_PARSIM, WITH_SYSTEMC
              Turn off features by specifying 'no' for these variables.

    PREFER_SQLITE_RESULT_FILES
              Specify 'yes' to write result files in SQLite database file
              format by default.
endef
export HELP_OPP_VARIABLES

define HELP_EXAMPLES

EXAMPLES
    $$ make MODE=debug
    Build in debug mode.

    $$ make MODE=release V=1
    Build in release mode, show compiler and linker command lines.

    $$ make SHARED_LIBS=no
    Build static instead of dynamic libraries.

    $$ make MODE=release CFLAGS_RELEASE='-O0 -DNDEBUG=1'
    Build in release mode, but do not optimize.

    $$ make MODE=sanitize CFLAGS_EXTRA=-fsanitize-trap=undefined
    Build with the default sanitizers, and in addition, generate
    traps where the code relies on undefined C/C++ language behavior.

    $$ make MODE=sanitize SANITIZE_FLAGS=-fsanitize=memory
    Instrument the code with the memory sanitizer only instead of the
    default sanitizers.

endef
export HELP_EXAMPLES