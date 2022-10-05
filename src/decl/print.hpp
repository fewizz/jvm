#pragma once

#include <array.hpp>
#include <list.hpp>
#include <range.hpp>

#include <posix/io.hpp>

inline struct buffered_print_t {
	list<array<storage<char>, 65536>> buff{};

	void flush() {
		posix::std_err.write_from(buff);
		buff.clear();
	}

	template<basic_range Range>
	void operator () (Range&& r) {
		auto iterator = range_iterator(r);
		auto elements_to_put = range_size(r);

		while(elements_to_put > buff.available()) {
			auto available = buff.available();
			buff.put_back_copied_elements_of(
				iterator_and_sentinel {
					iterator, iterator + available
				}.as_range()
			);
			flush();
			iterator += available;
			elements_to_put -= available;
		}

		buff.put_back_copied_elements_of(
			iterator_and_sentinel {
				iterator, iterator + elements_to_put
			}.as_range()
		);
	}

	~buffered_print_t() {
		flush();
	}
} buffered_print{};

template<nuint Size>
void print(const char (&str_array)[Size]) {
	buffered_print(c_string{ str_array });
};

template<contiguous_range Range>
void print(Range&& range) {
	buffered_print(range);
};

void print(unsigned_integer auto number) {
	nuint count = 0;
	for_each_digit_in_number(number, 10, [&](nuint) { ++count; });
	char digits[count];
	count = 0;
	for_each_digit_in_number(number, 10, [&](nuint digit) {
		digits[count++] = (char)digit + '0';
	});
	buffered_print(span{ digits, count });
};

void print(signed_integer auto number) {
	if(number < 0) {
		buffered_print("-");
		print((nuint)-number);
	}
	else {
		print((nuint)number);
	}
};