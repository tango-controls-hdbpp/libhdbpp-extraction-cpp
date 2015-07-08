#ifndef DATASIEVERPROGRESSLISTENER_H
#define DATASIEVERPROGRESSLISTENER_H

class DataSieverProgressListener
{
public:
    DataSieverProgressListener();

    void onFillProgressUpdate(int step, int total, double elapsed, double estimated_remaining) {};
};

#endif // DATASIEVERPROGRESSLISTENER_H
