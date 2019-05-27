#ifndef ZKEYDETTHREAD_H
#define ZKEYDETTHREAD_H

#include <QThread>

class ZKeyDetThread : public QThread
{
    Q_OBJECT
public:
    ZKeyDetThread();

protected:
    void run();

};

#endif // ZKEYDETTHREAD_H
