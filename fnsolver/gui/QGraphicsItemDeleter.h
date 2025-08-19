#ifndef FNSOLVER_GUI_QGRAPHICSITEMDELETER_H
#define FNSOLVER_GUI_QGRAPHICSITEMDELETER_H

#include <QGraphicsItem>
#include <QGraphicsScene>

/**
 * Deleter for QGraphicsItem.
 *
 * Removes item from scene (if applicable) before deleting.
 */
struct QGraphicsItemDeleter
{
    void operator()(QGraphicsItem* item) const
    {
        if (auto* scene = item->scene())
        {
            scene->removeItem(item);
        }
        delete item;
    }
};


#endif //FNSOLVER_GUI_QGRAPHICSITEMDELETER_H
