#include <utility/ostream.h>
#include <process.h>
#include <system/types.h>
#include <utility/queue.h>
#include <utility/random.h>

using namespace EPOS;
OStream cout;

const unsigned int Q = Traits<Thread>::QUANTUM;
char _counter = 'a';

struct DummyThread;

typedef Ordered_List<DummyThread *, List_Element_Rank, List_Elements::Doubly_Linked_Ordered<DummyThread, List_Element_Rank>> ProfileQueue;
typedef ProfileQueue::Element ProfileElement;
typedef ProfileQueue::Rank_Type ProfileRank;

typedef Traits<Application> SimVars;

 struct Optimal {
    unsigned int queue_num;
    unsigned int cwt;

    Optimal(unsigned int q_num = -1, unsigned int cwt = 0) : queue_num(q_num), cwt(cwt) {}
};

/* Para termos de simulação, uma DummyThread não realmente executa,
* mas apenas mantém referências de tempo e estado.
* */
struct DummyThread {
    typedef Thread::State State;
    
    int d;
    State s;
    ProfileElement* pe;
    char l;
    unsigned int current_queue;
    int wcet_remaining[SimVars::N_QUEUES];

    // Existem momentos inconsistentes entre current_queue e a fila real em que a DummyThread está inserida
    // Devemos fazer um booleano para indicar esses momentos?
    // A DummyThread apenas apresenta essa inconsistência quando é calculado o rank, mas logo em seguida ela é inserida na fila correta


    /* Apesar de a deadline ser uma constante para todas as threads, 
    * o WCET é calculado 
    * */
    DummyThread(unsigned int deadline, State st = State::WAITING) : d(deadline), s(st), l(_counter++), current_queue(-1)
    {
        pe = new ProfileElement(this);
        for (unsigned int i = 0; i < SimVars::N_QUEUES; i++)
        {
            const unsigned int rand = static_cast<unsigned>(Random::random()) % 20000;
            wcet_remaining[i] = (SimVars::BASE_WCET + rand) + (i * 10000);
        }
    }

    ~DummyThread() { delete pe; }

    /* Calculo de "rankeamento" das threads para as filas de perfis.
    * 
    * Atribui a thread o melhor caso para que ela execute o mais proximo
    * possivel de sua deadline. Aqui é definido:
    *       - A fila de perfil em que a thread sera inserida;
    *       - O tempo de espera relativo ao pior caso de espera (1 thread em cada outra fila).
    *
    * O tempo de espera relativo (cwt), sera usado como o fator de ordenagem para as filas,
    * similarmente a implementações da classe Priority.
    * */
    void rank(ProfileQueue* (&qs)[SimVars::N_QUEUES]);

    /* Simula um quantum de execução, atualizando os WCETs restantes da thread para cada
    * fila de perfil, com base na porcentagem da frequência em que a thread estaria atuando.
    * Retorna verdadeiro se a Thread terminou sua execução.
    * */
    const bool run_quantum(const int f);

    /* Simula um quantum onde a thread não estaria em execução, ou seja, inativa.
    * Reduzindo o deadline a fim de simular o tempo passando.
    * */
    void pass_quantum();
};

void print_queues(ProfileQueue* (&qs)[SimVars::N_QUEUES], DummyThread* (&ts)[SimVars::N_THREADS])
{
    cout << '-' << endl;
    for (size_t i = 0; i < SimVars::N_QUEUES; i++)
    {   
        cout << "Q" << i;
        for (ProfileQueue::Iterator it = qs[i]->tail(); &(*it) != nullptr; it = it->prev())
        {
            cout << " | " << it->object()->l << " wcet: "<< it->object()->wcet_remaining[i] << " cwt: " << it->object()->pe->rank();
        }
        cout << " | " << endl;
    }
    cout << endl;
}

/**
* Retorna o numero de filas ocupadas
* F simboliza a fila que está sendo analisada
*/
int occupied_queues(ProfileQueue* (&qs)[SimVars::N_QUEUES] , int f)

{
    int oc = 0;
    for (ProfileQueue* q : qs)
    {
        oc += int(!q->empty());
    }

    // 3 ocupado, onde vai inserir nao ocupado -> 3
    // 3 ocupado, onde vai inserir ocupado ->  3 - 1 = 2
    return oc == 0 ? oc : (qs[f]->empty() ? oc : oc - 1);
}

void DummyThread::rank(ProfileQueue* (&qs)[SimVars::N_QUEUES]) 
{
    Optimal optimal;

    for (unsigned int f = 0; f < SimVars::N_QUEUES; f++) {
        ProfileQueue* queue = qs[f];
        // Thread com cwt maior dos menores que Thread que será inserido
        ProfileElement* t_fitted = nullptr;

        // WCET esperado para este perfil
        int wcet_profile = this->wcet_remaining[f];
        cout << "WCET of " << this->l << ": " << wcet_profile << " em " << f << endl;

        // Numero de ciclos do round profile para a proxima exec.
        // cout << "WCET/Q = " << (static_cast<float>(wcet_profile) / Q) << endl;
        const int rp_rounds = (static_cast<float>(wcet_profile) / Q) == static_cast<int>((wcet_profile / Q)) ? (static_cast<int>((wcet_profile / Q)) - 1) : static_cast<int>((wcet_profile / Q) );
        //cout << "Round profile rounds to wait " << this->l << ": " << rp_rounds << endl;

        // Tempo de espera do RR ate sua execucao, no pior caso (toda fila possui pelo menos uma thread)
        // int rp_waiting_time = Q * (SimVars::N_QUEUES - 1) * (rp_rounds);
        int rp_waiting_time = Q * (occupied_queues(qs, f)) * (rp_rounds);
        cout << "Round profile waiting time " << this->l << ": " << rp_waiting_time << endl;

        // Acessar elementos da fila tenha tempo de espera < deadline 
        for (ProfileElement* elem = queue->tail(); elem; elem = elem->prev()) {
            DummyThread* thread_in_queue = elem->object();
            //cwt + wcet_restante*15% (margem extra para não chegar no limite) + wcet(dele) + RP(dele) < deadline
            if (!t_fitted || (thread_in_queue->pe->rank() + static_cast<int>(thread_in_queue->wcet_remaining[f]*1.15) + this->wcet_remaining[f]) + rp_waiting_time < this->d ) {
                // Thread para fixar (referencial), para inserir novo thread na fila 
                // T4 - T3 - "INSERIR AQUI" -  T_fitted - T1
                t_fitted = elem;
                break;
            }
        }
        // cwt = cwt frente (RR + wcet) + RR dele 
        // Tempo de espera atual desta fila = tempo de espera (RR) + cwt (RR + wcet) do da frente
        int cwt_profile = rp_waiting_time + (t_fitted ? static_cast<int>(t_fitted->object()->pe->rank() + t_fitted->object()->wcet_remaining[f]) : 0);
        cout << "Total waiting time of " << this->l << ": " << cwt_profile << endl;

        // Tempo de execução disponivel, dada espera nesta fila
        int available_time_to_run = this->d - cwt_profile;
        //cout << "Time to run of " << this->l << ": " << available_time_to_run << endl;

        // Cálculo do tempo "inativo"
        int idle_time = available_time_to_run - wcet_profile;
        cout << "Idle time of " << this->l << ": " << idle_time << endl << endl;
        
        // Garante que sempre pega fila com menor frequencia possivel
        // ja que 
        if (idle_time >= 0) {
            optimal = Optimal(f, cwt_profile);
        }
    }

    // Atualiza o rank e a fila em que a Thread será inserida
    pe->rank(optimal.cwt);
    current_queue = optimal.queue_num;
}

const bool DummyThread::run_quantum(const int f)
{
    for (unsigned int i = 0; i < SimVars::N_QUEUES; i++)
    {
        // f -> numero da fila que estava executando anteriormente
        // i -> numero da fila atual
        // cout << ((100.0 - (f * SimVars::FREQ_STEP)) / (100.0 - (i * SimVars::FREQ_STEP))) * Q << endl;
        wcet_remaining[i] -= ((100.0 - (f * SimVars::FREQ_STEP)) / (100.0 - (i * SimVars::FREQ_STEP))) * Q;
        if (wcet_remaining[i] <= 0) {return true;}
    }
    
    // Diminui a deadline para simular o tempo passando
    d -= Q;

    return false;
}

void DummyThread::pass_quantum()
{
    // Diminui a deadline para simular o tempo passando
    d -= Q;
}
