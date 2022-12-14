#pragma once

#include "decl/class.hpp"
#include "decl/reference.hpp"
#include "decl/execute.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/method_handles_lookup.hpp"
#include "decl/class/resolve_method_type.hpp"
#include "decl/object.hpp"
#include "decl/lib/java/lang/string.hpp"

#include <class_file/constant.hpp>

#include <list.hpp>
#include <storage.hpp>

inline reference _class::get_resolved_call_site(
	class_file::constant::invoke_dynamic_index index
) {
	abort(); // TODO
	if(auto e = trampoline(index); e.has_value()) {
		if(!e.is_same_as<reference>()) {
			abort();
		}
		return e.get_same_as<reference>();
	}

	class_file::constant::invoke_dynamic invoke_dynamic
		= invoke_dynamic_constant(index);
	
	/* R gives a symbolic reference to a bootstrap method handle. */
	bootstrap_method& bm
		= bootstrap_methods::operator [] (
			invoke_dynamic.bootstrap_method_attr_index
		);
	/* The bootstrap method handle is resolved (§5.4.3.5) to obtain a reference
	   to an instance of java.lang.invoke.MethodHandle. */
	reference mh = get_resolved_method_handle(bm.method_handle_index);

	/* If R is a symbolic reference to a dynamically-computed call site, then it
	   gives a method descriptor. */
	class_file::constant::name_and_type nat
		= name_and_type_constant(invoke_dynamic.name_and_type_index);
	class_file::constant::utf8 descriptor
		= utf8_constant(nat.descriptor_index);
	[[maybe_unused]] class_file::constant::utf8 name
		= utf8_constant(nat.name_index);

	/* A reference to an instance of java.lang.invoke.MethodType is obtained, as
	   if by resolution of an unresolved symbolic reference to a method type
	   (§5.4.3.5) with the same parameter and return types as the method
	   descriptor. */
	reference mt = resolve_method_type(*this, descriptor);

	/* An array is allocated with component type Object and length n+3, where n
	is the number of static arguments given by R (n ≥ 0). */
	[[maybe_unused]] uint8 args_count = bm.arguments_indices.size() + 3;

	// TODO
	/*storage<stack_entry> args_storage[args_count];
	list<span<storage<stack_entry>>> args {
		span{ args_storage, args_count}
	};*/
	/* The zeroth component of the array is set to a reference to an instance of
	   java.lang.invoke.MethodHandles.Lookup for the class in which R occurs,
	   produced as if by invocation of the lookup method of
	   java.lang.invoke.MethodHandles. */
	//args.emplace_back(create_object(method_handles_lookup_class.get()));

	/* The first component of the array is set to a reference to an instance of
	   String that denotes N, the unqualified name given by R. */
	//args.emplace_back(create_string_from_utf8(name));

	/* The second component of the array is set to the reference to an instance
	   of Class or java.lang.invoke.MethodType that was obtained earlier for the
	   field descriptor or method descriptor given by R. */
	// TODO "to an instance of Class" ???
	//args.emplace_back(mt);

	/* Subsequent components of the array are set to the references that were
	   obtained earlier from resolving R's static arguments, if any.
	   The references appear in the array in the same order as the corresponding
	   static arguments are given by R. */

	/* R gives zero or more static arguments, which communicate
	   application-specific metadata to the bootstrap method. Each static
	   argument A is resolved, in the order given by R, as follows: */
	/*for(auto index : bm.arguments_indices) {
		constant(index).view([&]<typename Type>(Type v) {*/
			/* If A is a string constant, then a reference to its instance of
			   class String is obtained. */
			/*if constexpr(same_as<Type, class_file::constant::string>) {
				args.emplace_back(
					get_string((class_file::constant::string_index) index)
				);
			}*/
			/* If A is a numeric constant, then a reference to an instance of
			   java.lang.invoke.MethodHandle is obtained by the following
			   procedure: */
			// else if() TODO
			/*else if constexpr(same_as<Type, class_file::constant::method_type>)
			{
				class_file::constant::utf8 descriptor
					= utf8_constant(v.descriptor_index);
				args.emplace_back(
					resolve_method_type(*this, descriptor)
				);
			}
			else if constexpr(
				same_as<Type, class_file::constant::method_handle>
			) {
				args.emplace_back(
					get_resolved_method_handle(
						class_file::constant::method_handle_index{ index }
					)
				);
			}
			else {
				abort();
			}
		});
	}

	method_handle_invoke_exact(
		mh, parameters_count{ args_count }
	);
	
	reference result = stack.pop_back<reference>();

	trampoline(index) = result;
	return move(result);*/
}