
-include makefile.init



__mkslibname = lib$(1).so
__mkalibname = lib$(1).a
__mkbinname = $(1)

__mkbin = g++ -fPIC -rdynamic -g3 $(LIB_PATH) $(1) $(2) $(LIBS) -o $(3)

__mkslib = g++ -fPIC -rdynamic $(LIB_PATH) -shared $(1) $(2) $(LIBS) -o $(3)

__mkalib = g++ -fPIC -rdynamic $(LIB_PATH) -static $(1) $(2) $(LIBS) -o $(3)

ifeq ($(CONFIGURATION),Release)
__mkobj = g++ -std=c++0x -O -g -Wall -fPIC -c $(DEFINES) $(INCLUDE_PATH) -fmessage-length=0 -MMD -MP -MF$(2) -MT$(2) -o $(2:%.d=%.o) $(1)
endif

ifeq ($(CONFIGURATION),Debug) 
__mkobj = g++ -std=c++0x -O3 -g3 -Wall -fPIC -c $(DEFINES) $(INCLUDE_PATH) -fmessage-length=0 -MMD -MP -MF$(2) -MT$(2) -o $(2:%.d=%.o) $(1)
endif


-include prebuild.mk
-include $(SRCDIR)/targets.mk
-include $(SRCDIR)/prebuild.mk

###################################################
# Get the list of the sources to be build
##################################################

#_SRC=

_ALLTARGETS = $(_SLIB_TARGETS) $(_ALIB_TARGETS) $(_BIN_TARGETS)

_SRC+= $(foreach __TARGET,$(_ALLTARGETS),$($(__TARGET)_SRC))

_OBJS = $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(_SRC)))

_DEPS = $(subst .o,.d,$(_OBJS))

INCLUDE_PATH += $(addprefix -I, $(foreach __TARGET, $(_ALLTARGETS),$($(__TARGET)_INCLUDE)))
LIB_PATH += $(addprefix -L, $(foreach __TARGET, $(_ALLTARGETS),$($(__TARGET)_LIBPATH)))
LIBS += $(foreach __TARGET, $(_ALLTARGETS),$($(__TARGET)_LIBS))
DEFINES += $(addprefix -D, $(foreach __TARGET, $(_ALLTARGETS),$($(__TARGET)_DEFINESS)))





_BUILD_TARGETS=

_BUILD_SLIBTARGETS+=$(foreach __TARGET, $(_SLIB_TARGETS), $(call __mkslibname,$(__TARGET))) 
_BUILD_ALIBTARGETS+=$(foreach __TARGET, $(_ALIB_TARGETS), $(call __mkalibname, $(__TARGET))) 
_BUILD_BINTARGETS+=$(foreach __TARGET, $(_BIN_TARGETS), $(call __mkbinname, $(__TARGET))) 

_BUILD_TARGETS=$(addprefix $(LIBDIR)/,$(_BUILD_SLIBTARGETS)) $(addprefix $(LIBDIR)/,$(_BUILD_ALIBTARGETS)) $(addprefix $(BINDIR)/,$(_BUILD_BINTARGETS))



-include $(_DEPS)


$(_DEPS): $(OBJDIR)/%.d : $(SRCDIR)/%.cpp 
	-mkdir -p $(dir $(_OBJS))
	$(call __mkobj,$<,$@)


$(BINDIR):
	-mkdir -p $(BINDIR)

$(LIBDIR):
	-mkdir -p $(LIBDIR)


######################################################################
# Define templates for building shared libraries static libraries and binaries
######################################################################


define __bin_target_template
$$(BINDIR)/$(2) : $$(addprefix $$(OBJDIR)/,$$(subst .cpp,.o,$$($(1)_SRC)))
	$(call __mkbin,$$^,$$($(1)_LIB),$$@)
endef

define __slib_target_template
$$(LIBDIR)/$(2) : $$(addprefix $$(OBJDIR)/,$$(subst .cpp,.o,$$($(1)_SRC)))
	$(call __mkslib,$$^,$$($(1)_LIB),$$@)
endef

define __alib_target_template
$$(LIBDIR)/$(2) : $$(addprefix $$(OBJDIR)/,$$(subst .cpp,.o,$$($(1)_SRC)))
	$(call __mkalib,$$^,$$($(1)_LIB),$$@)
endef

#Build old the _BIN targets
$(foreach __ALIBTARG,$(_ALIB_TARGETS),$(eval $(call __alib_target_template,$(__ALIBTARG),$(call __mkalibname,$(__ALIBTARG)))))

$(foreach __SLIBTARG,$(_SLIB_TARGETS),$(eval $(call __slib_target_template,$(__SLIBTARG),$(call __mkslibname,$(__SLIBTARG)))))

$(foreach __BINTARG,$(_BIN_TARGETS),$(eval $(call __bin_target_template,$(__BINTARG),$(call __mkbinname,$(__BINTARG)))))

$(foreach __ALIBTARG,$(_ALIB_TARGETS),$(eval $(__ALIBTARG) : $(OBJDIR) $(LIBDIR) $(LIBDIR)/$(__ALIBTARG)))

$(foreach __SLIBTARG,$(_SLIB_TARGETS),$(eval $(__SLIBTARG) : $(OBJDIR) $(LIBDIR) $(LIBDIR)/$(__SLIBTARG)))

$(foreach __BINTARG,$(_BIN_TARGETS),$(eval $(__BINTARG) : $(OBJDIR) $(BINDIR) $(BINDIR)/$(__BINTARG)))



all : prebuild $(_BUILD_TARGETS) 
	echo "Building $^"

clean : 
	rm -f $(_BUILD_TARGETS) $(_OBJS) $(_DEPS)




