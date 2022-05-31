#pragma once

#include "../alloc.hpp"
#include "../abort.hpp"
#include "counted_object_ptr.hpp"
#include "reference.hpp"
#include <core/integer.hpp>
#include <core/array.hpp>
#include <stdio.h>

struct _class;

static inline struct objects_t :
	private array<counted_object_ptr, 128>
{
	using base_type = array<counted_object_ptr, 128>;
	inline reference find_free(_class& c);

} objects{};

#include "object.hpp"

reference objects_t::find_free(_class& c) {
	for(uint32 i = 1; i < 65536; ++i) {
		auto& p = base_type::operator [] (i);
		if(p.object_ptr == nullptr) {
			p.object_ptr =
				new(malloc(sizeof(object)))
				object(c, c.instance_fields().size());

			field_value fv;

			for(field* f : c.instance_fields()) {
				using namespace class_file;
				bool result = descriptor::read_field(
				f->descriptor().begin(),
				[&]<typename Type0>(Type0) {
					if constexpr(same_as<descriptor::B, Type0>) {
						fv = jbyte{ 0 };
					} else
					if constexpr(same_as<descriptor::C, Type0>) {
						fv = jchar{ 0 };
					} else
					if constexpr(same_as<descriptor::D, Type0>) {
						fv = jdouble{ 0.0 };
					} else
					if constexpr(same_as<descriptor::F, Type0>) {
						fv = jfloat{ 0.0 };
					} else
					if constexpr(same_as<descriptor::I, Type0>) {
						fv = jint{ 0 };
					} else
					if constexpr(same_as<descriptor::J, Type0>) {
						fv = jlong{ 0 };
					} else
					if constexpr(same_as<descriptor::Z, Type0>) {
						fv = jbool{ 0 };
					} else {
					//if constexpr(same_as<descriptor::object_type, Type0>) {
//					} else {
						return false;
					}
						return true;
				});
				if(!result) {
					fputs(
						"couldn't read field descriptor while creating object",
						stderr
					);
					abort();
				}
				p.object_ptr->values().emplace_back(fv);
			}

			return reference{ &p };
		}
	}
	fputs("no free place for object", stderr);
	abort();
}