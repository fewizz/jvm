#include "decl/method.hpp"

#include "decl/class.hpp"
#include "decl/lib/java/lang/invoke/method_handle.hpp"
#include "decl/lib/java/lang/invoke/var_handle.hpp"

/* A method is an instance initialization method
   if all of the following are true: */
inline bool method::is_instance_initialisation() const {
	return
		/* It is defined in a class (not an interface). */
		!_class().is_interface() &&
		/* It has the special name <init>. */
		name().has_equal_size_and_elements(c_string{ "<init>" }) &&
		/* It is void (§4.3.3). */
		is_void();
}

inline bool method::is_class_initialisation() const {
	return name().has_equal_size_and_elements(c_string{ "<clinit>" });
}

inline bool method::is_signature_polymorphic() const {
	const ::_class& c = _class();

	bool c_is_mh_or_vh =
		c.is(method_handle_class.get()) ||
		c.is(var_handle_class.get());
	
	bool param_is_object_array =
		descriptor().has_equal_size_and_elements(
			c_string{"([Ljava/lang/Object;)Ljava/lang/Object;"}
		);
	
	bool varargs_and_native_flags_set =
		has_variable_number_of_arguments() &&
		is_native();

	return
		c_is_mh_or_vh &&
		param_is_object_array &&
		varargs_and_native_flags_set;
}

inline bool method::can_override(method& ma) const {
	const method& mc = *this;

	/* An instance method mC can override another instance method mA if all of
	   the following are true: */
	
	/* * mC has the same name and descriptor as mA. */
	bool same_name_and_descriptor =
		mc.name().has_equal_size_and_elements(ma.name()) &&
		mc.descriptor().has_equal_size_and_elements(ma.descriptor());
	
	/* * mC is not marked ACC_PRIVATE. */
	bool mc_isnt_private = !mc.is_private();

	/* * One of the following is true: */
	bool third_requirement = [&]() -> bool {
		/* * mA is marked ACC_PUBLIC. */
		bool ma_is_public = ma.is_public();
		/* * mA is marked ACC_PROTECTED. */
		bool ma_is_protected = ma.is_protected();
		/* * mA is marked neither ACC_PUBLIC nor ACC_PROTECTED nor ACC_PRIVATE,
		     and either (a) the declaration of mA appears in the same run-time
		     package as the declaration of mC, or (b) if mA is declared in a
		     class A and mC is declared in a class C, then there exists a
		     method mB declared in a class B such that C is a subclass of B and
		     B is a subclass of A and mC can override mB and mB can override
		     mA. */
		bool ma_is_private = ma.is_private();
		const ::_class& a = ma._class();
		const ::_class& c = mc._class();
		bool thrird = !ma_is_public && !ma_is_protected && !ma_is_private && (
			/* a */ ma.package().has_equal_size_and_elements(mc.package()) ||
			/* b */ [&]() -> bool {
				if(!c.has_super()) return false;

				const ::_class* b_ptr = &c.super();

				while(!b_ptr->super().is(a)) {
					optional<::instance_method&> m
						= b_ptr->declared_instance_methods()
						.try_find(ma.name(), ma.descriptor());

					if(!m.has_value()) {
						continue;
					}

					bool can_override = m->can_override(ma);

					if(can_override) {
						return true;
					}
					if(!b_ptr->has_super()) break;

					b_ptr = b_ptr->super().ptr();
				}

				return false;
			}()
		);

		return ma_is_public || ma_is_protected || thrird;
	}();

	return same_name_and_descriptor && mc_isnt_private && third_requirement;
}