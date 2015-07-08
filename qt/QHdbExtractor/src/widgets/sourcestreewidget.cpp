#include "sourcestreewidget.h"
#include "sourcestreewidget_p.h"
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QtDebug>
#include <hdbxmacros.h>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QMimeData>

/* test/device/1/double_scalar */

SourcesTreeWidget::SourcesTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    d_ptr = new SourcesTreeWidgetPrivate();
    setDragEnabled(true);
    setMouseTracking(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDragDropMode(QTreeView::DragDrop);
}

QStringList SourcesTreeWidget::getSources() const
{
    QStringList ret;
    QList<QTreeWidgetItem *> domItems = findItems("*", Qt::MatchWildcard);
    foreach(QTreeWidgetItem *domIt, domItems)
    {
        for(int i = 0; i < domIt->childCount(); i++)
        {
            QTreeWidgetItem *familyIt = domIt->child(i);
            for(int h = 0; h < familyIt->childCount(); h++)
            {
                QTreeWidgetItem *memberIt = familyIt->child(h);
                for(int l = 0; l < memberIt->childCount(); l++)
                {
                    QTreeWidgetItem *attIt = memberIt->child(l);
                    ret << attIt->data(0, Qt::UserRole).toString();
                }
            }
        }
    }
    qDebug() << "THdbConfigDialog::m_sourcesFromTree() " << ret;
    return ret;
}

void SourcesTreeWidget::removeSelectedSources()
{
    foreach(QTreeWidgetItem *it, selectedItems())
        delete it;
}

QString SourcesTreeWidget::buildItemText(const QTreeWidgetItem *it) const
{
  QString text;
  while(it)
  {
        text = ("/" + it->text(0)) + text;
        it = it->parent();
  }
  text.remove(0, 1); /* remove first / */
  return text;
}

int SourcesTreeWidget::level(QTreeWidgetItem *it) const
{
  int lev = 0;
  QTreeWidgetItem *item = it;
  while(item)
  {
        lev++;
        item = item->parent();
  }
  return lev;
}

QMimeData * SourcesTreeWidget::mimeData( const QList<QTreeWidgetItem *> items ) const
{
  if(!items.size() == 1)
        return NULL;
  QMimeData *mimeData = new QMimeData();
  if(level(items.first()) >= 4)
  {
        QString text = buildItemText(items.first());
        mimeData->setText(text);
  }
  else
        return NULL;
  return mimeData;
}

void SourcesTreeWidget::dragMoveEvent ( QDragMoveEvent * event )
{
   if (event->mimeData()->hasFormat("text/plain") ||
           event->mimeData()->hasFormat("text/xml"))
   {
         event->acceptProposedAction();
   }
}

void SourcesTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("text/plain") ||
          event->mimeData()->hasFormat("text/xml"))
        event->acceptProposedAction();
}

void SourcesTreeWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if(mimeData->hasText())
    {
        QRegExp rx("([a-zA-z0-9_\\-\\.]+/[a-zA-z0-9_\\-\\.]+/[a-zA-z0-9_\\-\\.]+/[a-zA-z0-9_\\-\\.]+)");
        QString txt = mimeData->text();
        QStringList list;
        if(txt.contains(rx))
        {
            int pos = 0;
            while ((pos = rx.indexIn(txt, pos)) != -1)
            {
                list << rx.cap(1);
                pos += rx.matchedLength();
            }
        }
        foreach(QString src, list)
            addSource(src, true);
    }
    /* test/device/1/double_scalar */
}

QStringList SourcesTreeWidget::selectedSources() const
{
    QStringList srcs;
    foreach(QTreeWidgetItem *it, selectedItems())
    {
        QString src = it->data(0, Qt::UserRole).toString();
        if(src.split('/', QString::SkipEmptyParts).size() == level(it))
            srcs << src;
        else
            qDebug() << __FUNCTION__ << "cant add" << src << " Cuz" <<
                        src.split('/', QString::SkipEmptyParts).size() << "!=" << level(it) << it->text(0);
    }

    return srcs;
}

QTreeWidgetItem* SourcesTreeWidget::findChild(QTreeWidgetItem *parent, const QString& name)
{
    QTreeWidgetItem *ret = NULL;
    if(!parent)
    {
        QList<QTreeWidgetItem *> itList = findItems(name, Qt::MatchFixedString);
        if(itList.size())
            ret = itList.first();
    }
    else
    {
        for(int i = 0; i < parent->childCount(); i++)
        {
            if(!parent->child(i)->text(0).compare(name, Qt::CaseInsensitive))
            {
                ret = parent->child(i);
                break;
            }
        }
    }
    return ret;
}

void  SourcesTreeWidget::addSource(const QString &name, bool expand)
{
    QTreeWidgetItem* parent = NULL, *found = NULL;
    QString dom, fam, mem, att, facility, proto;
    QStringList sl = name.split('/', QString::SkipEmptyParts);
    for(int i = 0; i < sl.size(); i++)
    {
        QString pt = sl.at(i);
        if(pt.endsWith("::"))
            pt.remove("::");
        found = findChild(parent, pt);
        if(!parent && !found)
            parent = new QTreeWidgetItem(this, QStringList() << pt);
        else if(!found)
            parent = new QTreeWidgetItem(parent, QStringList(pt));
        else
            parent = found;

        if(i < 2)
            parent->setExpanded(true);
        if(i == sl.size() - 1)
            parent->setData(0, Qt::UserRole, name);
    }
//    if(name.count('/') == 3)
//    {
//        dom = sl.first();
//        fam = sl.at(1);
//        mem = sl.at(2);
//        att = sl.last();

//        QTreeWidgetItem *domIt = NULL, *famIt = NULL, *memIt = NULL, *attIt = NULL;
//        /* Qt::MatchFixedString Performs string-based matching. String-based comparisons are case-insensitive
//         * unless the MatchCaseSensitive flag is also specified.
//         */
//        QList<QTreeWidgetItem *> itList = findItems(dom, Qt::MatchFixedString);
//        if(itList.size())
//            domIt = itList.first();
//        else
//            domIt = new QTreeWidgetItem(this, QStringList() << dom);

//        /* domain is ready */
//        domIt->setExpanded(expand);
//        domIt->setFlags(domIt->flags() & ~Qt::ItemIsSelectable);

//        for(int i = 0; i < domIt->childCount(); i++)
//        {
//            if(!domIt->child(i)->text(0).compare(fam, Qt::CaseInsensitive))
//            {
//                famIt = domIt->child(i);
//                break;
//            }
//        }
//        if(!famIt) /* add a new family to domain dom */
//            famIt = new QTreeWidgetItem(domIt, QStringList() << fam);
//        famIt->setExpanded(expand);
//        famIt->setFlags(domIt->flags() & ~Qt::ItemIsSelectable);

//        /* family is ready */
//        for(int i = 0; i < famIt->childCount(); i++)
//        {
//            if(!famIt->child(i)->text(0).compare(mem, Qt::CaseInsensitive))
//            {
//                memIt = famIt->child(i);
//                break;
//            }
//        }
//        if(!memIt) /* add a new family to domain dom */
//            memIt = new QTreeWidgetItem(famIt, QStringList() << mem);

//        /* member is ready */
//        memIt->setExpanded(expand);
//        memIt->setFlags(domIt->flags() & ~Qt::ItemIsSelectable);

//        for(int i = 0; i < memIt->childCount(); i++)
//        {
//            if(!memIt->child(i)->text(0).compare(att, Qt::CaseInsensitive))
//            {
//                attIt = memIt->child(i);
//                break;
//            }
//        }
//        if(!attIt) /* add a new family to domain dom */
//            attIt = new QTreeWidgetItem(memIt, QStringList() << att);
//        attIt->setData(0, Qt::UserRole, name);
//    }
}

void SourcesTreeWidget::updateSourcesList(const QStringList& sources, bool expand)
{
    foreach(QString s, sources)
        addSource(s, expand);
}
