// EPOS ARMv8 CPU Mediator Declarations

#ifndef __armv8_h
#define __armv8_h

#define __cpu_common_only__
#include <architecture/cpu.h>
#include <architecture/armv7/armv7_cpu.h>
#undef __cpu_common_only__

__BEGIN_SYS

class ARMv8;

class ARMv8_M;

class ARMv8_A: public ARMv7_A
{
protected:
    static const bool multicore = Traits<System>::multicore;

public:
    // Bootstrap/service CPU id
    static const unsigned long BSP = 0;

    static const unsigned int EXCEPTIONS =  64; // 2^6 (bits 26 to 31 of ESR_EL1)

    // HCR bits
    enum {
        EL1_AARCH64_EN      = 1 << 31,
        EL2_IRQ_MAP         = 1 << 4,
        SWIO_HARDWIRED      = 1 << 1
    };

    // SPSR bits
    enum {
        FLAG_EL0        = 0,                    // Execution level 0 (User)
        FLAG_SP_ELn     = 1 << 0,               // SP selection according to Eln, 0 for SP0 usage
        FLAG_EL1        = 1 << 2,               // Execution level 1
        FLAG_EL2        = 1 << 3,               // Execution level 2 (Hypervisor)
        FLAG_EL3        = FLAG_EL1 | FLAG_EL2,  // Execution level 3 (Monitor)
        FLAG_F          = 1 << 6,               // FIQ disable
        FLAG_I          = 1 << 7,               // IRQ disable
        FLAG_A          = 1 << 8,               // Imprecise Abort disable
        FLAG_D          = 1 << 9,               // Debug exception disable
        FLAG_IL         = 1 << 20,              // Illegal execution state
        FLAG_SS         = 1 << 21,              // Software step
        FLAG_V          = 1 << 28,              // Overflow bit
        FLAG_C          = 1 << 29,              // Carry out bit
        FLAG_Z          = 1 << 30,              // Zero result bit
        FLAG_N          = 1 << 31,              // Negative result bit
        NZCV_MASK       = 0xf << 28,            // NZCV mask bits
        CURRENT_EL_MASK = 0x3 << 2,             // Current_EL mask bits
        DAIF_MASK       = 0xf << 6,             // DAIF mask bits
    };

    // id_aa64mmfr0 bits
    enum {
        PA_RANGE_MASK           = 0xf   << 0,
        PA_32BITS_4GB           = 0b0   << 0,
        PA_36BITS_64GB          = 0b01  << 0,
        PA_40BITS_1TB           = 0b10  << 0,
        PA_42BITS_4TB           = 0b11  << 0,
        PA_44BITS_16TB          = 0b100 << 0,
        PA_46BITS_256TB         = 0b101 << 0,
        TGRAN_4KB_NOT           = 0xf   << 28, // 4 KB not supported
        TGRAN_64KB_NOT          = 0xf   << 24, // 64 KB not supported
        TGRAN_16KB              = 0x1   << 20, // 16 KB supported
    };

    // MAIR bits - Refer to https://yurichev.com/mirrors/ARMv8-A_Architecture_Reference_Manual_(Issue_A.a).pdf
    enum {
        ATTR_HIGH_DEVICE                        = 0b0 << 4,
        ATTR_HIGH_NORMAL_OUTER_WT_TRANSIENT     = 0b0011 << 4,
        ATTR_HIGH_NORMAL_OUTER_NO_CACHE         = 0b0100 << 4,
        ATTR_HIGH_NORMAL_OUTER_WB_TRANSIENT     = 0b0111 << 4,
        ATTR_HIGH_NORMAL_OUTER_WT_NO_TRANSIENT  = 0b1011 << 4,
        ATTR_HIGH_NORMAL_OUTER_WB_NO_TRANSIENT  = 0b1111 << 4,
        ATTR_LOW_UNPREDICTABLE                  = 0b0 << 0,
        ATTR_LOW_DEVICE_nGnRnE                  = 0b0 << 0,
        ATTR_LOW_NORMAL_INNER_WT_TRANSIENT      = 0b0011 << 0,
        ATTR_LOW_DEVICE_UNPREDICTABLE           = 0b11 << 0,
        ATTR_LOW_NORMAL_INNER_NO_CACHE          = 0b0100 << 0,
        ATTR_LOW_DEVICE_nGnRE                   = 0b0100 << 0,
        ATTR_LOW_NORMAL_INNER_WB_TRANSIENT      = 0b0111 << 0,
        ATTR_LOW_NORMAL_INNER_WT_NO_TRANSIENT   = 0b1011 << 0,
        ATTR_LOW_DEVICE_nGRE                    = 0b1000 << 0,
        ATTR_LOW_NORMAL_INNER_WB_NO_TRANSIENT   = 0b1111 << 0,
        ATTR_LOW_DEVICE_GRE                     = 0b1100 << 0,
        ATTR_DEVICE_nGnRnE                      = (ATTR_HIGH_DEVICE | ATTR_LOW_DEVICE_nGnRnE),
        ATTR_NORMAL_WT                          = (ATTR_HIGH_NORMAL_OUTER_WT_TRANSIENT | ATTR_LOW_NORMAL_INNER_WT_TRANSIENT),
        ATTR_NORMAL_WB                          = (ATTR_HIGH_NORMAL_OUTER_WB_TRANSIENT | ATTR_LOW_NORMAL_INNER_WB_TRANSIENT),
        ATTR_NORMAL_NON_CACHE                   = (ATTR_HIGH_NORMAL_OUTER_NO_CACHE | ATTR_LOW_NORMAL_INNER_NO_CACHE),
        ATTR_OFFSET                             = 8
    };

    // TCR bits
    enum {
        TTBR0_SIZE_OFFSET           = 0,        // size = 2 ^(64 - TTBR0_SIZE)
        TTBR0_SIZE_512GB            = 25        << TTBR0_SIZE_OFFSET,
        TTBR0_SIZE_1GB              = 34        << TTBR0_SIZE_OFFSET,
        TTBR0_SIZE_4GB              = 32        << TTBR0_SIZE_OFFSET,
        TTBR0_SIZE_64GB             = 28        << TTBR0_SIZE_OFFSET,
        TTBR0_DISABLE               = 0b1       << 7,
        TTBR0_WALK_INNER_NO_CACHE   = 0b00      << 8,
        TTBR0_WALK_INNER_WB_WA      = 0b01      << 8,
        TTBR0_WALK_INNER_WT         = 0b10      << 8,
        TTBR0_WALK_INNER_WB_NO_WA   = 0b11      << 8,
        TTBR0_WALK_OUTER_NO_CACHE   = 0b00      << 10,
        TTBR0_WALK_OUTER_WB_WA      = 0b01      << 10,
        TTBR0_WALK_OUTER_WT         = 0b10      << 10,
        TTBR0_WALK_OUTER_WB_NO_WA   = 0b11      << 10,
        TTBR0_WALK_NON_SHAREABLE    = 0b00      << 12,
        TTBR0_WALK_OUTER_SHAREABLE  = 0b10      << 12,
        TTBR0_WALK_INNER_SHAREABLE  = 0b11      << 12,
        TTBR0_TG0_16KB              = 0b10      << 14,
        TTBR0_TG0_4KB               = 0b00      << 14,
        TTBR0_TG0_64KB              = 0b10      << 14,
        TTBR1_SIZE_OFFSET           = 16,       // size = 2 ^(64 - TTBR1_SIZE)
        TTBR1_SIZE_512GB            = 25        << TTBR1_SIZE_OFFSET,
        TTBR1_SIZE_1GB              = 34        << TTBR1_SIZE_OFFSET,
        TTBR1_SIZE_4GB              = 32        << TTBR1_SIZE_OFFSET,
        TTBR1_SIZE_64GB             = 28        << TTBR1_SIZE_OFFSET,
        ASID_DEFINED_BY_TTBR0       = 0b0       << 22,
        ASID_DEFINED_BY_TTBR1       = 0b1       << 22,
        TTBR1_DISABLE               = 0b1       << 23,
        TTBR1_WALK_INNER_NO_CACHE   = 0b00      << 24,
        TTBR1_WALK_INNER_WB_WA      = 0b01      << 24,
        TTBR1_WALK_INNER_WT         = 0b10      << 24,
        TTBR1_WALK_INNER_WB_NO_WA   = 0b11      << 24,
        TTBR1_WALK_OUTER_NO_CACHE   = 0b00      << 26,
        TTBR1_WALK_OUTER_WB_WA      = 0b01      << 26,
        TTBR1_WALK_OUTER_WT         = 0b10      << 26,
        TTBR1_WALK_OUTER_WB_NO_WA   = 0b11      << 26,
        TTBR1_WALK_NON_SHAREABLE    = 0b00      << 28,
        TTBR1_WALK_OUTER_SHAREABLE  = 0b10      << 28,
        TTBR1_WALK_INNER_SHAREABLE  = 0b11      << 28,
        TTBR1_TG1_16KB              = 0b01      << 30,
        TTBR1_TG1_4KB               = 0b10      << 30,
        TTBR1_TG1_64KB              = 0b11      << 30,
        INTERMEDIATE_ADDR_SIZE_32   = 0b0ULL    << 32,
        INTERMEDIATE_ADDR_SIZE_36   = 0b1ULL    << 32,
        INTERMEDIATE_ADDR_SIZE_40   = 0b10ULL   << 32,
        INTERMEDIATE_ADDR_SIZE_42   = 0b11ULL   << 32, 
        INTERMEDIATE_ADDR_SIZE_44   = 0b100ULL  << 32, 
        INTERMEDIATE_ADDR_SIZE_48   = 0b101ULL  << 32, 
        ASID_SIZE_8                 = 0b0ULL    << 36,
        ASID_SIZE_16                = 0b1ULL    << 36,
        TTBR0_TBI                   = 0b1ULL    << 37, // ignore top byte when calculating address on TTBR0
        TTBR1_TBI                   = 0b1ULL    << 38 // ignore top byte when calculating address on TTBR1
    };

    // ESR_ELx bits
    enum {
        EC_OFFSET                   = 26,
        EC_MASK                     = 0x3f,

        // Exception classes
        EXC_UNKNOWN                 = 0x0,
        EXC_SVC_32                  = 0x11,
        EXC_HVC_32                  = 0x12,
        EXC_SMC_32                  = 0x13,
        EXC_SVC_64                  = 0x15,
        EXC_HVC_64                  = 0x16,
        EXC_SMC_64                  = 0x17,
        EXC_SVC                     = (EXC_SVC_32 | EXC_SVC_64),
        EXC_HVC                     = (EXC_HVC_32 | EXC_HVC_64),
        EXC_SMC                     = (EXC_SMC_32 | EXC_SMC_64),
        EXC_SOFTWARE_INTERRUPT      = (EXC_SVC | EXC_HVC | EXC_SMC),
        EXC_PREFETCH_ABORT_LOWER    = 0x20,
        EXC_PREFETCH_ABORT_SAME     = 0x21,
        EXC_PREFETCH_ABORT          = (EXC_PREFETCH_ABORT_SAME | EXC_PREFETCH_ABORT_LOWER),
        EXC_PC_ALIGNMENT_ABORT      = 0x22,
        EXC_DATA_ABORT_LOWER_EL     = 0x24,
        EXC_DATA_ABORT_SAME_EL      = 0x25,
        EXC_DATA_ABORT              = (EXC_DATA_ABORT_SAME_EL | EXC_DATA_ABORT_LOWER_EL),
        EXC_SP_ALIGNMENT_ABORT      = 0x26
    };

public:
    class Context
    {
    public:
        Context(){}
        Context(Log_Addr entry, Log_Addr exit, Log_Addr usp): _pstate(FLAG_SP_ELn | FLAG_EL1 | FLAG_A | FLAG_D), _lr(exit), _pc(entry) {
            if(Traits<Build>::hysterically_debugged || Traits<Thread>::trace_idle) {
                _x0 = 0; _x1 = 1; _x2 = 2; _x3 = 3; _x4 = 4; _x5 = 5; _x6 = 6; _x7 = 7; _x8 = 8; _x9 = 9; _x10 = 10; _x11 = 11; _x12 = 12; _x13 = 13; _x14 = 14; _x15 = 15;
                _x16 = 16; _x17 = 17; _x18 = 18; _x19 = 19; _x20 = 20; _x21 = 21; _x22 = 22; _x23 = 23; _x24 = 24; _x25 = 25; _x26 = 26; _x27 = 27; _x28 = 28; _x29 = 29;
            }
        }

        static void pop(bool interrupt = false);
        static void push(bool interrupt = false);

        friend OStream & operator<<(OStream & os, const Context & c) {
            os << hex
               << "{x0="  << c._x0
               << ",x1="  << c._x1
               << ",x2="  << c._x2
               << ",x3="  << c._x3
               << ",x4="  << c._x4
               << ",x5="  << c._x5
               << ",x6="  << c._x6
               << ",x7="  << c._x7
               << ",x8="  << c._x8
               << ",x9="  << c._x9
               << ",x10=" << c._x10
               << ",x11=" << c._x11
               << ",x12=" << c._x12
               << ",x13=" << c._x13
               << ",x14=" << c._x14
               << ",x15=" << c._x15
               << ",x16=" << c._x16
               << ",x17=" << c._x17
               << ",x18=" << c._x18
               << ",x19=" << c._x19
               << ",x20=" << c._x20
               << ",x21=" << c._x21
               << ",x22=" << c._x22
               << ",x23=" << c._x23
               << ",x24=" << c._x24
               << ",x25=" << c._x25
               << ",x26=" << c._x26
               << ",x27=" << c._x27
               << ",x28=" << c._x28
               << ",x29=" << c._x29
               << ",sp="  << &c
               << ",lr="  << c._lr
               << ",pc="  << c._pc
               << ",ps="  << c._pstate
               << "}" << dec;
            return os;
        }
    public:
        Reg _pstate;
        Reg _x0;
        Reg _x1;
        Reg _x2;
        Reg _x3;
        Reg _x4;
        Reg _x5;
        Reg _x6;
        Reg _x7;
        Reg _x8;
        Reg _x9;
        Reg _x10;
        Reg _x11;
        Reg _x12;
        Reg _x13;
        Reg _x14;
        Reg _x15;
        Reg _x16; // intra-procedure-call temporary register 0
        Reg _x17; // intra-procedure-call temporary register 1
        Reg _x18;
        Reg _x19;
        Reg _x20;
        Reg _x21;
        Reg _x22;
        Reg _x23;
        Reg _x24;
        Reg _x25;
        Reg _x26;
        Reg _x27;
        Reg _x28;
        Reg _x29;
        Reg _lr;
        Reg _pc;
    };

protected:
    ARMv8_A() {};

public:
    static Log_Addr pc() { Reg r; ASM("adr %0, ." : "=r"(r) :); return r; }

    using ARMv7_A::sp;

    static Reg fp() { Reg r; ASM("mov %0, x29" : "=r"(r)); return r; }
    static void fp(Reg r) {  ASM("mov x29, %0" : : "r"(r) : "r0"); }

    using ARMv7_A::ra;

    static Reg fr() { Reg r; ASM("mov %0, x0" : "=r"(r)); return r; }
    static void fr(Reg r) {  ASM("mov x0, %0" : : "r"(r) : "r0"); }

    using ARMv7_A::clock;
    using ARMv7_A::max_clock;
    using ARMv7_A::min_clock;
    using ARMv7_A::bus_clock;

    static void int_enable() { cpsr(cpsr() & ~(FLAG_F | FLAG_I)); }
    static void int_disable() { cpsr(cpsr() | (FLAG_F | FLAG_I)); }

    static bool int_enabled() { return !int_disabled(); }
    static bool int_disabled() { return cpsr() & (FLAG_F | FLAG_I); }

    using ARMv7_A::halt;
    using ARMv7_A::sev;

    static unsigned int id() {
        if(multicore) {
            Reg32 id;
            ASM("mrs %0, mpidr_el1" : "=r"(id) : : );
            return id & 0x3;
        } else
            return 0;
    }
    static unsigned int cores() { return Traits<Build>::CPUS; }
    static void smp_barrier_init(unsigned int cores) { _cores = cores; }

    static void fpu_save();
    static void fpu_restore();

    // ARMv8 specifics
    static Reg  r0() { Reg r; ASM("mov %0, x0" :  "=r"(r) : : ); return r; }
    static void r0(Reg r) {   ASM("mov x0, %0" : : "r"(r): ); }

    static Reg  r1() { Reg r; ASM("mov %0, x1" :  "=r"(r) : : ); return r; }
    static void r1(Reg r) {   ASM("mov x1, %0" : : "r"(r): ); }

    static Reg  sctlr() { Reg r; ASM("mrs %0, sctlr_el1" :  "=r"(r)); return r; }
    static void sctlr(Reg r) {   ASM("msr sctlr_el1, %0" : : "r"(r) : "r0"); }

    static Reg  actlr() { Reg r; ASM("mrs %0, actlr_el1" :  "=r"(r)); return r; }
    static void actlr(Reg r) {   ASM("msr actlr_el1, %0" : : "r"(r) : "r0"); }

    static Reg  cpsr() { Reg r; ASM("mrs %0, daif" :  "=r"(r) :); return r; }
    static void cpsr(Reg r) {   ASM("msr daif, %0" : : "r"(r) : "cc" ); }

    static Reg  cpsrc() { Reg r; ASM("mrs %0, daif" : "=r"(r) :); return r; }
    static void cpsrc(Reg r) {   ASM("msr daif, %0" : "=r"(r) :); }

    static Reg  esr_el1() { Reg r; ASM("mrs %0, esr_el1" : "=r"(r) :); return r; }
    static void esr_el1(Reg r) {   ASM("msr esr_el1, %0" : : "r"(r) :); }

    static Reg  sp_el0() { Reg r; ASM("mrs %0, sp_el0" :  "=r"(r) : : ); return r; }
    static void sp_el0(Reg r) {   ASM("msr sp_el0, %0" : : "r"(r): ); }

    static Reg  sp_el1() { Reg r; ASM("mrs %0, sp_el1" :  "=r"(r) : : ); return r; }
    static void sp_el1(Reg r) {   ASM("msr sp_el1, %0" : : "r"(r): ); }

    // x16 and x17 are the intra-procedure-call temporary registers
    static void pstate_to_tmp() {
        ASM("   mrs  x17, daif          \t\n\
                mrs  x16, nzcv          \t\n\
                orr  x16, x17, x16      \t\n\
                mrs  x17, CurrentEL     \t\n\
                orr  x16, x17, x16      \t\n\
                mrs  x17, SPSel         \t\n\
                orr  x16, x17, x16      \t" : :);
    }

    static void tmp_to_pstate() {
        ASM("   mov x17, #(0xf << 28)   \t\n\
                and x17, x16, x17       \t\n\
                msr nzcv, x17           \t\n\
                mov x17, #(0xf << 6)    \t\n\
                and x17, x16, x17       \t\n\
                msr daif, x17           \t\n\
                mov x17, #0x1           \t\n\
                and x17, x16, x17       \t\n\
                msr SPSel, x17          \t\n\
                mov x17, #(0x3 << 2)    \t\n\
                and x17, x16, x17       \t\n\
                msr CurrentEL, x17      \t" : : : "cc");
    }

     static Reg pstate() {
         Reg r;
         pstate_to_tmp();
         ASM("mov %0, x16" : "=r"(r) :);
        return r;
    }

    static void pstate(Reg r) {
        ASM("mov x16, %0" : : "r"(r));
        tmp_to_pstate();
    }

    static void iret() { ASM("1: br lr"); }

    static Reg  mode() { Reg m; ASM("mrs %0, currentEl" : "=r"(m) : : ); return m; }
    static void mode(Reg m) {   ASM("msr currentEl, %0" : : "r"(m) : "cc"); }

    static void svc_enter(unsigned int from, bool ret = true) { Context::push(true); }
    static void svc_leave() { Context::pop(true); }
    static void svc_stay() {}

    static void dsb() { ASM("dsb ish"); }

    static void eret() { ASM("eret"); }

    static Reg  ttbr0() { Reg r; ASM ("mrs %0, ttbr0_el1" :  "=r"(r) : :); return r; }
    static void ttbr0(Reg r) {   ASM ("msr ttbr0_el1, %0" : : "p"(r) :); }

    static Reg  ttbr1() { Reg r; ASM ("mrs %0, ttbr1_el1" :  "=r"(r) : :); return r; }
    static void ttbr1(Reg r) {   ASM ("msr ttbr1_el1, %0" : : "p"(r) :); }

    static Reg  ttbcr() { Reg r; ASM ("mrs %0, tcr_el1" :  "=r"(r) : :); return r; }
    static void ttbcr(Reg r) {   ASM ("msr tcr_el1, %0" : : "p"(r) :); }

    static Reg  dacr() { Reg r = 0; /*ASM ("mrc p15, 0, %0, c3, c0, 0" : "=r"(r) : :);*/ return r; }
    static void dacr(Reg r) {  /*ASM ("mcr p15, 0, %0, c3, c0, 0" : : "p"(r) :);*/ }

    static void vbar_el2(Reg r) {ASM("msr vbar_el2, %0" : : "r"(r) : ); }
    static Reg vbar_el2() {Reg r; ASM("mrs %0, vbar_el2" : "=r"(r) : : ); return r; }

    static void vbar_el1(Reg r) {ASM("msr vbar_el1, %0" : : "r"(r) : ); }
    static Reg vbar_el1() {Reg r; ASM("mrs %0, vbar_el1" : "=r"(r) : : ); return r; }

    static void hcr(Reg r) {ASM("msr hcr_el2, %0" : : "r"(r) : );}
    static Reg  hcr() { Reg r; ASM("mrs %0, hcr_el2" : "=r"(r) :); return r; }

    static Reg spsr_el2() { Reg r; ASM("mrs %0, spsr_el2" : "=r"(r) :); return r; }
    static void spsr_el2(Reg r) { ASM("msr spsr_el2, %0" : : "r"(r) :); }

    static Reg spsr_el1() { Reg r; ASM("mrs %0, spsr_el1" : "=r"(r) :); return r; }
    static void spsr_el1(Reg r) { ASM("msr spsr_el1, %0" : : "r"(r) :); }

    static Reg elr_el2() { Reg r; ASM("mrs %0, elr_el2" : "=r"(r) :); return r; }
    static void elr_el2(Reg r) { ASM("msr elr_el2, %0" : : "r"(r) :); }

    static Reg elr_el1() { Reg r; ASM("mrs %0, elr_el1" : "=r"(r) :); return r; }
    static void elr_el1(Reg r) { ASM("msr elr_el1, %0" : : "r"(r) :); }

    static Reg id_aa64mmfr0() {Reg r; ASM("mrs %0, id_aa64mmfr0_el1" : "=r"(r) :); return r;}

    static Reg mair_el1() { Reg r; ASM("mrs %0, mair_el1" : "=r"(r) :); return r; }
    static void mair_el1(Reg r) { ASM("msr mair_el1, %0" : : "r"(r) :); }

    // MMU operations
    static Reg  pd() { return ttbr0(); }
    static void pd(Reg r) { ttbr0(r); }

    static void flush_tlb() { ASM("tlbi vmalle1"); ASM("dsb ish"); isb(); }
    static void flush_tlb(Reg r) { ASM("dsb ishst"); ASM("tlbi vaae1, %0" : : "r"(r)); } // DC CVAU, (addr); DSB ISH; for multicore

    static void flush_branch_predictors();

    static void flush_caches();
    
protected:
    static volatile unsigned int _cores;
};


inline void ARMv8_A::Context::push(bool interrupt)
{
if(interrupt)
    ASM("       str   x30, [sp, #-8]!           // push LR                      \t");
else {
    ASM("       str   x30, [sp, #-8]!           // make room for PC             \t\n\
                str   x30, [sp, #-8]!           // push LR                      \t\n\
                adr   x30, 1f                   // calculate PC                 \t\n\
                str   x30, [sp, #8]             // save PC                      \t\n\
                ldr   x30, [sp, #0]                                             \t");
}

    ASM("       stp   x28, x29, [sp, #-16]!                                     \t\n\
                stp   x26, x27, [sp, #-16]!                                     \t\n\
                stp   x24, x25, [sp, #-16]!                                     \t\n\
                stp   x22, x23, [sp, #-16]!                                     \t\n\
                stp   x20, x21, [sp, #-16]!                                     \t\n\
                stp   x18, x19, [sp, #-16]!                                     \t\n\
                stp   x16, x17, [sp, #-16]!                                     \t\n\
                stp   x14, x15, [sp, #-16]!                                     \t\n\
                stp   x12, x13, [sp, #-16]!                                     \t\n\
                stp   x10, x11, [sp, #-16]!                                     \t\n\
                stp    x8,  x9, [sp, #-16]!                                     \t\n\
                stp    x6,  x7, [sp, #-16]!                                     \t\n\
                stp    x4,  x5, [sp, #-16]!                                     \t\n\
                stp    x2,  x3, [sp, #-16]!                                     \t\n\
                stp    x0,  x1, [sp, #-16]!                                     \t");

if(interrupt)
    ASM("       mrs    x0, elr_el1                                              \t\n\
                str    x0, [sp,#-8]!                                            \t\n\
                mrs    x1, spsr_el1                                             \t\n\
                str    x1, [sp,#-8]!                                            \t\n\
                ldr    x0, [sp,#16]                                             \t\n\
                ldr    x1, [sp,#24]                                             \t");
else {
    pstate_to_tmp();
    ASM("       str x16, [sp, #-8]!                                             \t");
}
}

inline void ARMv8_A::Context::pop(bool interrupt)
{
if(interrupt) {
    ASM("       ldr    x0, [sp], #8                                             \t\n\
                msr    spsr_el1, x0                                             \t\n\
                ldr    x0, [sp], #8                                             \t\n\
                msr    elr_el1, x0                                              \t");
} else {
    ASM("       ldr   x30, [sp], #8             // pop PSTATE into x30          \t" : : : "cc");
}

    ASM("       ldp    x0,  x1, [sp], #16                                       \t\n\
                ldp    x2,  x3, [sp], #16                                       \t\n\
                ldp    x4,  x5, [sp], #16                                       \t\n\
                ldp    x6,  x7, [sp], #16                                       \t\n\
                ldp    x8,  x9, [sp], #16                                       \t\n\
                ldp   x10, x11, [sp], #16                                       \t\n\
                ldp   x12, x13, [sp], #16                                       \t\n\
                ldp   x14, x15, [sp], #16                                       \t\n\
                ldp   x16, x17, [sp], #16                                       \t\n\
                ldp   x18, x19, [sp], #16                                       \t\n\
                ldp   x20, x21, [sp], #16                                       \t\n\
                ldp   x22, x23, [sp], #16                                       \t\n\
                ldp   x24, x25, [sp], #16                                       \t\n\
                ldp   x26, x27, [sp], #16                                       \t\n\
                ldp   x28, x29, [sp], #16                                       \t");

if(interrupt)
    ASM("       ldr   x30, [sp], #8             // pop PSTATE into x30         \t");
else {
    ASM("       msr   spsr_el1, x30                                             \t\n\
                ldr   x30, [sp], #8             // pop LR to get to PC          \t\n\
                ldr   x30, [sp], #8             // pop PC                       \t\n\
                msr   ELR_EL1, x30                                              \t\n\
                ldr   x30, [sp, #-16]           // pop LR                       \t\n\
                eret                                                            \t" : : : "cc");
}
}

class CPU: public ARMv8_A
{
    friend class Init_System;

private:
    typedef ARMv8_A Base;

public:
    class Context: public Base::Context
    {
    public:
        Context() {}
        Context(Log_Addr entry, Log_Addr exit, Log_Addr usp): Base::Context(entry, exit, 0) {}

        void save() volatile __attribute__ ((naked));
        void load() const volatile __attribute__ ((naked));
    };

public:
    CPU() {}

    using Base::pc;
    using Base::ra;
    using Base::sp;
    using Base::fr;

    static Hertz clock() { return _cpu_clock; }
    static void clock(const Hertz & frequency); // defined along with each machine's IOCtrl
    static Hertz max_clock();
    static Hertz min_clock();
    static Hertz bus_clock() { return _bus_clock; }

    using Base::int_enable;
    using Base::int_disable;
    using Base::int_enabled;
    using Base::int_disabled;

    using Base::halt;

    using Base::fpu_save;
    using Base::fpu_restore;

    using Base::id;
    using Base::cores;

    template<typename T>
    static T tsl(volatile T & lock) {
        register T old = 0;
        register T one = 1;
        ASM("1: ldaxr   %w0, [%1]       \n"
            "   stlxr   w3, %2, [%1]    \n"
            "   cbnz    w3, 1b          \n" : "=&r"(old) : "r"(&lock), "r"(one) : "x3", "cc");
        return lock;
    }

    template<typename T>
    static T finc(volatile T & value) {
        register T old;
        ASM("1: ldaxr  %0, [%1]         \n"
            "   add    %0, %0, #1       \n"
            "   stlxr  w3, %0, [%1]     \n"
            "   cbnz   w3, 1b           \n" : "=&r"(old) : "r"(&value) : "x3", "cc");
        return old - 1;
    }

    template<typename T>
    static T fdec(volatile T & value) {
        register T old;
        ASM("1: ldaxr  %0, [%1]         \n"
            "   add    %0, %0, #-1      \n"
            "   stlxr  w3, %0, [%1]     \n"
            "   cbnz   w3, 1b           \n" : "=&r"(old) : "r"(&value) : "x3", "cc");
        return old + 1;
    }

    template <typename T>
    static T cas(volatile T & value, T compare, T replacement) {
        register T old;
        ASM("1: ldaxr  %w0, [%1]        \n"
            "   cmp    %0, %2           \n"
            "   bne    2f               \n"
            "   stlxr  w6, %3, [%1]     \n"
            "   cbnz   w6, 1b           \n"
            "2:                         \n" : "=&r"(old) : "r"(&value), "r"(compare), "r"(replacement) : "x6", "cc");
        return old;
    }
 
    static void smp_barrier(unsigned int cores = ARMv8_A::cores()) { if(multicore) CPU_Common::smp_barrier<&finc>(cores, id()); }

    static void switch_context(Context ** o, Context * n);

    template<typename ... Tn>
    static Context * init_stack(Log_Addr usp, Log_Addr sp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(entry, exit, usp); // init_stack is called with usp = 0 for kernel threads
        init_stack_helper(&ctx->_x0, an ...);
        return ctx;
    }

    using CPU_Common::htole64;
    using CPU_Common::htole32;
    using CPU_Common::htole16;
    using CPU_Common::letoh64;
    using CPU_Common::letoh32;
    using CPU_Common::letoh16;

    using CPU_Common::htobe64;
    using CPU_Common::htobe32;
    using CPU_Common::htobe16;
    using CPU_Common::betoh64;
    using CPU_Common::betoh32;
    using CPU_Common::betoh16;

    using CPU_Common::htonl;
    using CPU_Common::htons;
    using CPU_Common::ntohl;
    using CPU_Common::ntohs;

private:
    template<typename Head, typename ... Tail>
    static void init_stack_helper(Log_Addr sp, Head head, Tail ... tail) {
        *static_cast<Head *>(sp) = head;
        init_stack_helper(sp + sizeof(Head *), tail ...);
    }
    static void init_stack_helper(Log_Addr sp) {}

    static void context_load_helper();

    static void init();

private:
    static unsigned int _cpu_clock;
    static unsigned int _bus_clock;
};

inline CPU::Reg64 htole64(CPU::Reg64 v) { return CPU::htole64(v); }
inline CPU::Reg32 htole32(CPU::Reg32 v) { return CPU::htole32(v); }
inline CPU::Reg16 htole16(CPU::Reg16 v) { return CPU::htole16(v); }
inline CPU::Reg64 letoh64(CPU::Reg64 v) { return CPU::letoh64(v); }
inline CPU::Reg32 letoh32(CPU::Reg32 v) { return CPU::letoh32(v); }
inline CPU::Reg16 letoh16(CPU::Reg16 v) { return CPU::letoh16(v); }

inline CPU::Reg64 htobe64(CPU::Reg64 v) { return CPU::htobe64(v); }
inline CPU::Reg32 htobe32(CPU::Reg32 v) { return CPU::htobe32(v); }
inline CPU::Reg16 htobe16(CPU::Reg16 v) { return CPU::htobe16(v); }
inline CPU::Reg64 betoh64(CPU::Reg64 v) { return CPU::betoh64(v); }
inline CPU::Reg32 betoh32(CPU::Reg32 v) { return CPU::betoh32(v); }
inline CPU::Reg16 betoh16(CPU::Reg16 v) { return CPU::betoh16(v); }

inline CPU::Reg32 htonl(CPU::Reg32 v)   { return CPU::htonl(v); }
inline CPU::Reg16 htons(CPU::Reg16 v)   { return CPU::htons(v); }
inline CPU::Reg32 ntohl(CPU::Reg32 v)   { return CPU::ntohl(v); }
inline CPU::Reg16 ntohs(CPU::Reg16 v)   { return CPU::ntohs(v); }

__END_SYS

#endif
