/*
 * Copyright (c) 2018, SyedW
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

 /*
  * File:   DH_ThreadedObject.hpp
  * Author: phytress
  *
  * Created on July 9, 2017, 1:22 PM
  */

#pragma once

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace DigitalHaze {

	class ThreadLockedObject {
	public:
		ThreadLockedObject() noexcept;
		~ThreadLockedObject();

		inline void LockObject() {
			cs_mutex.lock();
		}

		inline void UnlockObject() {
			cs_mutex.unlock();
		}

		inline void WaitOnCondition(std::condition_variable& cond) {
			std::unique_lock<std::mutex> ulock(cs_mutex);
			cond.wait(ulock);
		}
	private:
		std::mutex cs_mutex;
	};

	class ThreadSpinLockedObject {
	public:
		ThreadSpinLockedObject() noexcept;
		~ThreadSpinLockedObject();

		inline void LockObject() {
			while(lock.test(std::memory_order_acquire)) {
#if defined(__cpp_lib_atomic_flag_test)
				while(lock.test(std::memory_order_relaxed));
#endif
			}
		}

		inline void UnlockObject() {
			lock.clear(std::memory_order_release);
		}
	private:
		std::atomic_flag lock;
	};
}