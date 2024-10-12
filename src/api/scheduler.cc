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


EAMQ::EAMQ(Microsecond p, Microsecond d, Microsecond c): RT_Common(rank_eamq(p, (d ? d : p), c), p, d, c) {}

void EAMQ::handle(Event event) {
    if (event & CREATE) {
        // NAO checar os anteriores, pois nao foi rankeado direito ainda (thread.cc, linha 45)
    }
    if (event & UPDATE) {
        // roda TODA vez que uma prempcao ocorre
        // TODO: fazer evento proprio de insercao entre threads
    }
    // if (periodic() && ((event & LEAVE) || (event & JOB_FINISH))) {
    //     // Se terminou tarefa guarda diferença de tempo atual - tempo iniciado 
    //     // Se tarefa foi preemptado, guarda o pedaço do tempo no tempo de execução
    //     _personal_statistics.job_execution_time += elapsed() - _personal_statistics.job_enter_time;
    //     if (event & JOB_FINISH) {
    //         // ATENCAO!! Não sei se esse if funciona, se esse valor é 0 mesmo no inicio 
    //         if (_personal_statistics.prev_execution_time == 0) {_personal_statistics.prev_execution_time = _personal_statistics.job_execution_time;}
    //         // (tempo de execução anterior + tempo de execução atual) / 2
    //         _personal_statistics.average_et = (_personal_statistics.prev_execution_time + _personal_statistics.job_execution_time) / 2;
    //         _personal_statistics.prev_execution_time = _personal_statistics.job_execution_time;
    //         _personal_statistics.job_execution_time = 0;
    //         // Fazer calculo relativo para cada frequência e guardar no _personal_statitics.job_estimated[q]
    //     }
    // }
    if (periodic() && (event & LEAVE)) {
        Tick this_quantum = elapsed() - _personal_statistics.job_enter_time;
        _personal_statistics.job_execution_time += this_quantum;
        for (unsigned int q = 0; q < QUEUES; q++)
        {
            // nojo
            // TODO: fazer operator -= em Microsecond
            _personal_statistics.remaining_et[q] =
                Time_Base(_personal_statistics.remaining_et[q])
                - Time_Base(Timer_Common::time(this_quantum, frequency_within(q)));
        }
    }
    if (periodic() && (event & JOB_FINISH)) {
        // ATENCAO!! Não sei se esse if funciona, se esse valor é 0 mesmo no inicio 
        // non, pois vai ser lixo
        // if (_personal_statistics.prev_execution_time == 0) {
        //     _personal_statistics.prev_execution_time = _personal_statistics.job_execution_time;
        // }

        // (tempo de execução anterior + tempo de execução atual) / 2
        _personal_statistics.average_et = (_personal_statistics.average_et + _personal_statistics.job_execution_time) / 2;
        _personal_statistics.job_execution_time = 0;

        for (unsigned int q = 0; q < QUEUES; q++)
        {
            _personal_statistics.job_estimated_et[q] = Timer_Common::time(_personal_statistics.average_et, frequency_within(q));
        }
    }
    // if ((periodic() && ((event & JOB_RELEASE) || (event & ENTER)))) {
    //     // Guarda tempo que inicia tarefa (no inicio da execução e depois da preempção)
    //     _personal_statistics.job_enter_time = elapsed();
    // }
    if (periodic() && (event & ENTER)) {
        _personal_statistics.job_enter_time = elapsed();
    }
    if (periodic() && (event & JOB_RELEASE)) {
        for (unsigned int q = 0; q < QUEUES; q++)
        {
            _personal_statistics.remaining_et[q] = _personal_statistics.job_estimated_et[q];
        }
    }

    /* a = new Job()        -> JOB_RELEASE, CREATE
    * [b] premptado por [a] -> ENTER (a), LEAVE (b)
    * a acabou tarefa :(    -> JOB_FINISH
    * a tem nova tarefa >:) -> JOB_RELEASE 
    */
}

int EAMQ::rank_eamq(Microsecond p, Microsecond d, Microsecond c) {

    // nota: atribuir c ao remaining_capacity se provido, se n usar estipulacao
    // mas n aq

    // Caso de deadline = 0 e periodo = 0 -> atribuir rank para deixar no ultimo da fila
    // Se tarefa é periodica com deadline = 0 -> podemos usar periodo como deadline (?)

    //Optimal_Case optimal_case = {-1, -1};

    //for(unsigned int i = 0; i < QUEUES; i++) {
    for(unsigned int i = QUEUES - 1; i >= 0; i--) {
        Thread * t_fitted = nullptr;


        // tempo de execução restante estimado
        int eet_remaining = _personal_statistics.remaining_et[i];

        // tempo de execução relativo a frequência 
        // int eet_profile = calculate_profile_eet(i);
        // calcula round profile waiting time 
        int rp_waiting_time = estimate_rp_waiting_time(eet_remaining, i);

        // Não avaliamos a possibilidade de inserir threads na frente de outras recém inseridas para evitarmos um possível loop infinito
        for(auto it = Thread::scheduler()->end(i); it != Thread::scheduler()->begin(i) && !it->object()->criterion().is_recent_insertion() ; it = it->prev()) {
            Thread * thread_in_queue = it->object();
            // Thread da frente -> Tf
            // Thread que será inserido -> Ti
            int thread_capacity_remaining = thread_in_queue->criterion().personal_statistics().remaining_et[i];
            int total_time_execution = thread_in_queue->priority()      //tempo de espera da (Tf)
                    + static_cast<int>(thread_capacity_remaining*1.15)  //tempo de execução da (Tf)
                    + eet_remaining                                     //tempo de execução (Ti)
                    + rp_waiting_time;                                  //tempo de espera por RP (Ti)

            if (total_time_execution < d) {
                t_fitted = thread_in_queue;
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
            t_fitted_capacity_remaining = t_fitted->criterion()->personal_statistics().remaining_et[i];
        }

        int cwt_profile = rp_waiting_time + (t_fitted ? static_cast<int>(t_fitted->priority() + t_fitted_capacity_remaining) : 0);
        int available_time_to_run = d - cwt_profile;
        int idle_time = available_time_to_run - eet_remaining;

        if (idle_time >= 0) {
            set_queue(i);
            return cwt_profile;
            // optimal_case = {i, cwt_profile};
            // return optimal_case;
        }
    }
    // Não encontrou lugar na fila
    // return optimal_case;
    return -1;
}

__END_SYS
