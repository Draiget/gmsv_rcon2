#pragma once

template< class T >
static bool is_valid_ptr(T ptr);

template < class T >
bool is_valid_ptr(T ptr) {
	const auto pointer = reinterpret_cast<unsigned long>(ptr);
	return pointer && pointer != 0xcccccccc;
}
