/*=============================================================================
**                            MODULE SPECIFICATION
===============================================================================
**
**  Title : Particle filter
**
**  Description : Particle filter class define, the original code borrowed from
**                from Sebastian's Code at:
**                http://www.udacity.com/
**                Udacity CS373: Programming a Robotic Car
**
**
===============================================================================
**  Author            :     Xin Zhang
**  Creation Date     :     2012.11.24
===============================================================================
**/

#include <iostream>
#include <QColor>
#include "particleFilter.h"

using namespace cv;

particleFilter::particleFilter()
    : globleNoise(100.0),
    pMeasureArray(NULL)
{
    try {
        pMeasureArray = new M_Prob[NUMBER_OF_PARTICLES];
    }
    catch (std::bad_alloc& ba)
    {
        std::cerr<<"bad_alloc caught:"<<ba.what()<<std::endl;
    }

    //printParticles();
}

particleFilter::~particleFilter()
{
    delete[] pMeasureArray;
    pMeasureArray = NULL;
}

void particleFilter::measurementUpdate(const std::vector<Mat>& rgbHistogram, const QImage& rawImage)
{
    Mat bHist = rgbHistogram[0];
    Mat gHist = rgbHistogram[1];
    Mat rHist = rgbHistogram[2];
    QRgb p;
    float probRoad[rawImage.width()][rawImage.height()];
    std::cout<<"B hist Y:"<<bHist.rows<<" X:"<<bHist.cols<<std::endl;

    for(int x = 0; x < rawImage.width(); ++x)
    {
        for(int y = 0; y < rawImage.height(); ++y)
        {
            p = rawImage.pixel(x, y);
            // get probability of blue
            probRoad[x][y] = bHist.at<int>(qBlue (p)) *
                       gHist.at<int>(qGreen(p)) *
                       rHist.at<int>(qRed  (p)); 
        }
    }
}

void particleFilter::measurementUpdate(const QImage& image, bool grayImage)
{
    int height = image.height();
    int width  = image.width();
    int dist;
    int i,j,k;
    float prob;
    for(i = 0; i < width; ++i)
    {
        for(j = height/2; j < height; ++j)
        {
            if (QColor(image.pixel(i, j)) != QColor(Qt::black))
            {
                for(k = 0; k < NUMBER_OF_PARTICLES; ++k)
                {
                    //check particle filter is in this image
                    if (pMeasureArray[k].x > width || pMeasureArray[k].y > height)
                      continue;

                    dist = Distance(pMeasureArray[k].x, pMeasureArray[k].y, i, j);
                    prob = Gaussian(dist, globleNoise, 0);
                    if (prob > pMeasureArray[k].probability)
                    {
                       pMeasureArray[k].probability = prob;
                    //std::cout<<"X1 "<<pMeasureArray[k].x<<" Y1 "<<pMeasureArray[k].y<<" X2 "
                    //  <<i<<" Y2 "<<j<<" dist  "<<dist<<std::endl;
                    //std::cout<<"dist from "<<pMeasureArray[k].x<<" "<<pMeasureArray[k].y<<" is:"<<dist;
                    //std::cout<<" prob:"<<pMeasureArray[k].probabilityEdge<<std::endl;
                    }
                }//for(k = 0; k < NUMBER_OF_PARTICLES; ++k)
            }//if (QColor(image.pixel(i, j)) != QColor(Qt::black))
        }//for(j = height/2; j < height; ++j)
    }
    printParticles("Measure update");
}

void particleFilter::resample()
{
    //std::cout<<"resample"<<std::endl;
    int index = randomInt(0, NUMBER_OF_PARTICLES);
    M_Prob* newProbArray;
    try{
        newProbArray = new M_Prob[NUMBER_OF_PARTICLES];
    }
    catch (std::bad_alloc& ba)
    {
        std::cerr<<"bad_alloc caught:"<<ba.what()<<std::endl;
    }

    float maxProb = 0.0;
    float beta = 0.0;
    int i;
    for(i = 0; i < NUMBER_OF_PARTICLES; ++i)
    {
        if (pMeasureArray[i].probability > maxProb )
          maxProb = pMeasureArray[i].probability;
    }

    //std::cout<<"maxProb: "<<maxProb<<std::endl;
    //resample
    for(i = 0; i < NUMBER_OF_PARTICLES; ++i)
    {
        beta += (static_cast<float>(randomInt(0, 100))/100.0) * 2.0 * maxProb;
        //beta += (static_cast<float>(randomInt(0, 100))/100.0) * 2.0 * maxProb;
        //std::cout<<"index "<<index<<" beta: "<<beta;
        //std::cout<<" prob:"<<pMeasureArray[index].probabilityEdge<<std::endl;
        while (beta > pMeasureArray[index].probability)
        {
            beta -= pMeasureArray[index].probability;
            //std::cout<<"--->reinit:"<<index<<std::endl;

            //this particle have to be resampled
            //pMeasureArray[index].x = randomInt(0, 1600);
            //pMeasureArray[index].y = randomInt(0, 1200);
            //pMeasureArray[index].probabilityEdge = 0.0;

            index = (index + 1) % NUMBER_OF_PARTICLES;
        }
        //std::cout<<"index:"<<index<<" kept"<<std::endl;
        newProbArray[i] = pMeasureArray[index]; 
    }

    for(i = 0; i < NUMBER_OF_PARTICLES; ++i)
    {
        pMeasureArray[i] = newProbArray[i]; 
    }

    delete[] newProbArray;
    printParticles("Resample");
}

void particleFilter::printParticles(const char* header)
{
#if 0
    if (header)
      std::cout<<*header<<std::endl;
    int i = 0;
    float sum = 0.0;
    while (i<NUMBER_OF_PARTICLES)
    {
        sum +=pMeasureArray[i].probabilityEdge;
        ++i;
    }

    i = 0;
    while (i<NUMBER_OF_PARTICLES)
    {
        std::cout<<pMeasureArray[i].x<<" "
                 <<pMeasureArray[i].y<<" "
                 <<pMeasureArray[i].probabilityEdge<<" "
                 <<(pMeasureArray[i].probabilityEdge/sum) * 100.0<<"% "
                 //<<pMeasureArray[i].probabilityMarker<<" "
                 //<<pMeasureArray[i].probabilityRoad<<" "
                 //<<pMeasureArray[i].probabilityNonRoad
                 <<std::endl;
        ++i;
    }
    std::cout<<"\n==========END=========="<<std::endl;
#endif
}

void particleFilter::move(const int pixels)
{
    for(int i = 0; i < NUMBER_OF_PARTICLES; ++i)
    {
        pMeasureArray[i].y += pixels; 
    }
}
