#pragma once

#include "./decl.hpp"

#include "object/decl.hpp"
#include "alloc.hpp"
#include "abort.hpp"

#include <core/exchange.hpp>

#include <stdio.h>

inline reference::reference(::object& obj) : obj_{ &obj } {
	obj.on_reference_added();
}

inline reference::reference(const reference& other) :
	obj_{ other.obj_ }
{
	if(obj_ != nullptr) {
		obj_->on_reference_added();
	}
}

inline reference::reference(reference&& other) :
	obj_{ exchange(other.obj_, nullptr) }
{}

inline reference& reference::operator = (const reference& other) {
	if(obj_ != nullptr) {
		obj_->on_reference_removed();
	}
	obj_ = other.obj_;
	if(obj_ != nullptr) {
		obj_->on_reference_added();
	}
	return *this;
}

inline reference& reference::operator = (reference&& other) {
	if(obj_ != nullptr) {
		obj_->on_reference_removed();
	}
	obj_ = exchange(other.obj_, nullptr);
	return *this;
}

inline object& reference::object() {
	if(obj_ == nullptr) {
		fprintf(stderr, "obj_ is nullptr");
		abort();
	}
	return *obj_;
}

inline reference::~reference() {
	if(obj_ != nullptr) {
		obj_->on_reference_removed();
		obj_ = nullptr;
	}
}

inline bool reference::is_null() const {
	return obj_ == nullptr;
}