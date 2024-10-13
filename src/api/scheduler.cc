// EPOS CPU Scheduler Component Implementation

#include <process.h>
#include <time.h>
#include "scheduler.h"


__BEGIN_SYS

inline RT_Common::Tick RT_Common::elapsed() { return Alarm::elapsed(); }

RT_Common::Tick RT_Common::ticks(Microsecond time) {
    return Timer_Common::ticks(time, Alarm::timer()->frequency());
}

Microsecond RT_Common::time(Tick ticks) {
    return Timer_Common::time(ticks, Alarm::timer()->frequency());
}

void RT_Common::handle(Event event) {
    db<Thread>(TRC) << "RT::handle(this=" << this << ",e=";
    if(event & CREATE) {
        db<Thread>(TRC) << "CREATE";

        _statistics.thread_creation = elapsed();
        _statistics.job_released = false;
    }
    if(event & FINISH) {
        db<Thread>(TRC) << "FINISH";

        _statistics.thread_destruction = elapsed();
    }
    if(event & ENTER) {
        db<Thread>(TRC) << "ENTER";

        _statistics.thread_last_dispatch = elapsed();
    }
    if(event & LEAVE) {
        Tick cpu_time = elapsed() - _statistics.thread_last_dispatch;

        db<Thread>(TRC) << "LEAVE";

        _statistics.thread_last_preemption = elapsed();
        _statistics.thread_execution_time += cpu_time;
//        if(_statistics.job_released) {
            _statistics.job_utilization += cpu_time;
//        }
    }
    if(periodic() && (event & JOB_RELEASE)) {
        db<Thread>(TRC) << "RELEASE";

        _statistics.job_released = true;
        _statistics.job_release = elapsed();
        _statistics.job_start = 0;
        _statistics.job_utilization = 0;
        _statistics.jobs_released++;
    }
    if(periodic() && (event & JOB_FINISH)) {
        db<Thread>(TRC) << "WAIT";

        _statistics.job_released = false;
        _statistics.job_finish = elapsed();
        _statistics.jobs_finished++;
//        _statistics.job_utilization += elapsed() - _statistics.thread_last_dispatch;
    }
    if(event & COLLECT) {
        db<Thread>(TRC) << "|COLLECT";
    }
    if(periodic() && (event & CHARGE)) {
        db<Thread>(TRC) << "|CHARGE";
    }
    if(periodic() && (event & AWARD)) {
        db<Thread>(TRC) << "|AWARD";
    }
    if(periodic() && (event & UPDATE)) {
        db<Thread>(TRC) << "|UPDATE";
    }
    db<Thread>(TRC) << ") => {i=" << _priority << ",p=" << _period << ",d=" << _deadline << ",c=" << _capacity << "}" << endl;
}


template <typename ... Tn>
FCFS::FCFS(int p, Tn & ... an): Priority((p == IDLE) ? IDLE : RT_Common::elapsed()) {}


EDF::EDF(Microsecond p, Microsecond d, Microsecond c): RT_Common(int(elapsed() + ticks(d)), p, d, c) {}

void EDF::handle(Event event) {
    RT_Common::handle(event);

    // Update the priority of the thread at job releases, before _alarm->v(), so it enters the queue in the right order (called from Periodic_Thread::Xxx_Handler)
    if(periodic() && (event & JOB_RELEASE))
        _priority = elapsed() + _deadline;
}


LLF::LLF(Microsecond p, Microsecond d, Microsecond c): RT_Common(int(elapsed() + ticks((d ? d : p) - c)), p, d, c) {}

void LLF::handle(Event event) {
    if(periodic() && ((event & UPDATE) | (event & JOB_RELEASE) | (event & JOB_FINISH))) {
        _priority = elapsed() + _deadline - _capacity + _statistics.job_utilization;
    }
    RT_Common::handle(event);

    // Update the priority of the thread at job releases, before _alarm->v(), so it enters the queue in the right order (called from Periodic_Thread::Xxx_Handler)
//    if((_priority >= PERIODIC) && (_priority < APERIODIC) && ((event & JOB_FINISH) || (event & UPDATE_ALL)))
}

// Since the definition of FCFS above is only known to this unit, forcing its instantiation here so it gets emitted in scheduler.o for subsequent linking with other units is necessary.
template FCFS::FCFS<>(int p);


volatile unsigned EAMQ::_current_queue = 0;

EAMQ::EAMQ(Microsecond p, Microsecond d, Microsecond c): RT_Common(-1, p, d, c) {
    d = (d ? d : p);
    
    if (c != UNKNOWN) {
        _personal_statistics.average_et = c;
    } else {
        _personal_statistics.average_et = time(d) / 3;   // initial ET estimation (1/3 of deadline)
    }

    _personal_statistics.remaining_deadline = d;

    Optimal_Case op = rank_eamq();
    _last_insert = op;
    _queue = op.queue;
    _priority = op.priority;
}

void EAMQ::handle(Event event) {
    if (event & LEAVE) {
        EAMQ::next_queue();
        CPU::clock(frequency_within(_current_queue));
    }
    if (event & CREATE) {  // after insert(Thread)

    }
    if (event & UPDATE) {
        // roda TODA vez que uma prempcao ocorre
        // TODO: fazer evento proprio de insercao entre threads
    }
    if (periodic() && (event & UPDATE)) {
        _personal_statistics.remaining_deadline -= Microsecond(Q);
    }
    if (periodic() && (event & CREATE)) {
        if (_last_insert.jumped) {
            _last_insert.jumped->link()->prev()->object()->for_all_behind(ASSURE_BEHIND);
        }
    }
    if (periodic() && (event & LEAVE)) {
        Tick in_cpu = elapsed() - _personal_statistics.job_enter_time;
        _personal_statistics.job_execution_time += in_cpu;
        for (unsigned int q = 0; q < QUEUES; q++)
        {
            // Reduz tempo executado deste quantum
            _personal_statistics.remaining_et[q] -= Timer_Common::time(in_cpu, frequency_within(q));
        }
    }
    if (periodic() && (event & ENTER)) {
        _personal_statistics.job_enter_time = elapsed();
    }
    if (periodic() && (event & JOB_RELEASE)) {
        for (unsigned int q = 0; q < QUEUES; q++)
        {
            _personal_statistics.remaining_et[q] = _personal_statistics.job_estimated_et[q];
        }
    }
    if (periodic() && (event & JOB_FINISH)) {
        // (tempo de execução anterior + tempo de execução atual) / 2
        _personal_statistics.average_et = (_personal_statistics.average_et + _personal_statistics.job_execution_time) / 2;
        _personal_statistics.job_execution_time = 0;

        for (unsigned int q = 0; q < QUEUES; q++)
        {
            _personal_statistics.job_estimated_et[q] = Timer_Common::time(_personal_statistics.average_et, frequency_within(q));
        }
    }
    if (periodic() && (event & ASSURE_BEHIND)) {

    }

    /* a = new Job()        -> JOB_RELEASE, CREATE
    * [b] premptado por [a] -> ENTER (a), LEAVE (b)
    * a acabou tarefa :(    -> JOB_FINISH
    * a tem nova tarefa >:) -> JOB_RELEASE 
    */
}

EAMQ::Optimal_Case EAMQ::rank_eamq() {

    // nota: atribuir c ao remaining_capacity se provido, se n usar estipulacao
    // mas n aq

    // Caso de deadline = 0 e periodo = 0 -> atribuir rank para deixar no ultimo da fila
    // Se tarefa é periodica com deadline = 0 -> podemos usar periodo como deadline (?)

    Optimal_Case optimal_case;

    //for(unsigned int i = 0; i < QUEUES; i++) {
    for (unsigned int i = QUEUES - 1; i >= 0; i--) {
        Thread * t_fitted = nullptr;

        // tempo de execução restante estimado
        int eet_remaining = _personal_statistics.remaining_et[i];

        // calcula round profile waiting time 
        int rp_waiting_time = estimate_rp_waiting_time(eet_remaining, i);

        // Não avaliamos a possibilidade de inserir threads na frente de outras recém inseridas para evitarmos um possível loop infinito
        for (auto it = Thread::scheduler()->end(i); it != Thread::scheduler()->begin(i) && !it->object()->criterion().is_recent_insertion() ; it = it->prev()) {
            Thread * thread_in_queue = it->object();
            // Thread da frente -> Tf
            // Thread que será inserido -> Ti
            int thread_capacity_remaining = thread_in_queue->criterion().personal_statistics().remaining_et[i];
            int total_time_execution = thread_in_queue->priority()          //tempo de espera da (Tf)
                    + (thread_capacity_remaining * 115 / 100)               //tempo de execução da (Tf)
                    + eet_remaining                                         //tempo de execução (Ti)
                    + rp_waiting_time;                                      //tempo de espera por RP (Ti)

            if (total_time_execution < int(Time_Base(_personal_statistics.remaining_deadline))) {
                t_fitted = thread_in_queue;
                // vai inserir na frente de alguem, entao salvar onde
                optimal_case.jumped = t_fitted;
                break;
            }
        }

        // Se não encontrou nenhuma thread que cabe na fila (não está vazia) avalie a próxima
        if (!t_fitted) {
            continue;
        }
        
        // Se a fila estiver vazia t_fitted = NULL
        int t_fitted_capacity_remaining = 0;

        // Se a fila não estiver vazia precisamos levar em consideração o tempo que a thread da frente esperará
        if (!Thread::scheduler()->empty(i)) {
            t_fitted_capacity_remaining = t_fitted->criterion().personal_statistics().remaining_et[i];
        }

        int cwt_profile = rp_waiting_time + (t_fitted ? t_fitted->priority() + t_fitted_capacity_remaining : 0);
        int available_time_to_run = _personal_statistics.remaining_deadline - cwt_profile;
        int idle_time = available_time_to_run - eet_remaining;

        if (idle_time >= 0) {
            optimal_case.queue = i;
            optimal_case.priority = cwt_profile;
            // set_queue(i);
            // return cwt_profile;
            // optimal_case = {i, cwt_profile};
            return optimal_case;
        }
    }
    // Não encontrou lugar na fila
    // return optimal_case;
    return optimal_case;    // TRATAR
}

int EAMQ::estimate_rp_waiting_time(unsigned int eet_profile, unsigned int looking_queue) {
    // const int rp_rounds = static_cast<int>((eet_profile / Q));
    // if ((static_cast<float>(eet_profile) / Q) == rp_rounds) {
    //     rp_rounds--;
    // }

    int rp_rounds = eet_profile/Q;

    // Se precisar de uma rodada extra com um tamanho menor que o Quantum
    if (eet_profile % Q) {
        rp_rounds++;
    }

    int oc = 0;
    for (unsigned int i = 0; i < QUEUES; i++)
    {
        if (i == looking_queue && !Thread::scheduler()->empty(i)) {
            continue;
        }
        oc++;
    }
        
    int rp_waiting_time = Q * (oc) * (rp_rounds);

    return rp_waiting_time;
}

__END_SYS
