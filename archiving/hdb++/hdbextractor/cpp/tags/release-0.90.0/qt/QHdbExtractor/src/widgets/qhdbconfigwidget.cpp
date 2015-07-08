#include "qhdbconfigwidget.h"
#include <QTimer>
#include "treewidgetsearch.h"
#include <QtDebug>
#include <QScrollBar>
#include <QSettings>
#include <QDateTime>

QHdbConfigWidget::~QHdbConfigWidget()
{
    qDebug() << __FUNCTION__;

    /* save items in views  */
    QSettings s;
    QStringList items;
    foreach(QTreeWidgetItem *it, ui.twSelected->findItems("*", Qt::MatchWildcard))
        items << it->text(0);
    s.setValue("SourcesList", items);

    /* history entries are saved/updated when the user presses the View button */

    QDate ds = ui.calStart->selectedDate();
    QTime ts = ui.teStart->time();
    QDateTime dts(ds, ts);
    QDate de = ui.calStop->selectedDate();
    QTime te = ui.teStop->time();
    QDateTime dte(de, te);

    s.setValue("startDate", dts);
    s.setValue("stopDate", dte);
}

QHdbConfigWidget::QHdbConfigWidget(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);
    resize(minimumSizeHint());

    QTimer::singleShot(400, this, SLOT(init()));

    m_viewSourcesListChanged();
}

void QHdbConfigWidget::init()
{
    connect(ui.cbLastDaysHours, SIGNAL(toggled(bool)), ui.sbDaysHours, SLOT(setEnabled(bool)));
    connect(ui.cbLastDaysHours, SIGNAL(toggled(bool)), ui.cbDaysHours, SLOT(setEnabled(bool)));

    connect(ui.pbRemoveSrc, SIGNAL(clicked()), this, SLOT(removeSourceClicked()));
    connect(ui.pbAdd, SIGNAL(clicked()), this, SLOT(addSourceClicked()));

    connect(ui.pbView, SIGNAL(clicked()), this, SIGNAL(viewClicked()));
    connect(ui.pbCancel, SIGNAL(clicked()), this, SIGNAL(cancelClicked()));

    connect(ui.pbAddToViewSources, SIGNAL(clicked()), this, SLOT(historySelectionChanged()));
    connect(ui.pbRemoveFromHistory, SIGNAL(clicked()), this, SLOT(removeFromHistory()));

    connect(ui.pbAddToViewSources, SIGNAL(clicked()), this, SLOT(addToViewSourcesFromDbList()));

    connect(ui.pbLoadSrcsFromDb, SIGNAL(clicked()), this, SIGNAL(buttonLoadSrcsFromDbClicked()));
    connect(ui.pbAddSrcFromDb, SIGNAL(clicked()), this, SLOT(addToViewSourcesFromDbList()));

    ui.cbLastDaysHours->setChecked(false);

    QDateTime dt = QDateTime::currentDateTime();
    ui.calStop->setSelectedDate(dt.date());
    ui.teStop->setTime(dt.time());
    dt = dt.addDays(-1);
    ui.calStart->setSelectedDate(dt.date());
    ui.teStart->setTime(dt.time());

    ui.sbDaysHours->setDisabled(true);
    ui.cbDaysHours->setDisabled(true);
    connect(ui.sbDaysHours, SIGNAL(valueChanged(int)), this, SLOT(lastDaysHoursChanged()));
    connect(ui.cbDaysHours, SIGNAL(currentIndexChanged(int)), this, SLOT(lastDaysHoursChanged()));

    /* restore previous values */
    QSettings s;
    QStringList savedItems = s.value("SourcesList").toStringList();
    foreach(QString src, savedItems)
        new QTreeWidgetItem(ui.twSelected, QStringList() << src);

    QDateTime dts = s.value("startDate").toDateTime();
    QDateTime dte = s.value("stopDate").toDateTime();

    ui.calStart->setSelectedDate(dts.date());
    ui.calStop->setSelectedDate(dte.date());
    ui.teStart->setTime(dts.time());
    ui.teStop->setTime(dte.time());

    /* restore history */
    QStringList historyEntries = s.value("HISTORY_ENTRIES").toStringList();
    foreach(QString he, historyEntries)
        new QTreeWidgetItem(ui.twHistory, QStringList() << he);

    m_viewSourcesListChanged();

    connect(ui.leFind, SIGNAL(textChanged(QString)), this, SLOT(filter(QString)));
    ui.twSelected->viewport()->setAcceptDrops(true);
}

void QHdbConfigWidget::addToViewSourcesFromDbList()
{
    QStringList selected = ui.twSources->selectedSources();
    foreach(QString s, selected)
    {
        QList<QTreeWidgetItem*> itemsInView = ui.twSelected->findItems(s, Qt::MatchExactly);
        if(itemsInView.size() == 0) /* not existing, create it */
        {
            QTreeWidgetItem *newIt = new QTreeWidgetItem(ui.twSelected, QStringList() << s);
            newIt->setFlags(newIt->flags()|Qt::ItemIsEditable);
        }
        else
            itemsInView.first()->setSelected(true);
    }
    m_viewSourcesListChanged();
    ui.twSelected->resizeColumnToContents(0);
    ui.twSelected->scrollToBottom();
    ui.twSelected->horizontalScrollBar()->setValue(ui.twSelected->horizontalScrollBar()->maximum() - 1);
}

void QHdbConfigWidget::updateSourcesList(const QStringList& srcs)
{
    ui.twSources->updateSourcesList(srcs);
}

QDateTime QHdbConfigWidget::startDateTime() const
{
    QDateTime dt;
    dt.setDate(ui.calStart->selectedDate());
    dt.setTime(ui.teStart->time());
    return dt;
}

QDateTime QHdbConfigWidget::stopDateTime() const
{
    QDateTime dt;
    dt.setDate(ui.calStop->selectedDate());
    dt.setTime(ui.teStop->time());
    return dt;
}

void QHdbConfigWidget::setConfig(const QString& host, const QString& db, const QString& user)
{

    ui.labelHost->setText("Host: " + host);
    ui.labelDb->setText("Database: " + db);
    ui.labelUser->setText("User: " + user);
}

void QHdbConfigWidget::setState(const QString& state)
{
    ui.labelState->setText(state);
}

void QHdbConfigWidget::filter(const QString &text)
{
    TreeWidgetSearch tws;
    tws.filter(ui.twSources, text);
}

QStringList QHdbConfigWidget::m_sourcesFromTree(QTreeWidget *tree) const
{
    QStringList ret;
    QList<QTreeWidgetItem *> items;
    QTreeWidget *stw = findChild<QTreeWidget *>(tree->objectName());
    if(stw)
        items = stw->findItems("*", Qt::MatchWildcard);
    foreach(QTreeWidgetItem *it, items)
    {
        if(it->text(0).length() > 0)
            ret << it->text(0);
    }

    qDebug() << "QHdbConfigWidget::m_sourcesFromTree() " << ret;
    return ret;
}

QStringList QHdbConfigWidget::sources() const
{
    qDebug() << "sources() " << m_sourcesFromTree(ui.twSelected);
    return m_sourcesFromTree(ui.twSelected);
}

void QHdbConfigWidget::updateHistory()
{
    QSettings s;
    QList<QTreeWidgetItem *> viewList = ui.twSelected->findItems("*", Qt::MatchWildcard, 0);
    foreach(QTreeWidgetItem *viewIt, viewList)
    {
        QList<QTreeWidgetItem*> inHistoryIts = ui.twHistory->findItems(viewIt->text(0), Qt::MatchExactly);
        if(inHistoryIts.isEmpty())
        {
            new QTreeWidgetItem(ui.twHistory, QStringList() << viewIt->text(0));
        }
        /* save/update  settings */
        QDateTime startDt(ui.calStart->selectedDate(), ui.teStart->time());
        QDateTime stopDt(ui.calStop->selectedDate(), ui.teStop->time());
        s.setValue(viewIt->text(0) + "_START", startDt);
        s.setValue(viewIt->text(0) + "_STOP", stopDt);
        QStringList historyEntries = s.value("HISTORY_ENTRIES", QStringList()).toStringList();
        if(!historyEntries.contains(viewIt->text(0)))
            historyEntries << viewIt->text(0);
        s.setValue("HISTORY_ENTRIES", historyEntries);
    }
}

void QHdbConfigWidget::historySelectionChanged()
{
    QList<QTreeWidgetItem*> srcs = ui.twHistory->selectedItems();
    if(srcs.size() == 1)
    {
        QString src = srcs.first()->text(0);

        QList<QTreeWidgetItem*> itemsInView = ui.twSelected->findItems(src, Qt::MatchExactly);
        if(itemsInView.size() == 0) /* not existing, create it */
        {
            QTreeWidgetItem *newIt = new QTreeWidgetItem(ui.twSelected, QStringList() << src);
            newIt->setFlags(newIt->flags()|Qt::ItemIsEditable);
        }
        else
            itemsInView.first()->setSelected(true);

        /* restore date time associated to selected item */
        if(ui.cbRestoreDateTime->isChecked())
        {
            QSettings s;
            QDateTime startdt = s.value(src + "_START").toDateTime();
            QDateTime stopdt = s.value(src + "_STOP").toDateTime();
            ui.calStart->setSelectedDate(startdt.date());
            ui.calStop->setSelectedDate(stopdt.date());
            ui.teStart->setTime(startdt.time());
            ui.teStop->setTime(stopdt.time());
        }
    }
}

void QHdbConfigWidget::removeFromHistory()
{
    QSettings s;
    QList<QTreeWidgetItem* > srcs = ui.twHistory->selectedItems();
    foreach(QTreeWidgetItem * it, srcs)
    {
        QString src = it->text(0);
        QStringList historyEntries = s.value("HISTORY_ENTRIES", QStringList()).toStringList();

        if(historyEntries.contains(it->text(0)))
            historyEntries.removeAll(src);
        s.setValue("HISTORY_ENTRIES", historyEntries);

        s.remove(src + "_START");
        s.remove(src + "_STOP");
        delete it;
    }
}

void QHdbConfigWidget::removeSourceClicked()
{
    QList<QTreeWidgetItem* > srcs = ui.twSelected->selectedItems();
    foreach(QTreeWidgetItem * it, srcs)
    {
        emit sourceRemoved(it->text(0));
        delete it;
    }
    m_viewSourcesListChanged();
}

void QHdbConfigWidget::addSourceClicked()
{
    QTreeWidgetItem *newIt = new QTreeWidgetItem(ui.twSelected);
    newIt->setFlags(newIt->flags()|Qt::ItemIsEditable);
    newIt->setSelected(true);
    ui.twSelected->editItem(newIt);
    m_viewSourcesListChanged();
}

void QHdbConfigWidget::m_viewSourcesListChanged()
{
    ui.gbFrom->setEnabled(sources().size() > 0);
    ui.gbTo->setEnabled(sources().size() > 0);

    QStringList srcs = sources();
    int ssize = srcs.size();

    if(ssize > 1 )
        ui.labelSource->setText(QString("%1, %2...").arg(srcs.first().section('/', -4, -1), srcs.at(1).section('/', -4, -1)));
    else if(ssize == 1)
        ui.labelSource->setText(srcs.first().section('/', -4, -1));
}

void QHdbConfigWidget::lastDaysHoursChanged()
{
    QDateTime dt = QDateTime::currentDateTime();
    ui.calStop->setSelectedDate(dt.date());
    ui.teStop->setTime(dt.time());
    switch(ui.cbDaysHours->currentIndex())
    {
    case 0: /* days */
        dt = dt.addDays(-ui.sbDaysHours->value());
        break;
       case 1: /* Hours */
        dt.addSecs(-ui.sbDaysHours->value() * 3600);
        break;
    case 2:
        dt.addSecs(-ui.sbDaysHours->value() * 60);
        break;
    case 3:
        dt.addSecs(-ui.sbDaysHours->value());
        break;
    default:
        break;
    }
    ui.calStart->setSelectedDate(dt.date());
    ui.teStart->setTime(dt.time());
}


