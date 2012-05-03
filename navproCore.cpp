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

navproCore::navproCore(int width, int height):
    pEngine(0),
    pSourceProbeInput(0), 
    pResultProbeInput(0),
    pImageFileReader(0),
    pEdgeDetector(0),
    pSourceImageDisplay(0),
    pResultImageDisplay(0)
{
    //realWidth = (width <= image.width() ? width : image.width()) * 2;
    //realHeight = height <= image.height() ? height : image.height();
    init();

    if (pEngine->state() != PiiOperation::Running)
    {
       try
        {
          pEngine->execute();
          /*
          if (!_pEngine->wait(PiiOperation::Running, 1000))
            PiiYdin::dumpOperation(_pEngine, 0,
            PiiYdin::ShowInputQueues | PiiYdin::ShowState);
          */
        }
      catch (PiiExecutionException& ex)
        {
          QMessageBox::critical(0, "Application error", ex.message() );
        }
    }

    searchRoad();
    resize(width, height);
}

void navproCore::init()
{
  // Create engine
  pEngine = new PiiEngine;

  try
  {
      pEngine->loadPlugin("piiimage"); // PiiImageFileReader/Writer and PiiThresholdingOperation
  }
  catch (PiiLoadException& ex)
  {
      qDebug("Cannot load all necessary plug-in modules: %s", qPrintable(ex.message()));
      return;
  }  // Create result image display

  pSourceProbeInput = new PiiProbeInput;
  pResultProbeInput = new PiiProbeInput;
  
  pImageAnnotator = new PiiImageAnnotator;

  // 1. Create operations
  pEdgeDetector = pEngine->createOperation("PiiEdgeDetector");
  pHoughTransform = pEngine->createOperation("PiiHoughTransformOperation");
  pImageFileReader = pEngine->createOperation("PiiImageFileReader");

  // 2. Set operation properties 

  //Operation: PiiImageFileReader
  pImageFileReader->setProperty("imageType", "GrayScale");
  pImageFileReader->setProperty("fileNamePattern", QString("debug/*.jpg"));

  //Operation: PiiEdgeDetector
  pEdgeDetector->setProperty("detector", "CannyDetector");

  //Operation: PiiHoughTransformOperation
  pHoughTransform->setProperty("TransformType", "Linear");
 
  // 3. Connnect Output->Input

  // |-------------------------------------------|
  // |    Output of    | as | Input of           |
  // |-------------------------------------------|
  // |FileReader.image | -> | EdgeDetector.image |
  // |-------------------------------------------|
  pImageFileReader->connectOutput("image", pEdgeDetector, "image");
  
  // |-------------------------------------------|
  // |    Output of     | as | Input of          |
  // |-------------------------------------------|
  // |FileReader.image  | -> | SourceProbeInput  |
  // |EdgeDetector.edges| -> | ResultProbeInput  |
  // |-------------------------------------------|
  pSourceProbeInput->connectOutput(pImageFileReader->output("image"));
  pResultProbeInput->connectOutput(pEdgeDetector->output("edges"));

  // |---------------------------------------------------------|
  // |    Output of            | as | Input of                 |
  // |---------------------------------------------------------|
  // |EdgeDetector.edges       | -> | HoughTransfor.image      |
  // |HoughTransfor.coordinates| -> | ImageAnnotator.annotation|
  // |ImageAnnotator.annotation| -> | ResultProbeInput         |
  // |---------------------------------------------------------|
  pEdgeDetector->connectOutput("edges", pHoughTransform, "image");
  pHoughTransform->connectOutput("coordinates", pImageAnnotator, "annotation");
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
//    QPainter painter(this);
//    painter.setPen(QPen(Qt::black, 1));
//
//    QImage output(image);
//
//    int i, j;
//    for (i = 0; i < image.width(); ++i)
//    {
//        for (j = 0; j < image.height(); ++j)
//        {
//            if (isGray(image.pixel(i,j)))
//            {
//               output.setPixel(i,j, 0/*QColor(Qt::red).rgb()*/); 
//            }
//        }
//    }
//    //std::cout<<image.depth()<<std::endl;
//    //std::cout<<image.width()<<std::endl;
//    //std::cout<<image.height()<<std::endl;
//    //std::cout<<pixels.size() <<std::endl;
//    //std::cout<<"Format: "<<image.format()<<std::endl;
//
//    // Draw the background pixmap
//    painter.drawPixmap(QPoint(0,0), QPixmap::fromImage(image));
//    painter.drawPixmap(QPoint(image.width() + 1,0), QPixmap::fromImage(output));
//    //painter.drawRect(0,0,realWidth/2,realHeight);
}

void navproCore::searchRoad()
{
  // Create Hough transform operation
  pImageFileReader->connectOutput("image", pEdgeDetector, "image");

}
