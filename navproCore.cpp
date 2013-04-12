/*=============================================================================
**                            MODULE SPECIFICATION
===============================================================================
**
**  Title :
**
**  Description :
**
**
===============================================================================
**  Author            :     
**  Creation Date     :     2011
-------------------------------------------------------------------------------
**  CVS Identification:     $Revision: $
**                          $Author: $
**                          $Date: $
===============================================================================
**/

#include <QDir>
#include "navproCore.h"

#define OPENCV_TO_QT_RGB888(CV_IMAGE) \
        (QImage((const unsigned char*)CV_IMAGE.data, \
                CV_IMAGE.cols, CV_IMAGE.rows,\
                CV_IMAGE.step, QImage::Format_RGB888))

#define OPENCV_TO_QT_INDEX8(CV_IMAGE) \
        (QImage((const unsigned char*)CV_IMAGE.data, \
                CV_IMAGE.cols, CV_IMAGE.rows, \
                QImage::Format_Indexed8))

navproCore::navproCore(laneTracker* tracker, inputManager* input):
    pTracker(tracker),
    p_input_manager_(input),
    p_histogram_(NULL),
    p_particle_edge_(NULL),
    p_particle_marker_(NULL),
    p_particle_color_(NULL),
    p_image_origin_(NULL),
    p_image_edge_(NULL),
    p_image_marker_(NULL),
    p_image_color_(NULL)
{
    try {
        //init images for display
        p_image_origin_ = new QImage();
        p_image_edge_ = new QImage();
        p_image_marker_ = new QImage();
        p_image_color_ = new QImage();

        p_particle_edge_ = new particleFilter();
        p_particle_marker_ = new particleFilter();
        p_particle_color_ = new particleFilter();
    }
    catch (std::bad_alloc&)
    {
        std::cerr<<"Bad alloc!"<<std::endl;
        throw;
    }

    //set color table used for 8-bits image, should do this only once
    for (int i = 0; i < 256; i++) colorTable.push_back(qRgb(i, i, i));
 
    probe();
}

navproCore::~navproCore()
{
    delete p_image_origin_;
    delete p_image_edge_;
    delete p_image_marker_;
    delete p_image_color_;
    delete p_particle_edge_;
    delete p_particle_marker_;
    delete p_particle_color_;
}

void navproCore::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(QBrush(Qt::red));
    QImage output;
    QImage roadcolor(1600, 1200, QImage::Format_RGB888);
    roadcolor.fill(0);
    roadcolor.setPixel(800, 600, 0xffffff);

    // used for 8-bits image
    QVector<QRgb> colorTable;
    for (int i = 0; i < 256; i++) colorTable.push_back(qRgb(i, i, i));
    

    //if (!cvImage.data)
    {
        //TODO output from InputManager
        //output = QImage(path.toAscii().data());
        //getNextImage(output);
    }
    //else
    {
        //RGB888 used for result image from edgeDetect()
        //output = QImage((const unsigned char*)cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
        //Indexed8 used for result image from laneMarkerDetect()
        //output = QImage((const unsigned char*)cvImage.data, cvImage.cols, cvImage.rows, QImage::Format_Indexed8);
        //output.setColorTable(colorTable);
    }

    QRgb p;
    int gray;
    float r, g,b;
//    for(int i = 0 ;i <256;i++)
//    {
//        std::cout<<"r:"<<histVec[2].at<float>(i); 
//        std::cout<<" g:"<<histVec[1].at<float>(i); 
//        std::cout<<" b:"<<histVec[0].at<float>(i); 
//        std::cout<<std::endl;
//    }
#if 0
    float array[output.width()][output.height()];
    float max = 0.0;
    for(int x = 0; x <output.width(); x++)
    {
        for(int y = 0; y< output.height(); y++)
        {
            p = output.pixel(x, y);
            b = histVec[0].at<float>(qBlue(p))/100.0;
            g = histVec[1].at<float>(qGreen(p))/100.0;
            r = histVec[2].at<float>(qRed(p))/100.0;
            array[x][y] = r*g*b;
            if (array[x][y] > max) max = array[x][y];
        }
    }
    std::cout<<"max:"<<max<<std::endl;
    for(int x = 0; x <output.width(); x++)
    {
        for(int y = 0; y< output.height(); y++)
        {
          array[x][y] = array[x][y]/max;
          gray = array[x][y] * 255;
          roadcolor.setPixel(x, y, qRgb(gray, gray, gray));
        }
    }
#endif
    
    //painter.drawPixmap(QPoint(0, 0), QPixmap::fromImage(p_image_edge_->scaledToWidth(p_image_edge_->width()/2)));
    //painter.drawPixmap(QPoint(0, 0), QPixmap::fromImage(output.scaledToWidth(output.width()/2)));
    //painter.drawPixmap(QPoint(0, 0), QPixmap::fromImage(roadcolor.scaledToWidth(output.width()/2)));

#if 0
    //draw particles
    const M_Prob* pArray = pFilter->getParticles();
    for(int i = 0; i < particleFilter::NUMBER_OF_PARTICLES; ++i)
    {
        painter.drawEllipse(QPoint(pArray[i].x/2, pArray[i].y/2), 2, 2);
    }
    //thresholding
    for (i = 0; i < image.width(); ++i)
    {
        for (j = 0; j < image.height(); ++j)
        {
            if (SINGAL ? singalFilter(image.pixel(i,j)):multiFilters(image.pixel(i,j)))
            {
               output.setPixel(i,j, image.pixel(i,j)); 
            }
            else
            {
               output.setPixel(i,j, 0); 
            }
        }
    }
    //std::cout<<image.depth()<<std::endl;
    //std::cout<<image.width()<<std::endl;
    //std::cout<<image.height()<<std::endl;
    //std::cout<<pixels.size() <<std::endl;
    //std::cout<<"Format: "<<image.format()<<std::endl;
#endif
    // Draw the background pixmap
    // if image is too large, half it size for display

//    std::cout<<"width: "<<image.width()<<" height: "<<image.height()<<std::endl;
//    std::cout<<"pox: "<<posX<<" poy: "<<posY<<std::endl;
//    std::cout<<"diffx: "<<diffX<<" diffy: "<<diffY<<std::endl;
//    painter.drawRect(posX - diffX , posY - diffY , rangeX, rangeY);
}

void navproCore::probe()
{
    //check pointers available
    assert(p_image_origin_);
    assert(p_image_edge_);
    assert(p_image_marker_);
    assert(p_image_color_);

    //current image must return true
    assert(p_input_manager_->getCurrentImage(*p_image_origin_));

    //process input image
    QString path;
 
    //get current image path
    p_input_manager_->getCurrentImagePath(path);
 
    int error = pTracker->preprocess(path.toAscii().data());
    assert(!error);

    //detect edge
    cv_edge_ = pTracker->edgeDetect();
 
    *p_image_edge_ = OPENCV_TO_QT_RGB888(cv_edge_);

    assert(p_particle_edge_);
    std::cout<<"edge------------------------------>"<<std::endl;
    p_particle_edge_->measurementUpdate(*p_image_edge_);
    p_particle_edge_->resample();
 
    //detect lane marker
    cv_maker_ = pTracker->laneMarkerDetect();
    *p_image_marker_ = OPENCV_TO_QT_INDEX8(cv_maker_);
    //set color table used for 8-bits image
    p_image_marker_->setColorTable(colorTable);

    assert(p_particle_marker_);
    std::cout<<"marker------------------------------>"<<std::endl;
    p_particle_marker_->measurementUpdate(*p_image_marker_);
    p_particle_marker_->resample();

    //detect color
    //array stores R,G,B probabilities
    p_histogram_ = pTracker->roadColorDetect();
    //for(int i = 0 ;i <256;i++)
    //{
    //    //std::cout<<"r:"<<(*p_histogram_)[2].at<float>(i);
    //    //std::cout<<" g:"<<(*p_histogram_)[1].at<float>(i);
    //    //std::cout<<" b:"<<(*p_histogram_)[0].at<float>(i);
    //    std::cout<<"cr:"<<(*p_histogram_)[0].at<float>(i);
    //    std::cout<<" cb:"<<(*p_histogram_)[1].at<float>(i);
    //    std::cout<<std::endl;
    //}

    QRgb p;
    int gray;
    float r, g,b,cr,cb;
    int width = p_image_origin_->width();
    int height = p_image_origin_->height();

    float array[width][height];
    float max = 0.0;
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            p = p_image_origin_->pixel(x, y);
            //b = (*p_histogram_)[0].at<float>(qBlue(p))/100.0;
            //g = (*p_histogram_)[1].at<float>(qGreen(p))/100.0;
            //r = (*p_histogram_)[2].at<float>(qRed(p))/100.0;
            cb = (*p_histogram_)[1].at<float>(RGB2CB(p))/100.0;
            cr = (*p_histogram_)[0].at<float>(RGB2CR(p))/100.0;
            //array[x][y] = r*g*b;
            array[x][y] = cr*cb;
            if (array[x][y] > max) max = array[x][y];
        }
    }
    std::cout<<"max:"<<max<<std::endl;

    *p_image_color_ = QImage (width, height, QImage::Format_RGB888);
    p_image_color_->fill(0);

    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
          array[x][y] = array[x][y]/max;
          gray = array[x][y] * 255;
          assert(gray >=0 && gray < 256);
          p_image_color_->setPixel(x, y, qRgb(gray, gray, gray));
        }
    }

    assert(p_particle_color_);
    bool grayImage = true;
    std::cout<<"color------------------------------>"<<std::endl;
    p_particle_color_->measurementUpdate(*p_image_color_, grayImage);
    p_particle_color_->resample();

#if 0
    //if(pTracker->preprocess(path.toAscii().data()) == -1)
    if(pTracker->preprocess("road/1.JPG") == -1)
    {
        std::cerr<<"Preprocessing error!!!";
        return;
    }
    //cvImage = pTracker->edgeDetect();

    //QImage edges((const unsigned char*)cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
    //pFilter->measurementUpdate(edges); 
    //cvImage = pTracker->preprocess("road/1.JPG");
    //pFilter->measurementUpdate(pTracker->roadColorDetect(), QImage(path)); 

    // 3-D array stores R,G,B probabilities
    //histVec = pTracker->roadColorDetect();

    //cvImage = pTracker->laneMarkerDetect();

    //QImage landMarker((const unsigned char*)cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
    //QImage edges(1600, 1200, QImage::Format_RGB888);
    //edges.fill(0);
    //edges.setPixel(800, 600, 0xffffff);

    //pFilter->measurementUpdate(landMarker); 
    //pFilter->resample();
#endif
}

void navproCore::move()
{
    //assume velocity is 1m/s, on image, every step move 40 pixles on Y
    //pFilter->move(40);
    //if (p_input_manager_->next())
      probe();
}

const M_Prob* navproCore::getParticles(int type)
{
  particleFilter *p;
  switch (type)
  {
      case particleFilter::EDGE:
        p = p_particle_edge_;
      break;
      case particleFilter::LANE_MARKER:
        p = p_particle_marker_;
      break;
      case particleFilter::COLOR:
        p = p_particle_color_;
      break;
      default:
        p = NULL;
      break;
  }
  return p ? p->getParticles() : NULL;
}

#if 0
bool navproCore::getStdDeviation(int rangeX, int rangeY, int *hue, int *sat, int *cb, int *cr)
{
    QColor hsv;
    int i,j;

    *hue = 0;
    *sat = 0;
    *cb  = 0;
    *cr  = 0;

    if (rangeX < 0 || rangeX > image.width()) return false;
    if (rangeY < 0 || rangeY > image.height()) return false;

    for (i = posX; i < rangeX + posX; ++i)
    {
        for (j = posY; j < rangeY + posY; ++j)
        {
            hsv = QColor::fromRgb(image.pixel(i,j));

            *hue += hsv.hue();
            *sat += hsv.saturation();

            *cb += RGB2CB(image.pixel(i,j));
            *cr += RGB2CR(image.pixel(i,j));
        }
    }

    //mean == average value
    int meanHue = *hue/(rangeX * rangeY); 
    int meanSat = *sat/(rangeX * rangeY); 
    int meanCb  = *cb/(rangeX * rangeY); 
    int meanCr  = *cr/(rangeX * rangeY); 

    //std::cout<<"mean: H: "<<meanHue<<" S: "<<meanSat<<" cb: "<<meanCb<<" cr: "<<meanCr<<std::endl;

    quint32 varianceHue = 0;
    quint32 varianceSat = 0;
    quint32 varianceCb  = 0;
    quint32 varianceCr  = 0;

    for (i = 0; i < rangeX; ++i)
    {
        for (j = 0; j < rangeY; ++j)
        {
            hsv = QColor::fromRgb(image.pixel(i,j));

            varianceHue += qAbs(meanHue - hsv.hue()) * qAbs(meanHue - hsv.hue());
            varianceSat += qAbs(meanSat - hsv.saturation()) * qAbs(meanSat - hsv.saturation());

            varianceCb  += qAbs(meanCb - RGB2CB(image.pixel(i,j))) * qAbs(meanHue - RGB2CB(image.pixel(i,j)));
            varianceCr  += qAbs(meanCr - RGB2CR(image.pixel(i,j))) * qAbs(meanHue - RGB2CR(image.pixel(i,j)));
        }
    }

    *hue = qSqrt(varianceHue/(rangeX * rangeY));
    *sat = qSqrt(varianceSat/(rangeX * rangeY));
    *cb = qSqrt(varianceCb/(rangeX * rangeY));
    *cr = qSqrt(varianceCr/(rangeX * rangeY));

    //std::cout<<"variance: H: "<<varianceHue<<" S: "<<varianceSat<<" cb: "<<varianceCb<<" cr: "<<varianceCr<<std::endl;
    //std::cout<<"SD: H: "<<*hue<<" S: "<<*sat<<" cb: "<<*cb<<" cr: "<<*cr<<std::endl;

    return true;
}
#endif

void navproCore::keyPressEvent(QKeyEvent * e)
{
    switch(e->key())
    {
      case Qt::Key_N:
       {
        //static int cur = 0;
        //if (fileList.size() > 0)
        //{
        //    if (cur < fileList.size())
        //    {
        //        int N = 10;
        //        int i = 0;
        //        while (i < N)
        //        {
        //            //probe(QString("road/") + fileList[cur]);
        //            probe();
        //            //update display
        //            update();
        //            //move();
        //            //if (i%10 == 0)
        //            {
        //            //    repaint();
        //            }
        //            ++i;
        //        }
        //        //++cur;
        //    }
        //}
        break;
       }
    }
    //std::cout<<"From: "<<*activeFrom<<" To: "<<*activeTo<<std::endl; 
    //update();
}
