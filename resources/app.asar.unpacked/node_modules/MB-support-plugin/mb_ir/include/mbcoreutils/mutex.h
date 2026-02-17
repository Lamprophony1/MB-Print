#ifndef MB_MUTEX_H_
#define MB_MUTEX_H_

#include <mutex>

#include "ilock.h"

namespace MakerBot {

	class Mutex : public ILock {
	public:

		Mutex() : ILock() {
		}

		virtual ~Mutex() {
		}

		virtual void acquire() {
			m_theMutex.lock();
		}

		virtual void release() {
			m_theMutex.unlock();
		}

		std::mutex& getUnderlying() {
			return m_theMutex;
		}

	private:

		std::mutex m_theMutex;
	};


}

#endif
