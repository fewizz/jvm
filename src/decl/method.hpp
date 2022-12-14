#pragma once

#include "./method/code.hpp"
#include "./parameters_count.hpp"
#include "./class/member.hpp"
#include "./descriptor_types.hpp"

#include <class_file/access_flag.hpp>
#include <class_file/constant.hpp>
#include <class_file/descriptor/type.hpp>
#include <class_file/descriptor/method_reader.hpp>
#include <class_file/attribute/code/exception_handler.hpp>

#include <span.hpp>
#include <variant.hpp>
#include <optional.hpp>
#include <list.hpp>
#include <c_string.hpp>

#include <posix/memory.hpp>

struct native_function_ptr {
private:
	void* value_;
public:
	native_function_ptr(void* value) : value_{ value } {}

	operator void*& () & { return value_; }
};

using code_or_native_function_ptr =
	variant<code, optional<native_function_ptr>>;

using exception_handlers = list<
	posix::memory_for_range_of<class_file::attribute::code::exception_handler>
>;

struct method : class_member {
private:

	code_or_native_function_ptr code_;
	exception_handlers exception_handlers_;
	uint8 parameters_stack_size_ = 0;
	posix::memory_for_range_of<one_of_non_void_descriptor_types>
		parameter_types_;
	one_of_descriptor_types return_type_{ class_file::v{} };

public:

	method(
		class_file::access_flags               access_flags,
		class_file::constant::utf8             name,
		class_file::constant::utf8             descriptor,
		code_or_native_function_ptr            code,
		exception_handlers&&                   exception_handlers
	) :
		class_member       { access_flags, name, descriptor },
		code_              { code                           },
		exception_handlers_{ move(exception_handlers)       }
	{
		class_file::method_descriptor::reader reader{ descriptor.iterator() };
		uint8 parameter_count = reader.try_read_parameters_count(
			[]([[maybe_unused]] auto err) { abort(); }
		).get();

		list parameter_types_list =
			posix::allocate_memory_for<one_of_non_void_descriptor_types>(
				parameter_count
			);

		auto return_type_reader
			= reader.try_read_parameter_types_and_get_return_type_reader(
				[&]<typename ParamType>(ParamType parameter_type) {
					if constexpr(same_as<ParamType, class_file::v>) {
						__builtin_unreachable();
					}
					else {
						parameter_types_list.emplace_back(parameter_type);
					}
				},
				[](auto) { abort(); }
			).get();

		return_type_reader.try_read_and_get_advanced_iterator(
			[&](auto ret_type) {
				return_type_ = ret_type;
			},
			[](auto) { abort(); }
		);

		if(!access_flags._static) {
			++parameters_stack_size_; // this
		}
		for(one_of_non_void_descriptor_types t : parameter_types_list) {
			t.view([&]<typename Type>(Type) {
				if constexpr(
					same_as<Type, class_file::j> ||
					same_as<Type, class_file::d>
				) {
					parameters_stack_size_ += 2;
				}
				else {
					parameters_stack_size_ += 1;
				}
			});
		}
		parameter_types_ = parameter_types_list.move_storage_range();
	}

	uint8 parameters_stack_size() const { return parameters_stack_size_; }

	auto parameter_types() const {
		return parameter_types_.as_span();
	}

	parameters_count parameters_count() {
		auto count = parameter_types_.size();
		return ::parameters_count{ (uint8) count };
	}

	code code() const { return code_.get_same_as<::code>(); }

	exception_handlers& exception_handlers() {
		return exception_handlers_;
	}

	bool is_native() const {
		return access_flags().native;
	}

	bool native_function_is_loaded() const {
		return code_.get_same_as<optional<native_function_ptr>>().has_value();
	}

	void native_function(native_function_ptr function) {
		if(!code_.is_same_as<optional<native_function_ptr>>()) {
			abort();
		}
		code_ = function;
	}

	native_function_ptr native_function() {
		return code_.get_same_as<optional<native_function_ptr>>().get();
	}

	one_of_descriptor_types return_type() const {
		return return_type_;
	}

	bool is_void() const {
		return return_type_.is_same_as<class_file::v>();
	}

	bool is_instance_initialisation() const;

};