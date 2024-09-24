#include <utility/list.h>
#include <machine/display.h>
#include <synchronizer.h>
#include <utility/random.h>
#include <time.h>

#include "utils.h"

using namespace EPOS;

ProfileQueue * qs[SimVars::N_QUEUES];
DummyThread * ts[SimVars::N_THREADS];

Optimal rank(DummyThread * t)
{
    Optimal optimal;

    for (unsigned int f = 0; f < SimVars::N_QUEUES; f++) {
        ProfileQueue* queue = qs[f];
        // deadline maior dos menores
        ProfileElement* t_maior_d = nullptr;
        // Tail??? acessar elementos da fila de maior deadline -> menor deadline
        for (ProfileElement* elem = queue->tail(); elem; elem = elem->next()) {
            DummyThread* thread_in_queue = elem->object();
            //cwt + wcet_restante*15% (margem extra para não chegar no limite) < deadline
            if (!t_maior_d || (thread_in_queue->cwt + static_cast<int>(thread_in_queue->wcet_remaining[f]*1.15) + t->wcet_remaining[f]) < t->d ) {
                t_maior_d = elem;
                break;
            }
        }

        // WCET esperado para este perfil
        int wcet_profile = t->wcet_remaining[f];
        cout << "WCET of " << t->l << ": " << wcet_profile << " em " << f << endl;

        // Numero de ciclos do round robin para a proxima exec.
        const int rr_rounds = static_cast<float>(wcet_profile / Q) == static_cast<int>((wcet_profile / Q)) ? (static_cast<int>((wcet_profile / Q)) - 1) : static_cast<int>((wcet_profile / Q) );
        cout << "Round robin rounds to wait " << t->l << ": " << rr_rounds << endl;

        // Tempo de espera do RR ate sua execucao, no pior caso (toda fila possui pelo menos uma thread)
        int rr_waiting_time = Q * (SimVars::N_QUEUES - 1) * (rr_rounds);
        cout << "Round robin waiting time " << t->l << ": " << rr_waiting_time << endl;
        
        // tempo de espera atual desta fila = tempo de espera (RR) + wcet restante da fila
        const int cwt_profile = rr_waiting_time + (t_maior_d ? t_maior_d->object()->wcet_remaining[f] : 0);
        cout << "Total waiting time of " << t->l << ": " << cwt_profile << endl;

        // Tempo de execução disponivel, dada espera nesta fila
        int available_time_to_run = t->d - cwt_profile;
        cout << "Time to run of " << t->l << ": " << available_time_to_run << endl;

        // Cálculo do tempo "inativo"
        int idle_time = available_time_to_run - wcet_profile;
        cout << "Idle time of " << t->l << ": " << idle_time << endl << endl;
        
        // Se o tempo calculado for positivo (??????????e menor que o "optimal" atual???????????)
        // Garante que sempre pega fila com menor frequencia possivel
        if (idle_time >= 0) {
            optimal = Optimal(f, cwt_profile);
        }
    }

    return optimal;
}

// Leva ponteiro para tras e recalcula deadline e reposiona talvez
void assure_behind(ProfileElement * inserted, Optimal op)
{
    // p3 p2 p1 inserted 
    for (ProfileElement * p = inserted->prev(); p != nullptr;)
    {
        //salva p->prev() no prev
        ProfileElement * prev = p->prev();
        // remover thread p
        qs[op.queue_num]->remove(p);
        // recalcula 
        Optimal op_prev = rank(p->object());
        p->object()->cwt = op_prev.cwt;
        // reinsere na posicao certa
        qs[op_prev.queue_num]->insert(p);

        // se migra para outra fila -> verifica seus anteriores
        if (op.queue_num != op_prev.queue_num) {assure_behind(p, op_prev);}
        // atualiza p para proximo prev
        p = prev;
    }
}


void init_structs()
{
    for (size_t i = 0; i < SimVars::N_QUEUES; i++)
    {
        qs[i] = new ProfileQueue();
    }
    for (size_t i = 0; i < SimVars::N_THREADS; i++)
    {
        ts[i] = new DummyThread(SimVars::DEADLINE_CAP);

        Optimal op = rank(ts[i]);
        ts[i]->cwt = op.cwt;

        ProfileElement* pe = new ProfileElement(ts[i]);
        pe->rank(ProfileQueue::Rank_Type(op.cwt));
        qs[op.queue_num]->insert(pe);
        assure_behind(pe, op);
        
        print_queues(qs, ts);
    }
}


void round_robin_simulation()
{
    TSC_Chronometer chronometer;
    int max_change = 8;
    int count_change = 0;

    while (count_change <= max_change) {
        for (size_t i = 0; i < SimVars::N_QUEUES; i++){
            count_change++;
            //round robin numa fila 
            ProfileQueue* queue = qs[i];

            chronometer.reset();
            chronometer.start();

            for (ProfileQueue::Iterator it = queue->begin(); it != queue->end(); it++){
                cout << "Executando thread " << it->object()->l << ": com deadline " << it->object()->d << " da fila " << i << endl;
                //logica do thread executando
                //Alarm::delay(expected_wcet(i));>>
                if (chronometer.read() >= Q) {
                    cout << "Quantum de " << Q << " us atingido para a fila " << i << endl;
                    break;
                }
            }
            chronometer.stop();
        }

    }
}


void clean()
{
    for (DummyThread *t : ts)
    {
        delete t;
    }
    for (ProfileQueue *q : qs)
    {
        delete q;
    }
}


int main()
{
    init_structs();
    round_robin_simulation();
    clean();
    return 0;
}
