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

#include "navproCore.h"
#include <QDir>

#define SINGAL 0

navproCore::navproCore(laneTracker* tracker, particleFilter* filter, int width, int height):
    hueFrom(20),
    hueTo(200),
    saturationFrom(5),
    saturationTo(80),
    cbFrom(140),
    cbTo(165),
    crFrom(130),
    crTo(155),
    activeFrom(&hueFrom),
    activeTo(&hueTo),
    active(HUE),
    pTracker(tracker),
    pFilter(filter),
    path("./road/1.JPG")
{
    //searchRoad();
    QDir dir("road","*.jpg");
    fileList = dir.entryList();
#if 0
    //image.load(QString(path));
    int centerX = int(image.width() * DEFAULT_X_PROPOTION);
    int centerY = int(image.height() * DEFAULT_Y_PROPOTION);

    int rangeX = image.width() * DEFAULT_SAMPLING_RANGE_PERCENTAGE/100;
    int rangeY = image.height() * DEFAULT_SAMPLING_RANGE_PERCENTAGE/100;

    //default pos(x,y), will be moved by key events
    posX = centerX - rangeX / 2;
    posY = centerY - rangeY / 2;

    int realWidth = (width >= image.width() ? width : image.width()*2);
    int realHeight = height >= image.height() ? height : image.height();

    if (DEFAULT_FULL_WIDTH <= realWidth)
    {
        //cut size to half if it's too large to display
        realWidth >>= 1;
        realHeight >>=1;
    }
    //std::cout<<realWidth<<std::endl; 
    //getRange();
    //resize(realWidth, realHeight);
#endif    
    resize(width/2,height/2);
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

    if (!cvImage.data)
    {
        output = QImage(path.toAscii().data());
    }
    else
    {
        output = QImage((const unsigned char*)cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
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
    //painter.drawPixmap(QPoint(0, 0), QPixmap::fromImage(output.scaledToWidth(output.width()/2)));
    painter.drawPixmap(QPoint(0, 0), QPixmap::fromImage(roadcolor.scaledToWidth(output.width()/2)));

    //draw particles
    //const M_Prob* pArray = pFilter->getParticles();
    //for(int i = 0; i < particleFilter::NUMBER_OF_PARTICLES; ++i)
    {
    //    painter.drawEllipse(QPoint(pArray[i].x/2, pArray[i].y/2), 2, 2);
    }
#if 0
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

void navproCore::probe(const QString& path)
{
    if(pTracker->preprocess(path.toAscii().data()) == -1)
    {
        std::cerr<<"Preprocessing error!!!";
        return;
    }
    //cvImage = pTracker->edgeDetect();

    //cvImage = pTracker->preprocess("road/1.JPG");
    //pFilter->measurementUpdate(pTracker->roadColorDetect(), QImage(path)); 

    // 3-D array stores R,G,B probabilities

    histVec = pTracker->roadColorDetect();

    //QImage edges((const unsigned char*)cvImage.data, cvImage.cols, cvImage.rows, cvImage.step, QImage::Format_RGB888);
    //QImage edges(1600, 1200, QImage::Format_RGB888);
    //edges.fill(0);
    //edges.setPixel(800, 600, 0xffffff);

    //pFilter->measurementUpdate(edges); 
    //pFilter->resample();
}

void navproCore::move()
{
    //assume velocity is 1m/s, on image, every step move 40 pixles on Y
    pFilter->move(40);
}

bool navproCore::singalFilter(QRgb clr)
{
    bool display = false;

    QColor hsv = QColor::fromRgb(clr);
    int cb = RGB2CB(clr);
    int cr = RGB2CR(clr);
    //std::cout<<"HSV:hue: "<<hsv.hue()<<std::endl;
    switch (active)
    {
      case HUE:
        {
            if (hsv.hue() >= hueFrom && hsv.hue()<=hueTo)
              display = true;
        }
        break;
      case SATURATION:
        {
            if (hsv.saturation() >= saturationFrom && hsv.saturation()<=saturationTo)
              display = true;
        }
        break;
      case CB:
        {
            if (cb >= cbFrom && cb<=cbTo)
              display = true;
        }
        break;
      case CR:
        {
            if (cr >= crFrom && cr<=crTo)
              display = true;
        }
        break;
    }

    return display;
}

bool navproCore::multiFilters(QRgb clr)
{
    //bool display = true;

    QColor hsv = QColor::fromRgb(clr);
    int cb = RGB2CB(clr);
    int cr = RGB2CR(clr);
    //ord: 1. saturation 2. cb/cr 3.hue

    if (hsv.saturation() < saturationFrom || hsv.saturation() > saturationTo)
    {
      //display = false;
      return false;
    }
   
    if (cb < cbFrom || cb > cbTo)
    {
      return false;
    }

    if (cr < crFrom || cr > crTo)
    {
      return false;
    }

    if (hsv.hue() < hueFrom || hsv.hue() > hueTo)
    {
      return false;
    }

    return true;
}

#if 0
void navproCore::changeThresholdFrom(const int threshold)
{
  // Change threshold
  setHueFrom(threshold);
  
  // Select the same image again
  emit update();
}

void navproCore::changeThresholdTo(const int threshold)
{

  // Change threshold
  setHueTo(threshold);
  
  // Select the same image again
  emit update();
}
void navproCore::getRange()
{
    //int centerX = int(image.width() * DEFAULT_X_PROPOTION);
    //int centerY = int(image.height() * DEFAULT_Y_PROPOTION);

    // a square around center, size is 10% of image
    //int startX, startY, rangeX, rangeY;
    int rangeX, rangeY;

    rangeX = image.width() * DEFAULT_SAMPLING_RANGE_PERCENTAGE/100;
    rangeY = image.height() * DEFAULT_SAMPLING_RANGE_PERCENTAGE/100;

    //startX = centerX - rangeX;
    //startY = centerY - rangeY;

    //std::cout<<"image: ("<<image.width()<<" "<<image.height()<<")"<<std::endl; 
    //std::cout<<"center: ("<<centerX<<" "<<centerY<<")"<<std::endl; 
    //std::cout<<"start: ("<<startX<<" "<<startY<<")"<<std::endl; 

    int i, j;
    QColor hsv;
    int cb,cr,saturation,hue;
    int SDcb,SDcr,SDsaturation,SDhue;
    int hueLow,hueHigh;
    int satLow,satHigh;
    int cbLow,cbHigh;
    int crLow,crHigh;

    hueLow=hueHigh=0;
    satLow=satHigh=0;
    cbLow=cbHigh=0;
    crLow=crHigh=0;

    cb=cr=saturation=hue=0;

    //maxiumal the ranges 
    hueLow  = 359;
    hueHigh = 0;
    
    satLow  = 255;
    satHigh = 0;
    
    cbLow  = 255;
    cbHigh = 0;
    
    crLow  = 255;
    crHigh = 0;

    
    for (i = posX; i < rangeX + posX; ++i)
    {
        for (j = posY; j < rangeY + posY; ++j)
        {
            hsv = QColor::fromRgb(image.pixel(i,j));

            if (hsv.hue() > 0 && hsv.hue() < 360) hue += hsv.hue();
            if (hsv.saturation() > 0 && hsv.saturation() <= 255) saturation += hsv.saturation();

            cb += RGB2CB(image.pixel(i,j));
            cr += RGB2CR(image.pixel(i,j));

            if (hsv.hue() < hueLow && hsv.hue() > 0) hueLow = hsv.hue();
            if (hsv.hue() > hueHigh) hueHigh = hsv.hue();

            if (hsv.saturation() < satLow) satLow = hsv.saturation();
            if (hsv.saturation() > satHigh) satHigh = hsv.saturation();

            if (RGB2CB(image.pixel(i,j)) < cbLow) cbLow = RGB2CB(image.pixel(i,j));
            if (RGB2CB(image.pixel(i,j)) > cbHigh) cbHigh = RGB2CB(image.pixel(i,j));

            if (RGB2CR(image.pixel(i,j)) < crLow) crLow = RGB2CR(image.pixel(i,j));
            if (RGB2CR(image.pixel(i,j)) > crHigh) crHigh = RGB2CR(image.pixel(i,j));
        }
    }
   
    getStdDeviation(rangeX, rangeY, &SDhue, &SDsaturation, &SDcb, &SDcr);
    std::cout<<"Hue H: "<<hueHigh<<" L: "<<hueLow<<"  AVE: "<<hue/(rangeX*rangeY)<<" SD: "<<SDhue<<std::endl; 
    std::cout<<"Sat H: "<<satHigh<<" L: "<<satLow<<" AVE: "<<saturation/(rangeX*rangeY)<<" SD: "<<SDsaturation<<std::endl; 
    std::cout<<"Cb  H: "<<cbHigh<<" L: "<<cbLow<<" AVE: "<<cb/(rangeX*rangeY)<<" SD: "<<SDcb<<std::endl; 
    std::cout<<"Cr  H: "<<crHigh<<" L: "<<crLow<<" AVE: "<<cr/(rangeX*rangeY)<<" SD: "<<SDcr<<std::endl; 

    // using standard deviation as range, as I assume targe road has normal distribution
    // hue
    hueFrom = hue/(rangeX*rangeY) - SDhue;
    hueTo = hue/(rangeX*rangeY) + SDhue;
    if (hueFrom < 1) hueFrom = 1;
    if (hueTo > 359) hueTo   = 359;
    //std::cout<<"AVE HUE From: "<<hueFrom<<" To: "<<hueTo<<std::endl; 

    // saturation
    saturationFrom = saturation/(rangeX * rangeY) - SDsaturation;
    saturationTo = saturation/(rangeX * rangeY) + SDsaturation;
    if (saturationFrom < 1) saturationFrom = 1;
    if (saturationTo > 255) saturationTo   = 255;
    //std::cout<<"AVE SAT From: "<<saturationFrom<<" To: "<<saturationTo <<std::endl; 

    //Cb
    cbFrom = cb/(rangeX * rangeY) - SDcb;
    cbTo = cb/(rangeX * rangeY) + SDcb;
    //std::cout<<"AVE Cb From: "<<cbFrom<<" To: "<<cbTo <<std::endl; 
    if (cbFrom < 130) cbFrom = 130;
    if (cbTo > 170) cbTo   = 170;

    //Cr
    crFrom = cr/(rangeX * rangeY) - SDcr;
    crTo = cr/(rangeX * rangeY) + SDcr;
    if (crFrom < 130) crFrom = 130;
    if (crTo > 160) crTo   = 160;
    //std::cout<<"AVE Cr From: "<<crFrom<<" To: "<<crTo <<std::endl; 
}

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
      //Left/Right for hue from
      //Up/Down for hue to
      case Qt::Key_Left:
        break;
      case Qt::Key_Down:
        break;
      case Qt::Key_Right:
        break;
      case Qt::Key_Up:
        break;
      case Qt::Key_S:
        {
          std::cout<<"SAT ";
          break;
        }
      case Qt::Key_U:
        {
          std::cout<<"HUE ";
          break;
        }
      case Qt::Key_B:
        {
          std::cout<<"CB ";
          break;
        }
      case Qt::Key_R:
        {
          std::cout<<"CR ";
          break;
        }
      case Qt::Key_H:
        {
          break;
        }
      case Qt::Key_L:
        {
          break;
        }
      case Qt::Key_J:
        {
          break;
        }
      case Qt::Key_K:
        {
          break;
        }
      case Qt::Key_N:
       {
        static int cur = 0;
        //std::cout<<"Cur: "<<cur<<" of : "<<fileList.size()<<std::endl; 
        if (fileList.size() > 0)
        {
            if (cur < fileList.size())
            {
    int N = 20;
    int i = 0;
    //while (i < N)
    {
            probe(QString("road/") + fileList[cur]);
            //update display
            update();
            move();
        //if (i%10 == 0)
        {
        //    repaint();
        }
        ++i;
    }
            ++cur;
            }
    
//            int realWidth = image.width()*2;
//            int realHeight = image.height();
//    
//            if (DEFAULT_FULL_WIDTH <= realWidth)
//            {
//                //cut size to half if it's too large to display
//                realWidth >>= 1;
//                realHeight >>=1;
//            }
//    
//            std::cout<<realWidth<<std::endl; 
//            resize(realWidth, realHeight);
//            getRange();
        }
        break;
       }
    }
    //std::cout<<"From: "<<*activeFrom<<" To: "<<*activeTo<<std::endl; 
    //update();
}

#ifdef LIB_PII
void navproCore::init()
{
  // Create engine
  pEngine = new PiiEngine;

  try
  {
      pEngine->loadPlugin("piiimage"); // PiiImageFileReader/Writer and PiiThresholdingOperation
      pEngine->loadPlugin("piitransforms"); // PiiHoughTransformOperation
  }
  catch (PiiLoadException& ex)
  {
      qDebug("Cannot load all necessary plug-in modules: %s", qPrintable(ex.message()));
      return;
  }  // Create result image display

  pSourceProbeInput = new PiiProbeInput;
  pResultProbeInput = new PiiProbeInput;
  
  // 1. Create operations
  //pEdgeDetector = pEngine->createOperation("PiiEdgeDetector");
  //pHoughTransform = pEngine->createOperation("PiiHoughTransformOperation");
  pImageFileReader = pEngine->createOperation("PiiImageFileReader");
  //pImageAnnotator = pEngine->createOperation("PiiImageAnnotator");

  pImageFileWriter = pEngine->createOperation("PiiImageFileWriter");
  pImageFileWriter->setProperty("outputDirectory", "./");
  pImageFileWriter->setProperty("extension", "jpg");
  // 2. Set operation properties 

  //Operation: PiiImageFileReader
  //pImageFileReader->setProperty("imageType", "GrayScale");
  pImageFileReader->setProperty("imageType", "Color");
  pImageFileReader->setProperty("fileNamePattern", QString(":/images/dummy_road.jpg"));

  //Operation: PiiEdgeDetector
  //pEdgeDetector->setProperty("detector", "SobelDetector");

  //Operation: PiiHoughTransformOperation
  //pHoughTransform->setProperty("type", "Linear");

  //pHoughTransform->setProperty("startAngle", 5);
  //pHoughTransform->setProperty("endAngle", 55);
  //qDebug("%d", pHoughTransform->property("startAngle").toInt());
  //std::cout<<pHoughTransform->property("startAngle").toInt()<<std::endl;

  //pImageAnnotator->setProperty("annotationType", "Line");
 
  // 3. Connnect Output->Input

  // |-------------------------------------------|
  // |    Output of    | as | Input of           |
  // |-------------------------------------------|
  // |FileReader.image | -> | EdgeDetector.image |
  // |-------------------------------------------|
  //pImageFileReader->connectOutput("image", pEdgeDetector, "image");
  
  // |-------------------------------------------|
  // |    Output of     | as | Input of          |
  // |-------------------------------------------|
  // |Annotator.image   | -> | SourceProbeInput  |
  // |EdgeDetector.edges| -> | ResultProbeInput  |
  // |-------------------------------------------|
  pSourceProbeInput->connectOutput(pImageFileReader->output("image"));
  //pSourceProbeInput->connectOutput(pImageAnnotator->output("image"));
  //pResultProbeInput->connectOutput(pEdgeDetector->output("edges"));

  // |---------------------------------------------------------|
  // |    Output of            | as | Input of                 |
  // |---------------------------------------------------------|
  // |ImageFileReader.image    | -> | ImageAnnotator.image     |
  // |EdgeDetector.edges       | -> | HoughTransfor.image      |
  // |HoughTransfor.coordinates| -> | ImageAnnotator.annotation|
  // |---------------------------------------------------------|
  //pImageFileReader->connectOutput("image", pImageAnnotator, "image");
  //pEdgeDetector->connectOutput("edges", pHoughTransform, "image");
  //pEdgeDetector->connectOutput("edges", pImageFileWriter, "image");
  //pEdgeDetector->connectOutput("edges", pImageAnnotator, "image");
  //pHoughTransform->connectOutput("coordinates", pImageAnnotator, "annotation");
  pResultProbeInput->connectOutput(pImageAnnotator->output("image"));

  // 4. Connnect SourceProbeInput, ResultProbeInput to display
  pResultImageDisplay = new PiiImageDisplay(this);
  pResultImageDisplay->setObjectName(QString::fromUtf8("pResultImageDisplay"));
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(pResultImageDisplay->sizePolicy().hasHeightForWidth());

  pSourceImageDisplay = new PiiImageDisplay(this);
  pSourceImageDisplay->setObjectName(QString::fromUtf8("pSourceImageDisplay"));
  sizePolicy.setHeightForWidth(pSourceImageDisplay->sizePolicy().hasHeightForWidth());
  pSourceImageDisplay->setSizePolicy(sizePolicy);

  connect(pSourceProbeInput, SIGNAL(objectReceived(PiiVariant)), pSourceImageDisplay, SLOT(setImage(PiiVariant)));
  connect(pResultProbeInput, SIGNAL(objectReceived(PiiVariant)), pResultImageDisplay, SLOT(setImage(PiiVariant)));

  pResultImageDisplay->setProperty("displayType", "AutoScale");

  QHBoxLayout *pLayout = new QHBoxLayout;
  pLayout->addWidget(pSourceImageDisplay);
  pLayout->addWidget(pResultImageDisplay);

  setLayout(pLayout);
}
#endif

