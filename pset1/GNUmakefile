# Default optimization level
O ?= 2

TESTS = $(patsubst %.cc,%,$(sort $(wildcard test[0-9][0-9][0-9].cc)))

all: $(TESTS) hhtest

-include build/rules.mk

LIBS = -lm

%.o: %.cc $(BUILDSTAMP)
	$(call run,$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPCFLAGS) $(O) -o $@ -c,COMPILE,$<)

all:
	@echo "*** Run 'make check' or 'make check-all' to check your work."

test%: m61.o basealloc.o test%.o
	$(call run,$(CXX) $(CXXFLAGS) $(O) -o $@ $^ $(LDFLAGS) $(LIBS),LINK $@)

hhtest: m61.o basealloc.o hhtest.o
	$(call run,$(CXX) $(CXXFLAGS) $(O) -o $@ $^ $(LDFLAGS) $(LIBS),LINK $@)

check: $(patsubst %,run-%,$(TESTS))
	@echo "*** All tests succeeded!"

check-all: $(TESTS)
	@good=true; for i in $(TESTS); do $(MAKE) --no-print-directory run-$$i || good=false; done; \
	if $$good; then echo "*** All tests succeeded!"; fi; $$good

check-%:
	@any=false; good=true; for i in `perl check.pl -e "$*"`; do \
	    any=true; $(MAKE) run-$$i || good=false; done; \
	if $$any; then $$good; else echo "*** No such test" 1>&2; $$any; fi

run-:
	@echo "*** No such test" 1>&2; exit 1

run-%: %
	@test -d out || mkdir out
	@perl check.pl -x $<

clean: clean-main
clean-main:
	$(call run,rm -f $(TESTS) hhtest *.o core *.core,CLEAN)
	$(call run,rm -rf out *.dSYM $(DEPSDIR))

distclean: clean

MALLOC_CHECK_=0
export MALLOC_CHECK_

.PRECIOUS: %.o
.PHONY: all clean clean-main clean-hook distclean \
	run run- run% prepare-check check check-all check-%
