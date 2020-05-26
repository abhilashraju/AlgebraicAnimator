#ifndef FUNCTIONALMATH_H
#define FUNCTIONALMATH_H
//author Abhilash R
#include<cmath>
#include <stdio.h>
namespace AMath{

constexpr auto var()
    {
         return [](auto x){
            return x;
        };
    }
    struct Int
    {
        unsigned long long int i;
        operator unsigned long long int(){return i;}
    };
    struct Real
    {
        long double d;
        operator long double(){return d;}
    };
    Int operator""_i(unsigned long long int x){
           return Int{x};
    }
    Real operator""_d( long double x){
        return Real{x};
    }
    template<typename T, typename U , typename Fun>
    struct BinaryFun
    {
        T t;
        U u;
        Fun fun;
        BinaryFun(T t,U u, Fun fun):t(std::move(t)),u(std::move(u)),fun(std::move(fun)){

        }
        auto operator()(auto x)const{
            return fun(t(x),u(x));
        }
    };
    template<typename T, typename Fun>
    struct BinaryFun<T,long long int,Fun>
    {
        T t;
        int u;
        Fun fun;
        BinaryFun(T t,long long int u, Fun fun):t(std::move(t)),u(std::move(u)),fun(std::move(fun)){

        }
        auto operator()(auto x)const{
            return fun(t(x),u);
        }
    };
    template<typename T, typename Fun>
    struct BinaryFun<T, double,Fun>
    {
        T t;
        double u;
        Fun fun;
        BinaryFun(T t, double u, Fun fun):t(std::move(t)),u(std::move(u)),fun(std::move(fun)){

        }
        auto operator()(auto x)const{
            return fun(t(x),u);
        }
    };
    #define DEFINE_OPERATORE(OP) \
    template <typename T, typename U> \
    auto operator OP(T&& t, U&& u) \
    {\
        return BinaryFun(std::forward<T>(t),std::forward<U>(u),[](auto a, auto b){\
            return a OP b;\
        });\
    }\
    template <typename T> \
    auto operator OP(T&& t, double r) \
    { \
        return BinaryFun(std::forward<T>(t),r,[](auto a, auto b){ \
            return a OP b; \
        }); \
    } \
    template <typename T> \
    auto operator OP( double r ,T&& t) \
    { \
        return std::forward<T>(t) OP r; \
    } \
    template <typename T> \
    auto operator OP(T&& t, int r) \
    { \
          return BinaryFun(std::forward<T>(t),r,[](auto a, auto b){ \
            return a OP b; \
        }); \
    } \
    template <typename T> \
    auto operator OP( int r ,T&& t) \
    { \
        return std::forward<T>(t) OP r; \
    }

    DEFINE_OPERATORE(*)
    DEFINE_OPERATORE(/)
    DEFINE_OPERATORE(+)
    DEFINE_OPERATORE(-)
    long long int fact(long long int i)
    {
        if(i<=1) return 1;
        return i* fact(i-1);
    }
    double operator!(Int i){
         return fact(i.i);
    }
    auto slop(auto f){
        return [f=std::move(f)](double x){
            double epsilon =0.00001;
            return (f(x+epsilon)-f(x))/epsilon;
        };
    }
    constexpr auto operator ^(auto f, long long int x){
        return [=,f=std::forward<decltype(f)>(f)](double r){
            if(std::abs(x) >=1){
                auto temp=std::abs(x) ;
                auto result = f(x>0 ? r:1/r);
                while(temp-->1){
                    result *= f(x>0 ? r:1/r);
                }
                return result;
            }
            return 1.;
        };


    }
    auto forall(auto expr,auto r)
    {
        int count =0;
        while(expr(count++))
        {
              r();
        }
    };
    auto _less(auto v){
        return [=](auto u){
            return u<v;
        };
    };
    auto operator<(decltype (var()) x,int bound)
    {
        auto lim=x(bound);
        return [=](int val){
            return val < lim;
        };
    }
}

#endif
