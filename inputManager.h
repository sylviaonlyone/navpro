#ifndef INPUTSTREAM_H
#define INPUTSTREAM_H

#include <QImage>
#include <QString>
#include "environment.h"

class inputManager
{
  public:
    inputManager(QString& inputPath);
    ~inputManager();
    
    bool getCurrentImage(QImage& image);
    bool getCurrentImagePath(QString& path);

    bool getNextImage(QImage& image);
    bool getNextImagePath(QString& path);
  private:
    void scale(QImage& image);

    //support file list in a Dir
    QStringList image_list_;
    QString input_path_;
    int cur_image_;
};

#endif  //INPUTSTREAM_H
