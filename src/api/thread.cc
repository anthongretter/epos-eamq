// EPOS Thread Implementation

#include <machine.h>
#include <system.h>
#include <process.h>

__BEGIN_SYS

extern OStream kout;

bool Thread::_not_booting;
volatile unsigned int Thread::_thread_count;
Scheduler_Timer *Thread::_timer;
Scheduler<Thread> Thread::_scheduler;
Spin Thread::_lock;

void Thread::constructor_prologue(unsigned int stack_size)
{
    lock();

    _thread_count++;
    _scheduler.insert(this);

    _stack = new (SYSTEM) char[stack_size];
}

void Thread::constructor_epilogue(Log_Addr entry, unsigned int stack_size)
{
    db<GEAMQ>(TRC) << "Thread(entry=" << entry
                    << ",state=" << _state
                    << ",priority=" << _link.rank()
                    << ",queue=" << _link.rank().queue()
                    << ",stack={b=" << reinterpret_cast<void *>(_stack)
                    << ",s=" << stack_size
                    << "},context={b=" << _context
                    << "," << *_context << "}) => " << _link.object() << "@" << _link.rank().queue() << endl;

    assert((_state != WAITING) && (_state != FINISHING)); // invalid states

    if (_link.rank() != IDLE)
        _task->enroll(this);

    if ((_state != READY) && (_state != RUNNING)) {
        db<Thread>(WRN) << "Thread not ready!" << endl;
        _scheduler.suspend(this);
    }

    criterion().handle(Criterion::CREATE);

    if(preemptive && (_state == READY) && (_link.rank() != IDLE)) {
        db<Thread>(WRN) << "Thread ready!" << endl;
        reschedule(_link.rank().queue());
    }

    unlock();
}

Thread::~Thread()
{
    // ainda não consegui ver o destrutor funcionando nenhuma vez

    db<PEAMQ>(WRN) << "Chamou o destrutor" << endl;
    lock();
    db<PEAMQ>(WRN) << "Lockou no destrutor" << endl;


    db<Thread>(TRC) << "~Thread(this=" << this
                    << ",state=" << _state
                    << ",priority=" << _link.rank()
                    << ",stack={b=" << reinterpret_cast<void *>(_stack)
                    << ",context={b=" << _context
                    << "," << *_context << "})" << endl;

    // The running thread cannot delete itself!
    assert(_state != RUNNING);

    switch (_state)
    {
    case RUNNING: // For switch completion only: the running thread would have deleted itself! Stack wouldn't have been released!
        exit(-1);
        break;
    case READY:
        _scheduler.remove(this);
        _thread_count--;
        break;
    case SUSPENDED:
        _scheduler.resume(this);
        _scheduler.remove(this);
        _thread_count--;
        break;
    case WAITING:
        _waiting->remove(this);
        _scheduler.resume(this);
        _scheduler.remove(this);
        _thread_count--;
        break;
    case FINISHING: // Already called exit()
        break;
    }

    _task->dismiss(this);

    if (_joining)
        _joining->resume();

    unlock();
    db<PEAMQ>(WRN) << "Unlockou no destrutor" << endl;

    delete _stack;
}

void Thread::priority(Criterion c)
{
    lock();

    db<Thread>(TRC) << "Thread::priority(this=" << this << ",prio=" << c << ")" << endl;

    // P3 - Veriaveis novos
    unsigned long old_cpu = _link.rank().queue();
    unsigned long new_cpu = c.queue();

    if(_state != RUNNING) { // reorder the scheduling queue
        _scheduler.suspend(this);
        // We dont throw the rerank event here because we are explicitly changing the priority
        _link.rank(c);
        _scheduler.resume(this);
    }
    else
        _link.rank(c);

    if(preemptive) {
    	if(smp) {
    	    if(old_cpu != CPU::id())
    	        reschedule(old_cpu);
    	    if(new_cpu != CPU::id())
    	        reschedule(new_cpu);
    	} else
    	    reschedule();
    }

    unlock();
}

int Thread::join()
{
    lock();

    db<Thread>(WRN) << "Thread::join(this=" << this << ",state=" << _state << ")" << endl;
    // Precondition: no Thread::self()->join()
    assert(running() != this);

    // Precondition: a single joiner
    assert(!_joining);

    if (_state != FINISHING)
    {
        Thread *prev = running();

        _joining = prev;
        prev->_state = SUSPENDED;
        prev->criterion().handle(EAMQ::CHANGE_QUEUE);
        _scheduler.suspend(prev); // implicitly choose() if suspending chosen()
        // db<GEAMQ>(WRN) << "JOIN prev: " << prev << endl;
        Thread *next = _scheduler.chosen();

        //db<PEAMQ>(WRN) << "PROXIMO THREAD: " << next->link() << endl;

        dispatch(prev, next);
    }

    unlock();

    return *reinterpret_cast<int *>(_stack);
}

void Thread::pass()
{
    lock();

    
    db<Thread>(TRC) << "Thread::pass(this=" << this << ")" << endl;

    Thread *prev = running();
    // db<GEAMQ>(WRN) << "PASS prev: " << prev << endl;
    prev->criterion().handle(EAMQ::CHANGE_QUEUE);
    Thread *next = _scheduler.choose(this);

    if (next)
        dispatch(prev, next, false);
    else
        db<Thread>(WRN) << "Thread::pass => thread (" << this << ") not ready!" << endl;

    unlock();
}

void Thread::suspend()
{
    lock();

    db<Thread>(TRC) << "Thread::suspend(this=" << this << ")" << endl;

    Thread *prev = running();

    _state = SUSPENDED;
    // TODO: Throw a new event to recalculate rank from behind when the thread is suspend (removed)
    _scheduler.suspend(this);

    Thread *next = _scheduler.chosen();

    dispatch(prev, next);

    unlock();
}

void Thread::resume()
{
    lock();

    db<GEAMQ>(TRC) << "Thread::resume(this=" << this << ") state= " << _state << endl;

    if (_state == SUSPENDED)
    {
        _state = READY;
        // Recalcular rank antes de voltar 
        db<GEAMQ>(TRC) << "Calling handle" << endl;
        this->criterion().handle(EAMQ::RESUME_THREAD);

        db<GEAMQ>(TRC) << "Calling resume" << endl;
        _scheduler.resume(this);

        if(preemptive) {
            db<GEAMQ>(TRC) << "Calling reschedule" << endl;
            reschedule(_link.rank().queue());
        }
    } else
        db<GEAMQ>(TRC) << "Resume called for unsuspended object!" << endl;

    unlock();
}

void Thread::yield()
{
    lock();

    db<Thread>(TRC) << "Thread::yield(running=" << running() << ")" << endl;

    Thread *prev = running();
    // P4 - Acho que precisa disso 
    prev->criterion().handle(EAMQ::CHANGE_QUEUE);
    
    Thread *next = _scheduler.choose_another();

    dispatch(prev, next);

    unlock();
}

void Thread::exit(int status)
{
    lock();
    // db<Thread>(TRC) << "Thread::exit(status=" << status << ") [running=" << running() << "]" << endl;

    Thread *prev = running();

    // vejam remove() da lista, nós presumimos que a fila já está atualizada
    // caso o chosen seja removido
    // (na prática não há diferença alguma, pois ele será reinserido,
    // anteriormente pensei que fosse corrigir o ultimo bug que falei no whats)
    prev->criterion().handle(EAMQ::CHANGE_QUEUE);

    _scheduler.remove(prev);
    prev->_state = FINISHING;
    *reinterpret_cast<int *>(prev->_stack) = status;
    prev->criterion().handle(Criterion::FINISH);

    _thread_count--;

    if (prev->_joining)
    {
        prev->_joining->_state = READY;
        prev->_joining->criterion().handle(EAMQ::RESUME_THREAD);
        _scheduler.resume(prev->_joining);
        prev->_joining = 0;
    }

    Thread *next = _scheduler.choose(); // at least idle will always be there

    dispatch(prev, next);

    unlock();
}

void Thread::sleep(Queue *q)
{
    assert(locked()); // locking handled by caller
    
    Thread *prev = running();
    prev->criterion().handle(EAMQ::CHANGE_QUEUE);
    _scheduler.suspend(prev);
    prev->_state = WAITING;
    prev->_waiting = q;
    q->insert(&prev->_link);

    Thread *next = _scheduler.chosen();
    //db<PEAMQ>(WRN) << "PREV: " << prev->link() << ", NEXT: "<< next->link() << endl;
    if (next == nullptr) {
        db<GEAMQ>(WRN) << "TAMANHO DA FILA"<< GEAMQ::current_queue() << ": " << _scheduler.size(GEAMQ::current_queue())<< endl;
    }

    dispatch(prev, next);

}

void Thread::wakeup(Queue *q)
{
    assert(locked()); // locking handled by caller

    if (!q->empty())
    {

        Thread *t = q->remove()->object();
        
        t->_state = READY;
        t->_waiting = 0;


        t->criterion().handle(EAMQ::RESUME_THREAD);

        // TODO: Throw a new event to recalculate rank from behind when the thread is woken up
        _scheduler.resume(t);


        if(preemptive) {
            reschedule(t->_link.rank().queue());

        }
    }
}

void Thread::wakeup_all(Queue *q)
{
    db<Thread>(TRC) << "Thread::wakeup_all(running=" << running() << ",q=" << q << ")" << endl;

    assert(locked()); // locking handled by caller

    if(!q->empty()) {
        assert(Criterion::QUEUES <= sizeof(unsigned long) * 8);
        unsigned long cpus = 0;
        while(!q->empty()) {
            Thread * t = q->remove()->object();
            t->_state = READY;
            t->_waiting = 0;
            _scheduler.resume(t);
            cpus |= 1 << t->_link.rank().queue();
        }

        // P3 - Alteração 
        if(preemptive) {
            for(unsigned long i = 0; i < Criterion::QUEUES; i++)
                if(cpus & (1 << i))
                    reschedule(i);
        }
    }
}

void Thread::reschedule()
{
    if (!Criterion::timed || Traits<Thread>::hysterically_debugged)
        db<Thread>(TRC) << "Thread::reschedule()" << endl;

    assert(locked()); // locking handled by caller

    Thread *prev = running();
    //db<Thread>(WRN) << "Thread PREV: " << prev->link() << endl;
    // Atualiza current_queue para proxima fila (next tem que ser thread da proxima fila)
    db<PEAMQ>(WRN) << "RESCHEDULE prev: " << prev << endl;
    prev->criterion().handle(EAMQ::CHANGE_QUEUE);
    Thread *next = _scheduler.choose();
    db<PEAMQ>(WRN) << "!!Thread NEXT: " << next << endl;    
    dispatch(prev, next);
}


void Thread::reschedule(unsigned int cpu)
{
    assert(locked()); // locking handled by caller

    if(!smp || (cpu == CPU::id()))
        reschedule();
    else {
        db<Thread>(TRC) << "Thread::reschedule(cpu=" << cpu << ")" << endl;
        IC::ipi(cpu, IC::INT_RESCHEDULER);
    }
}


void Thread::rescheduler(IC::Interrupt_Id i)
{
    lock();
    reschedule();
    unlock();
}


void Thread::time_slicer(IC::Interrupt_Id i)
{
    lock();
    reschedule();
    unlock();
}

void Thread::dispatch(Thread *prev, Thread *next, bool charge)
{
    // "next" is not in the scheduler's queue anymore. It's already "chosen"
    if (charge && Criterion::timed)
        _timer->restart();

    if (prev != next)
    {
        if (Criterion::dynamic)
        {
            prev->criterion().handle(Criterion::CHARGE | Criterion::LEAVE);
            for_all_threads(Criterion::UPDATE);
            next->criterion().handle(Criterion::AWARD | Criterion::ENTER);
            // P7 : se migração é ativo e condição para migrar for satisfeito
            // if (Criterion::migration && next->criterion().condition_migrate()) {
            //     // não sei se é melhor alterar evaluate para receber um parametro para não escolher mesmo core...
            //     next->criterion().queue(Criterion::evaluate()); // avalia qual core melhor e troca
            //     next->criterion().rank_eamq();                  // atribui novo rank no novo core
            //     next = _scheduler.choose_another();             // escolhe novo proximo
            // }
            // OU
            // if (Criterion::migration) { // && condicao por certo periodo de tempo? (tipo 10ms ou 100ms?)
            //     for_all_threads(Criterion::MIGRATE); // não sei se é boa fazer de todos threads
            //     // fazer analise de migração em todos thread
            // }
            // Também podemos colocar no idle() em vez de dipatch() -> apenas quando o sistema detecta que um núcleo está ocioso ou subutilizado.
        }

        if (prev->_state == RUNNING) {
            db<PEAMQ>(WRN) << "DEIXA READY" << endl;
            prev->_state = READY;}
        next->_state = RUNNING;

        if (Traits<Thread>::debugged && Traits<Debug>::info)
        {
            CPU::Context tmp;
            tmp.save();
            db<Thread>(INF) << "Thread::dispatch:prev={" << prev << ",ctx=" << tmp << "}" << endl;
        }
        db<Thread>(INF) << "Thread::dispatch:next={" << next << ",ctx=" << *next->_context << "}" << endl;
        if(smp)
            _lock.release();

        // The non-volatile pointer to volatile pointer to a non-volatile context is correct
        // and necessary because of context switches, but here, we are locked() and
        // passing the volatile to switch_constext forces it to push prev onto the stack,
        // disrupting the context (it doesn't make a difference for Intel, which already saves
        // parameters on the stack anyway).
        CPU::switch_context(const_cast<Context **>(&prev->_context), next->_context);

        if(smp)
            _lock.acquire();

    }
}

int Thread::idle()
{

    while(_thread_count > CPU::cores()) { // someone else besides idles

        if(Traits<Thread>::trace_idle)
            db<Thread>(TRC) << "Thread::idle(cpu=" << CPU::id() << ",this=" << running() << ")" << endl;

        CPU::int_enable();
        CPU::halt();

        if(_scheduler.schedulables() > 0) // a thread might have been woken up by another CPU
        {
            yield();
        }
    }

    if(CPU::id() == CPU::BSP) {
        kout << "\n\n*** The last thread under control of EPOS has finished." << endl;
        kout << "*** EPOS is shutting down!" << endl;
    }

    CPU::smp_barrier();
    Machine::reboot();

    return 0;
}

__END_SYS
