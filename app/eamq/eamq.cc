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
    // t1 t2 t3 INSERTED ...
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

/**
 * Assumimos que a thread que terminou de executar já foi retirada da fila
 */
void end_thread(ProfileQueue * queue) 
{
    ProfileElement * new_head = queue->head();
    
    // WCET esperado para este perfil
    int wcet_profile = new_head->object()->wcet_remaining[new_head->object()->current_queue];

    // Numero de ciclos do round profile para a proxima exec.
    const int rp_rounds = (static_cast<float>(wcet_profile) / Q) == static_cast<int>((wcet_profile / Q)) ? (static_cast<int>((wcet_profile / Q)) - 1) : static_cast<int>((wcet_profile / Q) );

    // Tempo de espera do RP ate sua execucao, no pior caso (toda fila possui pelo menos uma thread)
    // int rp_waiting_time = Q * (SimVars::N_QUEUES - 1) * (rp_rounds);
    int rp_waiting_time = Q * (occupied_queues(qs, new_head->object()->current_queue)) * (rp_rounds);

    // Atualiza o rank
    new_head->object()->pe->rank(rp_waiting_time);
    // Atualiza threads anterios
    assure_behind(new_head);
    
}

/* Troca de turnos circular entre as filas de perfis
* */
void round_profile_simulation(int max_change)
{
    // TSC_Chronometer chronometer;
    int count_change = 0;

    ProfileElement * running_thread = nullptr;

    while (count_change < max_change) {
        for (size_t i = 0; i < SimVars::N_QUEUES; i++)
        {

            // Se fila vazia, pula
            if (qs[i]->empty()) continue;

            // Retira a thread da fila ordenada
            running_thread = qs[i]->head();
            qs[i]->remove(running_thread);

            // O objeto diminui seu wcet e sua deadline para simular passagem do tempo e execução
            const bool has_finished = running_thread->object()->run_quantum(i);

            for (size_t j = 0; j < SimVars::N_QUEUES; j++)
            {
                for (ProfileQueue::Iterator it = qs[j]->tail(); &(*it) != nullptr; it = it->prev())
                {
                    // Outras threads que não estão em execução passam o tempo
                    it->object()->pass_quantum();
                }
            }

            // Se a thread terminou, pula
            if (has_finished) 
            {
                end_thread(qs[i]);
                print_queues(qs, ts);
                continue;
            };

            // Se a thread não terminou, reinserir na fila
            // Reposiciona thread na fila 
            qs[running_thread->object()->current_queue]->insert(running_thread);
            
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

        qs[ts[i]->current_queue]->insert(ts[i]->pe);
        assure_behind(ts[i]->pe);

        print_queues(qs, ts);
        
        // para simular o escalonamento em conjunto a novas threads 
        round_profile_simulation(2);
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
    round_profile_simulation(20);
    clean();
    return 0;
}
