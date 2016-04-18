#include <memory>
#include <future>
#include <mutex>
#include <thread>
#include <boost/optional.hpp>
#include <chrono>
#include <iostream>
#include <atomic>
#include <cstdint>

using namespace std;
using namespace std::chrono;
using boost::optional;
using boost::none;

class Solver : public enable_shared_from_this<Solver>
{
	typedef pair<unsigned, unsigned> Answer;
	future<void> m_worker;
	atomic_bool m_cancelled;
	atomic<Answer*> m_pAnswer;
	Answer m_answer;
public:
	void StartSolving(unsigned product)
	{
		weak_ptr<Solver> weakSelf = shared_from_this();
		m_worker = async(launch::async, [=] {
			unsigned a = 2;
			do
			{
				auto maxB = product / a;
				for (unsigned b = a; b <= maxB; ++b)
				{
					if ((a * b == product))
					{
						auto self = weakSelf.lock();
						if (self)
						{
							m_answer = { a, b };
							m_pAnswer.store(&m_answer);
						}
						return;
					}
				}
			} while ((a++ != product) 
				&& (weakSelf.lock() && !m_cancelled));
		});
	}

	optional<Answer> GetAnswer()const
	{
		auto answer = m_pAnswer.load();
		return answer ? *answer
					  : optional<Answer>();
	}

	~Solver()
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

	auto n = 16411 * 27449;
	cout << "Searching for prime factors of " << n << endl;
	solver->StartSolving(n);
	
	string line;
	do
	{
		auto answer = solver->GetAnswer();
		if (answer)
		{
			cout << "Factors are: " << answer->first << " and " 
				<< answer->second << endl;
			return 0;
		}
		cout << "Still no answer. Should I cancel (y)?";
	} while (getline(cin, line) && (line != "y"));

	cout << "Failed to solve the problem. Exiting" << endl;

	return 1;
}
