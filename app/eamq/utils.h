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
struct Optimal;

typedef Ordered_List<DummyThread *, List_Element_Rank, List_Elements::Doubly_Linked_Ordered<DummyThread, List_Element_Rank>> ProfileQueue;
typedef ProfileQueue::Element ProfileElement;
typedef ProfileQueue::Rank_Type ProfileRank;

typedef Traits<Application> SimVars;


struct DummyThread{
    typedef Thread::State State;

    unsigned int d;
    State s;
    ProfileRank rank;
    char l;
    unsigned int cwt; // current waiting time
    unsigned int wcet_remaining[SimVars::N_QUEUES];

    DummyThread(unsigned int deadline, State st = State::WAITING) : d(deadline), s(st), rank(-1), l(_counter++), cwt(0) 
    {
        for (unsigned int i = 0; i < SimVars::N_QUEUES; i++)
        {
            const unsigned int rand = static_cast<unsigned>(Random::random()) % 20000;
            wcet_remaining[i] = (SimVars::BASE_WCET + rand) + (i * 10000);
        }
    }
};

struct Optimal{
    unsigned int queue_num;
    unsigned int cwt;

    Optimal(unsigned int q_num = -1, unsigned int cwt = 0) : queue_num(q_num), cwt(cwt) {}
};


void print_queues(ProfileQueue* (&qs)[SimVars::N_QUEUES], DummyThread* (&ts)[SimVars::N_THREADS])
{
    cout << '-' << endl;
    for (size_t i = 0; i < SimVars::N_QUEUES; i++)
    {   
        cout << "Q" << i;
        for (ProfileQueue::Iterator it = qs[i]->tail(); &(*it) != nullptr; it = it->prev())
        {
            cout << " | " << it->object()->l << " cwt: "<< it->object()->cwt;
        }
        cout << " | " << endl;
    }
    cout << endl;
}