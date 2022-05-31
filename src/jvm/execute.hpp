#pragma once

#include "objects.hpp"

inline field_value execute(method& m, span<field_value, uint16> args = {});

#include "class/file/reader.hpp"
#include "class/file/descriptor/reader.hpp"
#include "../abort.hpp"

#include <stdio.h>
#include <core/number.hpp>

inline static constexpr bool info = true;
inline static nuint tab = 0;

inline void tabs() {
	for(nuint i = 1; i < tab; ++i) fputc('\t', stderr);
}

field_value execute(method& m, span<field_value, uint16> args) {
	_class& c = m._class();

	if(info) {
		++tab;
	}

	class_file::attribute::code::reader<
		uint8*,
		class_file::attribute::code::reader_stage::code
	> reader{ m.code().data() };

	field_value stack[m.code().max_stack];
	nuint stack_size = 0;

	field_value local[m.code().max_locals];
	for(int i = 0; i < args.size(); ++i) local[i] = move(args[i]);

	using namespace class_file::code::instruction;
	namespace cc = class_file::constant;

	field_value result;

	reader([&]<typename Type>(Type x, uint8*& pc) {
		if constexpr (same_as<Type, nop>) {}
		else if constexpr (same_as<Type, i_const_0>) {
			if(info) {
				tabs();
				fputs("i_const_0\n", stderr);
			}
			stack[stack_size++] = jint{ 0 };
		}
		else if constexpr (same_as<Type, i_const_1>) {
			if(info) {
				tabs();
				fputs("i_const_1\n", stderr);
			}
			stack[stack_size++] = jint{ 1 };
		}
		else if constexpr (same_as<Type, i_const_2>) {
			if(info) {
				tabs();
				fputs("i_const_2\n", stderr);
			}
			stack[stack_size++] = jint{ 2 };
		}
		else if constexpr (same_as<Type, i_const_3>) {
			if(info) {
				tabs();
				fputs("i_const_3\n", stderr);
			}
			stack[stack_size++] = jint{ 3 };
		}
		else if constexpr (same_as<Type, i_const_4>) {
			if(info) {
				tabs();
				fputs("i_const_4\n", stderr);
			}
			stack[stack_size++] = jint{ 4 };
		}
		else if constexpr (same_as<Type, i_const_5>) {
			if(info) {
				tabs();
				fputs("i_const_5\n", stderr);
			}
			stack[stack_size++] = jint{ 5 };
		}
		else if constexpr (same_as<Type, bi_push>) {
			stack[stack_size++] = jint{ x.value };
		}
		else if constexpr (same_as<Type, ldc>) {
			auto v = m._class().int32_constant(x.index); // TODO
			stack[stack_size++] = jint{ v.value }; // even if it is float
		}
		else if constexpr (same_as<Type, i_load_0>) {
			if(info) {
				tabs(); fputs("i_load_0\n", stderr);
			}
			stack[stack_size++] = local[0].get<jint>();
		}
		else if constexpr (same_as<Type, i_load_1>) {
			if(info) {
				tabs(); fputs("i_load_1\n", stderr);
			}
			stack[stack_size++] = local[1].get<jint>();
		}
		else if constexpr (same_as<Type, a_load_0>) {
			if(info) {
				tabs(); fputs("a_load_0\n", stderr);
			}
			stack[stack_size++] = local[0].get<reference>();
		}
		else if constexpr (same_as<Type, a_load_1>) {
			if(info) {
				tabs(); fputs("a_load_1\n", stderr);
			}
			stack[stack_size++] = local[1].get<reference>();
		}
		else if constexpr (same_as<Type, i_store_0>) {
			if(info) {
				tabs(); fputs("i_store_0\n", stderr);
			}
			local[0] = stack[--stack_size].get<jint>();
		}
		else if constexpr (same_as<Type, i_store_1>) {
			if(info) {
				tabs(); fputs("i_store_1\n", stderr);
			}
			local[1] = stack[--stack_size].get<jint>();
		}
		else if constexpr (same_as<Type, a_store_0>) {
			if(info) {
				tabs(); fputs("a_store_0\n", stderr);
			}
			local[0] = move(stack[--stack_size].get<reference>());
		}
		else if constexpr (same_as<Type, a_store_1>) {
			if(info) {
				tabs(); fputs("a_store_1\n", stderr);
			}
			local[1] = move(stack[--stack_size].get<reference>());
		}

		else if constexpr (same_as<Type, dup>) {
			if(info) {
				tabs(); fputs("dup\n", stderr);
			}
			auto& top = stack[stack_size - 1];
			stack[stack_size++] = top;
		}

		else if constexpr (same_as<Type, i_add>) {
			if(info) {
				tabs(); fputs("i_add\n", stderr);
			}
			jint value2 = stack[--stack_size].get<jint>();
			jint value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1.value + value2.value };
		}
		else if constexpr (same_as<Type, i_sub>) {
			if(info) {
				tabs(); fputs("i_sub\n", stderr);
			}
			jint value2 = stack[--stack_size].get<jint>();
			jint value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1.value - value2.value };
		}
		else if constexpr (same_as<Type, i_mul>) {
			if(info) {
				tabs(); fputs("i_mul\n", stderr);
			}
			jint value2 = stack[--stack_size].get<jint>();
			jint value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1.value * value2.value };
		}
		else if constexpr (same_as<Type, i_div>) {
			if(info) {
				tabs(); fputs("i_div\n", stderr);
			}
			jint value2 = stack[--stack_size].get<jint>();
			jint value1 = stack[--stack_size].get<jint>();
			stack[stack_size++] = jint{ value1.value / value2.value };
		}
		else if constexpr (same_as<Type, i_inc>) {
			if(info) {
				tabs(); fputs("i_inc\n", stderr);
			}
			local[x.index].template get<jint>().value += x.value;
		}
		else if constexpr (same_as<Type, if_i_cmp_ge>) {
			jint value2 = stack[--stack_size].get<jint>();
			jint value1 = stack[--stack_size].get<jint>();
			if(value1.value >= value2.value) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, if_i_cmp_gt>) {
			jint value2 = stack[--stack_size].get<jint>();
			jint value1 = stack[--stack_size].get<jint>();
			if(value1.value > value2.value) {
				pc += x.branch - sizeof(int16) - sizeof(uint8);
			}
		}
		else if constexpr (same_as<Type, go_to>) {
			pc += x.branch - sizeof(int16) - sizeof(uint8);
		}
		else if constexpr (same_as<Type, i_ret>) {
			if(info) {
				tabs(); fputs("i_ret\n", stderr);
			}
			result = stack[--stack_size].get<jint>();
			return true;
		}
		else if constexpr (same_as<Type, ret>) {
			if(info) {
				tabs(); fputs("ret\n", stderr);
			}
			return true;
		}
		else if constexpr (same_as<Type, get_static>) {
			field& f = c.get_static_field(x.index);
			auto& val = ((static_field*)&f)->value();
			if(val.is<jint>()) {
				stack[stack_size++].get<jint>() = val.get<jint>();
			}
			else {
				fputs("unknown static", stderr);
				abort();
			}
		}
		else if constexpr (same_as<Type, put_static>) {
			field& f = c.get_static_field(x.index);
			auto& val = ((static_field*)&f)->value();
			if(val.is<jint>()) {
				val.get<jint>() = stack[--stack_size].get<jint>();
			}
			else {
				fputs("unknown static", stderr);
				abort();
			}
		}
		else if constexpr (same_as<Type, get_field>) {
			if(info) {
				tabs(); fputs("get_field ", stderr);
				auto field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat = c.name_and_type_constant(
					field_ref.name_and_type_index
				);
				auto class_ = c.class_constant(field_ref.class_index);
				auto class_name = c.utf8_constant(class_.name_index);
				auto name = c.utf8_constant(nat.name_index);
				fwrite(class_name.data(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			auto instance_field_index {
				c.get_resolved_instance_field_index(x.index)
			};

			reference ref = move(stack[--stack_size].get<reference>());
			field_value& val = ref.object()[instance_field_index];
			stack[stack_size++] = val;
		}
		else if constexpr (same_as<Type, put_field>) {
			if(info) {
				tabs(); fputs("put_field ", stderr);
				auto field_ref = c.field_ref_constant(x.index);
				cc::name_and_type nat = c.name_and_type_constant(
					field_ref.name_and_type_index
				);
				auto class_ = c.class_constant(field_ref.class_index);
				auto class_name = c.utf8_constant(class_.name_index);
				auto name = c.utf8_constant(nat.name_index);
				fwrite(class_name.data(), 1, class_name.size(), stderr);
				fputc('.', stderr);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}

			auto instance_field_index {
				c.get_resolved_instance_field_index(x.index)
			};

			field_value value_to_put = move(stack[--stack_size]);
			reference ref = move(stack[--stack_size].get<reference>());
			field_value& val = ref.object()[instance_field_index];
			val = move(value_to_put);
		}
		else if constexpr (same_as<Type, invoke_virtual>) {
			cc::method_ref method_ref_info {
				c.method_ref_constant(x.index)
			};
			cc::name_and_type name_and_type_info {
				c.name_and_type_constant(method_ref_info.name_and_type_index)
			};

			auto name = c.utf8_constant(name_and_type_info.name_index);
			auto desc = c.utf8_constant(name_and_type_info.descriptor_index);

			if(info) {
				cc::_class class_info {
					c.class_constant(method_ref_info.class_index)
				};
				auto class_name = c.utf8_constant(class_info.name_index);
				tabs(); fputs("invoke_virtual ", stderr);
				fwrite(
					class_name.data(), 1,
					class_name.size(), stderr
				);
				fputc('.', stderr);
				fwrite(
					name.data(), 1,
					name.size(), stderr
				);
				fwrite(
					desc.data(), 1,
					desc.size(), stderr
				);
				fputc('\n', stderr);
			}

			class_file::descriptor::method_reader params_reader{ desc.begin() };
			uint16 args_count = 0;
			params_reader([&](auto){ ++args_count; return true; });

			method* m0 = nullptr;
			_class* c0 = &stack[stack_size - args_count - 1]
				.get<reference>()
				.object()
				._class();

			while(true) {
				if(m0 = c0->try_find_method(name, desc); m0 != nullptr) {
					break;
				}
				if(c0->super_class_index() == 0) {
					break;
				}
				c0 = &c0->get_class(c0->super_class_index());
			}

			if(m0 == nullptr) {
				nuint index = 0;
				c.for_each_maximally_specific_superinterface_method(
					name, desc,
					[&](method& m) {
						if(index++ == 0) {
							m0 = &m;
							return;
						}
						fputs(
							"more than one maximally-specific interface method",
							stderr
						);
						abort();
					}
				);
			}

			if(m0 == nullptr) {
				fputs("couldn't find method", stderr);
				abort();
			}

			++args_count; // this
			stack_size -= args_count;
			field_value result = execute(
				*m0, span{ stack + stack_size, args_count }
			);
			if(!result.is<jvoid>()) {
				stack[stack_size++] = result;
			}
		}
		else if constexpr (same_as<Type, invoke_special>) {
			method& m0 = c.get_resolved_method(x.index);
			auto desc = m0.descriptor();

			if(info) {
				tabs(); fputs("invoke_special ", stderr);
				fwrite(
					m0._class().name().data(), 1,
					m0._class().name().size(), stderr
				);
				fputc('.', stderr);
				fwrite(
					m0.name().data(), 1,
					m0.name().size(), stderr
				);
				fputc('\n', stderr);
			}

			class_file::descriptor::method_reader params_reader{ desc.begin() };
			uint16 args_count = 0;
			params_reader([&](auto){ ++args_count; return true; });
			++args_count; // this
			stack_size -= args_count;
			field_value result = execute(
				m0,
				span{ stack + stack_size, args_count }
			);
			if(!result.is<jvoid>()) {
				stack[stack_size++] = result;
			}
		}
		else if constexpr (same_as<Type, invoke_static>) {
			method& next_method = c.get_method(x.index);
			auto desc = next_method.descriptor();

			class_file::descriptor::method_reader params_reader{ desc.begin() };
			uint16 args_count = 0;
			params_reader([&](auto){ ++args_count; return true; });
			stack_size -= args_count;
			field_value result = execute(
				next_method,
				span{ stack + stack_size, args_count }
			);
			if(!result.is<jvoid>()) {
				stack[stack_size++] = result;
			}
		}
		else if constexpr (same_as<Type, invoke_interface>) {
			cc::method_ref method_ref_info {
				c.method_ref_constant(x.index)
			};
			cc::name_and_type name_and_type_info {
				c.name_and_type_constant(method_ref_info.name_and_type_index)
			};

			auto name = c.utf8_constant(name_and_type_info.name_index);
			auto desc = c.utf8_constant(name_and_type_info.descriptor_index);

			if(info) {
				cc::_class class_info {
					c.class_constant(method_ref_info.class_index)
				};
				auto class_name = c.utf8_constant(class_info.name_index);
				tabs(); fputs("invoke_interface ", stderr);
				fwrite(
					class_name.data(), 1,
					class_name.size(), stderr
				);
				fputc('.', stderr);
				fwrite(
					name.data(), 1,
					name.size(), stderr
				);
				fwrite(
					desc.data(), 1,
					desc.size(), stderr
				);
				fputc('\n', stderr);
			}

			uint16 args_count = x.count;

			method* m0 = nullptr;
			_class* c0 = &stack[stack_size - args_count]
				.get<reference>()
				.object()
				._class();

			while(true) {
				if(m0 = c0->try_find_method(name, desc); m0 != nullptr) {
					break;
				}
				if(c0->super_class_index() == 0) {
					break;
				}
				c0 = &c0->get_class(c0->super_class_index());
			}

			if(m0 == nullptr) {
				nuint index = 0;
				c.for_each_maximally_specific_superinterface_method(
					name, desc,
					[&](method& m) {
						if(index++ == 0) {
							m0 = &m;
							return;
						}
						fputs(
							"more than one maximally-specific interface method",
							stderr
						);
						abort();
					}
				);
			}

			if(m0 == nullptr) {
				fputs("couldn't find method", stderr);
				abort();
			}

			stack_size -= args_count;
			field_value result = execute(
				*m0, span{ stack + stack_size, args_count }
			);
			if(!result.is<jvoid>()) {
				stack[stack_size++] = result;
			}
		}
		else if constexpr (same_as<Type, _new>) {
			if(info) {
				tabs(); fputs("new ", stderr);
				auto name = c.utf8_constant(
					c.class_constant(x.index).name_index
				);
				fwrite(name.data(), 1, name.size(), stderr);
				fputc('\n', stderr);
			}
			_class& c0 = c.get_class(x.index);
			stack[stack_size++] = objects.find_free(c0);
		}
		else if constexpr (same_as<Type, uint8>) {
			fprintf(stderr, "unknown instruction ");
			for_each_digit_in_number(
				number{ x },
				base{ 10 },
				[](auto digit) { fputc('0' + digit, stderr); }
			);
			abort();
		}
		else {
			fprintf(stderr, "unimplemented instruction ");
			for_each_digit_in_number(
				number{ Type::code },
				base{ 10 },
				[](auto digit) { fputc('0' + digit, stderr); }
			);
			abort();
		}

	}, m.code().size());

	if(info) {
		--tab;
	}

	return result;
}