#include "decl/classes.hpp"
#include "decl/execution/stack.hpp"
#include "decl/execution/thread.hpp"
#include "decl/executable_path.hpp"

#include "impl/impl.hpp"

#include <posix/default_unhandled.cpp>
#include <print/print.hpp>

int main(int argc, const char** argv) {
	if(argc != 2) {
		print::err("usage: 'class name'\n");
		return 1;
	}

	// TODO replace with somethig more reliable
	executable_path = c_string{ (const utf8::unit*) argv[0] }.sized();

	lib_path = [] {
		optional<nuint> last_slash_index =
			executable_path->
			try_find_index_of_last_satisfying([](utf8::unit ch) {
				return ch == '\\' || ch == '/';
			});

		return span {
			executable_path->iterator(),
			last_slash_index.get()
		};
	}();

	init_java_lang_object();

	void_class = classes.define_primitive_class(
		u8"void"sv, u8"V"[0]
	);

	auto define_primitive_and_its_array_class = [](
		span<const utf8::unit> name, auto array
	) -> c& {
		c& component_class = classes.define_primitive_class(
			name, array[1]
		);

		expected<c&, reference> possible_array_class =
			classes.try_define_array_class(array, nullptr);
		if(possible_array_class.is_unexpected()) {
			posix::abort();
		}

		c& array_class = possible_array_class.get_expected();
		array_class.component_class(component_class);
		component_class.array_class(array_class);

		return component_class;
	};

	bool_class = define_primitive_and_its_array_class(
		u8"boolean"sv, u8"[Z"sv
	);
	byte_class = define_primitive_and_its_array_class(
		u8"byte"sv, u8"[B"sv
	);
	short_class = define_primitive_and_its_array_class(
		u8"short"sv, u8"[S"sv
	);
	char_class = define_primitive_and_its_array_class(
		u8"char"sv, u8"[C"sv
	);
	int_class = define_primitive_and_its_array_class(
		u8"int"sv, u8"[I"sv
	);
	long_class = define_primitive_and_its_array_class(
		u8"long"sv, u8"[J"sv
	);
	float_class = define_primitive_and_its_array_class(
		u8"float"sv, u8"[F"sv
	);
	double_class = define_primitive_and_its_array_class(
		u8"double"sv, u8"[D"sv
	);

	bool_array_class   = bool_class  ->get_array_class();
	byte_array_class   = byte_class  ->get_array_class();
	short_array_class  = short_class ->get_array_class();
	char_array_class   = char_class  ->get_array_class();
	int_array_class    = int_class   ->get_array_class();
	long_array_class   = long_class  ->get_array_class();
	float_array_class  = float_class ->get_array_class();
	double_array_class = double_class->get_array_class();

	init_lib();

	auto on_exit = [&](optional<reference> possible_thrown){
		on_thread_exit(possible_thrown);
		return possible_thrown.has_value() ? -1 : 0;
	};

	expected<reference, reference> possible_thread = try_create_thread();
	if(possible_thread.is_unexpected()) {
		return on_exit(possible_thread.move_unexpected());
	}

	c& app_cl_class = classes.load_class_by_bootstrap_class_loader(
		u8"jvm/AppClassLoader"sv
	);

	app_cl_class.try_initialise_if_need()
		.if_has_value([]{ posix::abort(); });

	optional<field&> possible_app_cl_instance_field
		= try_resolve_field0(
			app_cl_class,
			u8"INSTANCE"sv,
			u8"Ljava/lang/ClassLoader;"sv
		);
	if(possible_app_cl_instance_field.has_no_value()) {
		posix::abort();
	}

	static_field& app_cl_instance_field
		= (static_field&) possible_app_cl_instance_field.get();

	reference& app_cl_ref =
		get_static_resolved<reference>(app_cl_instance_field);

	auto main_class_name = c_string{ (const utf8::unit*) argv[1] }.sized();

	expected<c&, reference> possible_c
		= classes.try_load_class(
			main_class_name,
			(j::c_loader*) app_cl_ref.object_ptr()
		);

	if(possible_c.is_unexpected()) {
		return on_exit(possible_c.move_unexpected());
	}

	c& main_c = possible_c.get_expected();
	expected<method&, reference> possible_main = try_resolve_method(
		main_c,
		main_c,
		u8"main"sv,
		u8"([Ljava/lang/String;)V"sv
	);

	if(possible_main.is_unexpected()) {
		return on_exit(possible_main.move_unexpected());
	}

	static_method& main = (static_method&) possible_main.get_expected();

	expected<reference, reference> possible_args_array
		= try_create_array_of(j::string::c.get(), 0);

	if(possible_args_array.is_unexpected()) {
		return on_exit(possible_args_array.move_unexpected());
	}

	reference args_array = possible_args_array.move_expected();

	stack.emplace_back(move(args_array));

	return on_exit(try_invoke_static_resolved(main));
}