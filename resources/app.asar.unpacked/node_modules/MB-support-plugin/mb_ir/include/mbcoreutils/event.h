/*
 * event.hh
 *
 *  Created on: Apr 9, 2013
 *      Author: paul
 *
 *  Description:  Contains header definition for all Event-based classes.
 */

#ifndef EVENT_HH_
#define EVENT_HH_

#include <functional>
#include <map>
#include "mutex.h"
#include "lockguard.h"

namespace MakerBot {

	/*
	 * This class is the base class for Events.  Any new instances of Event (i.e. to expand the number of arguments that can be provided to an event)
	 * must derive from this base class.
	 *
	 * Template parameters:
	 *
	 * 	CallbackFnType - Type that defines the signature for event notification callback.
	 */
	template<class CallbackFnType>
	class EventBase {
	public:

		/*
		 * Default constructor.
		 */
		EventBase() : m_lastHandleId(0) {

		}

		/*
		 * Destructor
		 */
		virtual ~EventBase() {

		}

		// Redefine for  use in this class.
		typedef CallbackFnType FnType;

		/*
		 * Adds a callback to the list of subscribers for an event.
		 */
		virtual int subscribe(FnType& func, bool fireOnSubscribe = false) {
			(void)fireOnSubscribe;  // Unused argument

			LockGuard<Mutex> lock(&m_mutex);
			m_subscribers.insert(std::pair<int, FnType>(++m_lastHandleId, func));
			return m_lastHandleId;
		}

		/*
		 * Removes a callback from the list of subscribers
		 */
		void unsubscribe(int handle) {
			LockGuard<Mutex> lock(&m_mutex);
			m_subscribers.erase(handle);
		}

	protected:

		// Subscriber list for the event.
		std::map<int, FnType> m_subscribers;
		Mutex m_mutex;

	private:

		// Last handle ID given out to a subscriber.
		int m_lastHandleId;
	};

	/*
	 * Class for notification of subscribers to an event.  This class defines the maximum number of arguments that can be passed into an event.
	 * All other event classes that accept fewer arguments must be partial specializations of this template.
	 * If the number of arguments for an event needs to be increased, the number of template parameters on this class must be increased AND partial
	 * specializations for all new argument combinations must be provided.
	 *
	 * Template arguments:
	 *
	 * Arg0 - The first argument type for callbacks.
	 * Arg1 - The second argument type for callbacks.
	 * Arg2 - The thrid argument type for callbacks.
	 */
	template<class Arg0=void, class Arg1=void, class Arg2=void>
	class Event : public EventBase<std::function<void(Arg0, Arg1, Arg2)>> {
	public:

		/*
		 * Default constructor.
		 */
		Event() : EventBase<std::function<void(Arg0, Arg1, Arg2)>>(),
				  m_first(),
				  m_second(),
				  m_third() {

		}

		/*
		 * Sends out last cached version of event data on subscribe.
		 */
		int subscribe(std::function<void(Arg0, Arg1, Arg2)>& func, bool fireOnSubscribe = false) {
			int handleId = EventBase<std::function<void(Arg0, Arg1, Arg2)>>::subscribe(func);

			if (fireOnSubscribe)
				this->fire(m_first, m_second, m_third);

			return handleId;
		}

		/*
		 * Notifies subscribers to the event, passing to them all defined arguments.
		 */
		void fire(Arg0 first, Arg1 second, Arg2 third) {

			for( auto itSubscribers = this->m_subscribers.begin(); itSubscribers != this->m_subscribers.end(); ++itSubscribers) {
				(itSubscribers->second)(first, second, third);
			}

			// Cache the data just sent.
			m_first = first;
			m_second = second;
			m_third = third;
		}

	private:

		Arg0 m_first;
		Arg1 m_second;
		Arg2 m_third;
	};

	/*
	 * Event class that provides zero arguments on callback.
	 */
	template<>
	class Event<void, void, void> : public EventBase<std::function<void()>> {
	public:

		/*
		 * Default constructor.
		 */
		Event() : EventBase<std::function<void()>>() {

		}

		/*
		 * Notifies subscribers to the event.
		 */
		void fire() {
			for( auto itSubscribers = this->m_subscribers.begin(); itSubscribers != this->m_subscribers.end(); ++itSubscribers) {
				(itSubscribers->second)();
			}
		}
	};

	/*
	 * Event class that provides one argument on callback.
	 */
	template <class Arg0>
	class Event<Arg0, void, void> : public EventBase<std::function<void(Arg0)>>{
	public:

		/*
		 * Default constructor.
		 */
		Event() : EventBase<std::function<void(Arg0)>>(),
				  m_first(){

		}

		/*
		 * Sends out last cached version of event data on subscribe.
		 */
		int subscribe(std::function<void(Arg0)>& func, bool fireOnSubscribe = false) {
			int handleId = EventBase<std::function<void(Arg0)>>::subscribe(func);

			if (fireOnSubscribe)
				this->fire(m_first);

			return handleId;
		}


		/*
		 * Notifies subscribers to the event, passing the defined argument to each.
		 */
		void fire(Arg0 first) {
			for( auto itSubscribers = this->m_subscribers.begin(); itSubscribers != this->m_subscribers.end(); ++itSubscribers) {
				(itSubscribers->second)(first);
			}

			m_first = first;
		}

	private:

		Arg0 m_first;
	};

	/*
	 * Event class that provides two arguments on callback.
	 */
	template <class Arg0, class Arg1>
	class Event<Arg0, Arg1, void> : public EventBase<std::function<void(Arg0, Arg1)>>{
	public:

		/*
		 * Default constructor.
		 */
		Event() : EventBase<std::function<void(Arg0, Arg1)>>(),
			      m_first(),
			      m_second() {

		}

		/*
		 * Sends out last cached version of event data on subscribe.
		 */
		int subscribe(std::function<void(Arg0, Arg1)>& func, bool fireOnSubscribe = false) {
			int handleId = EventBase<std::function<void(Arg0, Arg1)>>::subscribe(func);

			if (fireOnSubscribe)
				this->fire(m_first, m_second);

			return handleId;
		}


		/*
		 * Notifies subscribers to the event, passing the defined arguments to each.
		 */
		void fire(Arg0 first, Arg1 second) {
			for( auto itSubscribers = this->m_subscribers.begin(); itSubscribers != this->m_subscribers.end(); ++itSubscribers) {
				(itSubscribers->second)(first, second);
			}

			m_first = first;
			m_second = second;
		}

	private:

		Arg0 m_first;
		Arg1 m_second;
	};

}


#endif /* EVENT_HH_ */
