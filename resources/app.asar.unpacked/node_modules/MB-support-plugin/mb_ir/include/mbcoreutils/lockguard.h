#ifndef MB_LOCKGUARD_H_
#define MB_LOCKGUARD_H_

namespace MakerBot {

	template<class ILockType>
	class LockGuard {
	public:

		LockGuard(ILockType* lock) : 
			m_lockPtr(lock) {
				if (m_lockPtr != NULL)
					m_lockPtr->acquire();
		}

		~LockGuard() {
			if (m_lockPtr != NULL)
				m_lockPtr->release();
		}

	private:
		ILockType* m_lockPtr;
	};
}

#endif