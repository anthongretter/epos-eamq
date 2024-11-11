#include <utility/ostream.h>
#include <process.h>
#include <time.h>
#include <real-time.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

const int N_THREADS_A = 16; // Aperiodic
const int SEARCH_N = 1000;

int primes_found = 0;
Semaphore m;

struct aperiodic_test
{
  int i;
  int begin;
  int end;

  aperiodic_test(int i, int begin, int end) : i(i), begin(begin), end(end) {}
};


bool isPrime(const int n) {

  // 0 and 1 are not prime numbers
  if (n == 1) {
    return false;
  }

  // loop to check if n is prime
  for (int i = 2; i*i <= n; i++) {
    if (n % i == 0) {
      return false;
    }
  }

  return true;
}


int work(aperiodic_test *struct_test)
{
    int begin = struct_test->begin;
    int end = struct_test->end;

    int found = 0;
    for (int i = begin; i <= end; i++)
    {
        found += int(isPrime(i));
    }

    if (found) {
      m.p();
      primes_found += found;
      m.v();
    }

    // cout << "WORK " << t << ": Bye! " << "I found " << found << " primes!" << endl;
    return 0;
}


int main()
{
    cout << "MAIN: Hello world!" << endl;
    cout << "MAIN: Searching number of primes in between 0 - " << SEARCH_N << "\n" << endl;

    const Thread::Criterion CRITS[3]{Thread::LOW, Thread::NORMAL, Thread::HIGH};


    Thread *ts[N_THREADS_A];
    int search_range = (SEARCH_N + (N_THREADS_A - 1)) / N_THREADS_A;
    int start = 1;

    int i = 0;
    for (; i < N_THREADS_A; i++)
    {
        if (start >= SEARCH_N) break;

        int end = (start + search_range) < SEARCH_N 
                      ? start + search_range
                      : SEARCH_N;
        
        aperiodic_test *work_struct = new aperiodic_test(i, start, end);
        cout << "WORK " << i << " will search " << start << " - " << end << endl;
        auto conf = Thread::Configuration(Thread::READY, CRITS[i % 3]);
        ts[i] = new Thread(conf, &work, work_struct);
        start += search_range + 1;
    }

    for (int j = 0; j < i; j++) {
        ts[j]->join();
    }

    cout << "\nMAIN: We found " << primes_found << " prime numbers!" << endl;
    cout << "MAIN: Test is going down!" << endl;

    return 0;
}
