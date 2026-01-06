#ifndef BIRD_H
#define BIRD_H

#include <QGraphicsPixmapItem>
#include <QPixmap>

class Bird : public QGraphicsPixmapItem {
public:
    Bird();
    void flap();
    void updatePosition();
    void reset();
    qreal velocity;
    qreal gravity;
    qreal lift;
};

#endif // BIRD_H
