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
		if(!p.object.has_value()) {
			p.object = *
				new(malloc(sizeof(object)))
				object(c, c.instance_fields().size());

			for(field& f : c.instance_fields()) {
				using namespace class_file;

				field_value fv;

				bool result = descriptor::read_field(
					f.descriptor().begin(),
					[&]<typename DescriptorType>(DescriptorType) {
						return fv.set_default_value<DescriptorType>();
					}
				);

				if(!result) {
					fputs(
						"couldn't read field descriptor while creating object",
						stderr
					);
					abort();
				}

				p.object->values().emplace_back(move(fv));
			}

			return reference{ p };
		}
	}
	fputs("no free place for object", stderr);
	abort();
}