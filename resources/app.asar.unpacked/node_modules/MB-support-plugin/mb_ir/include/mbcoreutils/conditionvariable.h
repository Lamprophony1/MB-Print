#ifndef MB_CONDITIONVARIABLE_H_
#define MB_CONDITIONVARIABLE_H_

#include <condition_variable>

namespace MakerBot {

	class ConditionVariable {
	public:

		ConditionVariable() {
		}

		virtual ~ConditionVariable() {
		}

		void notifyAll() {
			m_condVar.notify_all();
		}

		void notify() {
			m_condVar.notify_one();
		}

		void wait(MakerBot::Mutex* lock) {
			lock->release();
			m_condVar.wait(std::unique_lock<std::mutex>(lock->getUnderlying()));
		}

	private:

		std::condition_variable m_condVar;

	};
}
#endif