# Default optimization level
O ?= 2

all: simpong61 pong61 proxypong61

WANT_TSAN = 1
-include build/rules.mk

LIBS = -lm

%.o: %.cc $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) $(O) -o $@ -c,COMPILE,$<)

simpong61: simpong61.o helpers.o
	$(call run,$(CXX) $(CXXFLAGS) $(O) -o $@ $^ $(LDFLAGS) $(LIBS),LINK $@)

pong61: pong61.o helpers.o
	$(call run,$(CXX) $(CXXFLAGS) $(O) -o $@ $^ $(LDFLAGS) $(LIBS),LINK $@)

proxypong61: proxypong61.o helpers.o
	$(call run,$(CXX) $(CXXFLAGS) $(O) -o $@ $^ $(LDFLAGS) $(LIBS),LINK $@)

check: always
	perl checksim.pl

clean: clean-main
clean-main:
	$(call run,rm -rf simpong61 pong61 proxypong61 *.o *~ *.bak core *.core,CLEAN)
	$(call run,rm -rf out *.dSYM $(DEPSDIR))

.PRECIOUS: %.o
.PHONY: all clean clean-main clean-hook distclean run check check-% prepare-check
