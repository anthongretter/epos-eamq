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
        // tempo atual + deadline = ponto real de deadline
        // capacidade (restante do job executar) + o que ja foi executado = total a executar
        // ponto real de deadline - total a executar = slack
    }
    RT_Common::handle(event);

    // Update the priority of the thread at job releases, before _alarm->v(), so it enters the queue in the right order (called from Periodic_Thread::Xxx_Handler)
//    if((_priority >= PERIODIC) && (_priority < APERIODIC) && ((event & JOB_FINISH) || (event & UPDATE_ALL)))
}

// Since the definition of FCFS above is only known to this unit, forcing its instantiation here so it gets emitted in scheduler.o for subsequent linking with other units is necessary.
template FCFS::FCFS<>(int p);


EAMQ::EAMQ(Microsecond p, Microsecond d, Microsecond c): RT_Common(int(elapsed() + ticks((d ? d : p) - c)), p, d, c) {}

void EAMQ::handle(Scheduling_Criterion_Common::Event event) {
    if(periodic() && ((event & UPDATE) | (event & JOB_RELEASE) | (event & JOB_FINISH))) {
        // update rank
    }
    RT_Common::handle(event);
}

// EAMQ::EAMQ(Microsecond p, Microsecond d, Microsecond c): RT_Common(int(rank_eamq(p, d, c)), p, d, c) {}

// int EAMQ::occupied_queues(int f)
//         {
//             int oc = 0;
//             for (unsigned int i = 0; i < QUEUES; i++)
//             {
//                 oc += int(!_multilist[i]->empty());
//             }
//             return oc == 0 ? oc : (_multilist[f]->empty() ? oc : oc - 1);
//         }

// int EAMQ::rank_eamq(Microsecond p, Microsecond d, Microsecond c) {
//     int queue_chosen = NULL;
//     int new_rank = NULL;
//     for(unsigned int i = 0; i < QUEUES; i++) {
//         Thread * t_fitted = nullptr;
//         // pega EET = tempo estimado de execucao (capacity??)
//         //int eet = get_eet(i);
//         const int rp_rounds = (static_cast<float>(eet) / Q) == static_cast<int>((eet / Q)) ? (static_cast<int>((eet / Q)) - 1) : static_cast<int>((eet / Q) );
        
//         int rp_waiting_time = Q * (occupied_queues(i)) * (rp_rounds);
//         for(auto it = _multilist[i].tail(); it != _multilist[i].head(); it = it->prev()) {
//             Thread * thread_in_queue = it->object();
//             if (thread_in_queue->pe->rank() + static_cast<int>(thread_in_queue->eet_remaining[f]*1.15) + this->eet_remaining[f]+ rp_waiting_time < d) {
//                 t_fitted = it;
//                 break;
//             }
//         }
//         // precisa ver como vai ser dados de cada threads (deadline nao tem como acessar -> protegido)
//         // se vai criar atributos novos para threads 
//         int cwt_profile = rp_waiting_time + (t_fitted ? static_cast<int>(t_fitted->rank() + t_fitted->wcet_remaining[f]) : 0);
//         int available_time_to_run = d - cwt_profile;
//         int idle_time = available_time_to_run - eet;
//         if (idle_time >= 0) {
//             queue_chosen = f;
//             new_rank = cwt_profile;
//         }
//     }
//     // algum jeito de escolher a fila para inserir
//     //current_queue();
//     return new_rank;
// }

__END_SYS
