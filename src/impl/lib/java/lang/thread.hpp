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
	expected<reference, reference> possible_ref
		= try_create_object(thread_class.get());
	
	if(possible_ref.is_unexpected()) {
		return unexpected{ move(possible_ref.get_unexpected()) };
	}

	reference ref = move(possible_ref.get_expected());

	method& constructor = ref._class().instance_methods().find(
		c_string{ "<init>" }, c_string{ "()V" }
	);
	stack.emplace_back(ref);
	optional<reference> possible_throwable = try_execute(constructor);
	if(possible_throwable.has_value()) {
		return unexpected{ move(possible_throwable.get()) };
	}

	return ref;
}

inline expected<reference, reference> try_create_thread(reference runnable) {
	expected<reference, reference> possible_ref
		= try_create_object(thread_class.get());
	
	if(possible_ref.is_unexpected()) {
		return unexpected{ move(possible_ref.get_unexpected()) };
	}

	reference ref = move(possible_ref.get_expected());

	method& constructor = ref._class().instance_methods().find(
		c_string{ "<init>" }, c_string{ "(Ljava/lang/Runnable;)V" }
	);
	stack.emplace_back(ref);
	stack.emplace_back(move(runnable));
	optional<reference> possible_throwable = try_execute(constructor);
	if(possible_throwable.has_value()) {
		return unexpected{ move(possible_throwable.get()) };
	}

	return ref;
}

static void on_thread_exit(optional<reference> possible_throwable) {
	if(possible_throwable.has_value()) {
		reference thrown0 = move(possible_throwable.get());
		print::err("unhandled throwable: \n");

		method& print_stack_trace = thrown0->_class().instance_methods().find(
			c_string{ "printStackTrace" }, c_string{ "()V" }
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
		= try_execute(runnable->_class().declared_instance_methods().find(
			c_string{ "run" }, c_string{ "()V" }
		));

	on_thread_exit(move(possible_throwable));

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