#pragma once

#include "type.hpp"

#include <core/meta/elements/of.hpp>

namespace class_file::descriptor {

	template<typename Handler, nuint MaxDepth = 4>
	[[ nodiscard ]]
	bool read_field(auto&& iterator, Handler&& handler) {
		uint8 c = *iterator++;
		switch (c) {
			case 'V': return handler(descriptor::V{});
			case 'B': return handler(descriptor::B{});
			case 'C': return handler(descriptor::C{});
			case 'D': return handler(descriptor::D{});
			case 'F': return handler(descriptor::F{});
			case 'I': return handler(descriptor::I{});
			case 'J': return handler(descriptor::J{});
			case 'S': return handler(descriptor::S{});
			case 'Z': return handler(descriptor::Z{});
			case 'L': {
				auto e = iterator;
				while(*e != ';') ++e;
				bool result = handler(
					descriptor::object_type{
						(uint8*)iterator, uint16(e - iterator)
					}
				);
				iterator = ++e;
				return result;
			}
			case '[': {
				if constexpr (MaxDepth > 0) {
					auto h =
						[&](auto component) {
							return handler(
								descriptor::array_type{ component }
							);
						};
					return read_field<decltype((h)), MaxDepth - 1>(
						iterator,
						h
					);
				}
				else return false;
			}
			default:
				return false;
		}
	}

	enum class method_reader_stage {
		parameters, ret, end
	};

	template<
		typename Iterator,
		method_reader_stage Stage = method_reader_stage::parameters
	>
	struct method_reader {
		Iterator iterator;

		template<typename Handler>
		elements::of<method_reader<Iterator, method_reader_stage::ret>, bool>
		operator () (Handler&& handler) const
		requires(Stage == method_reader_stage::parameters) {
			Iterator cpy = iterator;
			if(*cpy++ != '(') return { { iterator }, false };
			while(*cpy != ')') {
				bool result = read_field(cpy, handler);
				if(!result) return { { iterator }, false };
			}
			++cpy;
			return { { cpy }, true };
		}

		elements::of<method_reader<Iterator, method_reader_stage::ret>, bool>
		skip_parameters() const
		requires(Stage == method_reader_stage::parameters) {
			return operator()([](auto){ return true; });
		}

		template<typename Handler>
		elements::of<method_reader<Iterator, method_reader_stage::end>, bool>
		operator () (Handler&& handler) const
		requires(Stage == method_reader_stage::ret) {
			Iterator cpy = iterator;
			bool result = read_field(cpy, handler);
			if(!result) return { { iterator }, false };
			return { { cpy }, true };
		}

	};

	template<typename Iterator>
	method_reader(Iterator&&) -> method_reader<Iterator>;

}