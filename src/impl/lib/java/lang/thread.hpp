#include "decl/lib/java/lang/thread.hpp"

#include "decl/object.hpp"
#include "classes.hpp"
#include "native/call.hpp"
#include "native/environment.hpp"
#include "execution/stack.hpp"
#include "execution/thread.hpp"
#include "thrown.hpp"
#include "execute.hpp"

#include <posix/thread.hpp>

inline reference create_thread() {
	reference ref = create_object(thread_class.get());
	method& constructor = ref._class().instance_methods().find(
		c_string{ "<init>" }, c_string{ "()V" }
	);
	stack.emplace_back(ref);
	execute(constructor);
	return ref;
}

inline reference create_thread(reference runnable) {
	reference ref = create_object(thread_class.get());
	method& constructor = ref._class().instance_methods().find(
		c_string{ "<init>" }, c_string{ "(Ljava/lang/Runnable;)V" }
	);
	stack.emplace_back(ref);
	stack.emplace_back(move(runnable));
	execute(constructor);
	return ref;
}

static void on_thread_exit() {
	if(!thrown.is_null()) {
		reference thrown0 = move(thrown);
		print::err("unhandled throwable\n");

		method& print_stack_trace = thrown0->_class().instance_methods().find(
			c_string{ "printStackTrace" }, c_string{ "()V" }
		);

		stack.emplace_back(thrown0);
		execute(print_stack_trace);
	}
	thread = reference{};
}

static void* thread_start(void* arg) {
	thread = *(reference*)arg;
	posix::free_raw_memory(arg);

	reference& runnable
		= thread->get<reference>(thread_runnable_field_position);
	stack.emplace_back(runnable);
	
	execute(runnable->_class().declared_instance_methods().find(
		c_string{ "run" }, c_string{ "()V" }
	));

	on_thread_exit();

	return nullptr;
}

inline void init_java_lang_thread() {
	thread_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ "java/lang/Thread" }
	);

	thread_runnable_field_position
		= thread_class->instance_field_position(
			c_string{ "runnable_" }, c_string{ "Ljava/lang/Runnable;" }
		);
	
	thread_class->declared_instance_methods().find(
		c_string{ "start" }, c_string{ "()V" }
	).native_function((void*)+[](native_environment*, object* ths) {
		span<reference> ref = posix::allocate_raw_memory_of<reference>(1);
		new (ref.iterator()) ::reference(*ths);

		posix::create_thread(thread_start, ref[0]);
	});
}