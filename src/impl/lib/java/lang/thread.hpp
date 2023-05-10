#include "decl/lib/java/lang/thread.hpp"

#include "decl/object.hpp"
#include "classes.hpp"
#include "native/call.hpp"
#include "native/environment.hpp"
#include "execution/stack.hpp"
#include "execution/thread.hpp"
#include "execute.hpp"

#include <posix/thread.hpp>

inline expected<reference, reference> try_create_thread() {
	method& constructor = thread_class->instance_methods().find(
		c_string{ u8"<init>" }, c_string{ u8"()V" }
	);
	
	return try_create_object(constructor);
}

inline expected<reference, reference> try_create_thread(reference runnable) {
	method& constructor = thread_class->instance_methods().find(
		c_string{ u8"<init>" }, c_string{ u8"(Ljava/lang/Runnable;)V" }
	);
	return try_create_object(constructor, move(runnable));
}

static void on_thread_exit(optional<reference> possible_throwable) {
	if(possible_throwable.has_value()) {
		reference thrown0 = possible_throwable.move();
		print::err("unhandled throwable: \n");

		method& print_stack_trace = thrown0->c().instance_methods().find(
			c_string{ u8"printStackTrace" }, c_string{ u8"()V" }
		);

		optional<reference> death = try_execute(print_stack_trace, thrown0);
		if(death.has_value()) {
			posix::abort();
		}
	}
	thread = reference{};
}

static void* thread_start(void* arg) {
	thread = *(reference*)arg;
	posix::free_raw_memory(arg);

	reference& runnable
		= thread->get<reference>(thread_runnable_field_position);
	stack.emplace_back(runnable);

	optional<reference> possible_throwable
		= try_execute(runnable->c().declared_instance_methods().find(
			c_string{ u8"run" }, c_string{ u8"()V" }
		));

	on_thread_exit(move(possible_throwable));

	return nullptr;
}

inline void init_java_lang_thread() {
	thread_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/Thread" }
	);

	thread_runnable_field_position
		= thread_class->instance_field_position(
			c_string{ u8"runnable_" }, c_string{ u8"Ljava/lang/Runnable;" }
		);
	
	thread_class->declared_instance_methods().find(
		c_string{ u8"start" }, c_string{ u8"()V" }
	).native_function((void*)+[](native_environment*, object* ths) {
		span<reference> ref = posix::allocate_raw_memory_of<reference>(1);
		new (ref.iterator()) ::reference(*ths);

		posix::create_thread(thread_start, ref[0]);
	});
}