
#include "qpatternanimators.h"
#include "qpainter.h"
#include "qapplication.h"
#include "QGraphicsPathItem"
#include "qgraphicswidget.h"
#include "qstyle.h"
#include "qgraphicsview.h"
#include "QRandomGenerator"
#include "qmath.h"
#include "qtimer.h"


class Pixmap : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    Pixmap(const QPixmap &pix)
        : QObject(), QGraphicsPixmapItem(pix)
    {
        setCacheMode(DeviceCoordinateCache);
    }
};

class Button : public QGraphicsWidget
{
    Q_OBJECT
public:
    Button(const QPixmap &pixmap, QGraphicsItem *parent = nullptr)
        : QGraphicsWidget(parent), _pix(pixmap)
    {
        setAcceptHoverEvents(true);
        setCacheMode(DeviceCoordinateCache);
    }

    QRectF boundingRect() const override
    {
        return QRectF(-65, -65, 130, 130);
    }

    QPainterPath shape() const override
    {
        QPainterPath path;
        path.addEllipse(boundingRect());
        return path;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) override
    {
        bool down =false;
        QRectF r = boundingRect();
        QLinearGradient grad(r.topLeft(), r.bottomRight());
        grad.setColorAt(down ? 0 : 1, Qt::darkGray);
        painter->setPen(Qt::darkGray);
        painter->setBrush(grad);
        painter->drawEllipse(r);
        QLinearGradient grad2(r.topLeft(), r.bottomRight());
        grad.setColorAt(down ? 1 : 0, Qt::darkGray);
        grad.setColorAt(down ? 0 : 1, Qt::lightGray);
        painter->setPen(Qt::NoPen);
        painter->setBrush(grad);
        if (down)
            painter->translate(2, 2);
        painter->drawEllipse(r.adjusted(5, 5, -5, -5));
        painter->drawPixmap(-_pix.width()/2, -_pix.height()/2, _pix);
    }

signals:
    void pressed();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *) override
    {
        emit pressed();
        update();
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override
    {
        update();
    }

private:
    QPixmap _pix;
};

class View : public QGraphicsView
{
public:
    View(QGraphicsScene *scene) : QGraphicsView(scene) { }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QGraphicsView::resizeEvent(event);
        fitInView(sceneRect(), Qt::KeepAspectRatio);
    }
};
using namespace Animation;
int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(animatedtiles);

    QApplication app(argc, argv);

    QPixmap kineticPix(":/images/kinetic.png");
    QPixmap bgPix(":/images/Time-For-Lunch-2.jpg");

    QGraphicsScene scene(-350, -350, 700, 700);

    QList<Pixmap *> items;
    for (int i = 0; i < 64; ++i) {
        Pixmap *item = new Pixmap(kineticPix);
        item->setOffset(-kineticPix.width()/2, -kineticPix.height()/2);
        item->setZValue(i);
        items << item;
        scene.addItem(item);
    }

    // Buttons
    QGraphicsItem *buttonParent = new QGraphicsRectItem;
    Button *animateButton = new Button(QPixmap(), buttonParent);
    animateButton->setPos(200, 0);

    scene.addItem(buttonParent);
    buttonParent->setTransform(QTransform::fromScale(0.75, 0.75), true);
    buttonParent->setPos(200, 200);
    buttonParent->setZValue(65);

    // States


    // Values
    for (int i = 0; i < items.count(); ++i) {
        Pixmap *item = items.at(i);
        // Ellipse
        item->setPos(QPointF());

    }


    QObject::connect(animateButton,&Button::pressed,[=,animtype=0]()mutable{
        auto stepside = [&](bool right){
            auto anim =std::accumulate(std::begin(items),std::end(items),getNullAnimator(),[i=0,right](auto a, auto item)mutable{
                auto slide = QPointF(right ? 200:-200,0);
                return a = a & Animation::getObjectAnimator(item,"pos",item->pos(),item->pos()+slide,400+ 25 * i++,
                                                            QEasingCurve::InOutBack);
            });
            return  anim;
        };
        auto dance = [&](){
            int quarter = items.count()/4;
            auto anim1 =std::accumulate(std::begin(items),std::begin(items)+quarter,getNullAnimator(),[i=0](auto a, auto item)mutable{
                return a = a & Animation::getObjectAnimator(item,"pos",item->pos(),item->pos()+QPointF(200,0),400+ 25 * i++,
                                                            QEasingCurve::InOutBack);
            });
            auto anim2 =std::accumulate(std::begin(items)+quarter,std::begin(items)+2*quarter,getNullAnimator(),[i=0](auto a, auto item)mutable{
                return a = a & Animation::getObjectAnimator(item,"pos",item->pos(),item->pos()+QPointF(-200,0),400+ 25 * i++,
                                                            QEasingCurve::InOutBack);
            });
            auto anim3 =std::accumulate(std::begin(items)+2*quarter,std::begin(items)+3*quarter,getNullAnimator(),[i=0](auto a, auto item)mutable{
                return a = a & Animation::getObjectAnimator(item,"pos",item->pos(),item->pos()+QPointF(0,200),400+ 25 * i++,
                                                            QEasingCurve::InOutBack);
            });
            auto anim4 =std::accumulate(std::begin(items)+3*quarter,std::end(items),getNullAnimator(),[i=0](auto a, auto item)mutable{
                return a = a & Animation::getObjectAnimator(item,"pos",item->pos(),item->pos()+QPointF(0,-200),400+ 25 * i++,
                                                            QEasingCurve::InOutBack);
            });
            return (anim1 & anim2 & anim3 & anim4) ;
        };
        auto make_round =[&](){
            auto stepangle = 2*3.14 / items.count();
            auto radius=150;
            auto anim1 =std::accumulate(std::begin(items),std::end(items),getNullAnimator(),[=,i=0](auto a, auto item)mutable{
                return a = a & Animation::getObjectAnimator(item,"pos",item->pos(),item->pos()+QPointF(radius*std::cos(stepangle*i),radius*std::sin(stepangle*i)),400+ 25 * i++,
                                                            QEasingCurve::InOutBack);
            });


            return anim1 ;
        };
        auto stepleft = stepside(false);
        auto stepright = stepside(true);
        auto jump = dance();
        auto round=make_round();
        switch (animtype++ % 3) {
        case 0:
             ((stepright | ~stepright| jump | ~jump | stepleft | ~stepleft | jump | ~jump)*3) ().start() ;
            break;
        case 1:
            (round | ~round )().start();
            break;
        case 2:
            ((stepright | round | ~round | ~stepright|  stepleft | round | ~round |~stepleft | jump | ~jump)*3) ().start() ;
            break;
        default:
            break;
        }



    });



    // Ui
    View *view = new View(&scene);
    view->setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "Animated Tiles"));
    view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    view->setBackgroundBrush(bgPix);
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    view->show();
    return app.exec();
}

#include "main.moc"
