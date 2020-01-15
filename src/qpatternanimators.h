#ifndef QPATTERNANIMATORS_H
#define QPATTERNANIMATORS_H
#include "qtypetraits.h"
#include "qobject.h"
#include "qparallelanimationgroup.h"
#include "qpropertyanimation.h"
#include "qsequentialanimationgroup.h"
#include "qpauseanimation.h"
#include <memory>
#include <deque>
namespace Animation{
//#define S__PRETTY_FUNCTION__ std::string(__PRETTY_FUNCTION__).left(50).charString()
#define S__PRETTY_FUNCTION__ __PRETTY_FUNCTION__
struct patternFunctionlogger
{
    static std::string& getIndentationString()
    {
        static std::string str;
        return  str;
    }
    const char* log;
    patternFunctionlogger(const char* a):log(a){
//        getIndentationString().append("  ");
//        SDEBUG_AT_LEVEL(0)<<getIndentationString()<<"Pat Fun "<<log<<"Start";
    }
    ~patternFunctionlogger(){
//        SDEBUG_AT_LEVEL(0)<<getIndentationString()<<"Pat Fun "<<log<<"End";
//        getIndentationString().remove(getIndentationString().length()-2);
    }
};

struct patternobjectlogger
{
    const char* log;
    patternobjectlogger(const char* a):log(a){
//        patternFunctionlogger::getIndentationString().append("  ");
//        SDEBUG_AT_LEVEL(0)<<patternFunctionlogger::getIndentationString()<<"Pat Obj"<<log<<"Start";
    }
    ~patternobjectlogger(){
//        SDEBUG_AT_LEVEL(0)<<patternFunctionlogger::getIndentationString()<<"Pat Obj"<<log<<"End";
//        patternFunctionlogger::getIndentationString().remove(patternFunctionlogger::getIndentationString().length()-2);
    }
};


template<typename T>
class QAnimationGenericWrapper
{
    mutable T* mAnimation;
public:
    QAnimationGenericWrapper(T* animation=nullptr):mAnimation(animation){}
    template <typename U>
    QAnimationGenericWrapper(const QAnimationGenericWrapper<U>& otheranimation):mAnimation(otheranimation.get()){}
   // T* operator->(){return get();}
    T* get() const{return mAnimation;}

    QAnimationGenericWrapper start() const
    {
        get()->start(QAbstractAnimation::DeleteWhenStopped);
        return*this;
    }
    QAnimationGenericWrapper setLoopCount(int val) const
    {
        get()->setLoopCount(val);
        return *this;
    }
    template<typename U>
    QAnimationGenericWrapper onfinish(U func) const
    {
//        static_assert(s_is_callable<U,QVariant>::value,"pass lamda of type [](QVariant){}");
        QObject::connect(get(),"finished",func);
        return *this;
    }
    QAnimationGenericWrapper stop() const
    {
        if(mAnimation)
            mAnimation->stop();
        mAnimation=nullptr;
        return *this;
    }

};
using QAnimationWrapper= QAnimationGenericWrapper<QAbstractAnimation>;
using QAnimationGroupWrapper= QAnimationGenericWrapper<QAnimationGroup>;
using animator= std::function<QAnimationWrapper()>;
using groupanimator=std::function<QAnimationGroupWrapper()>;

enum AGroupType
{
    Parallel,
    Sequential
};



inline animator getObjectAnimator(const QObject* w, const std::string& prop, const QVariant& start,
                                  const QVariant& end, int dur,QEasingCurve curve = QEasingCurve::Linear)
{
    patternFunctionlogger a(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationWrapper {
       patternobjectlogger a(S__PRETTY_FUNCTION__);
       QPropertyAnimation* anim = new QPropertyAnimation(const_cast<QObject*>(w),prop.data());
       anim->setStartValue(start);
       anim->setEndValue(end);
       anim->setDuration(dur);
       anim->setEasingCurve(curve);
       return anim;
    };
}

//inline animator getObjectAnimator_withGetter(const QObject* w, const std::string& prop, std::function<QVariant()> sgetter,
//                                 std::function<QVariant()> endgetter, int dur,QEasingCurve curve = QEasingCurve::Linear)
//{
//    patternFunctionlogger a(S__PRETTY_FUNCTION__);
//    return [=]()->QAnimationWrapper {
//       patternobjectlogger a(S__PRETTY_FUNCTION__);
//       QPropertyAnimation* anim = new QPropertyAnimation(const_cast<QObject*>(w),prop.data());
//       anim->setStartValueGetter(sgetter);
//       anim->setEndValueGetter(endgetter);
//       anim->setDuration(dur);
//       anim->setEasingCurve(curve);
//       return anim;
//    };
//}
template <typename Func>
struct InterPolaterAdaptor:public QObject
{
    Func fun;
    QObject* target;
    std::string tprop;
    int start;
    int end;

    InterPolaterAdaptor(QObject* t,const std::string& prop,int s,int e,Func f):fun(std::move(f)),target(t),tprop(prop),start(s),end(e){}
    bool handleSetProperty(const std::string&  n, const QVariant& v){
        if(n == "value"){
            target->setProperty(tprop,fun(v.toInt()));
            return  true;
        }
        return false;
    }

};
template <typename INTERPOLATOR>
inline animator getObjectAnimator(const QObject* w, const std::string& prop, int start,
                                  int end, int dur,QEasingCurve curve,INTERPOLATOR func)
{
    patternFunctionlogger a(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationWrapper {
       patternobjectlogger a(S__PRETTY_FUNCTION__);
       InterPolaterAdaptor<INTERPOLATOR>* adaptor=new  InterPolaterAdaptor<INTERPOLATOR>(const_cast<QObject*>(w),prop,start,end,std::move(func));
       QPropertyAnimation* anim = new QPropertyAnimation(adaptor,"value");
       adaptor->setParent(anim);
       anim->setStartValue(start);
       anim->setEndValue(end);
       anim->setDuration(dur);
       anim->setEasingCurve(curve);
       return anim;
    };
}


inline animator getPauseAnimator(int duration){
    patternFunctionlogger a(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationWrapper {
       patternobjectlogger a(S__PRETTY_FUNCTION__);
       QPauseAnimation * anim = new QPauseAnimation ();
       anim->setDuration(duration);
       return anim;
    };
}
inline animator getPauseAnimator(std::chrono::milliseconds msecs){
    patternFunctionlogger a(S__PRETTY_FUNCTION__);
    return getPauseAnimator(msecs.count());
}


template<typename U>
bool appendAnimation(const QAnimationGroupWrapper& grp, U anim)
{
    patternobjectlogger a(S__PRETTY_FUNCTION__);
    grp.get()->addAnimation(anim().get());
    return true;
}
template<typename U>
bool prependAnimation(const QAnimationGroupWrapper& grp, const U& anim)
{
    patternobjectlogger a(S__PRETTY_FUNCTION__);
    grp.get()->insertAnimation(0,anim().get());
    return true;
}


template<typename... List>
void appender(const QAnimationGroupWrapper& );
inline void appender(const QAnimationGroupWrapper& ){}
template<typename T ,typename... List>
void appender(const QAnimationGroupWrapper& grp,const T& anim,const List&... l){
    appendAnimation(grp,anim);
    appender(grp,l...);
}

template<typename... T>
groupanimator getGroupAnimator(AGroupType type,const T&... a)
{

    patternFunctionlogger log(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationGroupWrapper {
        patternobjectlogger temp(S__PRETTY_FUNCTION__);
        static_assert(sizeof...(T)<3,"Max Animator argument is 2");
        static_assert(sizeof...(T)>0,"Min Animator argument is 1");
        QAnimationGroup* group=0;
        if(type==Parallel){
            group = new  QParallelAnimationGroup ();
        }
        else{

            group=new  QSequentialAnimationGroup();
        }
        appender(group,a...);
        return group;
    };
}

template<typename T,typename U>
groupanimator getOrAnimator(const T& a1,const U& a2);
template<typename T>
groupanimator getOrAnimator(const T& a1,const T& a2){
    patternFunctionlogger log(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationGroupWrapper {
       patternobjectlogger a(S__PRETTY_FUNCTION__);
       groupanimator grp= getGroupAnimator(AGroupType::Sequential,a1,a2);
       return grp();
    };
}

template<typename AnimationType>
inline std::unique_ptr<QAnimationGroup>
      getGroupIfMatch(const groupanimator& grpanim )
{
    std::unique_ptr<QAnimationGroup> group;

    if(grpanim){
          group.reset( grpanim().get());
    }

    if(dynamic_cast<AnimationType*>(group.get())){
         return group;
    }
    return std::unique_ptr<QAnimationGroup>();//return null
}
inline groupanimator getOrAnimator(const groupanimator& grpanim,const animator& a){
    patternFunctionlogger log(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationGroupWrapper {
       patternobjectlogger log(S__PRETTY_FUNCTION__);
       auto animation = getGroupIfMatch<QSequentialAnimationGroup>(grpanim);
       if(animation){
           appendAnimation(animation.get(),a);
           return  animation.release();
       }
       groupanimator grp2= getGroupAnimator(AGroupType::Sequential,grpanim,a);
       return grp2();
    };
}

inline groupanimator getOrAnimator(const animator& a,const groupanimator& grpAnim){
    patternFunctionlogger log(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationGroupWrapper {
       patternobjectlogger log(S__PRETTY_FUNCTION__);
       auto animation = getGroupIfMatch<QSequentialAnimationGroup>(grpAnim);
       if(animation){
           prependAnimation(animation.get(),a);
           return  animation.release();
       }
       groupanimator grp2= getGroupAnimator(AGroupType::Sequential,a,grpAnim);
       return grp2();
    };
}

template<typename T,typename U>
groupanimator getAndAnimator(const T& a1,const U& a2);

template<typename T>
groupanimator getAndAnimator(const T& a1,const T& a2)
{
    patternFunctionlogger log(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationGroupWrapper {
       patternobjectlogger a(S__PRETTY_FUNCTION__);
       groupanimator grp= getGroupAnimator(AGroupType::Parallel,a1,a2);
       return grp();
    };
}

inline groupanimator getAndAnimator(const groupanimator& grpAnim,const animator& a){
    patternFunctionlogger log(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationGroupWrapper {
       patternobjectlogger log(S__PRETTY_FUNCTION__);
       auto animation = getGroupIfMatch<QParallelAnimationGroup >(grpAnim);
       if(animation){
           appendAnimation(animation.get(),a);
           return  animation.release();
       }
       groupanimator grp2= getGroupAnimator(AGroupType::Parallel,grpAnim,a);
       return grp2();
    };
}

inline groupanimator getAndAnimator(const animator& a,const groupanimator& grp){
    return getAndAnimator(grp,a);
}
inline groupanimator operator |(const groupanimator& a1,const groupanimator& a2)
{
   return getOrAnimator(a1,a2);
}

inline groupanimator operator |(const groupanimator& a1,const animator& a2)
{
   return getOrAnimator(a1,a2);
}
inline groupanimator operator |(const animator& a1,const groupanimator& a2)
{
   return getOrAnimator(a1,a2);
}
inline groupanimator operator |(const animator& a1,const animator& a2)
{
   return getOrAnimator(a1,a2);
}

inline groupanimator operator &(const groupanimator& a1,const groupanimator& a2)
{
   return getAndAnimator(a1,a2);
}

inline groupanimator operator &(const groupanimator& a1,const animator& a2)
{
   return getAndAnimator(a1,a2);
}
inline groupanimator operator &(const animator& a1,const groupanimator& a2)
{
   return getAndAnimator(a1,a2);
}
inline groupanimator operator &(const animator& a1,const animator& a2)
{
   return getAndAnimator(a1,a2);
}


inline groupanimator operator*(const groupanimator& anim,int count){
    patternFunctionlogger log(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationGroupWrapper {
       patternobjectlogger log(S__PRETTY_FUNCTION__);
       QAnimationGroup* group = anim().get();
       group->setLoopCount(count);
       return group;
    };
}

inline animator operator*(const animator& anim,int count){
    patternFunctionlogger log(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationWrapper {
       patternobjectlogger log(S__PRETTY_FUNCTION__);
       QAbstractAnimation* animation = anim().get();
       animation->setLoopCount(count);
       return animation;
    };
}
inline void reverseAnimation(QAbstractAnimation* anim)
{
    QAnimationGroup* group = dynamic_cast<QAnimationGroup*>(anim);
    if(group){
        std::deque<QAbstractAnimation*> children;
        while(group->animationCount()){
            children.emplace_front(group->takeAnimation(0));
        }
        foreach (QAbstractAnimation* _anim, children) {
            reverseAnimation(_anim);
            group->addAnimation(_anim);
        }
    }
    QPropertyAnimation* animation = dynamic_cast<QPropertyAnimation*>(anim);
    if(animation){
        QVariant startValue = animation->startValue();
        QVariant endValue = animation->endValue();
        animation->setStartValue(endValue);
        animation->setEndValue(startValue);
    }
}

inline groupanimator operator~(const groupanimator& anim){
    patternFunctionlogger a(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationGroupWrapper {
       patternobjectlogger log(S__PRETTY_FUNCTION__);
       QAnimationGroup* group = anim().get();
       reverseAnimation(group);
       return group;
    };
}

inline animator operator~(const animator& anim){
    patternFunctionlogger a(S__PRETTY_FUNCTION__);
    return [=]()->QAnimationWrapper {
       patternobjectlogger log(S__PRETTY_FUNCTION__);
       QAbstractAnimation* animation = anim().get();
       reverseAnimation(animation);
       return animation;
    };
}

inline groupanimator getNullAnimator(){
    return getPauseAnimator(0) & getPauseAnimator(0);
}

}
#endif // QPATTERNANIMATORS_H
