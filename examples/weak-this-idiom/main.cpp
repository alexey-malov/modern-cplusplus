#include <memory>
#include <future>
#include <iostream>
#include <atomic>
#include <cstdint>
#include <vector>
#include <iterator>
#include <string>

using namespace std;
using namespace std::chrono;

class Solver : public enable_shared_from_this<Solver>
{
	typedef vector<unsigned> Factors;
	future<void> m_worker;
	atomic_bool m_cancelled;
	atomic_bool m_solved;
	Factors m_factors;
public:
	void Factorize(unsigned n)
	{
		weak_ptr<Solver> weakSelf = shared_from_this();
		m_worker = async(launch::async, [=]() mutable {
			unsigned factor = 1;
			do
			{
				auto rem = n % factor;
				if (rem == 0)
				{
					auto self = weakSelf.lock();
					if (!self)
						return;
					m_factors.push_back(factor);
					n /= factor;
					if (n == 1)
					{
						m_solved = true;
						return;
					}
				}
				if ((factor == 1) || (rem != 0))
					++factor;
			} while ((n >= factor)
				  && (weakSelf.lock() && !m_cancelled));
		});
	}

	Factors GetFactors()const
	{
		return m_solved ? m_factors : Factors();
	}

	void Cancel()
	{
		m_cancelled = true;
		if (m_worker.valid())
		{
			m_worker.get();
		}
	}
};

int main()
{
	auto solver = make_shared<Solver>();

	auto n = 1'983'872'491;
	cout << "Searching for prime factors of " << n << endl;
	solver->Factorize(n);
	
	string line;
	do
	{
		auto factors = solver->GetFactors();
		if (!factors.empty())
		{
			cout << "Factors are: \n";
			copy(factors.begin(), factors.end(), ostream_iterator<unsigned>(cout, " "));
			cout << endl;
			return 0;
		}
		cout << "Still no answer. Should I cancel (y)?";
	} while (getline(cin, line) && (line != "y"));

	cout << "Failed to solve the problem. Exiting" << endl;
	//solver->Cancel();
	return 1;
}
