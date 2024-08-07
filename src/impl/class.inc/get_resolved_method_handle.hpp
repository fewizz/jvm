#include "decl/class.hpp"

#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/method_type.hpp"
#include "decl/lib/jvm/mh/getter.hpp"
#include "decl/lib/jvm/mh/setter.hpp"
#include "decl/lib/jvm/mh/static_getter.hpp"
#include "decl/lib/jvm/mh/static_setter.hpp"
#include "decl/lib/jvm/mh/virtual.hpp"
#include "decl/lib/jvm/mh/static.hpp"
#include "decl/lib/jvm/mh/special.hpp"
#include "decl/lib/jvm/mh/constructor.hpp"
#include "decl/class/resolve_field.hpp"
#include "decl/class/resolve_class.hpp"

#include <class_file/constant.hpp>
#include <span.hpp>

inline expected<reference, reference> c::try_get_resolved_method_handle(
	class_file::constant::method_handle_index index
) {
	mutex_->lock();
	on_scope_exit unlock {[&] {
		mutex_->unlock();
	}};

	if(auto e = trampoline(index); e.has_value()) {
		if(!e.is_same_as<reference>()) {
			posix::abort();
		}
		return e.get_same_as<reference>();
	}

	/* Let MH be the symbolic reference to a method handle (§5.1)
	   being resolved. Also: */
	class_file::constant::method_handle mh = (*this)[index];

	/* * Let R be the symbolic reference to the field or method contained within
	     MH. R is derived from the CONSTANT_Fieldref, CONSTANT_Methodref, or
	     CONSTANT_InterfaceMethodref structure referred to by the
	     reference_index item of the CONSTANT_MethodHandle from which MH is
	     derived.*/
	class_file::constant::index r = mh.reference_constant_index;

	using behavior_kind = class_file::constant::method_handle::behavior_kind;

	/* * Let T be the type of the field referenced by R, or the return type of
	     the method referenced by R. Let A* be the sequence (perhaps empty) of
	     parameter types of the method referenced by R. */
	/*   T and A* are derived from the CONSTANT_NameAndType structure referred
	     to by the name_and_type_index item in the CONSTANT_Fieldref,
	     CONSTANT_Methodref, or CONSTANT_InterfaceMethodref structure from which
	     R is derived. */

	/* To resolve MH, all symbolic references to classes, interfaces, fields,
	   and methods in MH's bytecode behavior are resolved, using the following
	   four steps: */

	/* 1. R is resolved. This occurs as if by field resolution (§5.4.3.2) when
	      MH's bytecode behavior is kind 1, 2, 3, or 4, and as if by method
	      resolution (§5.4.3.3) when MH's bytecode behavior is kind 5, 6, 7,
	      or 8, and as if by interface method resolution (§5.4.3.4) when MH's
	      bytecode behavior is kind 9. */

	/* 2. The following constraints apply to the result of resolving R. These
	      constraints correspond to those that would be enforced during
	      verification or execution of the instruction sequence for the relevant
	      bytecode behavior. */
	/*    *  If MH's bytecode behavior is kind 8 (REF_newInvokeSpecial), then R
	         must resolve to an instance initialization method declared in class
	         C.*/
	/*    *  R must resolve to a static or non-static member depending on the
	         kind of MH's bytecode behavior:*/
	
	/* 3. Resolution occurs as if of unresolved symbolic references to classes
	      and interfaces whose names correspond to each type in A*, and to the
	      type T, in that order.*/
	
	/* 4. A reference to an instance of java.lang.invoke.MethodType is obtained
	      as if by resolution of an unresolved symbolic reference to a method
	      type that contains the method descriptor specified in Table 5.4.3.5-B
	      for the kind of MH. */

	switch (mh.kind) {
		case behavior_kind::get_field:
		case behavior_kind::get_static:
		case behavior_kind::put_field:
		case behavior_kind::put_static: {
			/* 1 */
			expected<field&, reference> possible_r
				= try_resolve_field(
					(class_file::constant::field_ref_index) r
				);
			if(possible_r.is_unexpected()) {
				return unexpected{ possible_r.move_unexpected() };
			}

			field& r = possible_r.get_expected();
			c& c = r.c();
			/* 2 */ // TODO

			/* 3 */
			expected<::c&, reference> possible_t
				= r.type.view(
					[&]<class_file::field_descriptor_type Type>(Type t) {
						return try_resolve_class_from_type(*this, t);
					}
				);
			if(possible_t.is_unexpected()) {
				return unexpected{ possible_t.move_unexpected() };
			}
			::c& t = possible_t.get_expected();

			/* 4 */
			expected<reference, reference> possible_mt
			= [&]() -> expected<reference, reference> {
				switch(mh.kind) {
					case behavior_kind::get_field:
						return try_create_method_type(
							t,
							::span{ &c }
						);
					case behavior_kind::get_static:
						return try_create_method_type(
							t, span<::c>{}
						);
					case behavior_kind::put_field: {
						array arg_types{ &c, &t };
						return try_create_method_type(
							void_class.get(),
							arg_types.dereference_view()
						);
					}
					case behavior_kind::put_static:
						return try_create_method_type(
							void_class.get(),
							::span{ &t }
						);
					default: posix::abort(); // impossible
				}
			}();

			if(possible_mt.is_unexpected()) {
				return unexpected{ possible_mt.move_unexpected() };
			}

			reference mt = possible_mt.move_expected();

			expected<reference, reference> possible_mh
			= [&]() -> expected<reference, reference> {
				switch(mh.kind) {
					case behavior_kind::get_field:
						return try_create_getter_mh(mt, r);
					case behavior_kind::get_static:
						return try_create_static_getter_mh(mt, r);
					case behavior_kind::put_field:
						return try_create_setter_mh(mt, r);
					case behavior_kind::put_static:
						return try_create_static_setter_mh(mt, r);

					default: posix::abort(); // impossible
				}
			}();

			if(possible_mh.is_unexpected()) {
				return unexpected{ possible_mh.move_unexpected() };
			}

			return possible_mh.move_expected();
		}

		case behavior_kind::invoke_virtual:
		case behavior_kind::invoke_static:
		case behavior_kind::invoke_special:
		case behavior_kind::new_invoke_special:
		case behavior_kind::invoke_interface: {
			/* 1 */
			expected<method&, reference> possible_r =
				mh.kind == behavior_kind::invoke_interface ?
				try_resolve_interface_method(
					(class_file::constant::interface_method_ref_index) r
				) :
				try_resolve_method(
					(class_file::constant::method_ref_index) r
				);

			if(possible_r.is_unexpected()) {
				return unexpected{ possible_r.move_unexpected() };
			}
			method& r = possible_r.get_expected();
			c& c = r.c();
			/* 2 */ // TODO

			/* 3 */

			nuint params_count = r.parameters_count();
			::c* params_raw[params_count];
			::c* c_with_params_raw[params_count + 1];
			span<::c*> params{ params_raw, params_count };
			span<::c*> c_with_params{ c_with_params_raw, params_count + 1};

			c_with_params[0] = &c;

			reference thrown;

			for (auto [index, type] : r.parameter_types().indexed_view()) {
				expected<::c&, reference> possible_a
					= type.view([&](auto t) {
						return try_resolve_class_from_type(*this, t);
					});
				if(possible_a.is_unexpected()) {
					thrown = possible_a.move_unexpected();
					break;
				}
				::c& a = possible_a.get_expected();
				params[index] = &a;
				c_with_params[index + 1] = &a;
			}

			if(!thrown.is_null()) {
				return unexpected{ move(thrown) };
			}

			expected<::c&, reference> possible_t
				= r.return_type().view([&](auto type) {
					return try_resolve_class_from_type(*this, type);
				});
			if(possible_t.is_unexpected()) {
				return unexpected{ possible_t.move_unexpected() };
			}

			::c& t = possible_t.get_expected();

			/* 4 */
			expected<reference, reference> possible_mt
			= [&]() -> expected<reference, reference> {
				switch(mh.kind) {
					case behavior_kind::invoke_virtual:
						return try_create_method_type(
							t, c_with_params.dereference_view()
						);
					case behavior_kind::invoke_static:
						return try_create_method_type(
							t, params.dereference_view()
						);
					case behavior_kind::invoke_special: {
						return try_create_method_type(
							t, c_with_params.dereference_view()
						);
					}
					case behavior_kind::new_invoke_special:
						return try_create_method_type(
							c, params.dereference_view()
						);
					case behavior_kind::invoke_interface:
						return try_create_method_type(
							t, c_with_params.dereference_view()
						);
					default: posix::abort(); // impossible
				}
			}();

			if(possible_mt.is_unexpected()) {
				return unexpected{ possible_mt.move_unexpected() };
			}

			reference mt_ref = possible_mt.move_expected();
			j::method_type& mt = (j::method_type&) mt_ref.object();

			expected<reference, reference> possible_mh
			= [&]() -> expected<reference, reference> {
				switch(mh.kind) {
					case behavior_kind::invoke_virtual:
						return try_create_virtual_mh(mt, r);
					case behavior_kind::invoke_static:
						return try_create_static_mh(mt, r);
					case behavior_kind::invoke_special:
						return try_create_special_mh(mt, (instance_method&) r);
					case behavior_kind::new_invoke_special:
						return try_create_constructor_mh(mt, r);
					//case behavior_kind::invoke_interface: // TODO
					default: posix::abort(); // impossible
				}
			}();

			if(possible_mh.is_unexpected()) {
				return unexpected{ possible_mh.move_unexpected() };
			}

			return possible_mh.move_expected();
		}
	}

	// should not be reachable
}