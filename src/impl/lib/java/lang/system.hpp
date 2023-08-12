#include "decl/lib/java/lang/system.hpp"

#include "decl/object.hpp"
#include "decl/array.hpp"
#include "decl/classes.hpp"
#include "decl/native/environment.hpp"
#include "decl/native/thrown.hpp"
#include "decl/lib/java/lang/null_pointer_exception.hpp"
#include "decl/lib/java/lang/array_store_exception.hpp"
#include "decl/lib/java/lang/index_out_of_bounds_exception.hpp"

#include <posix/time.hpp>

static inline void init_java_lang_system() {

	system_class = classes.load_class_by_bootstrap_class_loader(
		c_string{ u8"java/lang/System" }
	);

	system_class->declared_static_methods().find(
		c_string{ u8"arraycopy" },
		c_string{ u8"(Ljava/lang/Object;ILjava/lang/Object;II)V" }
	).native_function(
		(void*)+[](
			native_environment*,
			object* src, int32 src_pos,
			object* dst, int32 dst_pos,
			int32 len
		) {
			/* If dest is null, then a NullPointerException is thrown.
			   If src is null, then a NullPointerException is thrown and the
			   destination array is not modified.
			*/
			if(src == nullptr || dst == nullptr) {
				thrown_in_native = try_create_null_pointer_exception().get();
				return;
			}
			/* Otherwise, if any of the following is true, an
			   ArrayStoreException is thrown and the destination is not
			   modified: */
			if(
				// The src argument refers to an object that is not an array.
				!src->c().is_array() ||
				// The dest argument refers to an object that is not an array.
				!dst->c().is_array()
			) {
				thrown_in_native = try_create_array_store_exception().get();
				return;
			}

			c& src_component_class = src->c().get_component_class();
			c& dst_component_class = dst->c().get_component_class();
			bool src_is_primitive = src_component_class.is_primitive();
			bool dst_is_primitive = dst_component_class.is_primitive();
			bool both_are_primitives = src_is_primitive && dst_is_primitive;

			if(
				/* The src argument and dest argument refer to arrays whose
				   component types are different primitive types. */
				(
					both_are_primitives &&
					src_component_class.is_not(dst_component_class)
				) ||
				/* The src argument refers to an array with a primitive
				   component type and the dest argument refers to an array with
				   a reference component type. */
				/* The src argument refers to an array with a reference
				   component type and the dest argument refers to an array with
				   a primitive component type. */
				src_is_primitive != dst_is_primitive
			) {
				thrown_in_native = try_create_array_store_exception().get();
				return;
			}

			/* Otherwise, if any of the following is true, an
			   IndexOutOfBoundsException is thrown and the destination is not
			   modified: */
			if(
				// The srcPos argument is negative.
				src_pos < 0 ||
				// The destPos argument is negative.
				dst_pos < 0 ||
				// The length argument is negative.
				len < 0 ||
				// srcPos+length is greater than src.length, the length of the
				// source array.
				src_pos + len > array_length(*src) ||
				// destPos+length is greater than dest.length, the length of the
				// destination array.
				dst_pos + len > array_length(*dst)
			) {
				thrown_in_native
					= try_create_index_of_of_bounds_exception().get();
				return;
			}

			if(both_are_primitives) {
				src_component_class.view_non_void_raw_type([&]<typename Type>()
				{
					span<Type> src_span {
						array_data<Type>(*src) + src_pos, (nuint) len
					};
					span<Type> dst_span {
						array_data<Type>(*dst) + dst_pos, (nuint) len
					};

					// objects are same, dst lies after src
					if(
						src == dst &&
						dst_span.iterator() > src_span.iterator()
					) {
						src_span.reverse_view().copy_to(
							dst_span.reverse_view()
						);
					}
					else {
						src_span.copy_to(dst_span);
					}
				});
				return;
			}

			reference* src_data = array_data<reference>(*src);
			reference* dst_data = array_data<reference>(*dst);
			span src_span{ src_data + src_pos, (nuint) len };
			span dst_span{ dst_data + dst_pos, (nuint) len };

			for(nuint x = 0; x < (nuint) len; ++x) {
				reference& s = src_span[x];
				reference& d = dst_span[x];
				if(!s.is_null() && !d.is_null()) {
					c& sc = s.c();
					c& dc = d.c();

					/* Otherwise, if any actual component of the source array
					   from position srcPos through srcPos+length-1 cannot be
					   converted to the component type of the destination array
					   by assignment conversion, an ArrayStoreException is
					   thrown. In this case, let k be the smallest nonnegative
					   integer less than length such that src[srcPos+k] cannot
					   be converted to the component type of the destination
					   array; when the exception is thrown, source array
					   components from positions srcPos through srcPos+k-1 will
					   already have been copied to destination array positions
					   destPos through destPos+k-1 and no other positions of the
					   destination array will have been modified. (Because of
					   the restrictions already itemized, this paragraph
					   effectively applies only to the situation where both
					   arrays have component types that are reference types.) */
					bool assignable = sc.is(dc) || sc.is_sub_of(dc); // TODO
					if(!assignable) {
						thrown_in_native
							= try_create_array_store_exception().get();
						return;
					}
				}
				d = s;
			}
		}
	);

	system_class->declared_static_methods().find(
		c_string{ u8"nanoTime" }, c_string{ u8"()J" }
	).native_function(
		(void*) (int64(*)(native_environment*))
		[](native_environment*) {
			auto [seconds, nanoseconds]
				= posix::monolitic_clock.secods_and_nanoseconds();
			return (int64) (seconds * 1'000'000'000ll + nanoseconds);
		}
	);
}