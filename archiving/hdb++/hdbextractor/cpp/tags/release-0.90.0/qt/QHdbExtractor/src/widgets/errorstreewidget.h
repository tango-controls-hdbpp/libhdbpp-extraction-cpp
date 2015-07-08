#ifndef ERRORSTREEWIDGET_H
#define ERRORSTREEWIDGET_H

#include <QTreeWidget>

class ErrorsTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit ErrorsTreeWidget(QWidget *parent = 0);

    QTreeWidgetItem *addItem(const QString& src, double timestamp, int code, const QString& message);

    QTreeWidgetItem *findItem(const QString& src, double timestamp);

    void select(QTreeWidgetItem *it);

signals:

public slots:

};

#endif // ERRORSTREEWIDGET_H
