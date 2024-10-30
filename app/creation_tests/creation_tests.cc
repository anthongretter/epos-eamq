#include <process.h>
#include <time.h>
#include <real-time.h>
#include <synchronizer.h>
#include <utility/spin.h>
#include <utility/random.h>

using namespace EPOS;

const int thread_periodic = 16;
const int thread_aperiodic = 16;

int count = 0;

Mutex mutex;
OStream cout;
Thread * threads_A[thread_aperiodic];
Thread * threads_P[thread_periodic];

int rand_range(int start, int stop);
int simple_task();

void print();

void aperiodic_threads_with_long_task();
void creating_aperiodic_threads();
void creating_periodic_threads();

int rand_range(int start, int stop)
{
    return start + (Random::random() % stop);
}

int main()
{
    cout << "Running multicore tests with " << CPU::cores() << " cores:" << endl;
    
    creating_aperiodic_threads();
    creating_periodic_threads();
    aperiodic_threads_with_long_task();


    return 0;
}

void print() {
    mutex.lock();
    cout << "CPU: " << CPU::id() <<  " Thread: " << Thread::self() << " printing: " <<  count++ << endl;
    mutex.unlock();
}

int simple_task() {
    print();
    return 0;
}

int iterate_task(int iterations) {
    int i = 0;
    do {
        print();
        i++;
    } while (i < iterations);

    return 0;
}

void creating_aperiodic_threads() {
    cout << "Test 1: Creating multiple aperiodic threads " << endl;
    for (int i = 0; i < thread_aperiodic; i++) {
        threads_A[i] = new Thread(&simple_task);
    }

    for (int i = 0; i < thread_aperiodic; i++) {
        int status = threads_A[i]->join();
        delete threads_A[i];
        if (status != 0) {
            cout << "Test 1: Failed" << endl;
            return;
        }
    }

    cout << "Test 1: Passed\n" << endl;
}

void creating_periodic_threads() {
    cout << "Test 2: Creating multiple periodic threads " << endl;
    int p = rand_range(500000, 1000000);
    int d = rand_range(100000, 200000);

    for (int i = 0; i < thread_periodic; i++) {
        auto conf = Periodic_Thread::Configuration(
            p,        // periodo
            d,          // deadline
            Periodic_Thread::UNKNOWN,
            Periodic_Thread::NOW,
            2
        );
        threads_P[i] = new Periodic_Thread(conf, &iterate_task, i);
    }

    for (int i = 0; i < thread_periodic; i++) {
        int status = threads_P[i]->join();
        delete threads_P[i];
        if (status != 0) {
            cout << "Test 2: Failed" << endl;
            return;
        }
    }

    cout << "Test 2: Passed\n" << endl;
}


void aperiodic_threads_with_long_task() {
    cout << "Test 3: Creating threads with long tasks" << endl;
    for (int i = 0; i < thread_aperiodic; i++) {
        int iterations = rand_range(100, 250);
        threads_A[i] = new Thread(&iterate_task, iterations);
    }

    for (int i = 0; i < thread_aperiodic; i++) {
        int status = threads_A[i]->join();
        delete threads_A[i];
        if (status != 0) {
            cout << "Test 3: Failed" << endl;
            return;
        }
    }

    cout << "Test 3: Passed\n" << endl;
}
