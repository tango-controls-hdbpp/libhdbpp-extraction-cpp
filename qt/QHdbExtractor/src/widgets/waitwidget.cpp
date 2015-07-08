#include "waitwidget.h"
#include <QProgressBar>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QLabel>

WaitWidget::WaitWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *lo = new QVBoxLayout(this);
    QProgressBar *p = new QProgressBar(this);
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QLabel *l = new QLabel(this);
    lo->addWidget(l);
    sp.setVerticalStretch(2);
    l->setSizePolicy(sp);
    QFont f = l->font();
    f.setBold(true);
    l->setFont(f);
    l->setText("Database query sent... please wait...");
    l->setAlignment(Qt::AlignCenter|Qt::AlignBottom);

    sp.setVerticalStretch(4);
    p->setSizePolicy(sp);
    lo->addWidget(p);
    p->setMinimum(0);
    p->setMaximum(0);


}

