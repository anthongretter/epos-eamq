
// EPOS CPU Scheduler Component Implementation

#include <process.h>
#include <time.h>
#include <utility/random.h>
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

//P3 - Alteração
volatile unsigned int Variable_Queue_Scheduler::_next_queue;

// The following Scheduling Criteria depend on Alarm, which is not available at scheduler.h
template <typename ... Tn>
FCFS::FCFS(int p, Tn & ... an): Priority((p == IDLE) ? IDLE : Alarm::elapsed()) {}
template FCFS::FCFS<>(int p);

/////////////////////////////// P2 - Single core /////////////////////////////// 
//volatile unsigned EAMQ::_current_queue = QUEUES - 1;
volatile unsigned int EAMQ::_current_queue[Traits<Machine>::CPUS] = {QUEUES - 1};
// volatile unsigned int GEAMQ::_current_queue[GEAMQ::HEADS] = {QUEUES - 1}; // apenas inicializa o core 0
// bool GEAMQ::initialized = false; // workaround para fazer uma lazy initialization no _current_queue
bool EAMQ::initialized = false;

// Construtor para threads aperiódicas
EAMQ::EAMQ(int p) : RT_Common(p), _is_recent_insertion(false), _personal_statistics{}, _behind_of(nullptr), _periodic(false)
{
    EAMQ::initialize_current_queue();
//    if (Traits<System>::RUN_TO_HALT) {
//        _queue_eamq = 0;
//        return;
//    }

    // Coloca thread MAIN e IDLE na mesma fila (fila com menor frequência possível)
    // prioridade igual a LOW ou mais baixos
    if (p == MAIN || p == IDLE || p >= LOW) {
        if (p == MAIN) {db<PEAMQ>(WRN) << "CRIOU MAIN" << endl;}
        if (p == IDLE) {db<PEAMQ>(WRN) << "CRIOU IDLE" << endl;}
        _queue_eamq = QUEUES - 1;
    } else {
        // Se a prioridade é NORMAL ou HIGH (p < LOW)
        _queue_eamq = QUEUES - 2;
    }
    
}

// PERIODIC passado para RT_Common pois logo em seguida ele é atualizado
EAMQ::EAMQ(Microsecond p, Microsecond d, Microsecond c) : RT_Common(PERIODIC, p, d, c), _is_recent_insertion(false), _personal_statistics{}, _behind_of(nullptr), _periodic(true)
{

    db<PEAMQ>(WRN) << "ranking with p: " << p << endl;
    d = (d ? d : p);
    _deadline = d;

    //int unsigned rand = 3u + (unsigned(Random::random()) % 8u);

    _personal_statistics.remaining_deadline = d;
    for (unsigned int q = 0; q < QUEUES; q++)
    {
        // initial ET estimation (1/3 of deadline)
        _personal_statistics.average_et[q] = Timer_Common::sim(c ? c : Microsecond(d / 10), CPU::max_clock(), frequency_within(q));
        _personal_statistics.job_estimated_et[q] = _personal_statistics.average_et[q];
        // Atualiza tempo de execução restante para EET
        _personal_statistics.remaining_et[q] = _personal_statistics.job_estimated_et[q];
    }

    rank_eamq();
    db<EAMQ>(TRC) << "ranked with: " << _priority << " on queue: " << _queue_eamq << endl;
}

void EAMQ::handle(Event event) {
    // Antes de toda troca de threads (choose / chosen) precisa-se avancar 
    // o ponteiro da fila de escolha 
    if (event & CHANGE_QUEUE) {
        unsigned int last = current_queue_eamq();
        do {
            EAMQ::next_queue();
            db<PEAMQ>(WRN) << "current_queue_eamq: " << current_queue_eamq() << endl;
        // Enquanto fila atual não vazia ou uma volta completa
        } while (Thread::scheduler()->empty() && (current_queue_eamq() != last));
        db<PEAMQ>(WRN) << "CPU " << CPU::id() << " prox: " << current_queue_eamq() << endl;

        // Ajustando a frequência conforme a fila
        Hertz f = frequency_within(current_queue_eamq());

        // P6: Nao ajustavel com o uso da PMU
        // CPU::clock(f);

        // So that IDLE doesnt spam this
        if (last != current_queue_eamq()) {
            db<EAMQ>(TRC) << "[!!!] Operating next queue, in frequency: " << f / 1000000 << "Mhz " << "Queue: " << current_queue_eamq() << endl;
        }
        db<PEAMQ>(WRN) << "HEAD: " << Thread::scheduler()->head()->object() << ", TAIL: " << Thread::scheduler()->tail()->object() << endl;
    }
    if (event & CREATE) {
        db<PEAMQ>(WRN) << "CRIANDO THREAD" << endl;
        // P6 : inicializa estatisticas do PMU
        reset_pmu_personal_stats();
    }
    if (event & UPDATE) {
        db<PEAMQ>(WRN) << "UPDATE" <<endl;
        // Depois da proxima ser definida e avisada de sua entrada, podemos desproteger as recem entradas
        // Todas as threads recebem um evento UPDATE
        _is_recent_insertion = false;
        _behind_of = nullptr;
    }
    // Quando acontece prempcao do quantum
    if (periodic() && (event & UPDATE)) {
        if (Q > Time_Base(_personal_statistics.remaining_deadline)) {
            // underflow
            _personal_statistics.remaining_deadline = Microsecond(0);
            // somehow discard this thread
        } else {
            // Decrementa do deadline o quantum executado
            _personal_statistics.remaining_deadline -= Microsecond(Q);
        }
    }
    if (periodic() && (event & CREATE)) {
        db<PEAMQ>(WRN) << "CRIANDO PERIODICO" <<endl;
        // Se foi inserido no meio da fila (ou seja, se tem t_fitted)
        if (_behind_of) {
            // Faz atualização de rank da thread que foi inserida chamando assure_behind
            _behind_of->link()->prev()->object()->for_all_behind(ASSURE_BEHIND);
        }
    }
    if (periodic() && (event & LEAVE)) {
        db<PEAMQ>(WRN) << "LEAVE PERIODICO" <<endl;

        // Guarda o tempo que passou depois que começou a execução da tarefa
        Microsecond in_cpu = time(PMU::read(0));

        // Coletando dados de PMU 
        _personal_statistics.instructions += PMU::read(2);
        _personal_statistics.branch_miss += PMU::read(3);
        _personal_statistics.branches += PMU::read(4);
        _personal_statistics.cache_hit += PMU::read(5);
        _personal_statistics.cache_miss += PMU::read(6);

        _personal_statistics.job_execution_time += in_cpu;

        for (unsigned int q = 0; q < QUEUES; q++)
        {
            // Reduz o tempo executado deste quantum, transformando Tick em Microsecond
            Microsecond executed_in_profile = Timer_Common::sim(in_cpu, frequency_within(_queue_eamq), frequency_within(q));
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
        db<PEAMQ>(WRN) << "ENTER PERIODICO" <<endl;
        PMU::reset(0);
        PMU::start(0);
    }
    // Quando uma thread foi liberado para executar tarefa
    if (periodic() && (event & JOB_RELEASE)) {
        db<PEAMQ>(WRN) << "RELEASE PERIODICO" <<endl;
        _personal_statistics.remaining_deadline = _deadline;
        _personal_statistics.job_execution_time = 0;
        rank_eamq();
    }
    // Quando uma thread periodica termina tarefa
    if (periodic() && (event & JOB_FINISH)) {
        db<PEAMQ>(WRN) << "FINISH PERIODICO" <<endl;
        for (auto it = Thread::scheduler()->begin(); it != Thread::scheduler()->end(); ++it) {
            unsigned new_rank = it->rank() - (_personal_statistics.average_et[it->object()->criterion().current_queue_eamq()] + Thread::scheduler()->chosen()->priority());
            it->rank(new_rank);
        }
        if ( Thread::scheduler()->end()) {
            unsigned new_rank = Thread::scheduler()->end()->rank() + (_personal_statistics.average_et[Thread::scheduler()->end(current_queue_eamq())->object()->criterion().current_queue_eamq()] + Thread::scheduler()->chosen()->priority());
            Thread::scheduler()->end()->rank(new_rank);
        }

        
        for (unsigned int q = 0; q < QUEUES; q++)
        {
            // (tempo de execução anterior + tempo de execução atual) / 2
            _personal_statistics.average_et[q] = (_personal_statistics.average_et[q] + _personal_statistics.job_execution_time) / 2;
            // Atualiza EET da tarefa para cada fila (relativo a frequência)
            _personal_statistics.job_estimated_et[q] = Timer_Common::sim(_personal_statistics.average_et[q], frequency_within(_queue_eamq), frequency_within(q));
            // Timer_Common::time(_personal_statistics.average_et[q], frequency_within(q));
        }
        _personal_statistics.job_execution_time = 0;
    }
    if (periodic() && (event & ASSURE_BEHIND)) {
        db<EAMQ>(TRC) << "p: " << _priority << " visited for rerank (someone in front was inserted)" << endl;
    }
    if (periodic() && (event & RESUME_THREAD)) {
        rank_eamq(); // atualiza o rank
        if (_behind_of) {
            // Faz atualização de rank da thread que foi inserida chamando assure_behind
            _behind_of->link()->prev()->object()->for_all_behind(ASSURE_BEHIND);
        }
    }
    if (event & LEAVE) {
        // P6 : coleta de dados do PMU
        // _personal_statistics.branch_miss += PMU::read(3);
        // _personal_statistics.cache_miss += PMU::read(4);
        // P6 : resetar PMU
        PMU::reset(0);
        // PMU::reset(1);   Não estamos utilizando o canal 1 (UNHALTED_CORE_CYCLES)
        PMU::reset(2);
        PMU::reset(3);
        PMU::reset(4);
        PMU::reset(5);
        PMU::reset(6);
        // P6 : start PMU
        PMU::start(0);
        // PMU::start(1);
        PMU::start(2);
        PMU::start(3);
        PMU::start(4);
        PMU::start(5);
        PMU::start(6);
    }

    /* a = new Job()        -> JOB_RELEASE, CREATE
     * [b] premptado por [a] -> ENTER (a), LEAVE (b)
     * a acabou tarefa :(    -> JOB_FINISH
     * a tem nova tarefa >:) -> JOB_RELEASE
     */
}

Thread * EAMQ::search_t_fitted(unsigned int q)
{
    for (auto it = Thread::scheduler()->end(q); it != Thread::scheduler()->begin(q) && !it->object()->criterion().is_recent_insertion(); it = it->prev()) {
        Thread * thread_in_queue = it->object();
        // As ultimas threads da fila tendem a ser aperiodicas, então nós não queremos recalcular o rank delas
        if (!thread_in_queue->criterion().periodic()) { 
            //db<EAMQ>(TRC) << "Pulando uma thread aperiodica" << endl;
            continue;
        }

        // Thread da frente -> Tf
        // Thread que será inserido -> Ti
        int thread_capacity_remaining = thread_in_queue->criterion().personal_statistics().remaining_et[q];
        int total_time_execution = thread_in_queue->priority()                  // tempo de espera da (Tf)
                                    + (thread_capacity_remaining * 115 / 100)   // tempo de execução da (Tf)
                                    + _personal_statistics.remaining_et[q]      // tempo de execução (Ti)
                                    + estimate_rp_waiting_time(q);              // tempo de espera por RP (Ti)

        if (total_time_execution < int(Time_Base(_personal_statistics.remaining_deadline))) {
            return thread_in_queue;
            // vai inserir na frente de alguem, entao salvar onde
        }
    }
    return nullptr;
}

int EAMQ::rank_eamq() {
    // Baseado em Choosen não saindo da fila
    for (unsigned int i = QUEUES - 1; i >= 0; i--) {
        // tempo de execução restante estimado
        int eet_remaining = _personal_statistics.remaining_et[i];
        
        db<EAMQ>(TRC) << "EET restante: " << eet_remaining << endl;

        // calcula round profile waiting time
        int rp_waiting_time = estimate_rp_waiting_time(i);
        db<EAMQ>(TRC) << "RP waiting time: " << rp_waiting_time << endl;


        // Não avaliamos a possibilidade de inserir threads na frente de outras recém inseridas para evitarmos um possível loop infinito
        Thread * t_fitted = search_t_fitted(i);

        // Se não encontrou nenhuma fila (não vazia) que cabe a thread (e tem threads periodicas) avalie a próxima
        if (!t_fitted && !Thread::scheduler()->empty(i) && Thread::scheduler()->head(i)->object()->criterion().periodic()) {
            continue;
        }

        _behind_of = t_fitted;

        // forma de accesar a cabeça e checar se é periodica
        // Thread::scheduler()->head(i)->object()->criterion().periodic()

        // Se a fila estiver vazia t_fitted = NULL
        int t_fitted_capacity_remaining = 0;

        // Se a fila não estiver vazia precisamos levar em consideração o tempo que a thread da frente esperará
        if (!Thread::scheduler()->empty(i) && Thread::scheduler()->head(i)->object()->criterion().periodic()) {
            db<EAMQ>(TRC) << "Fila não vazia e achou fila inserir!" << endl;
            t_fitted_capacity_remaining = t_fitted->criterion().personal_statistics().remaining_et[i];
        }

        int cwt_profile = rp_waiting_time + (t_fitted ? t_fitted->priority() + t_fitted_capacity_remaining : 0);
        int available_time_to_run = _personal_statistics.remaining_deadline - cwt_profile;
        int idle_time = available_time_to_run - eet_remaining;
        db<EAMQ>(TRC) << "CWT: " << cwt_profile << ", Time to run: " << available_time_to_run << ", IDLE time: " << idle_time << endl;

        if (idle_time >= 0) {
            if (_queue_eamq != i) {
                // Precisamos jogar um evento para que o PEAMQ saiba que uma thread esta mudando de fila
                // Então ele vai salvar os dados coletados da PMU por aquela thread 
                handle(LEAVING_QUEUE);
                // Se for alterar de fila/'frequência', precisamos resetar os dados para coletar novos
                reset_pmu_personal_stats();
            }
            set_queue(i);
            _priority = cwt_profile;
            db<EAMQ>(TRC) << "Thread inserted in queue " << i << " with priority " << cwt_profile << endl;
            return 1;
        }
    }
    // Não encontrou lugar na fila e vai inserir na sub-fila com maior frequencia
    db<EAMQ>(TRC) << "Thread not inserted in any queue" << endl;
    _priority = 0;

    if (_queue_eamq != 0) {
        // Precisamos jogar um evento para que o PEAMQ saiba que uma thread esta mudando de fila
        // Então ele vai salvar os dados coletados da PMU por aquela thread 
        handle(LEAVING_QUEUE);
        // Se for alterar de fila/'frequência', precisamos resetar os dados para coletar novos
        reset_pmu_personal_stats();
    }

    set_queue(0);
    return 0;
}

void EAMQ::reset_pmu_personal_stats() {
    _personal_statistics.branch_miss = 0;
    _personal_statistics.cache_miss = 0;
    _personal_statistics.branches = 0;
    _personal_statistics.cache_hit = 0;
    _personal_statistics.instructions = 0;
    _personal_statistics.migrate = false;
}

int EAMQ::estimate_rp_waiting_time(unsigned int q) {
    int rp_rounds = _personal_statistics.remaining_et[q] / Q;

    // Se precisar de uma rodada extra com um tamanho 'menor que o Quantum'
    if (_personal_statistics.remaining_et[q] % Q) {
        rp_rounds++;
    }

    int oc = Thread::scheduler()->occupied_queues();
    oc -= !Thread::scheduler()->empty(q);

    int rp_waiting_time = Q * (oc) * (rp_rounds);

    return rp_waiting_time;
}

PEAMQ::Core_Statistics PEAMQ::_core_statistics = {
    /* branch_misses */       {0},
    /* cache_misses */        {0},
    /* instruction_retired */ {0},
    /* cache_hit */           {0},
    /* branch_instruction */  {0}
};

volatile unsigned int PEAMQ::evaluate(bool max_core)
{
    unsigned long long min = IDLE;
    unsigned int chosen_core = 0;
    unsigned long long max = 0;
    unsigned int max_core_id = 0;

    // avaliacao com dados da PMU que setam "pontos iniciais" para cada core
    unsigned long long most_instructions_retired = 0;
    for (unsigned int core = 0; core < CPU::cores(); core++) {
        if (_core_statistics.instruction_retired[core] > most_instructions_retired)
            most_instructions_retired = _core_statistics.instruction_retired[core];
    }

    // normalizing the value to divide
    most_instructions_retired /= 100;
    for (unsigned int core = 0; core < CPU::cores(); core++)
    {
        unsigned long long instruction_retired = 1;
        unsigned long long branch_miss_rate = 1;
        unsigned long long cache_miss_rate = 1;
        
        // avaliação com dados da PMU 
        // branch miss rate = resulta em um valor sem casas decimais, por isso multiplicamos por 100
        if (_core_statistics.branch_instruction[core])
            branch_miss_rate = (_core_statistics.branch_misses[core]*100) / _core_statistics.branch_instruction[core];
        
        if (_core_statistics.cache_hit[core] + _core_statistics.cache_misses[core])
            cache_miss_rate = (_core_statistics.cache_misses[core]*100) / (_core_statistics.cache_hit[core] + _core_statistics.cache_misses[core]);
        
        if (most_instructions_retired)
            instruction_retired = _core_statistics.instruction_retired[core] / most_instructions_retired;

        // ver em qual intervalo o core_rate se mantém para conseguir fazer a análise de quanto interferir nele com os dados da PMU

        unsigned long long core_rate = 0;
    
        for (unsigned int q = 0; q < QUEUES; q++)
        {
            auto last_element = Thread::scheduler()->tail(core, q);
            while (last_element && !last_element->object()->criterion().periodic())
            {
                last_element = last_element->prev();
            }
            
            if (!last_element)
            {
                core_rate += 0;
            } else {
                core_rate += last_element->object()->priority() - (((last_element->object()->priority() / 1000) * 125) * q);   // (1 - 0.125 x q)
            }
        }
        // Antes:
        // long long pmu = branch_miss_rate + cache_miss_rate + instruction_retired; // ~ 300 max
        
        // Agora:
        long long pmu = 1;
        if (cache_miss_rate > 50) {pmu = 20;} // errando muito cache
        else if (branch_miss_rate > 50 && instruction_retired > 70) {pmu = 16;} // errando muito branch e rodando muitas instruções 
        else if (branch_miss_rate > 50 && instruction_retired > 50) {pmu = 14;} // errando muito branch e rodando instruções razoaveis
        else if (branch_miss_rate > 50) {pmu = 12;}  // errando muito branch
        else if (instruction_retired < 30) {pmu = 4;} // por algum motivo rodando poucas instruções (não podemos garantir algo ruim)
        core_rate = core_rate * pmu;
        if (core_rate < min)
        {
            min = core_rate;
            chosen_core = core;
        }
        // P7 : coletar o maximo também 
        if (max_core) {
            if (core_rate > max) {
                max = core_rate;
                max_core_id = core;
            }
        }
    }
    if (max_core) {
        return max_core_id;
    }
    return chosen_core;
}

void PEAMQ::handle(Event event) {
    if (periodic() && (event & UPDATE)) {
        if (_recently_migrated) {
            _recently_migrated = false;
            // preciso remover a thread que está rodando aqui
            rank_eamq();
        }
    }

    if (periodic() && (event & LEAVE)) {
        _core_statistics.instruction_retired[CPU::id()] += PMU::read(2);
        _core_statistics.branch_misses[CPU::id()] += PMU::read(3);
        _core_statistics.branch_instruction[CPU::id()] += PMU::read(4);
        _core_statistics.cache_hit[CPU::id()] += PMU::read(5);
        _core_statistics.cache_misses[CPU::id()] += PMU::read(6);

        // P7 : analisando porcentagem e colocando se necessário migrar
        if (_core_statistics.cache_hit[CPU::id()]) {
            // pode dar divisão por zero
            db<AAA>(WRN) << "_core_statistics.cache_misses[CPU::id()]: " << _core_statistics.cache_misses[CPU::id()] << endl;
            db<AAA>(WRN) << "_core_statistics.cache_hit[CPU::id()]: " << _core_statistics.cache_hit[CPU::id()] << endl;

            unsigned long long cm_rate = (_core_statistics.cache_misses[CPU::id()]*100) / (_core_statistics.cache_misses[CPU::id()] + _core_statistics.cache_hit[CPU::id()]);
            db<AAA>(WRN) << "cm_rate: " << cm_rate << endl;

            // P7 : se taxa de cache miss for maior que 25% então seta migrate true
            _personal_statistics.migrate = cm_rate >= 25;
        }

//         P7 : identificar Core menos e mais com score
//        unsigned int id_max = ANY;
//        unsigned long long aux = ANY;
        _core_statistics.min_core = evaluate();
        _core_statistics.max_core = evaluate(true);

    }

    if (periodic() && (event & FINISH)) {
        _core_statistics.branch_misses[CPU::id()] -= _personal_statistics.branch_miss;
        _core_statistics.branch_instruction[CPU::id()] -= _personal_statistics.branches;
        _core_statistics.cache_hit[CPU::id()] -= _personal_statistics.cache_hit;
        _core_statistics.cache_misses[CPU::id()] -= _personal_statistics.cache_miss;
        _core_statistics.instruction_retired[CPU::id()] -= _personal_statistics.instructions;
    }
    // let EAMQ handle the rest and reset PMU
    EAMQ::handle(event);
}

// P7 : função ativado no thread::idle(), verifica qual core cada thread vai migrar
bool PEAMQ::migrate() {
    // se atual core é o que está sendo mais utilizado e min diferente de max
    // _queue é setado em Variable_Queue_Scheduler na criação do Criterion
    // não faz sentido sair do core atual se houver apenas ele (ele é o problema)
    if(_core_statistics.max_core == CPU::id() && _core_statistics.min_core != _core_statistics.max_core && Thread::scheduler()->size(_queue) > 1) {
        db<AAA>(WRN) << "AAAAA!!!! vai mudar para " << _core_statistics.min_core << endl;
        return true;
    }
    return false;
}


/////////////////////////////// P3 - Multicore Global Scheduling /////////////////////////////// 
// P3TEST - novo calculo de rank -> precisa alterar rp waiting time ainda tbm 
// Verificar se todos os cores irão setar para QUEUE - 1
// volatile unsigned int GEAMQ::_current_queue[GEAMQ::HEADS] = {QUEUES - 1};

// void GEAMQ::handle(Event event) {
//     // Antes de toda troca de threads (choose / chosen) precisa-se avancar 
//     // o ponteiro da fila de escolha 
//     if (event & CHANGE_QUEUE) {
//         unsigned int last = current_queue();
//         // db<Thread>(WRN) << "CPU " << CPU::id() << " Last Queue: " << last << endl;
//         do {
//             // Pula para próxima fila
//             GEAMQ::next_queue();
//             db<Lists>(TRC) << "next_queue chamado, levou para: " << current_queue() << endl;
//         // Enquanto fila atual não vazia ou uma volta completa
//         // ACHO que da certo apenas com chosen() 
//         } while (Thread::scheduler()->empty(current_queue()) && !(Thread::scheduler()->chosen()) && (current_queue() != last));

//         //db<Thread>(WRN) << "Current_queue ATUAL: " << current_queue() << endl;

//         // Se for RUN_TO_HALT, não ajusta a frequência (100%)
//         if (!Traits<System>::RUN_TO_HALT) {
//             // Ajustando a frequência conforme a fila
//             Hertz f = frequency_within(current_queue());
//             CPU::clock(f);
            
//             // So that IDLE doesnt spam this
//             if (last != current_queue()) {
//                 db<EAMQ>(TRC) << "[!!!] Operating next queue, in frequency: " << f / 1000000 << "Mhz " << "Queue: " << current_queue() << endl;
//             }
//         }
//     }
//     if (event & CREATE) {
//         // // db<Lists>(WRN) << "CRIANDO THREAD" << endl;
//         //unsigned int count = 5;
//         // for (int q = 0; q < QUEUES; q++) {
//         //     db<GEAMQ>(WRN) << "CPU " << CPU::id() << " Fila " << q << " Tamanho  " <<  Thread::scheduler()->size(q) << " ";
//         //     for (Thread* t = Thread::scheduler()->tail(q)->object(); t != nullptr; t = t->link()->prev()->object()) {
//         //         //if (count == 0) {break;} 
//         //         db<GEAMQ>(WRN) << t << " ";
//         //         //count--;
//         //     }
//         //     db<GEAMQ>(WRN) << "CPU " << CPU::id() << " CHOSEN: " << Thread::scheduler()->chosen_now(q)->object();
//         // db<GEAMQ>(WRN) << endl;
//         // }
//         // db<GEAMQ>(WRN) << endl;
//     }
//     if (event & UPDATE) {
//         // Depois da proxima ser definida e avisada de sua entrada, podemos desproteger as recem entradas
//         // Todas as threads recebem um evento UPDATE
//         _is_recent_insertion = false;
//         _behind_of = nullptr;
//     }
//     // Quando acontece prempcao do quantum
//     if (periodic() && (event & UPDATE)) {
//         if (Q > Time_Base(_personal_statistics.remaining_deadline)) {
//             // underflow
//             _personal_statistics.remaining_deadline = Microsecond(0);
//             // somehow discard this thread
//         } else {
//             // Decrementa do deadline o quantum executado
//             _personal_statistics.remaining_deadline -= Microsecond(Q);
//         }
//     }
//     if (periodic() && (event & CREATE)) {
//         // Se foi inserido no meio da fila (ou seja, se tem t_fitted)
//         if (_behind_of) {
//             // Faz atualização de rank da thread que foi inserida chamando assure_behind
//             _behind_of->link()->prev()->object()->for_all_behind(ASSURE_BEHIND);
//         }
//     }
//     if (periodic() && (event & LEAVE)) {
//         // Guarda o tempo que passou depois que começou a execução da tarefa
//         Microsecond in_cpu = time(elapsed() - _personal_statistics.job_enter_tick);
//         _personal_statistics.job_execution_time += in_cpu;
//         for (unsigned int q = 0; q < QUEUES; q++)
//         {
//             // Reduz o tempo executado deste quantum, transformando Tick em Microsecond
//             Microsecond executed_in_profile = Timer_Common::sim(in_cpu, frequency_within(_queue_eamq), frequency_within(q));
//             if (executed_in_profile > _personal_statistics.remaining_et[q]) {
//                 // underflow
//                 _personal_statistics.remaining_et[q] = 0;
//             } else {
//                 _personal_statistics.remaining_et[q] -= executed_in_profile;
//             }
//         }
//     }
//     // Quando uma thread periodica começa a tarefa
//     if (periodic() && (event & ENTER)) {
//         _personal_statistics.job_enter_tick = elapsed();

//         // int count = 5;
//         // for (int q = 0; q < QUEUES; q++) {
//         //     db<GEAMQ>(WRN) << "CPU " << CPU::id() << " Fila " << q << " Tamanho  " <<  Thread::scheduler()->size(q) << " ";
//         //     for (Thread* t = Thread::scheduler()->tail(q)->object(); t != nullptr; t = t->link()->prev()->object()) { 
//         //         db<GEAMQ>(WRN) << t->link() << " ";
//         //         count--;
//         //     }
//         //     count = 5;
//         //     db<GEAMQ>(WRN) << "CPU " << CPU::id() << " CHOSEN: " << Thread::scheduler()->chosen_now(q);
//         // db<GEAMQ>(WRN) << endl;
//         // }
//         // db<GEAMQ>(WRN) << endl;
//     }
//     // Quando uma thread foi liberado para executar tarefa
//     if (periodic() && (event & JOB_RELEASE)) {
//         // db<GEAMQ>(INF) << "JOB RELEASE" << endl;
//         _personal_statistics.job_execution_time = 0;
//     }
//     // Quando uma thread periodica termina tarefa
//     if (periodic() && (event & JOB_FINISH)) {
//         // Atualiza rank de todas as threads da fila diminuindo o tempo
//         // db<GEAMQ>(WRN) << "JOB FINISH" << endl;
//         for (auto it = Thread::scheduler()->begin(current_queue()); it != Thread::scheduler()->end(current_queue()); ++it) {
//             if (!it->object()->criterion().periodic()) break;
            
//             unsigned new_rank = it->rank()
//                                     - (_personal_statistics.average_et[it->object()->criterion().current_queue()] 
//                                         + Thread::scheduler()->chosen()->priority());
//             it->rank(new_rank);
//         }
//         // mais uma execução para cobrir o ultimo elemento da fila (fizemos uma bagunca com os end's e begin's possivelmente)
//         if ( Thread::scheduler()->end(current_queue()) && Thread::scheduler()->end(current_queue())->object()->criterion().periodic() ) {
//             unsigned new_rank = Thread::scheduler()->end(current_queue())->rank() 
//                                     + (_personal_statistics.average_et[Thread::scheduler()->end(current_queue())->object()->criterion().current_queue()] 
//                                         + Thread::scheduler()->chosen()->priority());
                                        
//             Thread::scheduler()->end(current_queue())->rank(new_rank);
//         }

        
//         for (unsigned int q = 0; q < QUEUES; q++)
//         {
//             // (tempo de execução anterior + tempo de execução atual) / 2
//             _personal_statistics.average_et[q] = (_personal_statistics.average_et[q] + _personal_statistics.job_execution_time) / 2;
//             // Atualiza EET da tarefa para cada fila (relativo a frequência)
//             _personal_statistics.job_estimated_et[q] = Timer_Common::sim(_personal_statistics.average_et[q], frequency_within(_queue_eamq), frequency_within(q));
//             // Timer_Common::time(_personal_statistics.average_et[q], frequency_within(q));
//         }
//         _personal_statistics.job_execution_time = 0;
//     }
//     if (periodic() && (event & ASSURE_BEHIND)) {
//         db<GEAMQ>(TRC) << "p: " << _priority << " visited for rerank (someone in front was inserted)" << endl;
//     }
//     if (periodic() && (event & RESUME_THREAD)) {
//         db<GEAMQ>(TRC) << "RESUME_THREAD called, is periodic" << endl;

//         rank_eamq(); // atualiza o rank
//         if (_behind_of) {
//             // Faz atualização de rank da thread que foi inserida chamando assure_behind
//             _behind_of->link()->prev()->object()->for_all_behind(ASSURE_BEHIND);
//         }
//     }
//     if (event & CHARGE) {
//         // for (int q = 0; q < QUEUES; q++) {
//         //     db<GEAMQ>(WRN) << " Fila " << q << " Tamanho  " <<  Thread::scheduler()->size(q) << " ";
//         //     for (Thread* t = Thread::scheduler()->tail(q)->object(); t != nullptr; t = t->link()->prev()->object()) { 
//         //         db<GEAMQ>(WRN) << t->link() << " ";
//         //     }
//         //     db<GEAMQ>(WRN) << endl;
//         //     db<GEAMQ>(WRN) << "CPU " << CPU::id() << " CHOSEN: " << Thread::scheduler()->chosen_now(q);
//         // db<GEAMQ>(WRN) << endl;
//         // }
//         // db<GEAMQ>(WRN) << endl;
//     }
//     /* a = new Job()        -> JOB_RELEASE, CREATE
//      * [b] premptado por [a] -> ENTER (a), LEAVE (b)
//      * a acabou tarefa :(    -> JOB_FINISH
//      * a tem nova tarefa >:) -> JOB_RELEASE
//      */
// }

// int GEAMQ::rank_eamq() {
//     for (int i = QUEUES - 1; i >= 0; i--) {
//         Thread * t_fitted = nullptr;

//         // tempo de execução restante estimado
//         int eet_remaining = _personal_statistics.remaining_et[i];
//         db<GEAMQ>(TRC) << "EET restante: " << eet_remaining << endl;

//         // calcula round profile waiting time
//         int rp_waiting_time = estimate_rp_waiting_time(i);
//         db<GEAMQ>(TRC) << "RP waiting time: " << rp_waiting_time << endl;

//         // Não avaliamos a possibilidade de inserir threads na frente de outras recém inseridas para evitarmos um possível loop infinito
//         for (auto it = Thread::scheduler()->end(i); it != Thread::scheduler()->begin(i) && !it->object()->criterion().is_recent_insertion(); it = it->prev()) {
//             Thread * thread_in_queue = it->object();
//             // As ultimas threads da fila tendem a ser aperiodicas, então nós não queremos recalcular o rank delas
//             if (!thread_in_queue->criterion().periodic()) { 
//                 db<GEAMQ>(TRC) << "Pulando uma thread aperiodica" << endl;
//                 continue;
//             }

//             // Thread da frente -> Tf
//             // Thread que será inserido -> Ti
//             int thread_capacity_remaining = thread_in_queue->criterion().personal_statistics().remaining_et[i];
//             int total_time_execution = thread_in_queue->priority()               // tempo de espera da (Tf)
//                                        + (thread_capacity_remaining * 115 / 100) // tempo de execução da (Tf)
//                                        + eet_remaining                           // tempo de execução (Ti)
//                                        + rp_waiting_time;                        // tempo de espera por RP (Ti)

//             if (total_time_execution < int(Time_Base(_personal_statistics.remaining_deadline))) {
//                 t_fitted = thread_in_queue;
//                 // vai inserir na frente de alguem, entao salvar onde
//                 _behind_of = t_fitted;
//                 db<GEAMQ>(TRC) << "Encontrou um lugar atrás de: " << t_fitted << endl;
//                 break;
//             }
//         }

//         // Não permitimos inserir threads na primeira posição da fila se alguma já tiver sido inserida
//         // Se não encontrou nenhuma fila (não vazia) que cabe a thread (e tem threads periodicas) avalie a próxima
//         if (!t_fitted && !Thread::scheduler()->empty(i) && Thread::scheduler()->head(i)->object()->criterion().periodic()) {
//             db<GEAMQ>(TRC) << "Thread não cabe na fila " << i << endl;
//             continue;
//         }

//         // forma de accesar a cabeça e checar se é periodica
//         // Thread::scheduler()->head(i)->object()->criterion().periodic()

//         // Se a fila estiver vazia t_fitted = NULL
//         int t_fitted_capacity_remaining = 0;

//         /////////////////////// P3TEST - Se tem thread ja sendo executado por Cores nessa fila ///////////////////////
//         if (Thread::scheduler()->empty(i) && (Thread::scheduler()->chosen_now(i))) {
//             t_fitted = Thread::scheduler()->chosen_now(i)->object();
//             db<GEAMQ>(TRC) << "Fila vazia mas já existe um chosen para esse core nela" << endl;
//         }

//         // Se a fila não estiver vazia precisamos levar em consideração o tempo que a thread da frente esperará
//         // if (!Thread::scheduler()->empty(i) && Thread::scheduler()->head(i)->object()->criterion().periodic() ) {
//         if (t_fitted && Thread::scheduler()->head(i)->object()->criterion().periodic()) {
//             db<GEAMQ>(TRC) << "Fila 'não vazia' e achou fila inserir!" << endl;
//             t_fitted_capacity_remaining = t_fitted->criterion().personal_statistics().remaining_et[i];
//         }

//         // Calculo de slack
//         // tempo de espera = tempo de espera por RP + tempo de espera da thread da frente + tempo de execução de thread da frente
//         // tempo para execução = deadline - tempo de espera 
//         // slack = tempo para execução - tempo de execução estimativa 
//         int cwt_profile = rp_waiting_time + (t_fitted ? t_fitted->priority() + t_fitted_capacity_remaining : 0);
//         int available_time_to_run = _personal_statistics.remaining_deadline - cwt_profile;
//         int idle_time = available_time_to_run - eet_remaining;
//         db<GEAMQ>(TRC) << "CWT: " << cwt_profile 
//                         << ", Time to run: " << available_time_to_run 
//                         << ", IDLE time: " << idle_time 
//                         << endl;

//         if (idle_time >= 0) {
//             set_queue(i);
//             _priority = cwt_profile;
//             db<GEAMQ>(TRC) << "Thread inserted in queue " << i << " with priority " << cwt_profile << endl;
//             db<GEAMQ>(TRC) << endl <<  endl;
//             return 1;
//         }
//     }
//     // Não encontrou lugar na fila
//     // Rever com uma análise mais severa se vale a pena ainda tentar
//     db<GEAMQ>(TRC) << "Não vai encaixar em nada, RODA EM POTENCIA MÁXIMA, talvez dê" << endl;
//     db<GEAMQ>(TRC) << endl <<  endl;
//     _priority = 0;
//     set_queue(0);

//     return 0;
// }

__END_SYS
