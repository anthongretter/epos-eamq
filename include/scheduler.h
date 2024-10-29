// EPOS Scheduler Component Declarations

#ifndef __scheduler_h
#define __scheduler_h

#include <architecture/cpu.h>
#include <architecture/pmu.h>
#include <architecture/tsc.h>
#include <utility/scheduling.h>
#include <utility/math.h>
#include <utility/convert.h>

__BEGIN_SYS

// All scheduling criteria, or disciplines, must define operator int() with
// the semantics of returning the desired order of a given object within the
// scheduling list
class Scheduling_Criterion_Common
{
    friend class Thread;                // for handle(), init 
    friend class Periodic_Thread;       // for handle()
    friend class RT_Thread;             // for handle()

protected:
    typedef Timer_Common::Tick Tick;

public:
    // Priorities
    enum : int
    {
        CEILING = -1000,
        MAIN = -1,
        HIGH = 0,
        NORMAL = (unsigned(1) << (sizeof(int) * 8 - 3)) - 1,
        LOW = (unsigned(1) << (sizeof(int) * 8 - 2)) - 1,
        IDLE = (unsigned(1) << (sizeof(int) * 8 - 1)) - 1
    };

    // Constructor helpers
    enum : unsigned int
    {
        SAME = 0,
        NOW = 0,
        UNKNOWN = 0,
        ANY = -1U
    };

    // Policy types
    enum : int
    {
        PERIODIC = HIGH,
        SPORADIC = NORMAL,
        APERIODIC = LOW
    };

    // Policy events
    typedef int Event;
    enum
    {
        CREATE = 1 << 0,
        FINISH = 1 << 1,
        ENTER = 1 << 2,
        LEAVE = 1 << 3,
        JOB_RELEASE = 1 << 4,
        JOB_FINISH = 1 << 5,
    };

    // Policy operations
    typedef int Operation;
    enum
    {
        COLLECT = 1 << 16,
        CHARGE = 1 << 17,
        AWARD = 1 << 18,
        UPDATE = 1 << 19
    };

    // Policy traits
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = true;
    static const unsigned int QUEUES = 1;

    // Runtime Statistics (for policies that don't use any; that's why its a union)
    union Dummy_Statistics
    { // for Traits<System>::monitored = false
        // Thread related statistics
        Tick thread_creation;        // tick in which the thread was created
        Tick thread_destruction;     // tick in which the thread was destroyed
        Tick thread_execution_time;  // accumulated execution time (in ticks)
        Tick thread_last_dispatch;   // tick in which the thread was last dispatched to the CPU
        Tick thread_last_preemption; // tick in which the thread left the CPU by the last time

        // Job related statistics
        bool job_released;
        Tick job_release;           // tick in which the last job of a periodic thread was made ready for execution
        Tick job_start;             // tick in which the last job of a periodic thread started (different from "thread_last_dispatch" since jobs can be preempted)
        Tick job_finish;            // tick in which the last job of a periodic thread finished (i.e. called _alarm->p() at wait_netxt(); different from "thread_last_preemption" since jobs can be preempted)
        Tick job_utilization;       // accumulated execution time (in ticks)
        unsigned int jobs_released; // number of jobs of a thread that were released so far (i.e. the number of times _alarm->v() was called by the Alarm::handler())
        unsigned int jobs_finished; // number of jobs of a thread that finished execution so far (i.e. the number of times alarm->p() was called at wait_next())
    };

    struct Real_Statistics
    { // for Traits<System>::monitored = true
        // Thread related statistics
        Tick thread_creation;        // tick in which the thread was created
        Tick thread_destruction;     // tick in which the thread was destroyed
        Tick thread_execution_time;  // accumulated execution time (in ticks)
        Tick thread_last_dispatch;   // tick in which the thread was last dispatched to the CPU
        Tick thread_last_preemption; // tick in which the thread left the CPU by the last time

        // Job related statistics
        bool job_released;
        Tick job_release;           // tick in which the last job of a periodic thread was made ready for execution
        Tick job_start;             // tick in which the last job of a periodic thread started (different from "thread_last_dispatch" since jobs can be preempted)
        Tick job_finish;            // tick in which the last job of a periodic thread finished (i.e. called _alarm->p() at wait_netxt(); different from "thread_last_preemption" since jobs can be preempted)
        Tick job_utilization;       // accumulated execution time (in ticks)
        unsigned int jobs_released; // number of jobs of a thread that were released so far (i.e. the number of times _alarm->v() was called by the Alarm::handler())
        unsigned int jobs_finished; // number of jobs of a thread that finished execution so far (i.e. the number of times alarm->p() was called at wait_next())
    };

    typedef IF<Traits<System>::monitored, Real_Statistics, Dummy_Statistics>::Result Statistics;

protected:
    Scheduling_Criterion_Common() {}

public:
    Microsecond period() { return 0; }
    Microsecond deadline() { return 0; }
    Microsecond capacity() { return 0; }

    bool periodic() { return false; }

    volatile Statistics & statistics() { return _statistics; }
    //P3 - Alteração
    unsigned int queue() const { return 0; }

protected:
    void handle(Event event) {}
    void queue(unsigned int q) {}
    void update() {}

    static void init() {}

protected:
    Statistics _statistics;
};

// Priority (static and dynamic)
class Priority : public Scheduling_Criterion_Common
{
public:
    template <typename... Tn>
    Priority(int p = NORMAL, Tn &...an) : _priority(p) {}

    operator const volatile int() const volatile { return _priority; }

protected:
    volatile int _priority;
};

// Round-Robin
class RR : public Priority
{
public:
    static const bool timed = true;
    static const bool dynamic = false;
    static const bool preemptive = true;

public:
    template <typename... Tn>
    RR(int p = NORMAL, Tn &...an) : Priority(p) {}
};

// First-Come, First-Served (FIFO)
class FCFS : public Priority
{
public:
    static const bool timed = false;
    static const bool dynamic = false;
    static const bool preemptive = false;

public:
    template <typename... Tn>
    FCFS(int p = NORMAL, Tn &...an);
};


// Multicore Algorithms
class Variable_Queue_Scheduler
{
protected:
    Variable_Queue_Scheduler(unsigned int queue): _queue(queue) {};

    const volatile unsigned int & queue() const volatile { return _queue; }
    void queue(unsigned int q) { _queue = q; }

protected:
    volatile unsigned int _queue;
    static volatile unsigned int _next_queue;
};

// Global Round-Robin
class GRR: public RR
{
public:
    static const unsigned int HEADS = Traits<Machine>::CPUS;

public:
    template <typename ... Tn>
    GRR(int p = NORMAL, Tn & ... an): RR(p) {}

    static unsigned int current_head() { return CPU::id(); }
};

// Fixed CPU (fully partitioned)
class Fixed_CPU: public Priority, public Variable_Queue_Scheduler
{
public:
    static const bool timed = true;
    static const bool dynamic = false;
    static const bool preemptive = true;

    static const unsigned int QUEUES = Traits<Machine>::CPUS;

public:
    template <typename ... Tn>
    Fixed_CPU(int p = NORMAL, unsigned int cpu = ANY, Tn & ... an)
    : Priority(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : (cpu != ANY) ? cpu : ++_next_queue %= CPU::cores()) {}

    using Variable_Queue_Scheduler::queue;
    static unsigned int current_queue() { return CPU::id(); }
};

// CPU Affinity
class CPU_Affinity: public Priority, public Variable_Queue_Scheduler
{
public:
    static const bool timed = true;
    static const bool dynamic = false;
    static const bool preemptive = true;
    static const bool heuristic = true;
    static const unsigned int QUEUES = Traits<Machine>::CPUS;

public:
    template <typename ... Tn>
    CPU_Affinity(int p = NORMAL, unsigned int cpu = ANY, Tn & ... an)
    : Priority(p), Variable_Queue_Scheduler(((_priority == IDLE) || (_priority == MAIN)) ? CPU::id() : (cpu != ANY) ? cpu : ++_next_queue %= CPU::cores()) {}

    bool charge(bool end = false);
    bool award(bool end = false);

    using Variable_Queue_Scheduler::queue;
    static unsigned int current_queue() { return CPU::id(); }
};

// Real-time Algorithms
class RT_Common : public Priority
{
    friend class FCFS;
    friend class Thread;          // for handle() and queue()
    friend class Periodic_Thread; // for handle() and queue()
    friend class RT_Thread;       // for handle() and queue()

public:
    static const bool timed = true;
    static const bool preemptive = true;

protected:
    RT_Common(int i) : Priority(i), _period(0), _deadline(0), _capacity(0) {} // aperiodic
    RT_Common(int i, Microsecond p, Microsecond d, Microsecond c) : Priority(i), _period(ticks(p)), _deadline(ticks(d ? d : p)), _capacity(ticks(c)) {}

public:
    Microsecond period() { return time(_period); }
    Microsecond deadline() { return time(_deadline); }
    Microsecond capacity() { return time(_capacity); }

    bool periodic() { return (_priority >= PERIODIC) && (_priority <= SPORADIC); }

    volatile Statistics &statistics() { return _statistics; }

protected:
    Tick ticks(Microsecond time);
    Microsecond time(Tick ticks);

    void handle(Event event);

    static Tick elapsed();

protected:
    Tick _period;
    Tick _deadline;
    Tick _capacity;
    Statistics _statistics;
};

// Rate Monotonic
class RM : public RT_Common
{
public:
    static const bool dynamic = false;

public:
    RM(int p = APERIODIC): RT_Common(p) {}
    RM(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN, unsigned int cpu = ANY): RT_Common(int(ticks(p)), p, d, c) {}
};

// Deadline Monotonic
class DM : public RT_Common
{
public:
    static const bool dynamic = false;

public:
    DM(int p = APERIODIC): RT_Common(p) {}
    DM(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN, unsigned int cpu = ANY): RT_Common(int(ticks(d ? d : p)), p, d, c) {}
};

// Laxity Monotonic
class LM : public RT_Common
{
public:
    static const bool dynamic = false;

public:
    LM(int p = APERIODIC): RT_Common(p) {}
    LM(Microsecond p, Microsecond d, Microsecond c, unsigned int cpu = ANY): RT_Common(int(ticks((d ? d : p) - c)), p, d, c) {}
};

// Earliest Deadline First
class EDF : public RT_Common
{
public:
    static const bool dynamic = true;

public:
    EDF(int p = APERIODIC): RT_Common(p) {}
    EDF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN, unsigned int cpu = ANY);

    void handle(Event event);
};

// Least Laxity First
class LLF : public RT_Common
{
public:
    static const bool dynamic = true;

public:
    LLF(int p = APERIODIC): RT_Common(p) {}
    LLF(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN, unsigned int cpu = ANY);

    void handle(Event event);
};

// Energy Aware Multi Queue
class EAMQ : public RT_Common
{

public:
    static const unsigned short QUEUES = 4;
    static const unsigned int Q = Traits<Thread>::QUANTUM;
    static const bool dynamic = true;

public:
    EAMQ(int p = APERIODIC);
    EAMQ(Microsecond p, Microsecond d = SAME, Microsecond c = UNKNOWN);

    enum
    {
        ASSURE_BEHIND = 1 << 6,
        CHANGE_QUEUE = 1 << 7,
        RESUME_THREAD = 1 << 8,
        // LEAVING_QUEUE = 1 << 9   // Nao implementado ainda
    };

    struct Personal_Statistics
    {
        Microsecond remaining_deadline;         // deadline restante 
        Microsecond remaining_et[QUEUES];       // tempo de execucao restante em cada frequencia (inicia como job_estimated_et)
        Microsecond job_estimated_et[QUEUES];   // tempo de execução estimado em cada frequencia dada media
        Microsecond average_et[QUEUES];         // tempo de execução média ponderada

        Microsecond job_execution_time; // tempo de execução real acumulada da tarefa
        Tick job_enter_tick;            // tempo de entrada do ultimo job
    };

    void handle(Event event);

    Personal_Statistics personal_statistics() { return _personal_statistics; }

    const bool is_recent_insertion() { return _is_recent_insertion; }
    void is_recent_insertion(bool b) { _is_recent_insertion = b; }

    int rank_eamq();
    const volatile unsigned int &queue() const volatile { return _queue; } // returns the Thread's queue

    static const volatile unsigned int &current_queue() { db<Thread>(WRN) << "PROBLEMAAAA: " << endl; return _current_queue;} // current global queue
    virtual void next_queue() { ++_current_queue %= QUEUES;  db<Thread>(WRN) << "PROBLEMAAAA: " << endl;}        // points to next global queue with threads

protected:    
    void set_queue(unsigned int q) { _queue = q; };

    /* Em caso de 4 filas em relacao a frequencia maxima:
     *   0 -> 100%
     *   1 -> 87%
     *   2 -> 75%
     *   3 -> 62%
     */
    static Hertz frequency_within(unsigned int queue)
    {
        Hertz f = CPU::max_clock() - (((CPU::max_clock() / 1000) * 125) * queue);
        return f;
    };

    /* Procura pela melhor thread na subfila q (posicao a colocar), 
     * onde o slack seja o menor possivel
     */
    Thread * search_t_fitted(unsigned int q);

    /* Estima o tempo de espera (em numeros de quantums) que
     * levaria ate o _current_queue chegar em q
     */
    int estimate_rp_waiting_time(unsigned int q);


protected:
    volatile unsigned int _queue;
    bool _is_recent_insertion;
    Personal_Statistics _personal_statistics;
    Thread *_behind_of;

    static volatile unsigned int _current_queue; 
};

// P3TEST - Multicore Global Scheduling 
class GEAMQ : public EAMQ
{
public:
    static const unsigned HEADS = Traits<Machine>::CPUS;

    // initialize_current_queue pois a main que é a primeira a rodar é aperiodica
    GEAMQ(int p = APERIODIC): EAMQ(p) {initialize_current_queue();}
    GEAMQ(const Microsecond p, const Microsecond d = SAME, const Microsecond c = UNKNOWN): EAMQ(p, d, c) {}
    
    using EAMQ::queue;
    // static volatile unsigned current_queue[HEADS];
    
protected:
    static volatile unsigned int _current_queue[HEADS];

    static void initialize_current_queue() {
        if (!initialized) {
            for (volatile unsigned int &q : _current_queue) {
                q = QUEUES - 1;
            }
            initialized = true;
        }
    }
    
    static bool initialized;

public:
    int rank_eamq();
    void handle(Event event);
    void next_queue() override { 
        _current_queue[CPU::id()] = (_current_queue[CPU::id()] + 1) % QUEUES;
        //db<Thread>(WRN) << "core: " << CPU::id() << " current queue: "  << _current_queue[CPU::id()] << endl;
    }
    static const volatile unsigned int &current_queue() {
        //db<Thread>(WRN) << "_current_queue[CPU::id()]:  " << _current_queue[CPU::id()] << endl; 
        return _current_queue[CPU::id()]; }
    static unsigned int current_head() { return CPU::id(); }


};

__END_SYS

__BEGIN_UTIL

// P3TEST - usando multihead multilist
template <typename T>
class Scheduling_Queue<T, GEAMQ> : public Multihead_Scheduling_Multilist<T>{};

template <typename T>
class Scheduling_Queue<T, EAMQ> : public Scheduling_Multilist_Single_Chosen<T>{};
// Scheduling Queues
template<typename T>
class Scheduling_Queue<T, GRR>:
public Multihead_Scheduling_List<T> {};

template<typename T>
class Scheduling_Queue<T, Fixed_CPU>:
public Scheduling_Multilist<T> {};

template<typename T>
class Scheduling_Queue<T, CPU_Affinity>:
public Scheduling_Multilist<T> {};

__END_UTIL

#endif
