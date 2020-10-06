#ifndef WEENSYOS_K_VMITER_HH
#define WEENSYOS_K_VMITER_HH
#include "kernel.hh"

// `vmiter` and `ptiter` are iterator types for x86-64 page tables.


// `vmiter` walks over virtual address mappings.
// `pa()` and `perm()` read current addresses and permissions;
// `map()` installs new mappings.

class vmiter {
  public:
    // Initialize a `vmiter` for `pt`, with initial virtual address `va`.
    inline vmiter(x86_64_pagetable* pt, uintptr_t va = 0);
    inline vmiter(const proc* p, uintptr_t va = 0);

    // Return current virtual address
    inline uintptr_t va() const;
    // Return one past last virtual address in this mapping range
    inline uintptr_t last_va() const;
    // Return physical address mapped at `va()`
    inline uint64_t pa() const;
    // Return a kernel-accessible pointer corresponding to `pa()`
    template <typename T = void*>
    inline T kptr() const;

    // Return permissions of current mapping
    inline uint64_t perm() const;
    // Return true iff `va()` is present (`PTE_P`)
    inline bool present() const;
    // Return true iff `va()` is present and writable (`PTE_P|PTE_W`)
    inline bool writable() const;
    // Return true iff `va()` is present and unprivileged (`PTE_P|PTE_U`)
    inline bool user() const;
    // Return intersection of permissions in [va(), va() + sz)
    uint64_t range_perm(size_t sz) const;

    // Move to virtual address `va`; return `*this`
    inline vmiter& find(uintptr_t va);
    // Advance to virtual address `va() + delta`; return `*this`
    inline vmiter& operator+=(intptr_t delta);
    // Advance to virtual address `va() - delta`; return `*this`
    inline vmiter& operator-=(intptr_t delta);
    // Move to next larger page-aligned virtual address, skipping large
    // unmapped regions
    void next();
    // Move to `last_va()`
    void next_range();

    // Map current virtual address to `pa` with permissions `perm`.
    // The current virtual address must be page-aligned. Calls `kalloc`
    // to allocate page table pages if necessary; panics on failure.
    inline void map(uintptr_t pa, int perm);
    // Same, but map a kernel pointer
    inline void map(void* kptr, int perm);

    // Map current virtual address to `pa` with permissions `perm`.
    // The current virtual address must be page-aligned. Calls `kalloc`
    // to allocate page table pages if necessary; returns 0 on success
    // and -1 on failure.
    [[gnu::warn_unused_result]] int try_map(uintptr_t pa, int perm);
    [[gnu::warn_unused_result]] inline int try_map(void* kptr, int perm);

  private:
    x86_64_pagetable* pt_;
    x86_64_pageentry_t* pep_;
    int level_;
    int perm_;
    uintptr_t va_;

    static constexpr int initial_perm = 0xFFF;
    static const x86_64_pageentry_t zero_pe;

    void down();
    void real_find(uintptr_t va);
};


// `ptiter` walks over the page table pages in a page table,
// returning them in depth-first order.
// This is mainly useful when freeing a page table, as in:
// ```
// for (ptiter it(pt); it.active(); it.next()) {
//     kfree(it.kptr());
// }
// kfree(pt);
// ```
// Note that `ptiter` will never visit the level 4 page table page.

class ptiter {
  public:
    // Initialize a physical iterator for `pt` with initial virtual address 0.
    inline ptiter(x86_64_pagetable* pt);
    inline ptiter(const proc* p);

    // Return true iff `ptiter` is still active.
    inline bool active() const;

    // Return current virtual address
    inline uintptr_t va() const;
    // Return one past the last virtual address in this mapping range
    inline uintptr_t last_va() const;
    // Return level of current page table page (0-2).
    inline int level() const;
    // Return kernel pointer to the current page table page.
    inline x86_64_pagetable* kptr() const;
    // Return physical address of current page table page.
    inline uintptr_t pa() const;

    // Move to next page table page in depth-first order.
    inline void next();

  private:
    x86_64_pagetable* pt_;
    x86_64_pageentry_t* pep_;
    int level_;
    uintptr_t va_;

    void go(uintptr_t va);
    void down(bool skip);
};


inline vmiter::vmiter(x86_64_pagetable* pt, uintptr_t va)
    : pt_(pt), pep_(&pt_->entry[0]), level_(3), perm_(initial_perm), va_(0) {
    real_find(va);
}
inline vmiter::vmiter(const proc* p, uintptr_t va)
    : vmiter(p->pagetable, va) {
}
inline uintptr_t vmiter::va() const {
    return va_;
}
inline uintptr_t vmiter::last_va() const {
    return (va_ | pageoffmask(level_)) + 1;
}
inline uint64_t vmiter::pa() const {
    if (*pep_ & PTE_P) {
        uintptr_t pa = *pep_ & PTE_PAMASK;
        if (level_ > 0) {
            pa &= ~0x1000UL;
        }
        return pa + (va_ & pageoffmask(level_));
    } else {
        return -1;
    }
}
template <typename T>
inline T vmiter::kptr() const {
    return reinterpret_cast<T>(pa());
}
inline uint64_t vmiter::perm() const {
    if (*pep_ & PTE_P) {
        return *pep_ & perm_;
    } else {
        return 0;
    }
}
inline bool vmiter::present() const {
    return (*pep_ & PTE_P) == PTE_P;
}
inline bool vmiter::writable() const {
    return (*pep_ & (PTE_P | PTE_W)) == (PTE_P | PTE_W);
}
inline bool vmiter::user() const {
    return (*pep_ & (PTE_P | PTE_U)) == (PTE_P | PTE_U);
}
inline vmiter& vmiter::find(uintptr_t va) {
    real_find(va);
    return *this;
}
inline vmiter& vmiter::operator+=(intptr_t delta) {
    return find(va_ + delta);
}
inline vmiter& vmiter::operator-=(intptr_t delta) {
    return find(va_ - delta);
}
inline void vmiter::next_range() {
    real_find(last_va());
}
inline void vmiter::map(uintptr_t pa, int perm) {
    int r = try_map(pa, perm);
    assert(r == 0);
}
inline void vmiter::map(void* kp, int perm) {
    map((uintptr_t) kp, perm);
}
inline int vmiter::try_map(void* kp, int perm) {
    return try_map((uintptr_t) kp, perm);
}

inline ptiter::ptiter(x86_64_pagetable* pt)
    : pt_(pt) {
    go(0);
}
inline ptiter::ptiter(const proc* p)
    : ptiter(p->pagetable) {
}
inline uintptr_t ptiter::va() const {
    return va_ & ~pageoffmask(level_);
}
inline uintptr_t ptiter::last_va() const {
    return (va_ | pageoffmask(level_)) + 1;
}
inline bool ptiter::active() const {
    return va_ <= VA_NONCANONMAX;
}
inline int ptiter::level() const {
    return level_ - 1;
}
inline void ptiter::next() {
    down(true);
}
inline uintptr_t ptiter::pa() const {
    return *pep_ & PTE_PAMASK;
}
inline x86_64_pagetable* ptiter::kptr() const {
    return reinterpret_cast<x86_64_pagetable*>(pa());
}

#endif
