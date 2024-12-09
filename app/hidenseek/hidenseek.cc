#include <utility/ostream.h>
#include <process.h>
#include <time.h>
#include <real-time.h>
#include <utility/random.h>

#define SPACE_SIZE 1000

using namespace EPOS;

OStream cout;
Mutex cout_m;
Mutex space_m;

int space[SPACE_SIZE];
int total_hids = 0;
int total_found = 0;

int hider(int tid);
int seeker(int tid);


int main()
{
    cout << "MAIN: Hello world!\n" << endl;

    const int N_THREADS_P = 5; // Periodic

    int (*routines[2])(int) = {&hider, &seeker};
    Thread *ts[N_THREADS_P];

    for (int i = 0; i < N_THREADS_P; i++)
    {
        // Thread periodic
        auto conf = Periodic_Thread::Configuration(
            500000 + (unsigned(Random::random()) % 500000),            //   0.5s - 1s
            (500000 + (unsigned(Random::random()) % 500000)) * 4,
            Periodic_Thread::UNKNOWN,
            Periodic_Thread::NOW,
            5
        );
        ts[i] = new Periodic_Thread(conf, routines[i % 2], i);
    }

    for (int i = 0; i < N_THREADS_P; i++) {
        ts[i]->join();
    }

    cout << "\nMAIN: " << "hidden: " << total_hids << ", found: " << total_found << endl;
    if (total_found < total_hids) {
        cout << "MAIN: " << "what a shame :(" << endl;
    }

    cout << "MAIN: Bye!" << endl;

    return 0;
}


int hider(int tid)
{
    do {
        cout_m.lock();
        cout << "<" << CPU::id() << ">: " << "h" << tid << ": hiding..." << endl;
        cout_m.unlock();

        int random_place_to_hide = unsigned(Random::random()) % SPACE_SIZE;

        space_m.lock();
        space[random_place_to_hide] = 42;
        total_hids++;
        space_m.unlock();

        cout_m.lock();
        cout << "<" << CPU::id() << ">: " << "h" << tid << ": bye!" << endl;
        cout_m.unlock();
    } while (Periodic_Thread::wait_next());
    return 0;
}


int seeker(int tid)
{
    do {
        cout_m.lock();
        cout <<  "<" << CPU::id() << ">: " << "s" << tid << ": seeking..." << endl;
        cout_m.unlock();

        bool found = false;

        // grandes pulos de acesso para causar cache miss
        for (int i = 0; i < SPACE_SIZE; i += (unsigned(Random::random()) % (SPACE_SIZE / 10))) {
            space_m.lock();
            int searched = space[i];
            found = searched == 42;
            space[i] = 0;
            space_m.unlock();

            if (found) {
                cout_m.lock();
                total_found++;
                cout <<  "<" << CPU::id() << ">: " << "s" << tid << ": FOUND ONE! at " << i << endl;
                cout_m.unlock();
            }
        }

        cout_m.lock();
        cout <<  "<" << CPU::id() << ">: " << "s" << tid << ": bye!" << endl;
        cout_m.unlock();
    } while (Periodic_Thread::wait_next());
    return 0;
}
