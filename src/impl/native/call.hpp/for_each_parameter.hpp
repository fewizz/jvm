#pragma once

#include "method.hpp"
#include "execution/stack.hpp"

#include <overloaded.hpp>

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
		pt.view_type(overloaded {
			[&]<same_as<class_file::object, class_file::array>> {
				reference& r = stack.at<reference>(stack_at);
				handler(r);
				stack_at += 1;
			},
			[&]<same_as<class_file::j>> {
				int64 v = stack.at<int64>(stack_at);
				handler(v);
				stack_at += 2;
			},
			[&]<same_as<
				class_file::z, class_file::b, class_file::c,
				class_file::s, class_file::i
			>> {
				int32 v = stack.at<int32>(stack_at);
				handler(v);
				stack_at += 1;
			},
			[&]<same_as<class_file::f>> {
				float v = stack.at<float>(stack_at);
				handler(v);
				stack_at += 1;
			},
			[&]<same_as<class_file::d>> {
				double v = stack.at<double>(stack_at);
				handler(v);
				stack_at += 2;
			},
			[&]<typename> { abort(); }
		});
	}
}