#ifndef SOURCESTREEWIDGET_H
#define SOURCESTREEWIDGET_H

#include <QTreeWidget>

class SourcesTreeWidgetPrivate;
class QTreeWidgetItem;

/** \brief a TreeWidget used by ArchivedSourcesTreeWidget class that manages
  *        3-level tree items such as tango attribute names.
  *
  * This class is used by ArchivedSourcesTreeWidget and provides some methods
  * to manage the tango sources in a tree.
  */
class SourcesTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit SourcesTreeWidget(QWidget *parent = 0);

    /** \brief returns all the tango sources in the tree
      *
      * \note only leaf elements without children are returned, i.e.
      *       tango attribute names (test/device/1/long_scalar).
      */
    QStringList getSources() const;

    /** \brief returns the selected sources.
      *
      * \note only leaf elements without children are returned, i.e.
      *       tango attribute names (test/device/1/long_scalar).
      */
    QStringList selectedSources() const;

public slots:
    /** \brief adds a source to the tree and expands the parent
      *        node if the expand parameter is true.
      *
      * @param source  the attribute name to add in the tree
      * @param expand true the parent is expanded to show the children
      */
    void addSource(const QString &source, bool expand);

    /** \brief calls addSource for each source in the sources parameter
      *
      * Simply calles addSource for each source in sources.
      *
      * @see addSource
      */
    void updateSourcesList(const QStringList& sources, bool expand = false);

    /** \brief removes the selected sources.
      *
      */
    void removeSelectedSources();
signals:

protected:

    /* drag */
    void dragMoveEvent ( QDragMoveEvent * event );
    void dragEnterEvent(QDragEnterEvent *event);
    QMimeData * mimeData ( const QList<QTreeWidgetItem *> items ) const;

    /* drop */
    void dropEvent(QDropEvent *event);

    QString buildItemText(const QTreeWidgetItem *it) const;
    int level( QTreeWidgetItem *it) const ;

    SourcesTreeWidgetPrivate *d_ptr;

public slots:

private:

    QTreeWidgetItem* findChild(QTreeWidgetItem *parent, const QString& name);


};

#endif // ARCHIVEDSOURCESTREEWIDGET_H
