#include <utility/ostream.h>
#include <process.h>
#include <system/types.h>
#include <utility/queue.h>
#include <utility/random.h>

using namespace EPOS;
OStream cout;


const unsigned int BASE_WCET = 20000;
const unsigned int DEADLINE_CAP = BASE_WCET + 200000;
const unsigned int N_QUEUES = 4;
const unsigned int Q = Traits<Thread>::QUANTUM;
const unsigned int N_THREADS = 10;

char _counter = 'a';

struct DummyThread;
struct Optimal;

typedef Ordered_List<DummyThread *, List_Element_Rank, List_Elements::Doubly_Linked_Ordered<DummyThread, List_Element_Rank>> ProfileQueue;
typedef ProfileQueue::Element ProfileElement;
typedef ProfileQueue::Rank_Type ProfileRank;


int expected_wcet(const unsigned int profile_queue)
{   
    const unsigned int rand = static_cast<unsigned>(Random::random()) % 20000;
    // return (BASE_WCET + rand) + (profile_queue * rand);
    return (BASE_WCET + rand) + (profile_queue * 10000);
}
struct DummyThread{
    typedef Thread::State State;

    unsigned int d;
    State s;
    ProfileRank rank;
    char l;
    unsigned int cwt; // current waiting time
    unsigned int wcet_remaining[N_QUEUES];

    DummyThread(unsigned int deadline, State st = State::WAITING) : d(deadline), s(st), rank(-1), l(_counter++), cwt(0) 
    {
        for (unsigned int i = 0; i < N_QUEUES; i++)
        {
            wcet_remaining[i] = expected_wcet(i);
        }
    }
};

struct Optimal{
    unsigned int queue_num;
    unsigned int cwt;

    Optimal(unsigned int q_num = -1, unsigned int cwt = 0) : queue_num(q_num), cwt(cwt) {}
};



void print_queues(ProfileQueue* (&qs)[N_QUEUES], DummyThread* (&ts)[N_THREADS])
{
    cout << '-' << endl;
    for (size_t i = 0; i < N_QUEUES; i++)
    {   
        cout << "Q" << i;
        // qs[i]->end()->object()->l retora nada por algum motivo -> tail() funciona 
        for (ProfileQueue::Iterator it = qs[i]->tail(); &(*it) != nullptr; it = it->prev())
        {
            cout << " | " << it->object()->l << ": "<< it->object()->cwt;
        }
        cout << " | " << endl;
    }
    cout << endl;
}