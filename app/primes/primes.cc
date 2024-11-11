#include <utility/ostream.h>
#include <process.h>
#include <time.h>
#include <real-time.h>
#include <utility/random.h>

using namespace EPOS;

OStream cout;

const int N_THREADS_A = 16; // Aperiodic
const int SEARCH_N = 1000000;

int primes_found = 0;
Mutex m;

struct Params
{
  int i;
  int begin;
  int end;

  Params(int i, int begin, int end) : i(i), begin(begin), end(end) {}
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


int work(Params *args)
{
    int t = args->i;
    int begin = args->begin;
    int end = args->end;
    cout << "WORK " << t << ": Hello! " << "I will search " << begin << " - " << end << endl;

    int found = 0;
    for (int i = begin; i <= end; i++)
    {
        found += int(isPrime(i));
    }

    if (found) {
      m.lock();
      primes_found += found;
      m.unlock();
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
        
        Params *args = new Params(i, start, end);

        auto conf = Thread::Configuration(Thread::READY, CRITS[i % 3]);
        cout << "i: " << i << endl;
        ts[i] = new Thread(conf, &work, args);
        start += search_range + 1;
    }

    for (int j = 0; j < i; j++) {
        ts[j]->join();
    }

    cout << "\nMAIN: We found " << primes_found << " prime numbers!" << endl;
    cout << "MAIN: Test is going down!" << endl;

    return 0;
}
