#include <utility/ostream.h>
#include <architecture.h>

using namespace EPOS;

OStream cout;

int main()
{
    long long c = PMU::read(1);
    cout << "Hello world!" << endl;
    cout << "ABSOLUTES = " << PMU::read(1) << ", " << PMU::read(0) << ", " << PMU::read(2) << endl;
    cout << "DIFFERENCE = " << PMU::read(1) - c << endl;

    return 0;
}
