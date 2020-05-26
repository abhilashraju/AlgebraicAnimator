#include <QApplication>
#include <QWidget>
#include <deque>
#include <QTimer>
#include <QPainter>
#include <QDebug>
#include "functionalmath.h"
#include "qpoint.h"
namespace AMath {
 QPointF operator+(const QPointF& first,const QPointF& second)
 {
     return ::operator+(first,second);
 }
}
using namespace  AMath;

struct Widget:public QWidget{

   Widget(){
   }
    void paintEvent(QPaintEvent *event)
    {
        QPainter p(this);
        p.fillRect(rect(),Qt::black);
        QPen rpen(Qt::red,3);
        QPen gpen(Qt::green,3);
        p.setPen(rpen);



        auto X = var();
        auto sinx = X - (X^3)/!3_i + (X^5)/!5_i - (X^7)/!7_i + (X^9)/!9_i - (X^11)/!11_i +(X^13)/!13_i-(X^15)/!15_i+(X^17)/!17_i;
        auto cosx = slop(sinx);
        constexpr auto PI = 3.14;
        constexpr auto increment = 2 * PI/360;
        QPointF center(width()/2,height()/2);
//        forall(X<100,[&,radius=20]()mutable{
//            forall(X<360,[&,i=0.]()mutable{
//                i = i+ increment;
//                auto pt = QPointF(radius * cosx(i)+center.x(), center.y() - radius * sinx(i));
//                p.drawPoint(pt);

//            });
//            radius +=1;
//        });








        auto pie = [&](auto start,auto range,auto pen){
            p.setPen(pen);
            forall(X<100,[&,radius=20]()mutable{
                forall(X<range,[&,i=start]()mutable{
                    i = i+ increment;
                    auto pt = QPointF(radius * cosx(i)+center.x(), center.y() - radius * sinx(i));
                    p.drawPoint(pt);

                });
                radius +=1;
            });
            return start+range*increment;
        };
        pie(
            pie(
                pie(
                    pie(0.,90.,QPen(Qt::green,3))
                    ,40.,QPen(Qt::white,3))
                ,140.,QPen(Qt::yellow,3))
            ,90.,QPen(Qt::blue,3));





    }



};


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.resize(1500,800);
    w.show();

    return a.exec();
}
