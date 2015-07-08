#ifndef HISTORICALVIEWWIDGET_H
#define HISTORICALVIEWWIDGET_H

#include <QStackedWidget>
#include <QDateTime>
#include <xvariant.h>

class QHistoricalViewWidgetPrivate;

/** \brief this class is a stacked widget that groups different types
  *        of widgets, each one tailored to display specific types of
  *        data recollected by the historical database.
  *
  * Instantiate a HistoricalViewWidget and then register a specific widget
  * associated to a specific DataType.
  * Then the switchWidget method will display the correct widget according
  * to the DataType in use.
  */
class QHistoricalViewWidget : public QStackedWidget
{
    Q_OBJECT
public:

    enum DataType { Wait, NumberScalar, NumberVector, StringScalar, StringVector,
             State, StateVector, User = 100 };

    /** \brief The constructor
      *
      * Builds a QStackedWidget.
      */
    QHistoricalViewWidget(QWidget *parent);

    virtual ~QHistoricalViewWidget();

    /** \brief inserts into the stacked widget the widget w and associates it to
      *        the DataType t.
      *
      * You have to call switchWidget method to later display the correct
      * widget in the stack.
      *
      * @see switchWidget
      */
    virtual void registerWidget(QWidget *w, DataType dt);

    /** \brief changes the current displayed widget so that the one associated to the
      *        DataType t is shown.
      *
      * @param t the data type, a value among the ones defined in DataType enumeration.
      *
      * @see registerWidget
      *
      * \note if no widget was registered with the DataType t, then no action is
      *       performed.
      */
    virtual void switchWidget(DataType t);

    QWidget *widget(DataType t);

protected:

private:

    QHistoricalViewWidgetPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QHistoricalViewWidget)
};

#endif // HISTORICALPLOT_H
