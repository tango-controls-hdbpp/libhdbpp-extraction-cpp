#ifndef THDBCONFIGWIDGET_H
#define THDBCONFIGWIDGET_H

#include <QWidget>
#include "ui_configWidget.h"

class EPlotCurve;
class SourcesTreeWidget;

/** \brief a configuration widget for the historical database.
  *
  * This widget contains a set of controls to get data from the historical
  * database.
  */
class QHdbConfigWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QHdbConfigWidget(QWidget *parent = 0);

    virtual ~QHdbConfigWidget();

    QDateTime startDateTime() const;

    QDateTime stopDateTime() const;

    QStringList sources() const;

    void setSources(const QStringList& src);

    void updateHistory();

    void setConfig(const QString& host, const QString& db, const QString& user);

    void setState(const QString& state);

    void updateSourcesList(const QStringList& srcs);

signals:

    void sourceRemoved(const QString& src);

    void viewClicked();

    void cancelClicked();

    void buttonLoadSrcsFromDbClicked();

protected slots:

    void removeSourceClicked();

    void removeFromHistory();

    void addSourceClicked();

    void addToViewSourcesFromDbList();

    void lastDaysHoursChanged();

    void historySelectionChanged();

    void init();

    void filter(const QString &text);

private:
    Ui::ConfigWidget ui;

    void m_viewSourcesListChanged();

    QStringList m_sourcesFromTree(QTreeWidget *tree) const;
};

#endif // THDBCONFIGDIALOG_H
