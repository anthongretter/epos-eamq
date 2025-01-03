// EPOS IA32 CPU Mediator Initialization

#include <architecture.h>
#include <machine/machine.h>
#include <system.h>

__BEGIN_SYS

void CPU::init()
{
    db<Init, CPU>(TRC) << "CPU::init()" << endl;

    _cpu_clock = System::info()->tm.cpu_clock;
    _cpu_current_clock = System::info()->tm.cpu_clock;
    _bus_clock = System::info()->tm.bus_clock;

    // Initialize the MMU
    if(CPU::id() == CPU::BSP) {
        if(Traits<MMU>::enabled)
            MMU::init();
        else
            db<Init, MMU>(WRN) << "MMU is disabled!" << endl;
    }

    // Initialize the PMU	
    if(Traits<PMU>::enabled) {
        PMU::init();
        // P6 : evento cache miss e branch miss, talvez precisa de branch instructions e cache hit para calculo?
        PMU::config(6,25);     // Cache miss (L1)
        PMU::config(5,24);     // Cache hit (L1)
        PMU::config(4,11);     // Branches
        PMU::config(3,15);     // Branch miss

        PMU::start(6);
        PMU::start(5);
        PMU::start(4);
        PMU::start(3);

        PMU::reset(6);
        PMU::reset(5);
        PMU::reset(4);
        PMU::reset(3);

        // P6 : adicionando start counters (PDF - Leonardo)
        PMU::config(2,2);
        PMU::config(1,1);
        PMU::config(0,0);

        PMU::start(2);
        PMU::start(1);
        PMU::start(0);

        PMU::reset(2);
        PMU::reset(1);
        PMU::reset(0);
    }
}

void CPU::smp_barrier_init(unsigned int cores) {
    // Core activation in IA32 is handled by the APIC
    _cores = cores;
    if(multicore)
        APIC::remap();
};

__END_SYS
