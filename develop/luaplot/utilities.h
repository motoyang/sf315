#ifndef UTILITIES_H
#define UTILITIES_H

void stackindex_stdcout(lua_State *ls, int idx);
void dumpStack(const char* where, lua_State* L, void(*f)(lua_State*, int) = stackindex_stdcout);

// --

class Spin_lock
{
public:
  Spin_lock( void );
  void lock( void );
  bool try_lock( void );
  void unlock( void );

protected:
  Spin_lock(const Spin_lock&) = delete;
  Spin_lock& operator =(const Spin_lock&) = delete;
  Spin_lock(Spin_lock&&) = delete;
  Spin_lock& operator =(Spin_lock&&) = delete;

private:
  std::atomic<bool> d_atomic_bool;
};

// --
/*
#include <utility>
#include <tuple>

template <typename Tuple, typename F, std::size_t... Idx>
void visit_tuple(std::size_t i, Tuple& t, F&& f, std::index_sequence<Idx...>)
{
//    bool arr[] = {(i == Idx && ((std::forward<F>(f)(std::get<Idx>(t))), false)) ...};
    std::initializer_list<bool>{(i == Idx && ((std::forward<F>(f)(std::get<Idx>(t))), false)) ...};
}
*/
/* example of visit_tuple
    auto t1 = std::make_tuple(1, 2, "aaa", "ccc", 2.3, "ace", 1e-10, 'c');
    for (std::size_t i = 0; i < std::tuple_size<decltype(t1)>::value; ++i) {
        visit(i, t1, [](auto p) {
            std::cout << p << ", ";
        }, std::make_index_sequence<std::tuple_size<decltype(t1)>::value>{});
    }
*/


#endif // UTILITIES_H
