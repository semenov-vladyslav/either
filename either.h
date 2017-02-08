#pragma once
#ifndef _EITHER_H
#define _EITHER_H

#include <cstddef>
#include <utility>
#include <tuple>
#include <type_traits>
#include <cassert>

//#define EITHER_VARIADIC
//#define EITHER_CONVERTIBLE
//#define EITHER_EMPTY_ALLOWED

#ifdef EITHER_NAMESPACE // monad
namespace EITHER_NAMESPACE {
#endif


template < class Left, class Right >
class either;

template < class Left, class Result >
struct bind_either
{
  typedef either< Left, Result > type;
};
template < class Left, class Right >
struct bind_either< Left, either< Left, Right > >
{
  typedef either< Left, Right > type;
};

struct left_args {};
struct right_args {};

//namespace 
//{
//left_args left_init;
//right_args right_init;
//}
namespace detail
{
template < size_t Left, size_t Right >
struct static_max : std::integral_constant< size_t, ((Left > Right) ? Left : Right) > {};
//constexpr size_t static_max( size_t left, size_t right )
//{ return left > right ? left : right; }

template < class T > struct size: 
  std::conditional< std::is_empty<T>::value 
    , std::integral_constant< size_t, 0 >
    , std::integral_constant< size_t, sizeof( T ) >
  >::type
{};
template <> struct size<void> : std::integral_constant< size_t, 0 > {};

template < class Left, class Right, size_t buf_size >
class basic_either 
{
protected:
  char _buf[buf_size];

  Left *_left() { return reinterpret_cast<Left *>(_buf); }
  Left const *_left() const { return reinterpret_cast<Left const *>(_buf); }
  Right *_right() { return reinterpret_cast<Right *>(_buf); }
  Right const *_right() const { return reinterpret_cast<Right const *>(_buf); }
};
template < class Left, class Right >
class basic_either< Left, Right, 0 >
{
protected:
  Left *_left() { return reinterpret_cast<Left *>(this); }
  Left const *_left() const { return reinterpret_cast<Left const *>(this); }
  Right *_right() { return reinterpret_cast<Right *>(this); }
  Right const *_right() const { return reinterpret_cast<Right const *>(this); }
};
template < class Left, class Right >
class either_base: protected basic_either< Left, Right
  , static_max< size<Left>::value, size<Right>::value >::value > 
{
protected:
  static unsigned int const _left_flag = 1;
  static unsigned int const _right_flag = 2;
  unsigned int _flags;

  either_base() : _flags( 0 ) {}

  template < class ...Args >
  void construct_left( Args &&...args )
  {
    ::new( this->_left() ) Left( std::forward<Args>(args)... );
    _flags |= _left_flag;
  }
  void construct_copy_left( Left const *left )
  {
    ::new( this->_left() ) Left( *left );
    _flags |= _left_flag;
  }
  void construct_move_left( Left *left )
  {
    ::new( this->_left() ) Left( std::move(*left) );
    _flags |= _left_flag;
  }
  template < class ...Args >
  void construct_right( Args &&...args )
  {
    ::new( this->_right() ) Right( std::forward<Args>(args)... );
    _flags |= _right_flag;
  }
  void construct_copy_right( Right const *right )
  {
    ::new( this->_right() ) Right( *right );
    _flags |= _right_flag;
  }
  void construct_move_right( Right *right )
  {
    ::new( this->_right() ) Right( std::move( *right ) );
    _flags |= _right_flag;
  }
  void destruct()
  {
    if( is_left() )
      this->_left()->~Left();
    if( is_right() )
      this->_right()->~Right();
    _flags = 0;
  }

public:
  bool empty() const { return 0 == _flags; }
  bool is_left() const { return 0 != (_flags & _left_flag); }
  bool is_right() const { return 0 != (_flags & _right_flag); }

  Left &left() { assert( is_left() ); return *this->_left(); }
  Left const &left() const { assert( is_left() ); return *this->_left(); }
  Right &right() { assert( is_right() ); return *this->_right(); }
  Right const &right() const { assert( is_right() ); return *this->_right(); }

  bool equal( either_base< Left, Right > const &rhs ) const
  {
    auto &lhs = *this;
    if( lhs.is_left() && rhs.is_left() ) 
      return lhs.left() == rhs.left();
    if( lhs.is_right() && rhs.is_right() ) 
      return lhs.right() == rhs.right();
    return false;
  }
  bool less( either_base< Left, Right > const &rhs ) const
  {
    auto &lhs = *this;

    if( lhs.is_left() )
    {
      if( rhs.is_left() ) return lhs.left() < rhs.left();
      if( rhs.is_right() ) return true;
      return false;
    }
    if( lhs.is_right() )
    {
      if( rhs.is_left() ) return false;
      if( rhs.is_right() ) return lhs.right() < rhs.right();
      return false;
    }
    if( rhs.is_left() ) return true;
    if( rhs.is_right() ) return true;
    return false;
  }

  //template < class Fn >
  //auto bind( Fn f ) -> typename bind_either< Left, decltype(f( right() )) >::type
  //{
  //  auto &e = *this;
  //  //std::declval<either_base< Left, Right >>().
  //  typedef typename bind_either< Left, decltype(f( e.right() )) >::type result_type;
  //  if( e.is_right() )
  //    return f( e.right() );
  //  return result_type( left_args(), e.left() );
  //}
};
template < class Right >
class either_base< void, Right > : protected basic_either< void, Right
  , size<Right>::value >
{
protected:
  static unsigned int const _left_flag = 1;
  static unsigned int const _right_flag = 2;
  unsigned int _flags;

  either_base() : _flags( 0 ) {}

  void construct_left()
  {
    _flags |= _left_flag;
  }
  void construct_copy_left( void const *left )
  {
    _flags |= _left_flag;
  }
  void construct_move_left( void *left )
  {
    _flags |= _left_flag;
  }
  template < class ...Args >
  void construct_right( Args &&...args )
  {
    ::new(this->_right()) Right( std::forward<Args>( args )... );
    _flags |= _right_flag;
  }
  void construct_copy_right( Right const *right )
  {
    ::new(this->_right()) Right( *right );
    _flags |= _right_flag;
  }
  void construct_move_right( Right *right )
  {
    ::new(this->_right()) Right( std::move( *right ) );
    _flags |= _right_flag;
  }
  void destruct()
  {
    if( is_right() )
      this->_right()->~Right();
    _flags = 0;
  }

public:
  bool empty() const { return 0 == _flags; }
  bool is_left() const { return 0 != (_flags & _left_flag); }
  bool is_right() const { return 0 != (_flags & _right_flag); }

  Right &right() { assert( is_right() ); return *this->_right(); }
  Right const &right() const { assert( is_right() ); return *this->_right(); }

  bool equal( either_base< void, Right > const &rhs ) const
  {
    auto &lhs = *this;
    if( lhs.is_left() && rhs.is_left() ) return true;
    if( lhs.is_right() && rhs.is_right() ) return lhs.right() == rhs.right();
    return false;
  }
  bool less( either_base< void, Right > const &rhs ) const
  {
    auto &lhs = *this;

    if( lhs.is_left() )
    {
      if( rhs.is_left() ) return false;
      if( rhs.is_right() ) return true;
      return false;
    }
    if( lhs.is_right() )
    {
      if( rhs.is_left() ) return false;
      if( rhs.is_right() ) return lhs.right() < rhs.right();
      return false;
    }
    if( rhs.is_left() ) return true;
    if( rhs.is_right() ) return true;
    return false;
  }

  //template < class Fn >
  //auto bind( Fn f ) -> typename bind_either< void, decltype(f( right() )) >::type
  //{
  //  auto &e = *this;
  //  //std::declval<either_base< void, Right >>().
  //  typedef typename bind_either< void, decltype(f( e.right() )) >::type result_type;
  //  if( e.is_right() )
  //    return f( e.right() );
  //  return result_type( left_args() );
  //}
};

template < class Arg >
struct is_init : std::integral_constant< bool, false
  || std::is_same< typename std::remove_cv< typename std::remove_reference< Arg >::type >::type, left_args >::value
  || std::is_same< typename std::remove_cv< typename std::remove_reference< Arg >::type >::type, right_args >::value
> {};

}

//using detail::left_args;
//using detail::right_args;
//using detail::left_init;
//using detail::right_init;

template < class Left, class Right >
class either: public detail::either_base< Left, Right >
{
  typedef detail::either_base< Left, Right > Base;
public:
  typedef Left left_type;
  typedef Right right_type;

#ifdef EITHER_EMPTY_ALLOWED
  either(): Base() {}
  either &operator=( either< Left, Right > const &e )
  {
    if( !this->empty() )
      throw std::logic_error( "non-empty either is assigned a value" );

    if ( e.is_left() )
      this->construct_copy_left( e._left() );
    if ( e.is_right() )
      this->construct_copy_right( e._right() );
    return *this;
  }
  either &operator=( either< Left, Right > &&e )
  {
    if( !this->empty() )
      throw std::logic_error( "non-empty either is assigned a value" );

    if ( e.is_left() )
      this->construct_move_left( e._left() );
    if ( e.is_right() )
      this->construct_move_right( e._right() );
    return *this;
  }
#else
  either() = delete;
  either &operator=( either< Left, Right > const &e ) = delete;
  either &operator=( either< Left, Right > &&e ) = delete;
#endif

#ifdef EITHER_VARIADIC
  template < class ...Args >
  either( left_args a, Args &&...args ): Base()
  {
    this->construct_left( std::forward<Args>(args)... );
  }
  template < class ...Args >
  either( right_args, Args &&...args ): Base()
  {
    this->construct_right( std::forward<Args>(args)... );
  }
  template < class ...Args >
  static either< Left, Right > mkleft( Args &&...args ) 
  { return either( left_args(), std::forward<Args>(args)... ); }
  template < class ...Args >
  static either< Left, Right > mkright( Args &&...args ) 
  { return either( right_args(), std::forward<Args>(args)... ); }
  
#else
  either( left_args ): Base()
  {
    this->construct_left();
  }
  either( right_args ): Base()
  {
    this->construct_right();
  }
  template < class Arg >
  either( left_args, Arg &&arg ): Base()
  {
    this->construct_left( std::forward<Arg>( arg ) );
  }
  template < class Arg >
  either( right_args, Arg &&arg ): Base()
  {
    this->construct_right( std::forward<Arg>( arg ) );
  }
  template < class Arg1, class Arg2 >
  either( left_args, Arg1 &&arg1, Arg2 &&arg2 ): Base()
  {
    this->construct_left( std::forward<Arg1>( arg1 ), std::forward<Arg2>( arg2 ) );
  }
  template < class Arg1, class Arg2 >
  either( right_args, Arg1 &&arg1, Arg2 &&arg2 ): Base()
  {
    this->construct_right( std::forward<Arg1>( arg1 ), std::forward<Arg2>( arg2 ) );
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
      && !std::is_same< typename std::remove_cv< typename std::remove_reference<Arg>::type >::type, either<Left, Right> >::value
      && !detail::is_init< Arg >::value
      && std::is_convertible< Arg, Left >::value 
      && ( !std::is_convertible< Arg, Right >::value || 
        ( std::is_same< Arg, Left >::value && !std::is_same< Arg, Right >::value ) 
        )
    >::type ** = nullptr
    ): Base()
  {
    this->construct_left( std::forward<Arg>(arg) );
  }
  template < class Arg >
  either( Arg &&arg
    , typename std::enable_if< true
      && !std::is_same< typename std::remove_cv< typename std::remove_reference<Arg>::type >::type, either<Left, Right> >::value
      && !detail::is_init< Arg >::value
      && std::is_convertible< Arg, Right >::value 
      && ( !std::is_convertible< Arg, Left >::value || 
        ( !std::is_same< Arg, Left >::value && std::is_same< Arg, Right >::value ) 
        )
    >::type ** = nullptr
    ): Base()
  {
    this->construct_right( std::forward<Arg>(arg) );
  }
  template < class Arg >
  either( Arg &&arg
    , typename std::enable_if< true
      && !std::is_same< typename std::remove_cv< typename std::remove_reference<Arg>::type >::type, either<Left, Right> >::value
      && !detail::is_init< Arg >::value
      && std::is_convertible< Arg, Left >::value 
      && std::is_convertible< Arg, Right >::value 
      && ( std::is_same< Arg, Left >::value == std::is_same< Arg, Right >::value )
    >::type ** = nullptr
    ): Base()
  {
    static_assert( !(true
      && !detail::is_init< Arg >::value
      && std::is_convertible< Arg, Left >::value
      && std::is_convertible< Arg, Right >::value 
      ), "arg is convertible to both Left type and Right type." );
  }
  template < class Arg >
  either( Arg &&arg
    , typename std::enable_if< true
      && !std::is_same< typename std::remove_cv< typename std::remove_reference<Arg>::type >::type, either<Left, Right> >::value
      && !detail::is_init< Arg >::value
      && !std::is_convertible< Arg, Left >::value 
      && !std::is_convertible< Arg, Right >::value 
    >::type ** = nullptr
    ): Base()
  {
    static_assert( !(true
      && !detail::is_init< Arg >::value
      && !std::is_convertible< Arg, Left >::value
      && !std::is_convertible< Arg, Right >::value 
      ), "arg is convertible to neither Left type nor Right type." );
  }
#endif

  either( either< Left, Right > const &e ): Base()
  {
    if ( e.is_left() )
      this->construct_copy_left( e._left() );
    if ( e.is_right() )
      this->construct_copy_right( e._right() );
  }
  either( either< Left, Right > &&e ): Base()
  {
    if ( e.is_left() )
      this->construct_move_left( e._left() );
    if ( e.is_right() )
      this->construct_move_right( e._right() );
  }
  //either &operator=( either< Left, Right > const &e ) = delete;
  //either &operator=( either< Left, Right > &&e ) = delete;
  
  ~either()
  {
    this->destruct();
  }

  either< Left *, Right * > operator&()
  {
    if( this->is_left() ) return this->_left();
    if( this->is_right() ) return this->_right();
    return either< Left *, Right * >();
  }
  either< Left const *, Right const * > operator&() const
  {
    if( this->is_left() ) return this->_left();
    if( this->is_right() ) return this->_right();
    return either< Left *, Right * >();
  }

  explicit operator bool() const throw ()
  {
    return this->is_right();
  }
};

template < class Left, class Right >
bool operator==( either< Left, Right > const &lhs, either< Left, Right > const &rhs )
{
  return lhs.equal(rhs);
}
template < class Left, class Right >
bool operator!=( either< Left, Right > const &lhs, either< Left, Right > const &rhs )
{
  return !(lhs == rhs);
}
template < class Left, class Right >
bool operator<( either< Left, Right > const &lhs, either< Left, Right > const &rhs )
{
  return lhs.less( rhs );
}


// because of associativity, `bind` operator is defined as operator >>, not as operator >>=.
template < class Left, class Right, class Fn >
auto operator>>( either< Left, Right > &e, Fn f ) 
-> typename bind_either< Left, decltype(f(e.right())) >::type
{
  typedef typename bind_either< Left, decltype(f(e.right())) >::type result_type;
  if ( e.is_right() )
    return f( e.right() );
  return result_type( left_args(), e.left() );
}
template < class Left, class Right, class Fn >
auto operator>>( either< Left, Right > &&e, Fn f ) 
-> typename bind_either< Left, decltype(f(std::move(e.right()))) >::type
{
  typedef typename bind_either< Left, decltype(f(std::move(e.right()))) >::type result_type;
  if ( e.is_right() )
    return f( std::move(e.right()) );
  return result_type( left_args(), std::move(e.left()) );
}
template < class Left, class Right, class Fn >
auto operator>>( either< Left, Right > const &e, Fn f ) 
-> typename bind_either< Left, decltype(f(e.right())) >::type
{
  typedef typename bind_either< Left, decltype(f(e.right())) >::type result_type;
  if ( e.is_right() )
    return f( e.right() );
  return result_type( left_args(), e.left() );
}
template < class Right, class Fn >
auto operator>>( either< void, Right > &e, Fn f ) 
-> typename bind_either< void, decltype(f(e.right())) >::type
{
  typedef typename bind_either< void, decltype(f(e.right())) >::type result_type;
  if( e.is_right() )
    return f( e.right() );
  return result_type( left_args() );
}
template < class Right, class Fn >
auto operator>>( either< void, Right > &&e, Fn f ) 
-> typename bind_either< void, decltype(f(std::move(e.right()))) >::type
{
  typedef typename bind_either< void, decltype(f(std::move(e.right()))) >::type result_type;
  if( e.is_right() )
    return f( std::move(e.right()) );
  return result_type( left_args() );
}
template < class Right, class Fn >
auto operator>>( either< void, Right > const &e, Fn f ) 
-> typename bind_either< void, decltype(f(e.right())) >::type
{
  typedef typename bind_either< void, decltype(f(e.right())) >::type result_type;
  if( e.is_right() )
    return f( e.right() );
  return result_type( left_args() );
}


template < class R >
using maybe = either< void, R >;

template < class R >
maybe< R > just( R const &r ) { return maybe< R >::mkright( r ); }

template < class R >
maybe< R > nothing() { return maybe< R >::mkleft(); }


struct unit {};


template < class Left, class Result >
struct fmap_either
{
  typedef either< Left, Result > type;
};

template < class Left, class Right, class Fn >
auto fmap( either< Left, Right > &e, Fn f ) 
-> typename fmap_either< Left, decltype(f( e.right() )) >::type
{
  typedef typename fmap_either< Left, decltype(f( e.right() )) >::type result_type;
  if( e.is_right() )
    return result_type( right_args(), f( e.right() ) );
  return result_type( left_args(), e.left() );
}
template < class Left, class Right, class Fn >
auto fmap( either< Left, Right > &&e, Fn f ) 
-> typename fmap_either< Left, decltype(f( std::move(e.right()) )) >::type
{
  typedef typename fmap_either< Left, decltype(f( std::move(e.right()) )) >::type result_type;
  if( e.is_right() )
    return result_type( right_args(), f( std::move(e.right()) ) );
  return result_type( left_args(), std::move(e.left()) );
}
template < class Left, class Right, class Fn >
auto fmap( either< Left, Right > const &e, Fn f ) 
-> typename fmap_either< Left, decltype(f( e.right() )) >::type
{
  typedef typename fmap_either< Left, decltype(f( e.right() )) >::type result_type;
  if( e.is_right() )
    return result_type( right_args(), f( e.right() ) );
  return result_type( left_args(), e.left() );
}
template < class Right, class Fn >
auto fmap( either< void, Right > &e, Fn f ) 
-> typename fmap_either< void, decltype(f( e.right() )) >::type
{
  typedef typename fmap_either< void, decltype(f( e.right() )) >::type result_type;
  if( e.is_right() )
    return result_type( right_args(), f( e.right() ) );
  return result_type( left_args() );
}
template < class Right, class Fn >
auto fmap( either< void, Right > &&e, Fn f ) 
-> typename fmap_either< void, decltype(f( std::move( e.right() ) )) >::type
{
  typedef typename fmap_either< void, decltype(f( std::move( e.right() ) )) >::type result_type;
  if( e.is_right() )
    return result_type( right_args(), f( std::move( e.right() ) ) );
  return result_type( left_args() );
}
template < class Right, class Fn >
auto fmap( either< void, Right > const &e, Fn f ) 
-> typename fmap_either< void, decltype(f( e.right() )) >::type
{
  typedef typename fmap_either< void, decltype(f( e.right() )) >::type result_type;
  if( e.is_right() )
    return result_type( right_args(), f( e.right() ) );
  return result_type( left_args() );
}


template < class It, class Fn >
auto for_each_( It first, It last, Fn f )
-> std::pair< either< typename std::remove_reference< decltype(f(*first++).left()) >::type, Fn >, It >
{
  typedef std::pair< either< typename std::remove_reference< decltype( f( *first++ ).left() ) >::type, Fn >, It > result_type;
  typedef typename result_type::first_type Either;
  for( ; first != last; ++first )
  {
    auto e = f( *first );
    if( e.is_left() )
      return std::make_pair( Either::mkleft( e.left() ), first );
  }
  return std::make_pair( Either::mkright(f), first );
}

#if 0
template < class Monoid, class It, class Fn >
auto msum( Monoid m0, It first, It last, Fn f )
-> std::tuple< either<decltype( f( *first++ ).left() ), void>, Monoid, It >
{
  typedef std::tuple< either<decltype( f( *first++ ).left() ), void>, Monoid, It > result_type;
  typedef decltype( f( *first++ ) ) Either;
  typedef typename Either::left_type Left;
  typedef typename Either::right_type MonoidElem;

  for( ; first != last; ++first )
  {
    auto e = f( *first );
    if( e.is_left() )
      return std::make_tuple( Either::mkleft( e.left() ), m, first );
    m += e.right();
  }
  return std::make_tuple( Either::mkright(), m, first );
}

template < class It, class Fn >
auto msum( It first, It last, Fn f ) 
-> std::tuple< either<decltype( f( *first++ ).left() ), void>, decltype( f( *first++ ).right() ), It >
{
  typedef std::tuple< either<decltype( f( *first++ ).left() ), void>, decltype( f( *first++ ).right() ), It > result_type;
  typedef decltype( f( *first++ ).right() ) Monoid;
  return msum( Monoid::mzero(), first, last, f );
}
#endif

//template < class Left, class Right, class Fn >
//auto with( Fn f, either<Left, Right> ) {}


//template < class Left, class Right, class Fn >
//auto operator||( either< Left, Right > const &e, Fn fn )
//{
//  if( e.is_right() )
//    return e;
//
//}



#ifdef EITHER_NAMESPACE
}
#endif

#endif
