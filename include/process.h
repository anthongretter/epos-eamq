// EPOS Thread Component Declarations

#ifndef __process_h
#define __process_h

#include <architecture.h>
#include <machine.h>
#include <utility/queue.h>
#include <utility/handler.h>
#include <scheduler.h>

extern "C" {
    void __exit();
    volatile unsigned long _running() __attribute__ ((alias("_ZN4EPOS1S6Thread7runningEv")));;
}

__BEGIN_SYS

class Thread
{
    friend class Init_End;              // context->load()
    friend class Init_System;           // for init() on CPU != 0
    friend class Scheduler<Thread>;     // for link()
    friend class Synchronizer_Common;   // for lock() and sleep()
    friend class Alarm;                 // for lock()
    friend class System;                // for init()
    friend class EAMQ;                  // for handle()
    friend class GEAMQ;                 // for handle()
    friend class IC;                            // for link() for priority ceiling
    friend volatile unsigned long ::_running(); // for running()

protected:
    static const bool smp = Traits<Thread>::smp;
    static const bool preemptive = Traits<Thread>::Criterion::preemptive;
    static const int priority_inversion_protocol = Traits<Thread>::priority_inversion_protocol;
    static const unsigned int QUANTUM = Traits<Thread>::QUANTUM;
    static const unsigned int STACK_SIZE = Traits<Application>::STACK_SIZE;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Context Context;

public:
    // Thread State
    enum State {
        RUNNING,
        READY,
        SUSPENDED,
        WAITING,
        FINISHING
    };

    // Thread Scheduling Criterion
    typedef Traits<Thread>::Criterion Criterion;
    enum {
        CEILING = Criterion::CEILING,
        HIGH    = Criterion::HIGH,
        NORMAL  = Criterion::NORMAL,
        LOW     = Criterion::LOW,
        MAIN    = Criterion::MAIN,
        IDLE    = Criterion::IDLE
    };

    // Thread Queue
    typedef Ordered_Queue<Thread, Criterion, Scheduler<Thread>::Element> Queue;

    // Thread Configuration
    struct Configuration {
        Configuration(State s = READY, Criterion c = NORMAL, unsigned int ss = STACK_SIZE, unsigned int d = 10)
        : state(s), criterion(c), stack_size(ss) {}

        State state;
        Criterion criterion;
        unsigned int stack_size;
    };


public:
    template<typename ... Tn>
    Thread(int (* entry)(Tn ...), Tn ... an);
    template<typename ... Tn>
    Thread(Configuration conf, int (* entry)(Tn ...), Tn ... an);
    ~Thread();

    const volatile State & state() const { return _state; }
    Criterion & criterion() { return const_cast<Criterion &>(_link.rank()); }
    volatile Criterion::Statistics & statistics() { return criterion().statistics(); }

    const volatile Criterion & priority() const { return _link.rank(); }
    void priority(Criterion p);

    Task * task() const { return _task; }

    int join();
    void pass();
    void suspend();
    void resume();

    static Thread * volatile self() { return running(); }
    static Scheduler<Thread> * volatile scheduler() { return &_scheduler; }
    static void yield();
    static void exit(int status = 0);

    // for_all_behind() is used to update the rank of all threads behind the current one
    // the first time it is called is when a thread is inserted in the scheduler
    // but it can be called again if other threads changes their queue
    void for_all_behind(Criterion::Event event) {
        this->criterion().is_recent_insertion(true);
   
        for (Queue::Element * behind = _link.next(); behind != nullptr;)
        {
            // if finds a aperiodic thread, stop
            if (!behind->object()->criterion().periodic()) break;

            // next element to be evaluated
            Queue::Element * next = behind->next();

            // throws event to the thread
            behind->object()->criterion().handle(event);

            // remove the thread from the scheduler
            _scheduler.remove(behind->object());

            // recalculates the rank of the thread
            behind->object()->criterion().rank_eamq();

            // insert thread in new position (or in the old one)
            _scheduler.insert(behind->object());

            // if the thread has changed its queue, it is necessary to check the new previous ones
            if (this->criterion().queue_eamq() != behind->object()->criterion().queue_eamq()) {
                behind->object()->for_all_behind(event);
            }

            behind = next;
        }
    }
    alignas (int) static bool _not_booting;

protected:
    void constructor_prologue(unsigned int stack_size);
    void constructor_epilogue(Log_Addr entry, unsigned int stack_size);

    Queue::Element * link() { return &_link; }

    static Thread * volatile running() { 
        if (_not_booting) {
            // db<PEAMQ>(WRN) << "AAAAAAAAA Thread::running() => " << _scheduler.chosen() << endl;
            return _scheduler.chosen();
        } else {
            //db<GEAMQ>(WRN) << "BBBBBBBBB NOT BOOTING!! " << reinterpret_cast<Thread * volatile>(CPU::id() + 1)<< endl;
            return reinterpret_cast<Thread * volatile>(CPU::id() + 1);
        }
    }

    static void lock() {
        CPU::int_disable();
        if(smp)
            _lock.acquire();
    }

    static void unlock() {
        if(smp)
            _lock.release();
        if(_not_booting)
            CPU::int_enable();
    }

    static volatile bool locked() { return (smp) ? _lock.taken() : CPU::int_disabled(); }

    static void sleep(Queue * q);
    static void wakeup(Queue * q);
    static void wakeup_all(Queue * q);

    static void reschedule();
    static void reschedule(unsigned int cpu);
    static void rescheduler(IC::Interrupt_Id interrupt);
    static void time_slicer(IC::Interrupt_Id interrupt);

    static void dispatch(Thread * prev, Thread * next, bool charge = true);

    static void for_all_threads(Criterion::Event event) {
        for(Queue::Iterator i = _scheduler.begin(); i != _scheduler.end(); ++i)
            if(i->object()->criterion() != IDLE)
                i->object()->criterion().handle(event);
    }

    static int idle();

private:
    static void init();

protected:
    Task * _task;

    char * _stack;
    Context * volatile _context;
    volatile State _state;
    Criterion _natural_priority;
    Queue * _waiting;
    Thread * volatile _joining;
    Queue::Element _link;

    static volatile unsigned int _thread_count;
    static Scheduler_Timer * _timer;
    static Scheduler<Thread> _scheduler;
    static Spin _lock;
};

class Task
{
    friend class Thread;           // for Task(), enroll() and dismiss()
    friend class Alarm;            // for enroll() and dismiss()
    friend class Mutex;            // for enroll() and dismiss()
    friend class Condition;        // for enroll() and dismiss()
    friend class Semaphore;        // for enroll() and dismiss()
    friend class Segment;          // for enroll() and dismiss()

private:
    typedef Typed_List<> Resources;
    typedef Resources::Element Resource;

protected:
    // This constructor is only used by Thread::init()
    template<typename ... Tn>
    Task(int (* entry)(Tn ...), Tn ... an) {
        db<Task, Init>(TRC) << "Task(entry=" << reinterpret_cast<void *>(entry) << ") => " << this << endl;

        _current = this;
        _main = new (SYSTEM) Thread(Thread::Configuration(Thread::RUNNING, Thread::MAIN), entry, an ...);
    }

public:
    ~Task();

    Thread * main() const { return _main; }

    int join() { return _main->join(); }

    static Task * volatile self() { return current(); }

private:
    template<typename T>
    void enroll(T * o) {
    	db<Task>(TRC) << "Task::enroll(t=" << Type<T>::ID << ", o=" << o << ")" << endl;
    	_resources.insert(new (SYSTEM) Resource(o, Type<T>::ID));
    }
    void dismiss(void * o) {
    	db<Task>(TRC) << "Task::dismiss(" << o << ")" << endl;
    	Resource * r = _resources.remove(o);
    	if(r) delete r;
    }

    static Task * volatile current() { return _current; }
    static void current(Task * t) { _current = t; }

    static void init();

private:
    Thread * _main;
    Resources _resources;

    static Task * volatile _current;
};


template<typename ... Tn>
inline Thread::Thread(int (* entry)(Tn ...), Tn ... an)
: _task(Task::self()), _state(READY), _waiting(0), _joining(0), _link(this, NORMAL)
{
    constructor_prologue(STACK_SIZE);
    _context = CPU::init_stack(0, _stack + STACK_SIZE, &__exit, entry, an ...);
    constructor_epilogue(entry, STACK_SIZE);
}

template<typename ... Tn>
inline Thread::Thread(Configuration conf, int (* entry)(Tn ...), Tn ... an)
: _task(Task::self()), _state(conf.state), _waiting(0), _joining(0), _link(this, conf.criterion)
{
    constructor_prologue(conf.stack_size);
    _context = CPU::init_stack(0, _stack + conf.stack_size, &__exit, entry, an ...);
    constructor_epilogue(entry, conf.stack_size);
}


// A Java-like Active Object
class Active: public Thread
{
public:
    Active(): Thread(Configuration(Thread::SUSPENDED), &entry, this) {}
    virtual ~Active() {}

    virtual int run() = 0;

    void start() { resume(); }

private:
    static int entry(Active * runnable) { return runnable->run(); }
};


// An event handler that triggers a thread (see handler.h)
class Thread_Handler : public Handler
{
public:
    Thread_Handler(Thread * h) : _handler(h) {}
    ~Thread_Handler() {}

    void operator()() { _handler->resume(); }

private:
    Thread * _handler;
};

__END_SYS

#endif
