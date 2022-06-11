#pragma once

struct native_function {
	void* ptr_;

	operator void* () { return ptr_; }

	inline void call();
};

inline void native_function::call() {
	
}