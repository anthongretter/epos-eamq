#include <utility/list.h>
#include <machine/display.h>
#include <synchronizer.h>
#include <utility/random.h>
#include <time.h>

#include "utils.h"

using namespace EPOS;

ProfileQueue * qs[N_QUEUES];
DummyThread * ts[N_THREADS];

Optimal rank(DummyThread * t)
{
    Optimal optimal(0, 99999);

    for (unsigned int f = 0; f < N_QUEUES; f++) {
        ProfileQueue* queue = qs[f];
        // deadline maior dos menores
        ProfileElement* t_maior_d = nullptr;
        // Tail??? acessar elementos da fila de maior deadline -> menor deadline
        for (ProfileElement* elem = queue->tail(); elem; elem = elem->next()) {
            DummyThread* thread_in_queue = elem->object();
            if (!t_maior_d || thread_in_queue->d < t->d) {
                t_maior_d = elem;
                break;
            }
        }

        // WCET esperado para este perfil
        Microsecond wcet_waiting_profile = expected_wcet(f);
        cout << "WCET de " << t->l << ": " << wcet_waiting_profile << endl;

        // Cálculo do tempo de espera
        const int queue_time = static_cast<float>(wcet_waiting_profile / Q) == static_cast<int>((wcet_waiting_profile / Q)) ? (static_cast<int>((wcet_waiting_profile / Q)) - 1) : static_cast<int>((wcet_waiting_profile / Q) );
        //const int round_float = static_cast<int>((wcet_waiting_profile/Q)*100);
        //const int queue_time = round_float == static_cast<int>((wcet_waiting_profile / Q))*100 ? (static_cast<int>((wcet_waiting_profile / Q)) - 1) : static_cast<int>((wcet_waiting_profile / Q) );
        Microsecond waiting_time = Q * (N_QUEUES - 1) * (queue_time);
        cout << "Waiting time de " << t->l << ": " << waiting_time << endl;

        // Tempo restante para a thread atual até o deadline (se estiver vazia = 0)
        Microsecond time_remaining = t->d - ((t_maior_d ? t_maior_d->object()->d : 0) + waiting_time);
        cout << "Time remaining de " << t->l << ": " << time_remaining << endl;

        // Cálculo do tempo "caso"
        Microsecond time_case = time_remaining - wcet_waiting_profile;
        cout << "Time case de " << t->l << ": " << time_case << endl;


        // Se o tempo calculado for positivo (??????????e menor que o "optimal" atual???????????)
        // Garante que sempre pega fila com menor frequencia possivel
        if (time_case > 0) {
            optimal = Optimal(f, time_case);
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
        qs[op.queueNum]->remove(p);
        // recalcula 
        Optimal op_prev = rank(p->object());
        // reinsere na posicao certa
        qs[op_prev.queueNum]->insert(p);

        // se migra para outra fila -> verifica seus anteriores
        if (op.queueNum != op_prev.queueNum) {assure_behind(p, op_prev);}
        // atualiza p para proximo prev
        p = prev;
    }
}

// void assure_behind3(ProfileElement * inserted, Optimal op)
// {
//     ProfileQueue::Iterator it(inserted->prev());
//     for (; &(*it) != nullptr;)
//     {
//         ProfileElement c = *it;
//         it--;

//         qs[op.queueNum]->remove(&c);
//         // recalcula 
//         Optimal op_prev = rank((&c)->object());
//         // reinsere na posicao certa
//         qs[op_prev.queueNum]->insert(&c);

//         // se migra para outra fila -> verifica seus anteriores
//         if (op.queueNum != op_prev.queueNum) {assure_behind(&c, op_prev);}
//     }
// }


void populate()
{
    for (size_t i = 0; i < N_QUEUES; i++)
    {
        qs[i] = new ProfileQueue();
    }

    for (size_t i = 0; i < N_THREADS; i++)
    {
        // Thread::Configuration conf(Thread::SUSPENDED, Thread::Criterion::NORMAL, 
        //     Traits<Application>::STACK_SIZE, static_cast<unsigned>(Random::random()));
        // Thread *t = new Thread(conf, &work);
        // ts[i] = t;
        unsigned int random_d = static_cast<unsigned>((Random::random() % DEADLINE_CAP) + 10);
        ts[i] = new DummyThread(random_d);

        Optimal op = rank(ts[i]);
        ProfileElement* pe = new ProfileElement(ts[i]);
        pe->rank(ProfileQueue::Rank_Type(op.time_deadline));
        qs[op.queueNum]->insert(pe);
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
        for (size_t i = 0; i < N_QUEUES; i++){
            count_change++;
            //round robin numa fila 
            ProfileQueue* queue = qs[i];

            chronometer.reset();
            chronometer.start();

            for (ProfileQueue::Iterator it = queue->begin(); it != queue->end(); it++){
                cout << "Executando thread" << it->object()->l << ": com deadline " << it->object()->d << " da fila " << i << endl;
                //logica do thread executando
                //Alarm::delay(expected_wcet(i));>>
                if (chronometer.read() >= 100) {
                    cout << "Quantum de " << Q << " us atingido para a fila " << i << endl;
                    break;
                }
            }
            chronometer.stop();
        }

    }
}

int main()
{
    populate();
    round_robin_simulation();

    return 0;
}