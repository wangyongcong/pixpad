#ifndef WYC_ARITHMETIC_HELPER
#define WYC_ARITHMETIC_HELPER

namespace wyc
{

/******************************************************************************
CLASS arithmetic_operator_helper
PARAMETERS
	T	--	arithmetic type like rounded, fixed_decimal, or fixed_binary below
IMPLEMENTATION
	for class T representing a numeric type (a type that acts like a number, 
	having commutative + and *, non-commutative - and /, and comparison 
	relations), and	which implement a basic set of methods 
		operator+=, -=, *=, /=, ==, and <
	arithmetic_operator_helper<T> adds additional operator methods
		operator+, -, *, /, unary-, !=, <=, >, >=
	with the usual meanings
******************************************************************************/
template <typename T> struct arithmetic_operator_helper
{
//	arithmetical operators
	friend T operator-(T const& lhs)
	{ T res;  res-=lhs; return res; }
	friend T operator+(T const& lhs, T const& rhs) 
	{ T res(lhs); res+=rhs; return res; }
	friend T operator-(T const& lhs, T const& rhs) 
	{ T res(lhs); res-=rhs; return res; }
	friend T operator*(T const& lhs, T const& rhs) 
	{ T res(lhs); res*=rhs; return res; }
	friend T operator/(T const& lhs, T const& rhs) 
	{ T res(lhs); res/=rhs; return res; }
//	friend T operator+(T lhs, T const& rhs)
//	{ return lhs+=rhs; } //	!named return value optimization version

//	arithmetic with other integral types
	template <typename R> friend T operator+(R lhs, T const& rhs)
	{ T res(lhs); res+=rhs; return res; }
	template <typename R> friend T operator-(R lhs, T const& rhs)
	{ T res(lhs); res-=rhs; return res; }
	template <typename R> friend T operator*(R lhs, T const& rhs)
	{ T res(lhs); res*=rhs; return res; }
	template <typename R> friend T operator/(R lhs, T const& rhs)
	{ T res(lhs); res/=rhs; return res; }

//	comparison operators
	friend bool operator!=(T const& lhs, T const& rhs)
	{ return !(lhs == rhs); }
	friend bool operator>=(T const& lhs, T const& rhs)
	{ return !(lhs <  rhs); }
	friend bool operator<=(T const& lhs, T const& rhs)
	{ return !(rhs <  lhs); }
	friend bool operator> (T const& lhs, T const& rhs)
	{ return  (rhs <  lhs); }
};// end class arithmetic_operator_helper

}; // namespace wyc

#endif // WYC_ARITHMETIC_HELPER
