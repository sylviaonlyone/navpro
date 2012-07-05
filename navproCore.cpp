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

navproCore::navproCore(int width, int height):
    pEngine(0),
    pSourceProbeInput(0), 
    pResultProbeInput(0),
    pImageFileReader(0),
    pImageFileWriter(0),
    pEdgeDetector(0),
    pSourceImageDisplay(0),
    pResultImageDisplay(0),
    hueFrom(1),
    hueTo(30),
    saturationFrom(30),
    saturationTo(50),
    cbFrom(140),
    cbTo(150),
    crFrom(140),
    crTo(150),
    activeFrom(&hueFrom),
    activeTo(&hueTo),
    active(HUE),
    image("./dummy_road.jpg") 
{
#ifdef LIB_PII
    init();

    if (pEngine->state() != PiiOperation::Running)
    {
       try
        {
          pEngine->execute();
          pEngine->wait(PiiOperation::Stopped);
          /*
          if (!_pEngine->wait(PiiOperation::Running, 1000))
 
          */
        }
      catch (PiiExecutionException& ex)
        {
          QMessageBox::critical(0, "Application error", ex.message() );
        }
    }
#endif
    //searchRoad();
#ifdef LIB_QT
    QDir dir("","*.jpg");
    fileList = dir.entryList();

    int realWidth = (width >= image.width() ? width : image.width()*2);
    int realHeight = height >= image.height() ? height : image.height();

    if (DEFAULT_FULL_WIDTH <= realWidth)
    {
        //cut size to half if it's too large to display
        realWidth >>= 1;
        realHeight >>=1;
    }
#endif
    std::cout<<realWidth<<std::endl; 
    resize(realWidth, realHeight);
}

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

void navproCore::paintEvent(QPaintEvent *event)
{
    (void)event;
#ifdef LIB_QT
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 1));

    QImage output(image);

    int i, j;
    QColor hsv;
    //int cb, cr;

    //thresholding
    bool display = false;
    QRgb clr;
    for (i = 0; i < image.width(); ++i)
    {
        for (j = 0; j < image.height(); ++j)
        {
            clr = image.pixel(i,j);
            hsv = QColor::fromRgb(clr);
            int cb = 0.148*qRed(clr)-0.291*qGreen(clr)+0.439*qBlue(clr)+128;
            int cr = 0.439*qRed(clr)-0.368*qGreen(clr)+0.071*qBlue(clr)+128;
            //std::cout<<"HSV:hue: "<<hsv.hue()<<std::endl;
            display = false;
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
            if (display)
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

    // Draw the background pixmap
    // if image is too large, half it size for display
    if (DEFAULT_FULL_WIDTH <= image.width()*2)
    {
        painter.drawPixmap(QPoint(0, 0), QPixmap::fromImage(image.scaledToWidth(image.width()/2)));
        painter.drawPixmap(QPoint(image.width()/2 + 1, 0), QPixmap::fromImage(output.scaledToWidth(output.width()/2)));
    }
    else
    {
        painter.drawPixmap(QPoint(0, 0), QPixmap::fromImage(image));
        painter.drawPixmap(QPoint(image.width() + 1, 0), QPixmap::fromImage(output));
    }

    //painter.drawRect(0,0,realWidth/2,realHeight);
#endif
}

void navproCore::searchRoad()
{
  // Create Hough transform operation
  pImageFileReader->connectOutput("image", pEdgeDetector, "image");
}

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

void navproCore::keyPressEvent(QKeyEvent * e)
{
    switch(e->key())
    {
      //Left/Right for hue from
      //Up/Down for hue to
      case Qt::Key_Left:
        if (*activeFrom > 1)
          --*activeFrom;
        break;
      case Qt::Key_Down:
        if (*activeTo > 1)
          --*activeTo;
        break;
      case Qt::Key_Right:
        ++*activeFrom;
        break;
      case Qt::Key_Up:
        ++*activeTo;
        break;
      case Qt::Key_S:
        {
          activeFrom = &saturationFrom;
          activeTo   = &saturationTo;
          active = SATURATION;
          std::cout<<"SAT ";
          break;
        }
      case Qt::Key_H:
        {
          activeFrom = &hueFrom;
          activeTo   = &hueTo;
          active = HUE;
          std::cout<<"HUE ";
          break;
        }
      case Qt::Key_B:
        {
          activeFrom = &cbFrom;
          activeTo   = &cbTo;
          active = CB;
          std::cout<<"CB ";
          break;
        }
      case Qt::Key_R:
        {
          activeFrom = &crFrom;
          activeTo   = &crTo;
          active = CR;
          std::cout<<"CR ";
          break;
        }
      case Qt::Key_N:
       {
        static int cur = 0;
        std::cout<<"Cur: "<<cur<<" of : "<<fileList.size()<<std::endl; 
        if (fileList.size() > 0)
        {
            if (cur == fileList.size()) cur = 0;
            image.load(fileList[cur]);
            ++cur;
    
            int realWidth = image.width()*2;
            int realHeight = image.height();
    
            if (DEFAULT_FULL_WIDTH <= realWidth)
            {
                //cut size to half if it's too large to display
                realWidth >>= 1;
                realHeight >>=1;
            }
    
            std::cout<<realWidth<<std::endl; 
            resize(realWidth, realHeight);
        }
        break;
       }
    }
    std::cout<<"From: "<<*activeFrom<<" To: "<<*activeTo<<std::endl; 
    update();
}
