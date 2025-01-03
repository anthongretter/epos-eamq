// EPOS RISC-V 64 CPU Mediator Declarations

#ifndef __rv64_h
#define __rv64_h

#include <architecture/cpu.h>

__BEGIN_SYS

class CPU: protected CPU_Common
{
    friend class Init_System; // for CPU::init()

private:
    static const bool supervisor = Traits<Machine>::supervisor;
    static const bool amo = Traits<CPU>::atomic_memory_operations;

public:
    // Bootstrap/service CPU id
    static const unsigned long BSP = Traits<Machine>::BSP;

    // CPU Native Data Types
    using CPU_Common::Reg8;
    using CPU_Common::Reg16;
    using CPU_Common::Reg32;
    using CPU_Common::Reg64;
    using CPU_Common::Reg;
    using CPU_Common::Log_Addr;
    using CPU_Common::Phy_Addr;
    using CPU_Common::Interrupt_Id;

    // Status Register ([m|s]status)
    typedef Reg Status;
    enum : Reg {
        UIE             = 1 <<  0,      // User Interrupts Enabled
        SIE             = 1 <<  1,      // Supervisor Interrupts Enabled
        MIE             = 1 <<  3,      // Machine Interrupts Enabled
        UPIE            = 1 <<  4,      // User Previous Interrupts Enabled
        SPIE            = 1 <<  5,      // Supervisor Previous Interrupts Enabled
        UBE             = 1 <<  6,      // Endianness of data memory accesses in user mode (0 -> little, 1 -> big); fetches are always little-endian
        MPIE            = 1 <<  7,      // Machine Previous Interrupts Enabled
        SPP             = 1 <<  8,      // Supervisor Previous Privilege
        SPP_U           = 0 <<  8,      // Supervisor Previous Privilege = user
        SPP_S           = 1 <<  8,      // Supervisor Previous Privilege = supervisor
        MPP             = 3 << 11,      // Machine Previous Privilege
        MPP_U           = 0 << 11,      // Machine Previous Privilege = user
        MPP_S           = 1 << 11,      // Machine Previous Privilege = supervisor
        MPP_M           = 3 << 11,      // Machine Previous Privilege = machine
        FS              = 3 << 13,      // FPU Status
        FS_OFF          = 0 << 13,      // FPU off
        FS_INIT         = 1 << 13,      // FPU on
        FS_CLEAN        = 2 << 13,      // FPU registers clean
        FS_DIRTY        = 3 << 13,      // FPU registers dirty (and must be saved on context switch)
        XS              = 3 << 15,      // Extension Status
        XS_OFF          = 0 << 15,      // Extension off
        XS_INIT         = 1 << 15,      // Extension on
        XS_CLEAN        = 2 << 15,      // Extension registers clean
        XS_DIRTY        = 3 << 15,      // Extension registers dirty (and must be saved on context switch)
        MPRV            = 1 << 17,      // Memory PRiVilege (when set, enables MMU if MPP uses MMU)
        SUM             = 1 << 18,      // Supervisor User Memory access allowed
        MXR             = 1 << 19,      // Make eXecutable Readable
        TVM             = 1 << 20,      // Trap Virtual Memory makes SATP inaccessible in supervisor mode
        TW              = 1 << 21,      // Timeout Wait for WFI outside machine mode
        TSR             = 1 << 22,      // Trap SRet in supervisor mode
        UXL             = 3UL << 32,    // ISA word length (1 -> 32, 2 -> 64, 3 -> 128) in user mode
        SXL             = 3UL << 34,    // ISA word length (1 -> 32, 2 -> 64, 3 -> 128) in supervisor mode
        SBE             = 1UL << 36,    // Endianness of data memory accesses in supervisor mode (0 -> little, 1 -> big); fetches are always little-endian
        MBE             = 1UL << 37,    // Endianness of data memory accesses in machine mode (0 -> little, 1 -> big); fetches are always little-endian
        SD              = 1UL << 63     // Status Dirty = (FS | XS)
    };

    // [M|S]TVEC modes
    enum : Reg {
        INT_DIRECT  = 0,
        INT_INDEXED = 1
    };

    // Interrupt-Enable, Interrupt-Pending and Machine Cause Registers ([M|S]IE, [M|S]IP, and [M|S]CAUSE when interrupt bit is set)
    enum : Reg {
        SSI             = 1 << 1,       // Supervisor Software Interrupt
        MSI             = 1 << 3,       // Machine Software Interrupt
        STI             = 1 << 5,       // Supervisor Timer Interrupt
        MTI             = 1 << 7,       // Machine Timer Interrupt
        SEI             = 1 << 9,       // Supervisor External Interrupt
        MEI             = 1 << 11,      // Machine External Interrupt
        SI              = supervisor ? SSI : MSI,
        TI              = supervisor ? STI : MTI,
        EI              = supervisor ? SEI : MEI
    };

    // Exceptions ([m|s]cause with interrupt = 0)
    static const unsigned int EXCEPTIONS = 16;
    enum : Reg {
        EXC_IALIGN       = 0,   // Instruction address misaligned
        EXC_IFAULT       = 1,   // Instruction access fault
        EXC_IILLEGAL     = 2,   // Illegal instruction
        EXC_BREAK        = 3,   // Breakpoint
        EXC_DRALIGN      = 4,   // Load address misaligned
        EXC_DRFAULT      = 5,   // Load access fault
        EXC_DWALIGN      = 6,   // Store/AMO address misaligned
        EXC_DWFAULT      = 7,   // Store/AMO access fault
        EXC_ENVU         = 8,   // Environment call from U-mode
        EXC_ENVS         = 9,   // Environment call from S-mode
        EXC_ENVH         = 10,  // reserved
        EXC_ENVM         = 11,  // Environment call from M-mode
        EXC_IPF          = 12,  // Instruction page fault
        EXC_DRPF         = 13,  // Load data page fault
        EXC_RES          = 14,  // reserved
        EXC_DWPF         = 15,  // Store/AMO data page fault
    };

    // CPU Context
    class Context
    {
        friend class CPU;       // for Context::push() and Context::pop()
        friend class IC;        // for Context::push() and Context::pop()
        friend class Thread;    // for Context::push()

    public:
        Context() {}
        // Contexts are loaded with [m|s]ret, which gets pc from [m|s]epc and updates some bits of [m|s]status, that's why _st is initialized with [M|S]PIE and [M|S]PP
        Context(Log_Addr entry, Log_Addr exit): _pc(entry), _st(supervisor ? ((exit ? SPIE : 0) | SPP_S | SUM) : ((exit ? MPIE : 0) | MPP_M)), _x1(exit) {
            if(Traits<Build>::hysterically_debugged || Traits<Thread>::trace_idle) {
                                                                        _x5 =  5;  _x6 =  6;  _x7 =  7;  _x8 =  8;  _x9 =  9;
                _x10 = 10; _x11 = 11; _x12 = 12; _x13 = 13; _x14 = 14; _x15 = 15; _x16 = 16; _x17 = 17; _x18 = 18; _x19 = 19;
                _x20 = 20; _x21 = 21; _x22 = 22; _x23 = 23; _x24 = 24; _x25 = 25; _x26 = 26; _x27 = 27; _x28 = 28; _x29 = 29;
                _x30 = 30; _x31 = 31;
            }
        }

        void save() const volatile __attribute__ ((naked));
        void load() const volatile __attribute__ ((naked));

        friend OStream & operator<<(OStream & os, const Context & c) {
            os << hex
               << "{sp="   << &c
               << ",pc="   << c._pc
               << ",st="   << c._st
               << ",lr="   << c._x1
               << ",x5="   << c._x5
               << ",x6="   << c._x6
               << ",x7="   << c._x7
               << ",x8="   << c._x8
               << ",x9="   << c._x9
               << ",x10="  << c._x10
               << ",x11="  << c._x11
               << ",x12="  << c._x12
               << ",x13="  << c._x13
               << ",x14="  << c._x14
               << ",x15="  << c._x15
               << ",x16="  << c._x16
               << ",x17="  << c._x17
               << ",x18="  << c._x18
               << ",x19="  << c._x19
               << ",x20="  << c._x20
               << ",x21="  << c._x21
               << ",x22="  << c._x22
               << ",x23="  << c._x23
               << ",x24="  << c._x24
               << ",x25="  << c._x25
               << ",x26="  << c._x26
               << ",x27="  << c._x27
               << ",x28="  << c._x28
               << ",x29="  << c._x29
               << ",x30="  << c._x30
               << ",x31="  << c._x31
               << "}" << dec;
            return os;
        }

    private:
        static void pop(bool interrupt = false);  // interrupt or context switch?
        static void push(bool interrupt = false); // interrupt or context switch?

    private:
        Reg _pc;      // pc
        Reg _st;      // [m|s]status
    //  Reg _x0;      // zero
        Reg _x1;      // ra, ABI Link Register
    //  Reg _x2;      // sp, ABI Stack Pointer, saved in EPOS as the Context's this pointer
    //  Reg _x3;      // gp, ABI Global Pointer, used in EPOS as a temporary inside the kernel
    //  Reg _x4;      // tp, ABI Thread Pointer, used in EPOS as core id
        Reg _x5;      // t0
        Reg _x6;      // t1
        Reg _x7;      // t2
        Reg _x8;      // s0
        Reg _x9;      // s1
        Reg _x10;     // a0
        Reg _x11;     // a1
        Reg _x12;     // a2
        Reg _x13;     // a3
        Reg _x14;     // a4
        Reg _x15;     // a5
        Reg _x16;     // a6
        Reg _x17;     // a7
        Reg _x18;     // s2
        Reg _x19;     // s3
        Reg _x20;     // s4
        Reg _x21;     // s5
        Reg _x22;     // s6
        Reg _x23;     // s7
        Reg _x24;     // s8
        Reg _x25;     // s9
        Reg _x26;     // s10
        Reg _x27;     // s11
        Reg _x28;     // t3
        Reg _x29;     // t4
        Reg _x30;     // t5
        Reg _x31;     // t6
    };

    // Interrupt Service Routines
    typedef void (ISR)();

    // Fault Service Routines (exception handlers)
    typedef void (FSR)();

public:
    CPU() {};

    static Log_Addr pc() { Reg r; ASM("auipc %0, 0" : "=r"(r) :); return r; }

    static Log_Addr sp() { Reg r; ASM("mv %0, sp" :  "=r"(r) :); return r; }
    static void sp(Reg r) {       ASM("mv sp, %0" : : "r"(r) :); }

    static Log_Addr fp() { Reg r; ASM("mv %0, fp" :  "=r"(r) :); return r; }
    static void fp(Reg r) {       ASM("mv fp, %0" : : "r"(r) :); }

    static Log_Addr ra() { Reg r; ASM("mv %0, ra" :  "=r"(r)); return r; }
    static void ra(Reg r) {       ASM("mv ra, %0" : : "r"(r) :); }

    static Log_Addr fr() { Reg r; ASM("mv %0, a0" :  "=r"(r)); return r; }
    static void fr(Reg r) {       ASM("mv a0, %0" : : "r"(r) :); }

    static unsigned int id() { return tp(); }
    static unsigned int cores() { return Traits<Build>::CPUS; }

    static void smp_barrier(unsigned long cores = CPU::cores()) { CPU_Common::smp_barrier<&finc>(cores, id()); }

    using CPU_Common::clock;
    using CPU_Common::min_clock;
    using CPU_Common::max_clock;
    using CPU_Common::bus_clock;

    static void int_enable()  { supervisor ? sint_enable()  : mint_enable(); }
    static void int_disable() { supervisor ? sint_disable() : mint_disable(); }
    static bool int_enabled() { return supervisor ? (sstatus() & SIE) : (mstatus() & MIE); }
    static bool int_disabled() { return !int_enabled(); }

    static void halt() { ASM("wfi"); }

    static void fpu_save();
    static void fpu_restore();

    static void switch_context(Context ** o, Context * n) __attribute__ ((naked));

    template<typename T>
    static T tsl(volatile T & lock) {
        register T old;
        if(sizeof(T) == sizeof(Reg64))
            if(amo)
                ASM("amoswap.d %0, %2, (%1)" : "=&r"(old) : "r"(&lock), "r"(1) : "memory");
            else
                ASM("1: lr.d    %0, (%1)        \n"
                    "   sc.d    t3, %2, (%1)    \n"
                    "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&lock), "r"(1) : "t3", "cc", "memory");
        else
            if(amo)
                ASM("amoswap.w %0, %2, (%1)" : "=&r"(old) : "r"(&lock), "r"(1) : "memory");
            else
                ASM("1: lr.w    %0, (%1)        \n"
                    "   sc.w    t3, %2, (%1)    \n"
                    "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&lock), "r"(1) : "t3", "cc", "memory");
        return old;
    }

    template<typename T>
    static T finc(volatile T & value) {
        register T old;
        if(sizeof(T) == sizeof(Reg64))
            if(amo)
                ASM("amoadd.d %0, %2, (%1)" : "=r&"(old) : "r"(&value), "r"(1) : "memory");
            else
                ASM("1: lr.d    %0, (%1)        \n"
                    "   addi    t3, %0, 1       \n"
                    "   sc.d    t3, t3, (%1)    \n"
                    "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&value) : "t3", "cc", "memory");
        else
            if(amo)
                ASM("amoadd.w %0, %2, (%1)" : "=&r"(old) : "r"(&value), "r"(1) : "memory");
            else
                ASM("1: lr.w    %0, (%1)        \n"
                    "   addi    t3, %0, 1       \n"
                    "   sc.w    t3, t3, (%1)    \n"
                    "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&value) : "t3", "cc", "memory");
        return old;
    }

    template<typename T>
    static T fdec(volatile T & value) {
        register T old;
        if(sizeof(T) == sizeof(Reg64))
            if(amo)
                ASM("amoadd.d %0, %2, (%1)" : "=r"(old) : "r"(&value), "r"(-1) : "memory");
            else
                ASM("1: lr.d    %0, (%1)        \n"
                    "   addi    t3, %0, -1      \n"
                    "   sc.d    t3, t3, (%1)    \n"
                    "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&value) : "t3", "cc", "memory");
        else
            if(amo)
                ASM("amoadd.w %0, %2, (%1)" : "=&r"(old) : "r"(&value), "r"(-1) : "memory");
            else
                ASM("1: lr.w    %0, (%1)        \n"
                    "   addi    t3, %0, -1      \n"
                    "   sc.w    t3, t3, (%1)    \n"
                    "   bnez    t3, 1b          \n" : "=&r"(old) : "r"(&value) : "t3", "cc", "memory");
        return old;
    }

    template <typename T>
    static T cas(volatile T & value, T compare, T replacement) {
        register T old;
        if(sizeof(T) == sizeof(Reg64))
            ASM("1: lr.d    %0, (%1)        \n"
                "   bne     %0, %2, 2f      \n"
                "   sc.d    t3, %3, (%1)    \n"
                "   bnez    t3, 1b          \n"
                "2:                         \n" : "=&r"(old) : "r"(&value), "r"(compare), "r"(replacement) : "t3", "cc", "memory");
        else
            ASM("1: lr.w    %0, (%1)        \n"
                "   bne     %0, %2, 2f      \n"
                "   sc.w    t3, %3, (%1)    \n"
                "   bnez    t3, 1b          \n"
                "2:                         \n" : "=&r"(old) : "r"(&value), "r"(compare), "r"(replacement) : "t3", "cc", "memory");
        return old;
    }

    static void flush_tlb() {         ASM("sfence.vma"    : :           : "memory"); }
    static void flush_tlb(Reg addr) { ASM("sfence.vma %0" : : "r"(addr) : "memory"); }

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

    template<typename ... Tn>
    static Context * init_stack(Log_Addr usp, Log_Addr sp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(entry, exit);
        init_stack_helper(&ctx->_x10, an ...); // x10 is a0
        return ctx;
    }

public:
    // RISC-V 64 specifics
    static Reg  status()   { return supervisor ? sstatus()   : mstatus(); }
    static void status(Status st) { supervisor ? sstatus(st) : mstatus(st); }

    static Reg  ie()     { return supervisor ? sie()         : mie(); }
    static void ie(Reg r)       { supervisor ? sie(r)        : mie(r); }
    static void iec(Reg r)      { supervisor ? siec(r)       : miec(r); }
    static void ies(Reg r)      { supervisor ? sies(r)       : mies(r); }

    static Reg  ip()     { return supervisor ? sip()         : mip(); }
    static void ip(Reg r)       { supervisor ? sip(r)        : mip(r); }

    static Reg  cause()  { return supervisor ? scause()      : mcause(); }

    static Reg  tval()   { return supervisor ? stval()       : mtval(); }

    static Reg  epc()    { return supervisor ? sepc()        : mepc(); }
    static void epc(Reg r)      { supervisor ? sepc(r)       : mepc(r); }

    static Reg  tvec()           { return supervisor ? stvec()       : mtvec(); }
    static void tvec(Reg m, Phy_Addr a) { supervisor ? stvec(m, a) : mtvec(m, a); }

    static Reg  tp() { Reg r; ASM("mv %0, x4" : "=r"(r) :); return r; }
    static void tp(Reg r) {   ASM("mv x4, %0" : : "r"(r) :); }

    static Reg  a0() { Reg r; ASM("mv %0, a0" :  "=r"(r)); return r; }
    static void a0(Reg r) {   ASM("mv a0, %0" : : "r"(r) :); }

    static Reg  a1() { Reg r; ASM("mv %0, a1" :  "=r"(r)); return r; }
    static void a1(Reg r) {   ASM("mv a1, %0" : : "r"(r) :); }

    static Reg  gp() { Reg r; ASM("mv %0, x3" :  "=r"(r)); return r; }
    static void gp(Reg r) {   ASM("mv x3, %0" : : "r"(r) :); }

    static void ecall() { ASM("ecall"); }
    static void iret() { supervisor ? sret() : mret(); }

    // Machine mode
    static void mint_enable()  { ASM("csrsi mstatus, %0" : : "i"(MIE) : "cc"); }
    static void mint_disable() { ASM("csrci mstatus, %0" : : "i"(MIE) : "cc"); }

    static Reg mhartid() { Reg r; ASM("csrr %0, mhartid" : "=r"(r) : : "memory", "cc"); return r; }

    static void mscratch(Reg r)   { ASM("csrw mscratch, %0" : : "r"(r) : "cc"); }
    static Reg  mscratch() { Reg r; ASM("csrr %0, mscratch" :  "=r"(r) : : ); return r; }

    static void mstatus(Reg r)   { ASM("csrw mstatus, %0" : : "r"(r) : "cc"); }
    static void mstatusc(Reg r)  { ASM("csrc mstatus, %0" : : "r"(r) : "cc"); }
    static void mstatuss(Reg r)  { ASM("csrs mstatus, %0" : : "r"(r) : "cc"); }
    static Reg  mstatus() { Reg r; ASM("csrr %0, mstatus" :  "=r"(r) : : ); return r; }

    static void mie(Reg r)   { ASM("csrw mie, %0" : : "r"(r) : "cc"); }
    static void miec(Reg r)  { ASM("csrc mie, %0" : : "r"(r) : "cc"); }
    static void mies(Reg r)  { ASM("csrs mie, %0" : : "r"(r) : "cc"); }
    static Reg  mie() { Reg r; ASM("csrr %0, mie" :  "=r"(r) : : ); return r; }

    static void mip(Reg r)   { ASM("csrw mip, %0" : : "r"(r) : "cc"); }
    static void mipc(Reg r)  { ASM("csrc mip, %0" : : "r"(r) : "cc"); }
    static void mips(Reg r)  { ASM("csrs mip, %0" : : "r"(r) : "cc"); }
    static Reg  mip() { Reg r; ASM("csrr %0, mip" :  "=r"(r) : : ); return r; }

    static void mtvec(Reg m, Phy_Addr a) { Reg p = (a & -4UL) | (m & 0x3); ASM("csrw mtvec, %0" : : "r"(p) : "cc"); }
    static Reg  mtvec() { Reg r; ASM("csrr %0, mtvec" : "=r"(r) : : ); return r; }

    static Reg mcause() { Reg r; ASM("csrr %0, mcause" : "=r"(r) : : ); return r; }
    static Reg mtval()  { Reg r; ASM("csrr %0, mtval" :  "=r"(r) : : ); return r; }

    static void mepc(Reg r)   { ASM("csrw mepc, %0" : : "r"(r) : "cc"); }
    static Reg  mepc() { Reg r; ASM("csrr %0, mepc" :  "=r"(r) : : ); return r; }

    static void mret() { ASM("mret"); }

    static void mideleg(Reg r) { ASM("csrw mideleg, %0" : : "r"(r) : "cc"); }
    static void medeleg(Reg r) { ASM("csrw medeleg, %0" : : "r"(r) : "cc"); }

    static void pmpcfg0(Reg r)   { ASM("csrw pmpcfg0,  %0" : : "r"(r) : "cc"); }
    static Reg  pmpcfg0() { Reg r; ASM("csrr %0, pmpcfg0" :  "=r"(r) : : ); return r; }

    static void pmpaddr0(Reg r)   { ASM("csrw pmpaddr0, %0" : : "r"(r) : "cc"); }
    static Reg  pmpaddr0() { Reg r; ASM("csrr %0, pmpaddr0" :  "=r"(r) : : ); return r; }

    // Supervisor mode
    static void sint_enable()  { ASM("csrsi sstatus, %0" : : "i"(SIE) : "cc"); }
    static void sint_disable() { ASM("csrci sstatus, %0" : : "i"(SIE) : "cc"); }

    static void sscratch(Reg r)   { ASM("csrw sscratch, %0" : : "r"(r) : "cc"); }
    static Reg  sscratch() { Reg r; ASM("csrr %0, sscratch" :  "=r"(r) : : ); return r; }

    static void sstatus(Reg r)   { ASM("csrw sstatus, %0" : : "r"(r) : "cc"); }
    static void sstatusc(Reg r)  { ASM("csrc sstatus, %0" : : "r"(r) : "cc"); }
    static void sstatuss(Reg r)  { ASM("csrs sstatus, %0" : : "r"(r) : "cc"); }
    static Reg  sstatus() { Reg r; ASM("csrr %0, sstatus" :  "=r"(r) : : ); return r; }

    static void sie(Reg r)   { ASM("csrw sie, %0" : : "r"(r) : "cc"); }
    static void siec(Reg r)  { ASM("csrc sie, %0" : : "r"(r) : "cc"); }
    static void sies(Reg r)  { ASM("csrs sie, %0" : : "r"(r) : "cc"); }
    static Reg  sie() { Reg r; ASM("csrr %0, sie" :  "=r"(r) : : ); return r; }

    static void sip(Reg r)   { ASM("csrw sip, %0" : : "r"(r) : "cc"); }
    static void sipc(Reg r)  { ASM("csrc sip, %0" : : "r"(r) : "cc"); }
    static void sips(Reg r)  { ASM("csrs sip, %0" : : "r"(r) : "cc"); }
    static Reg  sip() { Reg r; ASM("csrr %0, sip" :  "=r"(r) : : ); return r; }

    static void stvec(Reg m, Phy_Addr a) { Reg p = (a & -4UL) | (m & 0x3); ASM("csrw stvec, %0" : : "r"(p) : "cc"); }
    static Reg  stvec() { Reg r; ASM("csrr %0, stvec" : "=r"(r) : : ); return r; }

    static Reg scause() { Reg r; ASM("csrr %0, scause" : "=r"(r) : : ); return r; }
    static Reg stval()  { Reg r; ASM("csrr %0, stval" :  "=r"(r) : : ); return r; }

    static void sepc(Reg r)   { ASM("csrw sepc, %0" : : "r"(r) : "cc"); }
    static Reg  sepc() { Reg r; ASM("csrr %0, sepc" :  "=r"(r) : : ); return r; }

    static void sret() { ASM("sret"); }

    static void satp(Reg r) { ASM("csrw satp, %0" : : "r"(r) : "cc"); ASM("sfence.vma" : : : "memory"); }
    static Reg  satp() { Reg r; ASM("csrr %0, satp" :  "=r"(r) : : ); return r; }

private:
    template<typename Head, typename ... Tail>
    static void init_stack_helper(Log_Addr sp, Head head, Tail ... tail) {
        *static_cast<Head *>(sp) = head;
        init_stack_helper(sp + sizeof(Head *), tail ...);
    }
    static void init_stack_helper(Log_Addr sp) {}

    static void init();

private:
    static unsigned int _cpu_clock;
    static unsigned int _bus_clock;
};

inline void CPU::Context::push(bool interrupt)
{
    ASM("       addi     sp, sp, %0             \n" : : "i"(-sizeof(Context))); // adjust SP for the pushes below
if(interrupt) {
  if(supervisor) {
    ASM("       csrr     x3,    sepc            \n");   // push SEPC as PC on interrupts in supervisor mode
  } else {
    ASM("       csrr     x3,    mepc            \n");   // push MEPC as PC on interrupts in machine mode
  }
} else {
    ASM("       mv       x3,    x1              \n");   // push RA as PC on context switches
}
    ASM("       sd       x3,    0(sp)           \n");   // push PC
if(supervisor) {
    ASM("       csrr     x3, sstatus            \n");
} else {
    ASM("       csrr     x3, mstatus            \n");
}
    ASM("       sd       x3,    8(sp)           \n"     // push ST
        "       sd       x1,   16(sp)           \n"     // push RA
        "       sd       x5,   24(sp)           \n"     // push X5-X31
        "       sd       x6,   32(sp)           \n"
        "       sd       x7,   40(sp)           \n"
        "       sd       x8,   48(sp)           \n"
        "       sd       x9,   56(sp)           \n"
        "       sd      x10,   64(sp)           \n"
        "       sd      x11,   72(sp)           \n"
        "       sd      x12,   80(sp)           \n"
        "       sd      x13,   88(sp)           \n"
        "       sd      x14,   96(sp)           \n"
        "       sd      x15,  104(sp)           \n"
        "       sd      x16,  112(sp)           \n"
        "       sd      x17,  120(sp)           \n"
        "       sd      x18,  128(sp)           \n"
        "       sd      x19,  136(sp)           \n"
        "       sd      x20,  144(sp)           \n"
        "       sd      x21,  152(sp)           \n"
        "       sd      x22,  160(sp)           \n"
        "       sd      x23,  168(sp)           \n"
        "       sd      x24,  176(sp)           \n"
        "       sd      x25,  184(sp)           \n"
        "       sd      x26,  192(sp)           \n"
        "       sd      x27,  200(sp)           \n"
        "       sd      x28,  208(sp)           \n"
        "       sd      x29,  216(sp)           \n"
        "       sd      x30,  224(sp)           \n"
        "       sd      x31,  232(sp)           \n");
if(interrupt) {
    ASM("       mv       x3, sp                 \n");   // leave TMP pointing the context to easy subsequent access to the saved context
}
}

inline void CPU::Context::pop(bool interrupt)
{
    ASM("       ld       x3,    0(sp)           \n");   // pop PC into TMP
if(supervisor) {
    ASM("       csrw     sepc, x3               \n");   // SEPC = PC
} else {
    ASM("       csrw     mepc, x3               \n");   // MEPC = PC
}
    ASM("       ld       x3,    8(sp)           \n"     // pop ST into TMP
        "       li      x10, %0                 \n"     // use X10 as a second TMP, since it will be restored later
        "       or       x3, x3, x10            \n" : : "i"(supervisor ? SPP_S : MPP_M)); // [M|S]STATUS.[S|M]PP is automatically cleared on the [M|S]RET in the ISR, so we need to recover it here
    ASM("       ld       x1,   16(sp)           \n"     // pop RA
        "       ld       x5,   24(sp)           \n"     // pop X5-X31
        "       ld       x6,   32(sp)           \n"
        "       ld       x7,   40(sp)           \n"
        "       ld       x8,   48(sp)           \n"
        "       ld       x9,   56(sp)           \n"
        "       ld      x10,   64(sp)           \n"
        "       ld      x11,   72(sp)           \n"
        "       ld      x12,   80(sp)           \n"
        "       ld      x13,   88(sp)           \n"
        "       ld      x14,   96(sp)           \n"
        "       ld      x15,  104(sp)           \n"
        "       ld      x16,  112(sp)           \n"
        "       ld      x17,  120(sp)           \n"
        "       ld      x18,  128(sp)           \n"
        "       ld      x19,  136(sp)           \n"
        "       ld      x20,  144(sp)           \n"
        "       ld      x21,  152(sp)           \n"
        "       ld      x22,  160(sp)           \n"
        "       ld      x23,  168(sp)           \n"
        "       ld      x24,  176(sp)           \n"
        "       ld      x25,  184(sp)           \n"
        "       ld      x26,  192(sp)           \n"
        "       ld      x27,  200(sp)           \n"
        "       ld      x28,  208(sp)           \n"
        "       ld      x29,  216(sp)           \n"
        "       ld      x30,  224(sp)           \n"
        "       ld      x31,  232(sp)           \n"
        "       addi    sp, sp, %0              \n" : : "i"(sizeof(Context))); // complete the pops above by adjusting SP
if(supervisor) {
    ASM("       csrw    sstatus, x3             \n");   // SSTATUS = ST
} else {
    ASM("       csrw    mstatus, x3             \n");   // MSTATUS = ST
}
}

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
