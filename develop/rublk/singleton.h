#ifndef SINGLETON_H
#define SINGLETON_H

template <typename T>
class Singleton
{
public:
    static T& instance()
    {
        static T instance;
        return instance;
    }

protected:
    Singleton() {}
};

/*
//
// A是一个例子
//
class A
{
    friend class Singleton<A>;
public:
    void opA1() {
        std::cout << __FUNCTION__ << std::endl;
    }
private:
    A() {}
};
//
// 另一种A2的实现方式
//
class A2 final: public Singleton<A2>
{
    friend Singleton<A2>;
public:
//    using Singleton<A2>::instance;
    void opA2() {
        std::cout << __FUNCTION__ << std::endl;
    }
private:
    A2() {}
};
*/

#endif // SINGLETON_H
