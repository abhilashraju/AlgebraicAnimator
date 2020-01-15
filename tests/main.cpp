
#include "qpatternanimators.h"
#include "qpainter.h"
#include "qapplication.h"
#include "QGraphicsPathItem"
#include "qgraphicswidget.h"
#include "qstyle.h"
#include "qgraphicsview.h"
#include "qstate.h"
#include "QRandomGenerator"
#include "qmath.h"
#include "qstatemachine.h"
#include "qabstracttransition.h"
#include "qtimer.h"
#include "qsignaltransition.h"

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
    Button *ellipseButton = new Button(QPixmap(":/images/ellipse.png"), buttonParent);
    Button *figure8Button = new Button(QPixmap(":/images/figure8.png"), buttonParent);
    Button *randomButton = new Button(QPixmap(":/images/random.png"), buttonParent);
    Button *tiledButton = new Button(QPixmap(":/images/tile.png"), buttonParent);
    Button *centeredButton = new Button(QPixmap(":/images/centered.png"), buttonParent);
    Button *animateButton = new Button(QPixmap(), buttonParent);

    ellipseButton->setPos(-100, -100);
    figure8Button->setPos(100, -100);
    randomButton->setPos(0, 0);
    tiledButton->setPos(-100, 100);
    centeredButton->setPos(100, 100);
    animateButton->setPos(200, 0);

    scene.addItem(buttonParent);
    buttonParent->setTransform(QTransform::fromScale(0.75, 0.75), true);
    buttonParent->setPos(200, 200);
    buttonParent->setZValue(65);

    // States
    QState *rootState = new QState;
    QState *ellipseState = new QState(rootState);
    QState *figure8State = new QState(rootState);
    QState *randomState = new QState(rootState);
    QState *tiledState = new QState(rootState);
    QState *centeredState = new QState(rootState);

    // Values
    for (int i = 0; i < items.count(); ++i) {
        Pixmap *item = items.at(i);
        // Ellipse
        ellipseState->assignProperty(item, "pos",
                                         QPointF(qCos((i / 63.0) * 6.28) * 250,
                                                 qSin((i / 63.0) * 6.28) * 250));

        // Figure 8
        figure8State->assignProperty(item, "pos",
                                         QPointF(qSin((i / 63.0) * 6.28) * 250,
                                                 qSin(((i * 2)/63.0) * 6.28) * 250));

        // Random
        randomState->assignProperty(item, "pos",
                                        QPointF(-250 + QRandomGenerator::global()->bounded(500),
                                                -250 + QRandomGenerator::global()->bounded(500)));

        // Tiled
        tiledState->assignProperty(item, "pos",
                                       QPointF(((i % 8) - 4) * kineticPix.width() + kineticPix.width() / 2,
                                               ((i / 8) - 4) * kineticPix.height() + kineticPix.height() / 2));

        // Centered
        centeredState->assignProperty(item, "pos", QPointF());
    }



    QStateMachine states;
    states.addState(rootState);
    states.setInitialState(rootState);
    rootState->setInitialState(centeredState);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    for (int i = 0; i < items.count(); ++i) {
        QPropertyAnimation *anim = new QPropertyAnimation(items[i], "pos");
        anim->setDuration(750 + i * 25);
        anim->setEasingCurve(QEasingCurve::InOutBack);
        group->addAnimation(anim);
    }
    QAbstractTransition *trans = rootState->addTransition(ellipseButton, &Button::pressed, ellipseState);
    trans->addAnimation(group);

    trans = rootState->addTransition(figure8Button, &Button::pressed, figure8State);
    trans->addAnimation(group);

    trans = rootState->addTransition(randomButton, &Button::pressed, randomState);
    trans->addAnimation(group);

    trans = rootState->addTransition(tiledButton, &Button::pressed, tiledState);
    trans->addAnimation(group);

    trans = rootState->addTransition(centeredButton, &Button::pressed, centeredState);
    trans->addAnimation(group);
    auto stepside = [&](){

        auto anim =std::accumulate(std::begin(items),std::end(items),getNullAnimator(),[i=0](auto a, auto item)mutable{
            return a = a & Animation::getObjectAnimator(item,"pos",item->pos(),item->pos()+QPointF(200,0),400+ 25 * i++,
                                                        QEasingCurve::InOutBack);
        });
        return anim | ~anim;
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
        return (anim1 & anim2 & anim3 & anim4) |  ~(anim1 & anim2 & anim3 & anim4);
    };
    QObject::connect(animateButton,&Button::pressed,[=](){
          ((~stepside() | dance() | ~dance() | stepside()) * 3) ().start() ;
    });

    QTimer timer;
    timer.start(125);
    timer.setSingleShot(true);
    trans = rootState->addTransition(&timer, &QTimer::timeout, ellipseState);
    trans->addAnimation(group);

    states.start();

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
