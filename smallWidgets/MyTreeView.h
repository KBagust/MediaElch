#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QTreeView>

class MyTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit MyTreeView(QWidget *parent = 0);
protected:
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
};

#endif // MYTREEVIEW_H
