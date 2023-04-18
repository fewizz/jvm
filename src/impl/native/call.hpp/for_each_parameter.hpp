#pragma once

#include "method.hpp"
#include "execution/stack.hpp"

#include <overloaded.hpp>

template<typename Handler>
void for_each_parameter(method& m, nuint stack_begin, Handler&& handler) {
	nuint stack_at = stack_begin;
	handler((void*) nullptr);

	if(!m.is_static()) { // this
		reference& r = stack.get<reference>(stack_at);
		object* ptr = r.object_ptr();
		handler(ptr);
		stack_at += 1;
	}

	for(one_of_descriptor_parameter_types pt : m.parameter_types()) {
		pt.view_type(overloaded {
			[&]<same_as_any<class_file::object, class_file::array>> {
				reference& r = stack.get<reference>(stack_at);
				object* ptr = r.object_ptr();
				handler(ptr);
				stack_at += 1;
			},
			[&]<same_as<class_file::j>> {
				int64 v = stack.get<int64>(stack_at);
				handler(v);
				stack_at += 2;
			},
			[&]<same_as_any<
				class_file::z, class_file::b, class_file::c,
				class_file::s, class_file::i
			>> {
				int32 v = stack.get<int32>(stack_at);
				handler(v);
				stack_at += 1;
			},
			[&]<same_as<class_file::f>> {
				float v = stack.get<float>(stack_at);
				handler(v);
				stack_at += 1;
			},
			[&]<same_as<class_file::d>> {
				double v = stack.get<double>(stack_at);
				handler(v);
				stack_at += 2;
			}
		});
	}
}