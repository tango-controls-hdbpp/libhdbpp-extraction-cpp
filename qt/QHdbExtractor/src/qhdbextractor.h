#ifndef QHDBEXTRACTOR_H
#define QHDBEXTRACTOR_H

#include <QMainWindow>

class QHdbDataBoundaries;

namespace Ui {
    class MainWindow;
}

/** \page qhdbextractor QHdbExtractor: an example of Qt application using QHdbExtractorProxy, the Hdbextractor Qt module.
 *
 * This is an example of a Qt graphical interface application that uses the QHdbExtractorProxy
 * class to get data from the HDB MySql historical database and display it on a plot.
 *
 * \note Dependency needed: the QGraphicsPlot library.
 *
 * \image html qhdbextractor.png
 *
 * @see HdbExtractorProxy
 *
 * See the QHdbExtractor documentation page to view an example.
 *
 * \example qhdbextractor.h
 * This is an example of a main window visualizing historical data header file.
 *
 * \example qhdbextractor.cpp
 * This is the corresponding cpp file.
 *
 * \example main.cpp
 * This is the main.cpp file
 *
 * Other files are used by this example, in order to provide configuration and
 * visualization widgets that complete the application.
 *
 * \example qhdbconfigwidget.h
 * \example qhdbconfigwidget.cpp
 * \example qhistoricalviewwidget.h
 * \example qhistoricalviewwidget.cpp
 * \example qhistoricalviewwidgetprivate.h
 * \example qhistoricalviewwidgetprivate.cpp
 *
 */
class QHdbExtractor : public QMainWindow
{
    Q_OBJECT

public:
    explicit QHdbExtractor(QWidget *parent = 0);
    ~QHdbExtractor();

private:
    Ui::MainWindow *ui;

    void mAddCurve(const QString& source, const QColor& c, bool read = true);

private slots:
    void slotViewClicked();
    void slotConfigureClicked();

    void onNewDataAvailable(const QString& source, const QVector<double>& timestamp, const QVector<double>& data);

    void onNewDataAvailable(const QString& source,
                            const QVector<double>& timestamps,
                            const QVector<double>& read_data,
                            const QVector<double>& write_data);

    void onNewDataAvailable(const QString& source,
                            const double *surface,
                            size_t dataCount,
                            size_t dataSize,
                            const QHdbDataBoundaries& boundaries);

    void errorExtractionReady(const QString& src,
                                             const QVector<double> timestamps,
                                             const QVector<int> codes,
                                             const QStringList &messages);

    void onExtractionProgress(const QString& source, int step, int total);

    void onExtractionFinished(const QString& source, int srcStep, int srcTotal, double elapsed);

    void onError(const QString& message);

    void radioCurvesStyleToggled(bool);

    void sourcesListReady(const QStringList& srclist);

    void plotClicked(const QPointF& point);

    void errorItemSelectionChanged();

private:
    bool mUseGl;
};

#endif // MAINWINDOW_H
