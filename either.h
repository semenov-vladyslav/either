#pragma once

#include <cstddef>
#include <utility>
//#include <type_traits>
#include <cassert>

//#define EITHER_VARIADIC
//#define EITHER_NAMESPACE
//#define EITHER_CONVERTIBLE

EITHER_NAMESPACE_BEGIN
//namespace EITHER_NAMESPACE {

template < size_t Left, size_t Right >
struct static_max: std::integral_constant< size_t, ((Left > Right) ? Left : Right) > {};
//constexpr size_t static_max( size_t left, size_t right )
//{ return left > right ? left : right; }

struct left_args {};
struct right_args {};

template < class Left, class Right >
class either
{
    static unsigned int const _left_flag = 1;
    static unsigned int const _right_flag = 2;
    char _buf[ static_max< sizeof(Left), sizeof(Right) >::value ];
    //char _buf[ static_max( sizeof(Left), sizeof(Right) ) ];
    unsigned int _flags;

    Left *_left() { return reinterpret_cast< Left * >( _buf ); }
    Left const *_left() const { return reinterpret_cast< Left const * >( _buf ); }
    Right *_right() { return reinterpret_cast< Right * >( _buf ); }
    Right const *_right() const { return reinterpret_cast< Right const * >( _buf ); }

    either(): _flags(0) {}
    either &operator=( either< Left, Right > const &e );
    either &operator=( either< Left, Right > &&e );
public:
    typedef Left left_type;
    typedef Right right_type;

#ifdef EITHER_VARIADIC
    template < class ...Args >
    either( left_args, Args &&...args ): _flags(0)
    {
        ::new( _left() ) Left( std::forward<Args>(args)... );
        _flags |= _left_flag;
    }
    template < class ...Args >
    either( right_args, Args &&...args ): _flags(0)
    {
        ::new( _right() ) Right( std::forward<Args>(args)... );
        _flags |= _right_flag;
    }
    template < class ...Args >
    static either< Left, Right > mkleft( Args &&...args ) 
    { return either( left_args(), std::forward<Args>(args)... ); }
    template < class ...Args >
    static either< Left, Right > mkright( Args &&...args ) 
    { return either( right_args(), std::forward<Args>(args)... ); }
    
#else
    either( left_args ): _flags(0)
    {
        ::new( _left() ) Left();
        _flags = _left_flag;
    }
    either( right_args ): _flags(0)
    {
        ::new( _right() ) Right();
        _flags = _right_flag;
    }
    template < class Arg >
    either( left_args, Arg &&arg ): _flags(0)
    {
        ::new( _left() ) Left( std::forward<Arg>(arg) );
        _flags = _left_flag;
    }
    template < class Arg >
    either( right_args, Arg &&arg ): _flags(0)
    {
        ::new( _right() ) Right( std::forward<Arg>(arg) );
        _flags = _right_flag;
    }
    template < class Arg1, class Arg2 >
    either( left_args, Arg1 &&arg1, Arg2 &&arg2 ): _flags(0)
    {
        ::new( _left() ) Left( std::forward<Arg1>(arg1), std::forward<Arg2>(arg2) );
        _flags = _left_flag;
    }
    template < class Arg1, class Arg2 >
    either( right_args, Arg1 &&arg1, Arg2 &&arg2 ): _flags(0)
    {
        ::new( _right() ) Right( std::forward<Arg1>(arg1), std::forward<Arg2>(arg2) );
        _flags = _right_flag;
    }

    static either< Left, Right > mkleft() 
    { return either( left_args() ); }
    static either< Left, Right > mkright() 
    { return either( right_args() ); }
    template < class Arg >
    static either< Left, Right > mkleft( Arg &&arg ) 
    { return either( left_args(), std::forward<Arg>(arg) ); }
    template < class Arg >
    static either< Left, Right > mkright( Arg &&arg ) 
    { return either( right_args(), std::forward<Arg>(arg) ); }
    template < class Arg1, class Arg2 >
    static either< Left, Right > mkleft( Arg1 &&arg1, Arg2 &&arg2 ) 
    { return either( left_args(), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2) ); }
    template < class Arg1, class Arg2 >
    static either< Left, Right > mkright( Arg1 &&arg1, Arg2 &&arg2 ) 
    { return either( right_args(), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2) ); }
#endif

#ifdef EITHER_CONVERTIBLE
    template < class Arg >
    either( Arg &&arg
        , typename std::enable_if< true
            && std::is_convertible< Arg, Left >::value 
            && !std::is_convertible< Arg, Right >::value 
        >::type ** = nullptr
        ): _flags(0)
    {
        ::new( _left() ) Left( std::forward<Arg>(arg) );
        _flags = _left_flag;
    }
    template < class Arg >
    either( Arg &&arg
        , typename std::enable_if< true
            && std::is_convertible< Arg, Right >::value 
            && !std::is_convertible< Arg, Left >::value 
        >::type ** = nullptr
        ): _flags(0)
    {
        ::new( _right() ) Right( std::forward<Arg>(arg) );
        _flags = _right_flag;
    }
    template < class Arg >
    either( Arg &&arg
        , typename std::enable_if< true
            && std::is_convertible< Arg, Left >::value 
            && std::is_convertible< Arg, Right >::value 
        >::type ** = nullptr
        ): _flags(0)
    {
        static_assert( !(true
            && std::is_convertible< Arg, Left >::value 
            && std::is_convertible< Arg, Right >::value 
            ), "arg is convertible to both left_type and right_type." );
    }
    template < class Arg >
    either( Arg &&arg
        , typename std::enable_if< true
            && !std::is_convertible< Arg, Left >::value 
            && !std::is_convertible< Arg, Right >::value 
        >::type ** = nullptr
        ): _flags(0)
    {
        static_assert( !(true
            && !std::is_convertible< Arg, Left >::value 
            && !std::is_convertible< Arg, Right >::value 
            ), "arg is convertible to neither left_type nor right_type." );
    }
#endif

    either( either< Left, Right > const &e ): _flags(0)
    {
        if ( e.is_left() )
        {
            ::new( _left() ) Left( e.left() );
            _flags |= _left_flag;
        }
        if ( e.is_right() )
        {
            ::new( _right() ) Right( e.right() );
            _flags |= _right_flag;
        }
    }
    either( either< Left, Right > &&e ): _flags(0)
    {
        if ( e.is_left() )
        {
            ::new( _left() ) Left( std::move(e.left()) );
            _flags |= _left_flag;
        }
        if ( e.is_right() )
        {
            ::new( _right() ) Right( std::move(e.right()) );
            _flags |= _right_flag;
        }
    }
    //either &operator=( either< Left, Right > const &e ) = delete;
    //either &operator=( either< Left, Right > &&e ) = delete;
    
    ~either()
    {
        if ( is_left() )
            _left()->~Left();
        if ( is_right() )
            _right()->~Right();
        _flags = 0;
    }
    
    bool is_left() const { return 0 != (_flags & _left_flag); }
    bool is_right() const { return 0 != (_flags & _right_flag); }

    Left &left() { assert( is_left() ); return *_left(); }
    Left const &left() const { assert( is_left() ); return *_left(); }
    Right &right() { assert( is_right() ); return *_right(); }
    Right const &right() const { assert( is_right() ); return *_right(); }
};

template < class Left, class Right >
bool operator==( either< Left, Right > const &lhs, either< Left, Right > const &rhs )
{
    if ( lhs.is_left() && rhs.is_left() ) return lhs.left() == rhs.left();
    if ( lhs.is_right() && rhs.is_right() ) return lhs.right() == rhs.right();
    return false;
}
template < class Left, class Right >
bool operator!=( either< Left, Right > const &lhs, either< Left, Right > const &rhs )
{
    return !(lhs == rhs);
}

#if 0
template < class Left, class Result >
struct enable_either {};
template < class Left, class Right >
struct enable_either< Left, either< Left, Right > > 
{
    typedef either< Left, Right > type;
};

// because of associativity, `bind` operator is defined as operator >>, not as operator >>=.
template < class Left, class Right, class Fn >
auto operator>>(either< Left, Right > &e, Fn f) -> decltype(f(e.right())) 
{
    typedef decltype(f(e.right())) result_type;
    static_assert( std::is_same< Left, typename result_type::left_type >::value, "left_type must be the same." );
    if (e.is_right())
        return f(e.right());
    return result_type(left_args(), e.left());
}
template < class Left, class Right, class Fn >
auto operator>>(either< Left, Right > const &e, Fn f) -> decltype(f(e.right()))
{
    typedef decltype(f(e.right())) result_type;
    static_assert(std::is_same< Left, typename result_type::left_type >::value, "left_type must be the same.");
    if (e.is_right())
        return f(e.right());
    return result_type(left_args(), e.left());
}
#else
template < class Left, class Result >
struct enable_either 
{
    typedef either< Left, Result > type;
};
template < class Left, class Right >
struct enable_either< Left, either< Left, Right > >
{
    typedef either< Left, Right > type;
};

// because of associativity, `bind` operator is defined as operator >>, not as operator >>=.
template < class Left, class Right, class Fn >
auto operator>>( either< Left, Right > &e, Fn f ) -> typename enable_either< Left, decltype( f( e.right() ) ) >::type
{
    typedef typename enable_either< Left, decltype(f(e.right())) >::type result_type;
    if ( e.is_right() )
        return f( e.right() );
    return result_type( left_args(), e.left() );
}
template < class Left, class Right, class Fn >
auto operator>>(either< Left, Right > const &e, Fn f) -> typename enable_either< Left, decltype(f(e.right())) >::type
{
    typedef typename enable_either< Left, decltype(f(e.right())) >::type result_type;
    if ( e.is_right() )
        return f( e.right() );
    return result_type( left_args(), e.left() );
}
#endif

// because of incompatible precedence and associativity of >> & >>= operators, operator >> cannot be well-defined.
//template < class Left, class Right, class Fn >
//auto operator>>( either< Left, Right > &e, Fn f ) -> decltype( f() )
//{
//    typedef decltype( f() ) result_type;
//    static_assert( std::is_same< Left, typename result_type::left_type >::value, "left_type must be the same." );
//    if ( e.is_right() )
//        return f();
//    return result_type( left_args(), e.left() );
//}
//template < class Left, class Right, class Fn >
//auto operator>>( either< Left, Right > const &e, Fn f ) -> decltype( f() )
//{
//    typedef decltype( f() ) result_type;
//    static_assert( std::is_same< Left, typename result_type::left_type >::value, "left_type must be the same." );
//    if ( e.is_right() )
//        return f();
//    return result_type( left_args(), e.left() );
//}

//}
EITHER_NAMESPACE_END
