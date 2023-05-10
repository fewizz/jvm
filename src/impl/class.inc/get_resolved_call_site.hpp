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

/* To resolve an unresolved symbolic reference R to a dynamically-computed
   constant or call site, there are three tasks. First, R is examined to
   determine which code will serve as its bootstrap method, and which arguments
   will be passed to that code. Second, the arguments are packaged into an array
   and the bootstrap method is invoked. Third, the result of the bootstrap
   method is validated, and used as the result of resolution. */
inline expected<reference, reference> c::try_get_resolved_call_site(
	class_file::constant::invoke_dynamic_index index
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

	class_file::constant::invoke_dynamic invoke_dynamic
		= invoke_dynamic_constant(index);
	
	/* The first task involves the following steps: */
	/* 1. R gives a symbolic reference to a bootstrap method handle. */
	bootstrap_method& bm
		= bootstrap_methods::as_span().operator [] (
			invoke_dynamic.bootstrap_method_attr_index
		);
	/*    The bootstrap method handle is resolved (§5.4.3.5) to obtain a
	      reference to an instance of java.lang.invoke.MethodHandle. */
	expected<reference, reference> possible_mh
		= try_get_resolved_method_handle(bm.method_handle_index);
	
	if(possible_mh.is_unexpected()) {
		return unexpected{ possible_mh.move_unexpected() };
	}

	reference mh = possible_mh.move_expected();

	/* 2. If R is a symbolic reference to a dynamically-computed call site, then
	      it gives a method descriptor. */
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
	expected<reference, reference> possible_mt
		= try_resolve_method_type(*this, descriptor);
	if(possible_mt.is_unexpected()) {
		return unexpected{ possible_mt.move_unexpected() };
	}
	reference mt = possible_mt.move_expected();

	/* An array is allocated with component type Object and length n+3, where n
	is the number of static arguments given by R (n ≥ 0). */
	nuint args_count_stack = 0;

	/* The zeroth component of the array is set to a reference to an instance of
	   java.lang.invoke.MethodHandles.Lookup for the class in which R occurs,
	   produced as if by invocation of the lookup method of
	   java.lang.invoke.MethodHandles. */
	expected<reference, reference> possible_lookup
		= try_create_object(method_handles_lookup_class.get());
	if(possible_lookup.is_unexpected()) {
		return unexpected{ possible_lookup.move_unexpected() };
	}

	stack.emplace_back(possible_lookup.move_expected());
	++args_count_stack;

	/* The first component of the array is set to a reference to an instance of
	   String that denotes N, the unqualified name given by R. */
	expected<reference, reference> possible_name_ref
		= try_create_string_from_utf8(name);
	
	if(possible_name_ref.is_unexpected()) {
		return possible_name_ref.move_unexpected();
	}

	stack.emplace_back(possible_name_ref.move_expected());
	++args_count_stack;

	/* The second component of the array is set to the reference to an instance
	   of Class or java.lang.invoke.MethodType that was obtained earlier for the
	   field descriptor or method descriptor given by R. */
	// TODO "to an instance of Class" ???
	stack.emplace_back(mt);
	++args_count_stack;

	/* Subsequent components of the array are set to the references that were
	   obtained earlier from resolving R's static arguments, if any.
	   The references appear in the array in the same order as the corresponding
	   static arguments are given by R. */

	/* R gives zero or more static arguments, which communicate
	   application-specific metadata to the bootstrap method. Each static
	   argument A is resolved, in the order given by R, as follows: */
	for(class_file::constant::index index : bm.arguments_indices.as_span()) {
		reference thrown{};

		constant(index).view([&]<typename Type>(Type v) {
			/* If A is a string constant, then a reference to its instance of
			   class String is obtained. */
			if constexpr(same_as<Type, class_file::constant::string>) {
				expected<reference, reference> possible_string =
					try_get_string((class_file::constant::string_index) index);
				
				if(possible_string.is_unexpected()) {
					thrown = possible_string.move_unexpected();
					return;
				}

				stack.emplace_back(possible_string.move_expected());
				++args_count_stack;
			}
			/* If A is a numeric constant, then a reference to an instance of
			   java.lang.invoke.MethodHandle is obtained by the following
			   procedure: */
			// else if() TODO
			else if constexpr(same_as<Type, class_file::constant::method_type>)
			{
				class_file::constant::utf8 descriptor
					= utf8_constant(v.descriptor_index);
				expected<reference, reference> possible_mt
					= try_resolve_method_type(*this, descriptor);
				
				if(possible_mt.is_unexpected()) {
					thrown = possible_mt.move_unexpected();
					return;
				}
				stack.emplace_back(possible_mt.move_expected());
				++args_count_stack;
			}
			else if constexpr(
				same_as<Type, class_file::constant::method_handle>
			) {
				expected<reference, reference> possible_mh
					= try_get_resolved_method_handle(
						class_file::constant::method_handle_index{ index }
					);
				
				if(possible_mh.is_unexpected()) {
					thrown = possible_mh.move_unexpected();
					return;
				}

				stack.emplace_back(possible_mh.move_expected());
				++args_count_stack;
			}
			else {
				posix::abort();
			}
		});

		if(!thrown.is_null()) {
			return unexpected{ move(thrown) };
		}
	}

	/* The bootstrap method handle is invoked, as if by the invocation
	   BMH.invokeWithArguments(args), where BMH is the bootstrap method handle
	   and args is the array allocated above. */
	//method_handle_invoke_exact(mh, args_count_stack);
	posix::abort();
	
	reference result = stack.pop_back<reference>();

	trampoline(index) = result;

	return move(result);
}