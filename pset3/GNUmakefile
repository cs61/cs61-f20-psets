QEMUIMAGEFILES = weensyos.img
all: $(QEMUIMAGEFILES)

# Place local configuration options, such as `CC=clang`, in
# `config.mk` so you don't have to list them every time.
-include config.mk

# `$(V)` controls whether the makefiles print verbose commands (the shell
# commands run by Make) or brief commands (like `COMPILE`).
# For brief commands, run `make all`.
# For verbose commands, run `make V=1 all`.
V = 0
ifeq ($(V),1)
cxxcompile = $(CXX) $(CPPFLAGS) $(DEPCFLAGS) $(1)
assemble = $(CXX) $(CPPFLAGS) $(DEPCFLAGS) $(ASFLAGS) $(1)
link = $(LD) $(LDFLAGS) $(1)
run = $(1) $(3)
else
cxxcompile = @/bin/echo " " $(2) && $(CXX) $(CPPFLAGS) $(DEPCFLAGS) $(1)
assemble = @/bin/echo " " $(2) && $(CXX) $(CPPFLAGS) $(DEPCFLAGS) $(ASFLAGS) $(1)
link = @/bin/echo " " $(2) $(patsubst %.full,%,$@) && $(LD) $(LDFLAGS) $(1)
run = @$(if $(2),/bin/echo " " $(2) $(3) &&,) $(1) $(3)
endif

# `$(D)` controls how QEMU responds to faults. Run `make D=1 run` to
# ask QEMU to print debugging information about interrupts and CPU resets,
# and to quit after the first triple fault instead of rebooting.
#
# `$(NCPU)` controls the number of CPUs QEMU should use. It defaults to 1.
NCPU = 1
LOG ?= file:log.txt
QEMUOPT = -net none -parallel $(LOG) -smp $(NCPU)
ifeq ($(D),1)
QEMUOPT += -d int,cpu_reset,guest_errors -no-reboot
endif
ifneq ($(NOGDB),1)
QEMUGDB ?= -gdb tcp::12949
endif


# Sets of object files

BOOT_OBJS = $(OBJDIR)/bootentry.o $(OBJDIR)/boot.o

KERNEL_OBJS = $(OBJDIR)/k-exception.ko \
	$(OBJDIR)/kernel.ko $(OBJDIR)/k-vmiter.ko \
	$(OBJDIR)/k-hardware.ko $(OBJDIR)/k-memviewer.ko \
	$(OBJDIR)/lib.ko
KERNEL_LINKER_FILES = build/kernel.ld

PROCESS_BINARIES = $(OBJDIR)/p-allocator $(OBJDIR)/p-allocator2 \
	$(OBJDIR)/p-allocator3 $(OBJDIR)/p-allocator4 \
	$(OBJDIR)/p-fork $(OBJDIR)/p-forkexit
PROCESS_LIB_OBJS = $(OBJDIR)/lib.uo $(OBJDIR)/u-lib.uo
ALLOCATOR_OBJS = $(OBJDIR)/p-allocator.uo $(PROCESS_LIB_OBJS)
PROCESS_OBJS = $(OBJDIR)/p-allocator.uo $(OBJDIR)/p-fork.uo \
	$(OBJDIR)/p-forkexit.uo $(PROCESS_LIB_OBJS)
PROCESS_LINKER_FILES = build/process.ld


-include build/rules.mk


# How to make object files

$(OBJDIR)/%.ko: %.cc $(KERNELBUILDSTAMPS)
	$(call cxxcompile,$(KERNELCXXFLAGS) -O1 -DWEENSYOS_KERNEL -c $< -o $@,COMPILE $<)

$(OBJDIR)/%.ko: %.S $(OBJDIR)/k-asm.h $(KERNELBUILDSTAMPS)
	$(call assemble,-O2 -c $< -o $@,ASSEMBLE $<)

$(OBJDIR)/boot.o: $(OBJDIR)/%.o: boot.cc $(KERNELBUILDSTAMPS)
	$(call cxxcompile,$(CXXFLAGS) -Os -fomit-frame-pointer -DWEENSYOS_KERNEL -c $< -o $@,COMPILE $<)

$(OBJDIR)/bootentry.o: $(OBJDIR)/%.o: \
	bootentry.S $(OBJDIR)/k-asm.h $(KERNELBUILDSTAMPS)
	$(call assemble,-Os -fomit-frame-pointer -c $< -o $@,ASSEMBLE $<)

$(OBJDIR)/%.uo: %.cc $(BUILDSTAMPS)
	$(call cxxcompile,$(CXXFLAGS) -O1 -DWEENSYOS_PROCESS -c $< -o $@,COMPILE $<)


# How to make supporting source files

$(OBJDIR)/k-asm.h: kernel.hh lib.hh types.h x86-64.h build/mkkernelasm.awk $(KERNELBUILDSTAMPS)
	$(call cxxcompile,-dM -E kernel.hh | awk -f build/mkkernelasm.awk | sort > $@,CREATE $@)
	@if test ! -s $@; then echo '* Error creating $@!' 1>&2; exit 1; fi


# How to make binaries and the boot sector

$(OBJDIR)/kernel.full: $(KERNEL_OBJS) $(PROCESS_BINARIES) $(KERNEL_LINKER_FILES)
	$(call link,-T $(KERNEL_LINKER_FILES) -o $@ $(KERNEL_OBJS) -b binary $(PROCESS_BINARIES),LINK)

$(OBJDIR)/p-%.full: $(OBJDIR)/p-%.uo $(PROCESS_LIB_OBJS) $(PROCESS_LINKER_FILES)
	$(call link,-T $(PROCESS_LINKER_FILES) -o $@ $< $(PROCESS_LIB_OBJS),LINK)

$(OBJDIR)/p-allocator%.full: $(ALLOCATOR_OBJS) build/p-allocator%.ld
	$(call link,-T build/p-allocator$*.ld -o $@ $(ALLOCATOR_OBJS),LINK)

$(OBJDIR)/kernel: $(OBJDIR)/kernel.full $(OBJDIR)/mkchickadeesymtab
	$(call run,$(OBJDUMP) -C -S -j .text -j .ctors $< >$@.asm)
	$(call run,$(NM) -n $< >$@.sym)
	$(call run,$(OBJCOPY) -j .text -j .rodata -j .data -j .bss -j .ctors -j .init_array $<,STRIP,$@)
	$(call run,$(OBJDIR)/mkchickadeesymtab $@)

$(OBJDIR)/%: $(OBJDIR)/%.full
	$(call run,$(OBJDUMP) -C -S -j .text -j .ctors $< >$@.asm)
	$(call run,$(NM) -n $< >$@.sym)
	$(call run,$(QUIETOBJCOPY) -j .text -j .rodata -j .data -j .bss -j .ctors -j .init_array $<,STRIP,$@)

$(OBJDIR)/bootsector: $(BOOT_OBJS) build/boot.ld
	$(call link,-T build/boot.ld -o $@.full $(BOOT_OBJS),LINK)
	$(call run,$(OBJDUMP) -C -S $@.full >$@.asm)
	$(call run,$(NM) -n $@.full >$@.sym)
	$(call run,$(OBJCOPY) -S -O binary -j .text $@.full $@)


# How to make host program for ensuring a loaded symbol table

$(OBJDIR)/mkchickadeesymtab: build/mkchickadeesymtab.cc $(BUILDSTAMPS)
	$(call run,$(HOSTCXX) $(CPPFLAGS) $(HOSTCXXFLAGS) $(DEPCFLAGS) -g -o $@,HOSTCOMPILE,$<)


# How to make host programs for constructing & checking file systems

$(OBJDIR)/%.o: %.cc $(BUILDSTAMPS)
	$(call run,$(HOSTCXX) $(CPPFLAGS) $(HOSTCXXFLAGS) $(DEPCFLAGS) -c -o $@,HOSTCOMPILE,$<)

$(OBJDIR)/%.o: build/%.cc $(BUILDSTAMPS)
	$(call run,$(HOSTCXX) $(CPPFLAGS) $(HOSTCXXFLAGS) $(DEPCFLAGS) -c -o $@,HOSTCOMPILE,$<)

$(OBJDIR)/mkbootdisk: build/mkbootdisk.cc $(BUILDSTAMPS)
	$(call run,$(HOSTCXX) $(CPPFLAGS) $(HOSTCXXFLAGS) $(DEPCFLAGS) -g -o $@,HOSTCOMPILE,$<)


weensyos.img: $(OBJDIR)/mkbootdisk $(OBJDIR)/bootsector $(OBJDIR)/kernel
	$(call run,$(OBJDIR)/mkbootdisk $(OBJDIR)/bootsector $(OBJDIR)/kernel > $@,CREATE $@)


# How to run QEMU

QEMUIMG = -M q35 \
	-device piix4-ide,bus=pcie.0,id=piix4-ide \
	-drive file=weensyos.img,if=none,format=raw,id=bootdisk \
	-device ide-hd,drive=bootdisk,bus=piix4-ide.0

run: run-$(QEMUDISPLAY)
	@:
run-gdb-report:
	@if test "$(QEMUGDB)" = "-gdb tcp::12949"; then echo '* Run `gdb -x build/weensyos.gdb` to connect gdb to qemu.' 1>&2; fi
run-graphic: $(QEMUIMAGEFILES) check-qemu run-gdb-report
	$(call run,$(QEMU_PRELOAD) $(QEMU) $(QEMUOPT) $(QEMUGDB) $(QEMUIMG),QEMU $<)
run-console: $(QEMUIMAGEFILES) check-qemu-console run-gdb-report
	$(call run,$(QEMU) $(QEMUOPT) -curses $(QEMUGDB) $(QEMUIMG),QEMU $<)
run-monitor: $(QEMUIMAGEFILES) check-qemu
	$(call run,$(QEMU_PRELOAD) $(QEMU) $(QEMUOPT) -monitor stdio $(QEMUIMG),QEMU $<)
run-gdb: run-gdb-$(QEMUDISPLAY)
	@:
run-gdb-graphic: $(QEMUIMAGEFILES) check-qemu
	$(call run,$(QEMU_PRELOAD) $(QEMU) $(QEMUOPT) -gdb tcp::12949 $(QEMUIMG) &,QEMU $<)
	$(call run,sleep 0.5; gdb -x build/weensyos.gdb,GDB)
run-gdb-console: $(QEMUIMAGEFILES) check-qemu-console
	$(call run,$(QEMU) $(QEMUOPT) -curses -gdb tcp::12949 $(QEMUIMG),QEMU $<)

run-$(RUNSUFFIX): run
run-graphic-$(RUNSUFFIX): run-graphic
run-console-$(RUNSUFFIX): run-console
run-monitor-$(RUNSUFFIX): run-monitor
run-gdb-$(RUNSUFFIX): run-gdb
run-gdb-graphic-$(RUNSUFFIX): run-gdb-graphic
run-gdb-console-$(RUNSUFFIX): run-gdb-console

# Kill all my qemus
kill:
	-killall -u $$(whoami) $(QEMU)
	@sleep 0.2; if ps -U $$(whoami) | grep $(QEMU) >/dev/null; then killall -9 -u $$(whoami) $(QEMU); fi
