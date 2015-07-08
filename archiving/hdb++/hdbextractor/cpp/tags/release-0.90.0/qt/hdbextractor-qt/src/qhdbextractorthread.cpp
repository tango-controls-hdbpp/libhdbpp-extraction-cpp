#include "qhdbextractorthread.h"
#include <QtDebug>
#include <QTimer>
#include <QStringList>
#include <QWriteLocker>
#include <QReadLocker>

#include "hdbextractor.h"
#include "timeinterval.h"
#include "qhdbxqueryevent.h"
#include "qhdbxerrorqueryevent.h"
#include "hdbxmacros.h"
#include "qhdbxconnectionevent.h"
#include "qhdbnewerrordataevent.h"

/** \brief The destructor makes the thread exit the loop.
 *
 * \note wait() must not be invoked by the caller. This method wait()s.
 */
QHdbextractorThread::~QHdbextractorThread()
{
    qDebug() << __FUNCTION__ << QThread::currentThread() << this;
    emit exitLoop();
    wait(20 * 1e6);
}


QHdbextractorThread::QHdbextractorThread(QObject *parent) :
    QThread(parent)
{
    m_extractor = new Hdbextractor(this);
    m_queryConfiguration = NULL;
    setObjectName("QHdbExtractorThread");
}

/** \brief this method is invoked according to the numRows value configured in setUpgradeProgressStep
 *         whenever numRows rows are read from the database.
 *
 * \note By default, if numRows is not set, onProgressUpdate is not invoked and the results
 *       are available when onFinished is invoked.
 *
 * @param name the name of the tango device/attribute being extracted
 * @param step the number of processed rows
 * @param totalSteps the number of data rows for the source with name name
 * @param totalSources the number of sources involving the data extraction
 * @param elapsed the time elapsed (seconds.microseconds) since extraction started.
 *
 * @see onSourceExtracted
 */
void QHdbextractorThread::onSourceProgressUpdate(const char *name, int step, int totalSteps)
{
    emit sourceExtractionProgress(name, step, totalSteps);
}

/** \brief this method is invoked when data extraction is fully accomplished on the source with the given name
 *
 *
 * @param name the name of the tango device/attribute being extracted
 * @param totalRows the number of data rows for the source with name name
 * @param totalSources the number of sources involving the data extraction
 * @param elapsed the time elapsed (seconds.microseconds) since extraction started.
 *
 * @see onSourceProgressUpdate
 */
void QHdbextractorThread::onSourceExtracted(const char * name, int totalRows, int totalSources, double elapsed)
{
    emit sourceExtractionFinished(name, totalRows, totalSources, elapsed);
}

Hdbextractor *QHdbextractorThread::getHdbExtractor() const
{
    return m_extractor;
}

QHdbextractorThread::State QHdbextractorThread::getState() const
{
    return m_state;
}

void QHdbextractorThread::run()
{
    QTimer *processTimer = new QTimer(0);
    QTimer *closeTimer = new QTimer(0);

    connect(this, SIGNAL(exitLoop()), closeTimer, SIGNAL(timeout()), Qt::QueuedConnection);
    connect(closeTimer, SIGNAL(timeout()), this, SLOT(quit()), Qt::DirectConnection);

    connect(this, SIGNAL(processRequest()), processTimer, SIGNAL(timeout()), Qt::QueuedConnection);
    connect(processTimer, SIGNAL(timeout()), this, SLOT(process()), Qt::DirectConnection);


    /* event loop */
    QThread::currentThread()->setObjectName("QHdbextractorThread Thread");
    qDebug() << this <<  QThread::currentThread() << __FUNCTION__ << "entering main loop";
    /* some events might have been accumulated in the queue, let's check as soon as we start */
    processTimer->setSingleShot(true);
    processTimer->setInterval(300);
    processTimer->start();
    exec();
    qDebug() << this <<  QThread::currentThread() << __FUNCTION__ << "leaving main loop";
}

void QHdbextractorThread::addEvent(QHdbXEvent *hdbxe)
{
    qDebug() << this << objectName() << __FUNCTION__;
    QWriteLocker wLock(&m_rwLock);
    m_eventQueue << hdbxe;
    emit processRequest();
}

void QHdbextractorThread::process()
{
    while(!m_eventQueue.isEmpty())
    {
        /* dequeue the event */
        QHdbXEvent *e = m_eventQueue.dequeue();
        qDebug() << this << objectName() << __FUNCTION__ << "eventQueueSize" << m_eventQueue.size() << "type" <<
                    e->getType();
        if(e->getType() == QHdbXEvent::CONNECT)
        {
            QHdbXConnectionEvent *hxce = static_cast<QHdbXConnectionEvent *>(e);
            qDebug() << __FUNCTION__ << "connecting "  << hxce->host << hxce->dbnam;
            if(!m_extractor->connect(hxce->dbType, hxce->host.toStdString().c_str(),
                                     hxce->dbnam.toStdString().c_str(),
                                     hxce->user.toStdString().c_str(),
                                     hxce->pass.toStdString().c_str(),
                                     hxce->port))
                emit errorMessage(m_extractor->getErrorMessage());
        }
        else if(e->getType() == QHdbXEvent::DISCONNECT)
        {
            m_extractor->disconnect();
        }
        else if(e->getType() == QHdbXEvent::DATA_QUERY)
        {
            m_state = DataQuery;
            QHdbXQueryEvent *qe = static_cast<QHdbXQueryEvent *>(e);
            if(m_extractor->isConnected())
            {
                std::vector<std::string> srcs;
                foreach(QString s, qe->sources)
                    srcs.push_back(s.toStdString());
                if(!m_extractor->getData(srcs,
                                         qe->startDate.toStdString().c_str(),
                                         qe->stopDate.toStdString().c_str()))
                    emit errorMessage(m_extractor->getErrorMessage());
            }
            else
               emit errorMessage("QHdbextractorThread.process: not connected to database");
        }
        else if(e->getType() == QHdbXEvent::ERROR_QUERY)
        {
            m_state = ErrorQuery;
            QHdbXErrorQueryEvent *qeque = static_cast<QHdbXErrorQueryEvent *>(e);
            if(m_extractor->isConnected())
            {
                TimeInterval timeInterval(qeque->startTime, qeque->stopTime);
                qDebug() << __FUNCTION__ << "calling find errors";
                if(!m_extractor->findErrors(qeque->source.toStdString().c_str(), &timeInterval))
                    emit errorMessage(m_extractor->getErrorMessage());
            }
            else
               emit errorMessage("QHdbextractorThread.process: not connected to database");

        }
        else if(e->getType() == QHdbXEvent::SOURCESLIST)
        {
            m_state = SourcesListQuery;
            std::list<std::string> srcs;
            QStringList ret;
            bool success = m_extractor->getSourcesList(srcs);
            if(!success)
                emit errorMessage(QString("Error fetching sources list from db: %1").
                                   arg(m_extractor->getErrorMessage()));
            else
            {
                for(std::list<std::string>::iterator it = srcs.begin(); it != srcs.end(); it++)
                    ret.append(QString::fromStdString(*it));
            }
            emit sourcesListReady(ret);
        }
        /* delete the event */
        delete e;
    }
}


