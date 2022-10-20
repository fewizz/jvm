#pragma once

#include "method.hpp"
#include "execution/stack.hpp"

template<typename Handler>
void for_each_parameter(method& m, nuint stack_begin, Handler&& handler) {
	nuint stack_at = stack_begin;
	handler((void*) nullptr);

	if(!m.is_static()) { // this
		reference& r = stack.at<reference>(stack_at);
		handler(r);
		stack_at += 1;
	}

	for(one_of_non_void_descriptor_types pt : m.parameter_types()) {
		pt.view([&]<typename Type>(Type) {
			if constexpr(
				same_as<Type, class_file::object> ||
				same_as<Type, class_file::array>
			) {
				reference& r = stack.at<reference>(stack_at);
				handler(r);
				stack_at += 1;
			}
			else if constexpr(same_as<Type, class_file::j>) {
				int64 v = stack.at<int64>(stack_at);
				handler(v);
				stack_at += 2;
			}
			else if constexpr(
				same_as<Type, class_file::z> ||
				same_as<Type, class_file::b> ||
				same_as<Type, class_file::c> ||
				same_as<Type, class_file::s> ||
				same_as<Type, class_file::i>
			) {
				int32 v = stack.at<int32>(stack_at);
				handler(v);
				stack_at += 1;
			}
			else if constexpr(same_as<Type, class_file::f>) {
				float v = stack.at<float>(stack_at);
				handler(v);
				stack_at += 1;
			}
			else if constexpr(same_as<Type, class_file::d>) {
				double v = stack.at<double>(stack_at);
				handler(v);
				stack_at += 2;
			} else {
				abort();
			}
		});
	}
}