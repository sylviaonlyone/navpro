#ifndef _FAST_NONMAX_H
#define _FAST_NONMAX_H

#include <PiiMatrix.h>
#include <QVector>

PiiMatrix<int> fast_suppress_nonmax(const PiiMatrix<int>& corners,
                                    const QVector<int>& scores);

#endif //_FAST_NONMAX_H
