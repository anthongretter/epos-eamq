
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


volatile unsigned EAMQ::_current_queue = QUEUES - 1;

// Construtor para threads aperiódicas
EAMQ::EAMQ(int p) : RT_Common(p)
{
    // Aperiodic Threads (LOW priority) are thrown into the lowest frequency
    // and Threads with NORMAL into the penultimate one
    // Coloca thread MAIN e IDLE na mesma fila (fila com menor frequência possível)
    if (p == MAIN || p == IDLE || p >= LOW) {
        _queue = QUEUES - 1;
    } else {
        // Se a prioridade é NORMAL ou HIGH
        _queue = QUEUES - 2;
    }
    // _queue = 3;
    
    // _queue = QUEUES - 1;
}

// -1 passado para RT_Common pois logo em seguida ele é atualizado
EAMQ::EAMQ(Microsecond p, Microsecond d, Microsecond c) : RT_Common(-1, p, d, c)
{
    d = (d ? d : p);

    if (c != UNKNOWN) {
        _personal_statistics.average_et = c;
    } else {
        _personal_statistics.average_et = time(d) / 3;   // initial ET estimation (1/3 of deadline)
    }

    _personal_statistics.remaining_deadline = d;

    int success = rank_eamq();
    if (!success) {
        // something?
    }
}

void EAMQ::handle(Event event) {
    // Se thread foi preemptado / terminou
    if (event & ENTER) {
        // db<EAMQ>(TRC) << "entrando: " << _queue << endl;
    }
    if (event & LEAVE) {
        // db<EAMQ>(TRC) << "saindo: " << _queue << endl;
    }
    if (event & CHANGE_QUEUE) {
        unsigned int last = _current_queue;

        do {
            // Pula para próxima fila
            EAMQ::next_queue();
            // Enquanto fila atual não vazia
        } while (Thread::scheduler()->empty(_current_queue) && _current_queue != last);

        // Ajustando a frequência conforme a fila
        CPU::clock(frequency_within(_current_queue));
    }
    if (event & CREATE) {
    }
    if (event & UPDATE) {
    }
    // Quando acontece troca de contexto
    if (periodic() && (event & UPDATE)) {
        if (Q > Time_Base(_personal_statistics.remaining_deadline)) {
            // underflow
            _personal_statistics.remaining_deadline = Microsecond(0);
            // somehow discard this thread
        } else {
            // Decrementa do deadline o quantum
            _personal_statistics.remaining_deadline -= Microsecond(Q);
        }
    }
    if (periodic() && (event & CREATE)) {
        // Se foi inserido no meio da fila (ou seja, se tem t_fitted)
        if (_inserted_in_front_of) {
            // Faz atualização de rank da thread que foi inserida chamando assure_behind
            _inserted_in_front_of->link()->prev()->object()->for_all_behind(ASSURE_BEHIND);
        }
    }
    if (periodic() && (event & LEAVE)) {
        // Guarda o tempo que passou depois que começou a execução da tarefa
        Tick in_cpu = elapsed() - _personal_statistics.job_enter_time;
        _personal_statistics.job_execution_time += in_cpu;
        for (unsigned int q = 0; q < QUEUES; q++)
        {
            // Reduz o tempo executado deste quantum, transformando Tick em Microsecond
            Microsecond executed_in_profile = Timer_Common::time(in_cpu, frequency_within(q));
            if (executed_in_profile > _personal_statistics.remaining_et[q]) {
                // underflow
                _personal_statistics.remaining_et[q] = 0;
            } else {
                _personal_statistics.remaining_et[q] -= executed_in_profile;
            }
        }
    }
    // Quando uma thread periodica começa a tarefa
    if (periodic() && (event & ENTER)) {
        _personal_statistics.job_enter_time = elapsed();
    }
    // Quando uma thread foi liberado para executar tarefa
    if (periodic() && (event & JOB_RELEASE)) {
        for (unsigned int q = 0; q < QUEUES; q++)
        {
            // Atualiza tempo de execução restante para EET
            _personal_statistics.remaining_et[q] = _personal_statistics.job_estimated_et[q];
        }
    }
    // Quando uma thread periodica termina tarefa
    if (periodic() && (event & JOB_FINISH)) {
        // (tempo de execução anterior + tempo de execução atual) / 2
        _personal_statistics.average_et = (_personal_statistics.average_et + _personal_statistics.job_execution_time) / 2;
        _personal_statistics.job_execution_time = 0;

        for (unsigned int q = 0; q < QUEUES; q++)
        {
            // Atualiza EET da tarefa para cada fila (relativo a frequência)
            _personal_statistics.job_estimated_et[q] = Timer_Common::time(_personal_statistics.average_et, frequency_within(q));
        }
    }
    if (periodic() && (event & ASSURE_BEHIND)) {

    }

    // // Se a thread MAIN for esperar em join, então ela chama um dispatch se escalonada, devemos garantir que não é a IDLE que recebe o dispatch
    // // Se houver apenas duas threads na fila (MAIN e IDLE), então devemos passar para a próxima fila (IDLE sempre esta na ultima posicao)
    // if (event & MAIN_JOIN)
    // {
    //     unsigned int last = _current_queue;

    //     do {
    //         // Pula para próxima fila
    //         EAMQ::next_queue();
    //         // Enquanto fila atual não vazia
    //     } while (Thread::scheduler()->empty(_current_queue) && _current_queue != last);

    //     // alterar a frequência do processador igual no CHANGE_QUEUE
    // }

    /* a = new Job()        -> JOB_RELEASE, CREATE
     * [b] premptado por [a] -> ENTER (a), LEAVE (b)
     * a acabou tarefa :(    -> JOB_FINISH
     * a tem nova tarefa >:) -> JOB_RELEASE
     */
}

int EAMQ::rank_eamq() {

    for (unsigned int i = QUEUES - 1; i >= 0; i--) {
        Thread * t_fitted = nullptr;

        // tempo de execução restante estimado
        int eet_remaining = _personal_statistics.remaining_et[i];

        // calcula round profile waiting time
        int rp_waiting_time = estimate_rp_waiting_time(eet_remaining, i);

        // Não avaliamos a possibilidade de inserir threads na frente de outras recém inseridas para evitarmos um possível loop infinito
        for (auto it = Thread::scheduler()->end(i); it != Thread::scheduler()->begin(i) && !it->object()->criterion().is_recent_insertion() ; it = it->prev()) {
            Thread * thread_in_queue = it->object();

            // As ultimas threads da fila tendem a ser aperiodicas, então nós não queremos recalcular o rank delas
            if (!thread_in_queue->criterion().periodic()) continue;

            // Thread da frente -> Tf
            // Thread que será inserido -> Ti
            int thread_capacity_remaining = thread_in_queue->criterion().personal_statistics().remaining_et[i];
            int total_time_execution = thread_in_queue->priority()               // tempo de espera da (Tf)
                                       + (thread_capacity_remaining * 115 / 100) // tempo de execução da (Tf)
                                       + eet_remaining                           // tempo de execução (Ti)
                                       + rp_waiting_time;                        // tempo de espera por RP (Ti)

            if (total_time_execution < int(Time_Base(_personal_statistics.remaining_deadline))) {
                t_fitted = thread_in_queue;
                // vai inserir na frente de alguem, entao salvar onde
                // optimal_case.jumped = t_fitted;
                _inserted_in_front_of = t_fitted;
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
            // optimal_case.queue = i;
            // optimal_case.priority = cwt_profile;
            set_queue(i);
            _priority = cwt_profile;
            // return cwt_profile;
            db<EAMQ>(TRC) << "Thread inserted in queue " << i << " with priority " << cwt_profile << endl;
            return 1;
        }
    }
    // Não encontrou lugar na fila
    // return optimal_case;
    db<EAMQ>(TRC) << "Thread not inserted in any queue" << endl;
    return 0;
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

/*
Thread(entry=0x800000c9,state=1,priority=536870911,queue=3,stack={b=0xffc33c50,s
=16384},context={b=0xffc37c20,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
000000,sp=0xffc3fc34,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
,cr3=0x3fffc000}}) => 0x80800e84
Task::enroll(t=0, o=0x80800e84)
0 - 0
1 - 0
2 - 0
3 - 2
Thread(entry=0x800000c9,state=1,priority=536870911,queue=3,stack={b=0xffc2fc30,s
=16384},context={b=0xffc33c00,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
000000,sp=0xffc3fc34,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
,cr3=0x3fffc000}}) => 0x80800d00
Task::enroll(t=0, o=0x80800d00)
0 - 0
1 - 0
2 - 0
3 - 3
Thread::join(this=0x80800e84,state=1)
0 - 0
1 - 0
2 - 0
3 - 2
saindo: 3
entrando: 3
Thread::dispatch(prev=0xffc3fe1c,next=0x80800e84)
bruh
Thread::exit(status=0) [running=0x80800e84]
0 - 0
1 - 0
2 - 0
3 - 2
saindo: 3
entrando: 3
Thread::dispatch(prev=0x80800e84,next=0xffc3fe1c)
Thread::join(this=0x80800d00,state=1)
0 - 0
1 - 0
2 - 0
3 - 1
saindo: 3
entrando: 3
Thread::dispatch(prev=0xffc3fe1c,next=0x80800d00)
bruh
Thread::exit(status=0) [running=0x80800d00]
0 - 0
1 - 0
2 - 0
3 - 1
saindo: 3
entrando: 3
Thread::dispatch(prev=0x80800d00,next=0xffc3fe1c)
Thread::exit(status=0) [running=0xffc3fe1c]
0 - 0
1 - 0
2 - 0
3 - 0
saindo: 3
entrando: 3
Thread::dispatch(prev=0xffc3fe1c,next=0xffc3bc78)
Thread::idle(this=0xffc3bc78)
*/

/*
Task(entry=0x80000038) => 0xffc3ffa0
Thread(entry=0x80000038,state=0,priority=-1,queue=3,stack={b=0xffc3be14,s=16384}
,context={b=0xffc3fde4,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00000000,
sp=0xffb03c28,ip=0x80000038,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10,cr3=0x
3fffc000}}) => 0xffc3fe1c
Task::enroll(t=0, o=0xffc3fe1c)

Thread(entry=0x800049bc,state=1,priority=2147483647,queue=3,stack={b=0xffc37c70,
s=16384},context={b=0xffc3bc40,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x0
0000000,sp=0xffb03c28,ip=0x800049bc,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=1
0,cr3=0x3fffc000}}) => 0xffc3bc78
Init_Application()
Heap(addr=0x80401000,bytes=4194304) => 0x80400040

Init_End()
Hello world!

Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc33c50,s
=16384},context={b=0xffc37c20,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
,cr3=0x3fffc000}}) => 0x80800e84
Task::enroll(t=0, o=0x80800e84)

Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc2fc30,s
=16384},context={b=0xffc33c00,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
,cr3=0x3fffc000}}) => 0x80800d00
Task::enroll(t=0, o=0x80800d00)

Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc2bc10,s
=16384},context={b=0xffc2fbe0,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
,cr3=0x3fffc000}}) => 0x80800b7c
Task::enroll(t=0, o=0x80800b7c)

Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc27bf0,s
=16384},context={b=0xffc2bbc0,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
,cr3=0x3fffc000}}) => 0x808009f8
Task::enroll(t=0, o=0x808009f8)

Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc23bd0,s
=16384},context={b=0xffc27ba0,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
000000,sp=0xffc3fc24,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
,cr3=0x3fffc000}}) => 0x80800874
Task::enroll(t=0, o=0x80800874)

Thread::join(this=0x80800e84,state=1)
Thread::dispatch(prev=0xffc3fe1c,next=0xffc3bc78)
Thread::idle(this=0x80800e84)
Thread::dispatch(prev=0x80800e84,next=0x80800d00)
bruh
Thread::exit(status=0) [running=0x80800d00]
Thread::dispatch(prev=0x80800d00,next=0x808009f8)
bruh
Thread::exit(status=0) [running=0x808009f8]
Thread::dispatch(prev=0x808009f8,next=0x80800e84)
Thread::dispatch(prev=0x80800e84,next=0x80800b7c)
bruh
Thread::exit(status=0) [running=0x80800b7c]
Thread::dispatch(prev=0x80800b7c,next=0x80800e84)
Thread::dispatch(prev=0x80800e84,next=0x80800874)
bruh
Thread::exit(status=0) [running=0x80800874]
Thread::dispatch(prev=0x80800874,next=0x80800e84)

 */

 /*
Task(entry=0x80000038) => 0xffc3ffa0
Thread(entry=0x80000038,state=0,priority=-1,queue=3,stack={b=0xffc3be14,s=16384}
,context={b=0xffc3fde4,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00000000,
sp=0xffb03c28,ip=0x80000038,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10,cr3=0x
3fffc000}}) => 0xffc3fe1c
Task::enroll(t=0, o=0xffc3fe1c)

Thread(entry=0x8000494a,state=1,priority=2147483647,queue=3,stack={b=0xffc37c70,
s=16384},context={b=0xffc3bc40,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x0
0000000,sp=0xffb03c28,ip=0x8000494a,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=1
0,cr3=0x3fffc000}}) => 0xffc3bc78

Init_Application()
Heap(addr=0x80401000,bytes=4194304) => 0x80400040
Init_End()
Hello world!

Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc33c50,s
=16384},context={b=0xffc37c20,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
000000,sp=0xffc3fc34,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
,cr3=0x3fffc000}}) => 0x80800e84
Task::enroll(t=0, o=0x80800e84)

Thread(entry=0x800000c9,state=1,priority=536870911,queue=2,stack={b=0xffc2fc30,s
=16384},context={b=0xffc33c00,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
000000,sp=0xffc3fc34,ip=0x800000c9,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
,cr3=0x3fffc000}}) => 0x80800d00
Task::enroll(t=0, o=0x80800d00)

Thread::dispatch(prev=0xffc3fe1c,next=0x80800d00)   MAIN -> BRUH2
bruh

Thread::exit(status=0) [running=0x80800d00]         BRUH2 FINALIZADO
Thread::dispatch(prev=0x80800d00,next=0xffc3fe1c)   BRUH2 -> MAIN
Thread::join(this=0x80800e84,state=1)               Main faz join BRUH1
Thread::dispatch(prev=0xffc3fe1c,next=0xffc3bc78)   Main -> IDLE
Thread::idle(this=0xffc3bc78)                       Idle
make[2]: Leaving directory '/app/img'
make[1]: Leaving directory '/app'
  */