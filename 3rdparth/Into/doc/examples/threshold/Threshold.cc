/* This file is part of Into.
 * Copyright (C) Intopii.
 * All rights reserved.
 */

#include <PiiEngine.h>
#include <QApplication>
#include <QWidget>
#include <PiiTriggerButton.h>
#include <PiiImageDisplay.h>
#include <PiiProbeInput.h>

int main(int argc, char* argv[])
{
  // Initialize Qt's services
  QApplication app(argc, argv);

  // Create the engine and load plug-ins
  PiiEngine engine;
  try
    {
      engine.loadPlugin("piibase"); // PiiTriggerSource
      engine.loadPlugin("piiimage"); // PiiImageFileReader/Writer and PiiThresholdingOperation
    }
  catch (PiiLoadException& ex)
    {
      qDebug("Cannot load all necessary plug-in modules: %s", qPrintable(ex.message()));
      return 1;
    }

  // Create operations
  PiiOperation* trigger = engine.createOperation("PiiTriggerSource");
  PiiOperation* reader = engine.createOperation("PiiImageFileReader");
  PiiOperation* thresholder = engine.createOperation("PiiThresholdingOperation");
  PiiOperation* writer = engine.createOperation("PiiImageFileWriter");

  // Configure operations. See the API docs of the corresponding
  // operations for explanations.
  reader->setProperty("fileNamePattern", "../../../demos/images/*.jpg");
  reader->setProperty("imageType", "GrayScale");
  writer->setProperty("outputDirectory", ".");
  writer->setProperty("extension", "bmp");
  thresholder->setProperty("thresholdType", "SauvolaAdaptiveThreshold");

  // Connect operations
  trigger->connectOutput("trigger", reader, "trigger");
  reader->connectOutput("image", thresholder, "image" );
  thresholder->connectOutput("image", writer, "image");

  // Create user interface
  QWidget* buttons = new PiiTriggerButton;
  PiiImageDisplay* display = new PiiImageDisplay;
  display->setProperty("displayType", "AutoScale");

  // Create a probe that sends an image from the engine to the
  // display.
  PiiProbeInput* probe = new PiiProbeInput(reader->output("image"),
                                           display, SLOT(setImage(PiiVariant)));

  // Connect button UI to the trigger source.
  QObject::connect(buttons, SIGNAL(triggered(int)), trigger, SLOT(trigger(int)));

  buttons->show();
  display->show();

  // Start the engine (in background)
  try
    {
      engine.execute();
    }
  catch (PiiExecutionException& ex)
    {
      qDebug("Cannot start the engine: %s", qPrintable(ex.message()));
      return 1;
    }

  // Run Qt's main event loop that delivers events to UI components.
  // Engine continues to run in background.
  app.exec();

  // Qt is done. Send stop signal to the engine.
  engine.interrupt();
  // Wait until it really finishes.
  engine.wait(PiiOperation::Stopped);

  // Destroy UI
  delete buttons;
  delete display;
  delete probe;
  
  return 0;
}
