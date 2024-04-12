#include "decl/lib/java/lang/thread.hpp"

#include "decl/object.hpp"
#include "classes.hpp"
#include "native/environment.hpp"
#include "execution/stack.hpp"
#include "execution/thread.hpp"
#include "execute.hpp"

#include <posix/thread.hpp>

inline expected<reference, reference> try_create_thread() {
	instance_method& constructor = thread_class->instance_methods().find(
		u8"<init>"s, u8"()V"s
	);
	
	return try_create_object(constructor);
}

inline expected<reference, reference> try_create_thread(reference runnable) {
	instance_method& constructor = thread_class->instance_methods().find(
		u8"<init>"s, u8"(Ljava/lang/Runnable;)V"s
	);
	return try_create_object(constructor, move(runnable));
}

static void on_thread_exit(optional<reference> possible_throwable) {
	if(possible_throwable.has_value()) {
		reference thrown0 = possible_throwable.move();
		print::err("unhandled throwable: \n");

		method& print_stack_trace = thrown0->c().instance_methods().find(
			u8"printStackTrace"s, u8"()V"s
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
			u8"run"s, u8"()V"s
		));

	on_thread_exit(move(possible_throwable));

	return nullptr;
}

inline void init_java_lang_thread() {
	thread_class = classes.load_class_by_bootstrap_class_loader(
		u8"java/lang/Thread"s
	);

	thread_runnable_field_position
		= thread_class->instance_field_position(
			u8"runnable_"s, u8"Ljava/lang/Runnable;"s
		);
	
	thread_class->declared_instance_methods().find(
		u8"start"s, u8"()V"s
	).native_function((void*)+[](native_environment*, object* ths) {
		auto ref = posix::allocate_raw<::reference>(1);

		new (ref.iterator()) ::reference(*ths);

		posix::create_thread(thread_start, ref[0]);
	});

	thread_class->declared_static_methods().find(
		u8"sleep"s, u8"(J)V"s
	).native_function((void*)+ [](native_environment*, int64 millis) {
		posix::nanosleep(
			posix::seconds_and_nanoseconds {
				millis / uint64(1'000),
				uint32(millis % 1'000) * uint32(1'000'000ll)
			},
			[](auto){ posix::abort(); }
		);
	});
}