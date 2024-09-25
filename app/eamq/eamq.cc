/* Simulação do EAMQ (Energy Aware Multi-Queue) 
* 
* A ideia do EAMQ constitui em utilizar a folga da deadline para
* ajustar a frequência, de modo que, as threads terminem o mais próximo 
* possível da suas deadlines.
*
* Para isso, utiliza-se multiplas filas de escalonamento, onde
* cada fila atua em uma porcentagem da frequência (perfis). E cada
* fila possui um turno, que são revezados circularmente, 
* para executar a thread em um quantum.
*
* Uma thread é atribuída a uma fila de acordo com a melhor folga,
* onde, quanto maior a folga, menor a frequência que se pode atuar.
*
* Esta folga (idle time) pode ser visualizada como:
*
*     t0                                                 deadline
*     |                                                      |
*     +------------------------------------------------------+   
*                                                             
*                                              |-------------|
*                                                 tempo de espera  
*                             |----------------|   para turno da fila
*                               tempo de espera
*             |-+-+-+-+-+-+-+-|  dentro da fila       
*                folga da deadline             
*     |-------|      (idle time)             
*   pior caso do 
* tempo de execução 
* esperado do perfil (wcet)
*
* */ 

#include <utility/list.h>
#include <machine/display.h>
#include <synchronizer.h>
#include <utility/random.h>
#include <time.h>

#include "utils.h"

using namespace EPOS;

ProfileQueue * qs[SimVars::N_QUEUES];
DummyThread * ts[SimVars::N_THREADS];


/* Garante que, se uma thread for inserida a frente de outra(s) em uma fila,
* verifica se esta(s) thread(s) ainda consegue(m) esperar, dado o novo atraso
* da thread inserida, recalculando o rank.
* */
// Leva ponteiro para tras e recalcula rank e reposiona talvez
void assure_behind(ProfileElement * inserted)
{
    // p3 p2 p1 inserted 
    for (ProfileElement * p = inserted->next(); p != nullptr;)
    {
        //salva p->next() no next
        ProfileElement * next = p->next();
        // remover thread p
        qs[inserted->object()->current_queue]->remove(p);
        // recalcula 
        p->object()->rank(qs);
        // reinsere na posicao certa
        qs[p->object()->current_queue]->insert(p);

        // se migra para outra fila -> verifica seus anteriores
        if (inserted->object()->current_queue != p->object()->current_queue) {assure_behind(p);}
        // atualiza p para proximo next
        p = next;
    }
}



/* 
* */
void round_robin_simulation(int max_change)
{
    // TSC_Chronometer chronometer;
    int count_change = 0;

    ProfileElement * running_thread = nullptr;

    while (count_change <= max_change) {
        for (size_t i = 0; i < SimVars::N_QUEUES; i++){

            // Se fila vazia, pula
            if (qs[i]->empty()) continue;

            // Retira a thread da fila ordenada
            running_thread = qs[i]->head();
            qs[i]->remove(running_thread);

            // Simula execução por um Quantum ou menos
            int time_elapsed = running_thread->object()->wcet_remaining[i] < static_cast<int>(Q) ? running_thread->object()->wcet_remaining[i] : static_cast<int>(Q);
            // O objeto diminui seu wcet e sua deadline para simular passagem do tempo e execução
            const bool end_thread = running_thread->object()->run_quantum(i, time_elapsed);

            for (size_t j = 0; j < SimVars::N_QUEUES; j++)
            {
                for (ProfileQueue::Iterator it = qs[j]->tail(); &(*it) != nullptr; it = it->prev())
                {
                    // Outras threads que não estão em execução passam o tempo
                    it->object()->pass_quantum(time_elapsed);
                }
            }

            // Se a thread terminou, pula
            if (end_thread) continue;
            

            // Se a thread não terminou, reinserir na fila
            // Recalcula rank (assim como suas anteriores na fila)
            running_thread->object()->rank(qs);
            // Reposiciona thread na fila 
            qs[running_thread->object()->current_queue]->insert(running_thread);
            
            // count_change++;

            print_queues(qs, ts);
        }
        // Apenas para não rodar infinitamente enquanto não codamos as threads findando
        count_change++;
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

        ts[i]->rank(qs);
        //Optimal op = rank(ts[i]);
        // ts[i]->cwt = op.cwt;

        // ProfileElement* pe = new ProfileElement(ts[i]);

        // "rankeando" inicialmente as threads
        // pe->rank(ProfileQueue::Rank_Type(op.cwt));
        // qs[op.queue_num]->insert(ts[i]->pe);
        qs[ts[i]->current_queue]->insert(ts[i]->pe);
        assure_behind(ts[i]->pe);
        print_queues(qs, ts);
        round_robin_simulation(4);
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
    round_robin_simulation(20);
    clean();
    return 0;
}
