// EPOS Thread Initialization

#include <machine/timer.h>
#include <machine/ic.h>
#include <system.h>
#include <process.h>

__BEGIN_SYS

extern "C" { void __epos_app_entry(); }

void Thread::init()
{
    db<Init, Thread>(TRC) << "Thread::init()" << endl;

    // Install an interrupt handler to receive forced reschedules
    if(smp && (CPU::id() == CPU::BSP))
        IC::int_vector(IC::INT_RESCHEDULER, rescheduler);  // if an eoi handler is needed, then it was already installed at IC::init()

    // P3 - espera todos cores para não ativar interrupção de rescheduler
    // antes de BSP configurar interrupção
    CPU::smp_barrier();

    if(smp)
        IC::enable(IC::INT_RESCHEDULER);

    Criterion::init();

    if(CPU::id() == CPU::BSP) {
        typedef int (Main)();

        // If EPOS is a library, then adjust the application entry point to __epos_app_entry, which will directly call main().
        // In this case, _init will have already been called, before Init_Application to construct MAIN's global objects.
        Main * main = reinterpret_cast<Main *>(__epos_app_entry);
        
        new (SYSTEM) Thread(Thread::Configuration(Thread::RUNNING, Thread::MAIN), main);
    } 
    // P3 - outros cores precisa esperar BSP criar thread MAIN antes
    if (CPU::id() != CPU::BSP)
        CPU::smp_barrier();

    // Timer reset dito pelo Leonardo
    Timer::reset();
    // Idle thread creation does not cause rescheduling (see Thread::constructor_epilogue)
    new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::IDLE), &Thread::idle);

    if (CPU::id() == CPU::BSP)
        CPU::smp_barrier();

    // The installation of the scheduler timer handler does not need to be done after the
    // creation of threads, since the constructor won't call reschedule() which won't call
    // dispatch that could call timer->reset()
    // Letting reschedule() happen during thread creation is also harmless, since MAIN is
    // created first and dispatch won't replace it nor by itself neither by IDLE (which
    // has a lower priority)
    if(Criterion::timed && (CPU::id() == CPU::BSP))
        _timer = new (SYSTEM) Scheduler_Timer(QUANTUM, time_slicer);

    // No more interrupts until we reach init_end
    CPU::int_disable();

    CPU::smp_barrier();

    // Transition from CPU-based locking to thread-based locking
    // if(CPU::id() == CPU::BSP)
    //     _not_booting = true;
    _not_booting = true;
}

__END_SYS
