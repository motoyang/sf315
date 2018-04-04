#pragma once

template <class T, typename Tuple>
struct Constructor2;

template <class T, typename... Args>
struct Constructor2 <T, std::tuple<Args...>>
{
private:
    static T* create(Args&&... args)
    {
        return new T(std::forward<Args>(args)...);
    }

    struct Creater
    {
        void* mem;
        Creater(void* p) : mem(p) {}
        T* createWithPointer( Args&&... args)
        {
            return new (mem) T(std::forward<Args>(args)...);
        }
    };

public:
  static T* call (const ArgList2<2, std::tuple<Args...>> &tvl)
  {
    return std::apply(create, tvl.tuple());
  }

  static T* call (void* mem, const ArgList2<2, std::tuple<Args...>> &tvl)
  {
      Creater c(mem);
      return obj_apply(&c, &Creater::createWithPointer, tvl.tuple());
  }
};
