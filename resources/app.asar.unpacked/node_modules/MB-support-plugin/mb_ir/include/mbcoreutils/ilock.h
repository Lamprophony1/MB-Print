#ifndef MB_ILOCK_H_
#define MB_ILOCK_H_

namespace MakerBot {

	class ILock {
	public:
		ILock() {
		}

		virtual ~ILock() {
		}

		virtual void acquire() = 0;
		virtual void release() = 0;
	};

}

#endif