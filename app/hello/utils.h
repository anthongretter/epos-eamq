#include <utility/ostream.h>
#include <process.h>
#include <system/types.h>
#include <utility/queue.h>

using namespace EPOS;
OStream cout;


const unsigned int BASE_WCET = 10;
const unsigned int DEADLINE_CAP = 10;
const unsigned int N_QUEUES = 4;
const unsigned int Q = Traits<Thread>::QUANTUM;
const unsigned int N_THREADS = 5;

char _counter = 'a';


struct DummyThread;
struct Optimal;

typedef Ordered_List<DummyThread *, List_Element_Rank, List_Elements::Doubly_Linked_Ordered<DummyThread, List_Element_Rank>> ProfileQueue;
typedef ProfileQueue::Element ProfileElement;
typedef ProfileQueue::Rank_Type ProfileRank;

struct DummyThread{
    typedef Thread::State State;

    unsigned int d;
    State s;
    ProfileRank rank;
    char l;

    DummyThread(unsigned int deadline, State st = State::WAITING) : d(deadline), s(st), rank(-1), l(_counter++) {}
};


struct Optimal{
    unsigned int queueNum;
    Microsecond time_deadline;

    Optimal(unsigned int qNum, Microsecond t) : queueNum(qNum), time_deadline(t) {}
};



Microsecond expected_wcet(const unsigned int profile_queue)
{
    return Microsecond((Random::random() % BASE_WCET) + (profile_queue * 2));
}

void print_queues(ProfileQueue* (&qs)[N_QUEUES], DummyThread* (&ts)[N_THREADS])
{
    cout << '-' << endl;
    for (size_t i = 0; i < N_QUEUES; i++)
    {   
        cout << "Q" << i;
        for (ProfileQueue::Iterator it = qs[i]->end(); &(*it) != nullptr; i++)
        {
            cout << " | " << it->object()->l;
        }
        cout << " | " << endl;
    }
    cout << endl;
}