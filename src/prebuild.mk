

$(SRCDIR)/messages.hpp : $(SRCDIR)/messages.txt
	echo "Building $<"
	./tools/messagec.py --msgfile $< --hppfile $@

prebuild : 
#prebuild : $(SRCDIR)/messages.hpp
